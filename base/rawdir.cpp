#include "rawdir.h"

RawDir::RawDir() {}

RawDir::~RawDir() {}

bool RawDir::toFileList(CBMFileList &fileList) {
    m_dirBufPos = 0;

    // check header
    QString name, id;
    if (!readHeader(name, id))
        return false;
    name = name.trimmed();
    id = id.trimmed();
    fileList.setTitle(name, id);

    // read entries
    quint16 w;
    forever {
        if (!readWord(w))
            return false;
        if (w == 0)
            break;
        if (w != 0x0101)
            return false;

        quint16 blocks;
        if (!readWord(blocks))
            return false;
        quint8 b;
        if (!readByte(b))
            return false;
        if (b == 'B') {
            fileList.setFreeBlocks(blocks);
            break;
        } else if (b != ' ') {
            return false;
        }

        CBMFile file;
        if (!readFile(file, blocks))
            return false;
        fileList << file;
    }
    return true;
}

bool RawDir::readHeader(QString &name, QString &id) {
    quint16 w;
    quint8 b;

    if (!readWord(w))
        return false;
    if (w == 0x0401) {
        if (!readWord(w))
            return false;
        if (w != 0x0101)
            return false;
        m_skipLoadAddr = false;
    } else if (w == 0x0101) {
        m_skipLoadAddr = true;
    } else {
        return false;
    }
    if (!checkWord(0x0000))
        return false;
    if (!checkByte(0x12))
        return false;
    if (!checkByte('"'))
        return false;
    if (!readString(16, name))
        return false;
    if (!checkByte('"'))
        return false;
    if (!readByte(b))
        return false;
    if (!readString(2, id))
        return false;
    for (int i = 0; i < 3; i++) {
        if (!readByte(b))
            return false;
    }
    return checkByte(0);
}

bool RawDir::readFile(CBMFile &file, quint16 blocks) {
    QByteArray line;
    if (!readBytes(27, line))
        return false;

    // find start of file name
    int digits = calcNumDigits(blocks);
    int pos = 3 - digits;
    if (line[pos] != '"')
        return false;

    // get file name
    int endPos = line.indexOf('"', pos + 1);
    if (endPos == -1)
        return false;
    int size = endPos - pos - 1;
    QByteArray name;
    name.resize(size);
    for (int i = 0; i < size; i++)
        name[i] = line[pos + 1 + i];
    QString fileName = QString::fromUtf8(name);
    file.setName(fileName);

    // get type and flags
    pos += 18;
    QByteArray tf;
    tf.resize(5);
    for (int i = 0; i < 5; i++)
        tf[i] = line[pos + i];
    QString typeFlags = QString::fromUtf8(tf);
    if (!file.parseTypeFlagsFromString(typeFlags))
        return false;

    // set blocks
    file.setBlocks(blocks);
    return true;
}

void RawDir::fromFileList(const CBMFileList &fileList) {
    m_dirBufPos = 0;
    m_dirBuffer.clear();

    // header (32 bytes)
    QString name, id;
    fileList.title(name, id);
    if (!m_skipLoadAddr)
        writeWord(0x0401);
    writeWord(0x0101);
    writeWord(0x0000);
    writeByte(0x12);
    writeByte('"');
    writeString(name, 16);
    writeStringPad(name, 16);
    writeByte('"');
    writeByte(' ');
    writeString(id, 2);
    writeStringPad(id, 2);
    writeByte(' ');
    writeByte('2');
    writeByte('A');
    writeByte(0);

    // entries (32 bytes)
    for (int i = 0; i < fileList.size(); i++) {
        const CBMFile &file = fileList.at(i);
        int blocks, blockDigits;
        blocks = file.blocks();
        blockDigits = calcNumDigits(blocks);
        int spaces = 4 - blockDigits;
        QString name = file.name();
        QString type = file.convertTypeToString();

        writeWord(0x0101);
        writeWord(blocks);
        for (int i = 0; i < spaces; i++)
            writeByte(' ');

        writeByte('"');
        writeString(name, 16);
        writeByte('"');
        writeStringPad(name, 16);
        writeByte(file.isClosed() ? ' ' : '*');
        writeString(type, 3);
        writeByte(file.isLocked() ? '<' : ' ');

        for (int i = 0; i < blockDigits; i++)
            writeByte(' ');

        writeByte(0);
    }

    // blocks free
    writeWord(0x0101);
    writeWord(fileList.freeBlocks());
    writeString("BLOCKS FREE.", 12);
    for (int i = 0; i < 13; i++)
        writeByte(' ');
    writeByte(0);

    // end
    writeWord(0);
    m_dirBuffer.resize(m_dirBufPos);
}

// ----- Tools -----

bool RawDir::readByte(quint8 &b) {
    quint32 dirBufSize = m_dirBuffer.size();
    if (m_dirBufPos >= dirBufSize) {
        return false;
    }
    b = m_dirBuffer[m_dirBufPos++];
    return true;
}

bool RawDir::readWord(quint16 &w) {
    quint8 a, b;
    if (!readByte(a) || !readByte(b))
        return false;
    w = (quint16)a | ((quint16)b) << 8;
    return true;
}

bool RawDir::readString(int size, QString &str) {
    quint32 dirBufSize = m_dirBuffer.size();
    if ((m_dirBufPos + size) >= dirBufSize) {
        return false;
    }
    QByteArray ba;
    ba.resize(size);
    for (int i = 0; i < size; i++) {
        ba[i] = m_dirBuffer[m_dirBufPos + i];
    }
    m_dirBufPos += size;
    str = QString::fromUtf8(ba);
    return true;
}

bool RawDir::readBytes(int size, QByteArray &data) {
    quint32 dirBufSize = m_dirBuffer.size();
    if ((m_dirBufPos + size) >= dirBufSize) {
        return false;
    }
    data.resize(size);
    for (int i = 0; i < size; i++) {
        data[i] = m_dirBuffer[m_dirBufPos + i];
    }
    m_dirBufPos += size;
    return true;
}

bool RawDir::checkByte(quint8 b) {
    quint8 x;
    if (!readByte(x))
        return false;
    return (x == b);
}

bool RawDir::checkWord(quint16 w) {
    quint16 x;
    if (!readWord(x))
        return false;
    return (x == w);
}

void RawDir::writeByte(quint8 b) {
    quint32 dirBufSize = m_dirBuffer.size();
    if (m_dirBufPos >= dirBufSize) {
        m_dirBuffer.resize(dirBufSize + 1024);
    }
    m_dirBuffer[m_dirBufPos++] = b;
}

void RawDir::writeWord(quint16 w) {
    writeByte((quint8)(w & 0xff));
    writeByte((quint8)(w >> 8));
}

void RawDir::writeString(const QString &string, int pad) {
    int size = string.size();
    if (size > pad)
        size = pad;
    for (int i = 0; i < size; i++)
        writeByte(string[i].unicode());
}

void RawDir::writeStringPad(const QString &string, int pad) {
    int size = string.size();
    if (size > pad)
        return;
    size = pad - size;
    for (int i = 0; i < size; i++)
        writeByte(0x20);
}

int RawDir::calcNumDigits(quint16 blocks) {
    int digits;
    if (blocks > 99)
        digits = 3;
    else if (blocks > 9)
        digits = 2;
    else
        digits = 1;
    return digits;
}

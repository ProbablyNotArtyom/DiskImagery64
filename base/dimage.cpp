#include "dimage.h"
#include <assert.h>
#include <string.h>

DImage::DImage(DiskFormat f) : m_format(f) {
    m_fileName = "noname." + fileExtensionForDiskFormat(f);
    createNewImage();
}

DImage::DImage(const QString &fileName) : m_fileName(fileName) {
    m_format = determineDiskFormat(fileName);
    if (m_format != INVALID) {

        // try to open existing image
        QByteArray baFileName = fileName.toLocal8Bit();
        char *cFileName = baFileName.data();
        m_diskImage = di_load_image(cFileName);
        m_fileAvailable = true;
        m_dirty = false;
        if (m_diskImage == 0)
            createNewImage();

    } else {
        m_format = D64;
        createNewImage();
    }
}

DImage::~DImage() {
    if (m_diskImage != 0) {
        // prevent auto sync
        m_diskImage->modified = 0;

        di_free_image(m_diskImage);
    }
}

void DImage::createNewImage() {
    QByteArray baFileName = m_fileName.toLocal8Bit();
    char *cFileName = baFileName.data();

    int size;
    switch (m_format) {
    case D64:
        size = 174848;
        break;
    case D71:
        size = 349696;
        break;
    case D81:
        size = 819200;
        break;
    default:
        size = 0;
        break;
    }

    // create a new image
    m_diskImage = di_create_image(cFileName, size);
    format(m_fileName.left(m_fileName.size() - 4).toUpper(), "00");
    m_fileAvailable = false;
    m_dirty = false;
}

void DImage::setFileName(const QString &fileName) {
    if (m_diskImage == 0)
        return;

    free(m_diskImage->filename);
    QByteArray baFileName = fileName.toLocal8Bit();
    char *cFileName = baFileName.data();
    m_diskImage->filename = strdup(cFileName);
    m_fileName = fileName;
}

void DImage::sync() {
    if (m_diskImage != 0) {
        di_sync(m_diskImage);
        m_fileAvailable = QFileInfo(m_fileName).exists();
        m_dirty = false;
    }
}

QString DImage::status() const {
    QString result;
    if (m_diskImage == 0)
        return result;

    char buffer[256];
    di_status(m_diskImage, buffer);
    result = QString::fromLocal8Bit(buffer);
    return result.toUpper();
}

void DImage::format(const QString &name, const QString &id) {
    if (m_diskImage == 0)
        return;

    // convert name
    unsigned char rawName[17];
    convertFileNameToRaw(name, rawName);

    // full format
    if (id != "") {
        unsigned char rawId[3];
        convertIdToRaw(id, rawId);
        di_format(m_diskImage, rawName, rawId);
    }
    // quick format
    else {
        di_format(m_diskImage, rawName, NULL);
    }

    m_dirty = true;
}

bool DImage::readDirectory(CBMFileList &files) {
    if (m_diskImage == 0)
        return false;

    files.clear();

    ImageFile *fh = di_open(m_diskImage, (unsigned char *)"$", T_PRG, "rb");
    if (fh == 0)
        return false;

    unsigned char buffer[254];
    // read BAM
    if (di_read(fh, buffer, 254) != 254) {
        di_close(fh);
        return false;
    }

    // read dir blocks
    while (di_read(fh, buffer, 254) == 254) {
        for (int pos = 0; pos < 254; pos += 32) {
            if (buffer[pos] != 0) {
                // --- read raw dir ---
                // extract file name
                QString fileName;
                convertRawToFileName(&buffer[pos + 3], fileName);
                // get type
                CBMFile::Type fileType = (CBMFile::Type)(buffer[pos] & 7);
                // closed/locked file?
                bool closed = (buffer[pos] & 0x80) == 0x80;
                bool locked = (buffer[pos] & 0x40) == 0x40;
                // blocks size
                int blocks =
                    ((int)buffer[pos + 29]) << 8 | (int)buffer[pos + 28];

                // --- create CBM file ---
                CBMFile file(fileName, fileType);
                file.setClosed(closed);
                file.setLocked(locked);
                file.setBlocks(blocks);
                files << file;
            }
        }
    }

    di_close(fh);

    // copy title
    QString name, id;
    diskTitle(name, id);
    files.setTitle(name, id);

    return true;
}

bool DImage::readFile(CBMFile &file) {
    unsigned char rawName[17];
    convertFileNameToRaw(file.name(), rawName);
    filetype ft = getFileType(file.type());

    ImageFile *fh = di_open(m_diskImage, rawName, ft, "rb");
    if (fh == 0)
        return false;

    if (file.type() != CBMFile::DEL) {
        QByteArray data;
        int total = 0;
        int len;
        unsigned char buffer[4096];
        while ((len = di_read(fh, buffer, 4096)) > 0) {
            data.resize(total + len);
            memcpy(data.data() + total, buffer, len);
            total += len;
        }
        file.setData(data);
    }

    di_close(fh);
    return true;
}

bool DImage::writeFile(const CBMFile &file) {
    unsigned char rawName[17];
    convertFileNameToRaw(file.name(), rawName);
    filetype ft = getFileType(file.type());

    ImageFile *fh = di_open(m_diskImage, rawName, ft, "wb");
    if (fh == 0)
        return false;

    if (file.type() != CBMFile::DEL) {
        QByteArray data = file.data();
        if (data.size() > 0) {
            if (di_write(fh, (unsigned char *)data.data(), data.size()) !=
                data.size())
                return false;
        }
    }
    di_close(fh);

    m_dirty = true;
    return true;
}

bool DImage::deleteFile(const CBMFile &file) {
    unsigned char rawName[17];
    convertFileNameToRaw(file.name(), rawName);
    filetype ft = getFileType(file.type());

    m_dirty = true;
    return (di_delete(m_diskImage, rawName, ft) == 0);
}

bool DImage::renameFile(CBMFile &file, const QString &newName) {
    unsigned char oldRawName[17];
    convertFileNameToRaw(file.name(), oldRawName);
    unsigned char newRawName[17];
    convertFileNameToRaw(newName, newRawName);
    filetype ft = getFileType(file.type());

    if (di_rename(m_diskImage, oldRawName, newRawName, ft) == 0) {
        file.setName(newName.left(16));
        return true;
    }
    return false;
}

filetype DImage::getFileType(CBMFile::Type t) {
    filetype ft;
    switch (t) {
    case CBMFile::DEL:
        ft = T_DEL;
        break;
    case CBMFile::SEQ:
        ft = T_SEQ;
        break;
    default:
    case CBMFile::PRG:
        ft = T_PRG;
        break;
    case CBMFile::USR:
        ft = T_USR;
        break;
    case CBMFile::REL:
        ft = T_REL;
        break;
    case CBMFile::CBM:
        ft = T_CBM;
        break;
    case CBMFile::DIR:
        ft = T_DIR;
        break;
    }
    return ft;
}

// ----- query -----

int DImage::blocksFree() const {
    if (m_diskImage == 0)
        return 0;
    return m_diskImage->blocksfree;
}

void DImage::diskTitle(QString &name, QString &id) const {
    if (m_diskImage == 0)
        return;

    unsigned char *rawName = di_title(m_diskImage);
    convertRawToFileName(rawName, name);
    unsigned char *rawId = rawName + 18;
    convertRawToId(rawId, id);
}

// ----- tools -----

DImage::DiskFormat DImage::determineDiskFormat(const QString &fileName) {
    QString ext = fileName.right(4);
    if (ext.size() != 4)
        return INVALID;
    if (ext[0] != '.')
        return INVALID;
    if ((ext[1] != 'd') && (ext[1] != 'D'))
        return INVALID;
    switch (ext[2].toLatin1()) {
    case '6':
        return D64;
    case '7':
        return D71;
    case '8':
        return D81;
    }
    return INVALID;
}

QString DImage::fileExtensionForDiskFormat(DiskFormat format) {
    switch (format) {
    case D64:
        return "d64";
    case D71:
        return "d71";
    case D81:
        return "d81";
    default:
        return "";
    }
}

QString DImage::fileFilterForDiskFormat(DiskFormat format) {
    switch (format) {
    case D64:
        return "C1541 Disk Image (*.d64)";
    case D71:
        return "C1571 Disk Image (*.d71)";
    case D81:
        return "C1581 Disk Image (*.d81)";
    default:
        return "";
    }
}

// ----- conversion -----

void DImage::convertFileNameToRaw(const QString &str,
                                  unsigned char *raw) const {
    QByteArray ba;
    int size = str.size();
    if (size > 16)
        size = 16;
    ba.resize(size + 1);
    for (int i = 0; i < size; i++) {
        ba[i] = str[i].unicode();
    }
    ba[size] = 0;
    di_rawname_from_name(raw, ba.data());
    raw[16] = 0;
}

void DImage::convertRawToFileName(unsigned char *raw, QString &str) const {
    char name[17];
    name[16] = 0;
    di_name_from_rawname(name, raw);
    int size = strlen(name);
    str.resize(size);
    for (int i = 0; i < size; i++) {
        str[i] = QChar(name[i]);
    }
}

void DImage::convertIdToRaw(const QString &id, unsigned char *raw) const {
    int size = id.size();
    if (size != 2)
        size = 2;
    for (int i = 0; i < size; i++) {
        raw[i] = id[i].unicode();
    }
    for (int i = size; i < 2; i++) {
        raw[i] = ' ';
    }
    raw[2] = 0;
}

void DImage::convertRawToId(unsigned char *raw, QString &id) const {
    const int size = 2;
    id.resize(size);
    for (int i = 0; i < size; i++) {
        id[i] = QChar(raw[i]);
    }
}

// ----- Raw Access -----

int DImage::numTracks() const { return di_tracks(m_diskImage->type); }

int DImage::numSectors(int track) const {
    return di_sectors_per_track(m_diskImage->type, track + 1);
}

int DImage::numBlocks() const {
    int sum = 0;
    int numT = numTracks();
    for (int t = 0; t < numT; t++)
        sum += numSectors(t);
    return sum;
}

BlockMap DImage::blockMap() const {
    int numT = numTracks();
    BlockMap bm;
    bm.resize(numT);
    for (int t = 0; t < numT; t++)
        bm[t] = numSectors(t);
    return bm;
}

OffsetMap DImage::offsetMap(int *numBlocks) const {
    int numT = numTracks();
    OffsetMap om;
    om.resize(numT);
    int sum = 0;
    for (int t = 0; t < numT; t++) {
        om[t] = sum;
        sum += numSectors(t);
    }
    if (numBlocks != 0)
        *numBlocks = sum;
    return om;
}

int DImage::blockNum(int track, int sector) const {
    TrackSector ts = {track + 1, sector};
    return di_get_block_num(m_diskImage->type, ts);
}

void DImage::writeBlock(int block, const QByteArray &data) {
    assert(data.size() == 256);
    unsigned char *ptr = m_diskImage->image + block * 256;
    memcpy(ptr, data.constData(), 256);
}

void DImage::readBlock(int block, QByteArray &data) const {
    assert(data.size() == 256);
    const unsigned char *ptr = m_diskImage->image + block * 256;
    memcpy(data.data(), ptr, 256);
}

void DImage::putToRawImage(QByteArray &image) {
    image.resize(m_diskImage->size);
    memcpy(image.data(), m_diskImage->image, m_diskImage->size);
}

bool DImage::getFromRawImage(const QByteArray &image) {
    if (image.size() != m_diskImage->size)
        return false;
    memcpy(m_diskImage->image, image.constData(), m_diskImage->size);
    m_dirty = true;
    return true;
}

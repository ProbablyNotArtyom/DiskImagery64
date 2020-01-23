#include "netdrive.h"

// ----- NetDriveService -----

NetDriveService::NetDriveService() : NetService("NetDrive") {
    // set default c64 port
    m_addrPair.c64Port = 6463;
    m_addrPair.myPort = 6463;
}

NetDriveService::~NetDriveService() {}

void NetDriveService::shareFiles(const CBMFileList &files) {
    stopAllTasks();
    addTask(new NetDriveTask(files));
    reportEvent(tr("sharing %1 file(s)").arg(QString::number(files.size())));
    for (int i = 0; i < files.size(); i++) {
        reportEvent(tr("  Sharing File '%1'").arg(files.at(i).name()));
    }
}

// ----- NetDriveTask -----

NetDriveTask::NetDriveTask(const CBMFileList &files) : m_seqNum(0) {
    m_device = new NetDriveDevice(files);
}

NetDriveTask::~NetDriveTask() { delete m_device; }

NetTask::Result NetDriveTask::run(NetHost &host) {
    // get a packet
    if (!host.receivePacket(m_packet)) {
        return KEEP;
    }

    // check signature
    if (!checkSignature(m_packet)) {
        setStatusMessage(QObject::tr("packet has wrong signature"));
        return KEEP;
    }

    // check size
    int packetSize = m_packet.size();
    if (packetSize < 8) {
        setStatusMessage(QObject::tr("packet too small"));
        return KEEP;
    }

    quint8 command = (quint8)m_packet[2];
    quint8 seqNum = (quint8)m_packet[3];
    quint8 fileNum = (quint8)m_packet[4];
    quint8 channel = (quint8)m_packet[5] & 0x0f;
    quint8 device = (quint8)m_packet[6]; // currently unused
    quint8 size = (quint8)m_packet[7];

    // read from device
    if (packetSize < 8 + size) {
        setStatusMessage(QObject::tr("packet size mismatch"));
        return KEEP;
    }

    // check seq num
    if (seqNum == m_seqNum) {
        setStatusMessage(QObject::tr("packet repeated"));
        return KEEP;
    }
    m_seqNum = seqNum;

    QString resultEvent = QString("%1,%2,%3: ")
                              .arg(QString::number(fileNum))
                              .arg(QString::number(device))
                              .arg(QString::number(channel));

    // handle command
    NetDriveDevice::Result result;
    bool verbose = true;
    switch (command) {
    case OPEN:
        // open device
        {
            QString fileName = getFileName(m_packet, size - 1);
            result = m_device->open(fileName, channel);
            resultEvent += QObject::tr("OPEN '%1'").arg(fileName);
        }
        break;
    case CHKIN:
        // chkin device
        {
            qint8 pos = 0;
            if (size < 1) {
                result = NetDriveDevice::ND_ERROR_NET;
            } else {
                pos = (quint8)m_packet[8];
                result = m_device->offset(channel, pos);
            }
            resultEvent +=
                QObject::tr("CHKIN offset=%1").arg(QString::number(pos));
        }
        break;
    case READ:
        // read from device
        {
            quint8 readSize = 0;
            quint8 resultSize = 0;
            QByteArray readData;
            if (size < 1) {
                result = NetDriveDevice::ND_ERROR_NET;
            } else {
                readSize = (quint8)m_packet[8];
                result = m_device->read(channel, readSize, readData);
                resultSize = readData.size();
            }
            if (result == NetDriveDevice::ND_DONE) {
                m_packet.resize(readSize + 8);
                for (int i = 0; i < resultSize; i++)
                    m_packet[8 + i] = readData[i];
                // reply a data packet
                m_packet[2] = DATA;
                m_packet[7] = resultSize;
                verbose = false;
            }
            resultEvent += QObject::tr("read %1/got %2 bytes")
                               .arg(QString::number(readSize))
                               .arg(QString::number(resultSize));
        }
        break;
    case WRITE:
        // write to device
        {
            QByteArray writeData;
            for (int i = 0; i < size; i++)
                writeData[i] = m_packet[8 + i];
            result = m_device->write(channel, writeData);
            if (result == NetDriveDevice::ND_DONE)
                verbose = false;
            resultEvent +=
                QObject::tr("write %1 bytes").arg(QString::number(size));
        }
        break;
    case CLOSE:
        // close device
        result = m_device->close(channel);
        resultEvent += QObject::tr("CLOSE");
        break;
    default:
        // unknown command
        resultEvent +=
            QObject::tr("UNKNOWN command %1").arg(QString::number(command));
        break;
    }

    // verbose result
    resultEvent += ": ";
    switch (result) {
    case NetDriveDevice::ND_DONE:
        resultEvent += "done";
        break;
    case NetDriveDevice::ND_ERROR_NET:
        resultEvent += "ERROR: network";
        break;
    case NetDriveDevice::ND_ERROR_FILE_OPEN:
        resultEvent += "ERROR: file already open";
        break;
    case NetDriveDevice::ND_ERROR_FILE_NOT_OPEN:
        resultEvent += "ERROR: file not open";
        break;
    case NetDriveDevice::ND_ERROR_FILE_NOT_FOUND:
        resultEvent += "ERROR: file not found";
        break;
    }

    // create status packet
    if ((char)m_packet[2] != DATA) {
        m_packet.resize(10);
        m_packet[7] = 1;
        m_packet[8] = (quint8)result;
        m_packet[9] = 0;
    }

    // send result packet
    if (!host.sendPacket(m_packet)) {
        resultEvent += QObject::tr(" - ERROR sending result!");
    }
    if (verbose) {
        setStatusMessage(resultEvent);
    }

    return KEEP;
}

bool NetDriveTask::checkSignature(const QByteArray &array) {
    if (array.size() < 2)
        return false;
    quint8 val1 = (quint8)array[0];
    quint8 val2 = (quint8)array[1];
    return (val1 == 0xad) && (val2 == 0xf8);
}

QString NetDriveTask::getFileName(const QByteArray &array, quint8 size) {
    QString result;
    result.resize(size);
    for (int i = 0; i < size; i++) {
        result[i] = QChar((quint8)array[8 + i]);
    }
    return result;
}

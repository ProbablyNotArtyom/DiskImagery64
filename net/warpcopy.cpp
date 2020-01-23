#include "warpcopy.h"
#include "rawdir.h"

// ----- WarpCopyTask -----

WarpCopyTask::WarpCopyTask(Job job, int drive)
    : m_job(job), m_drive(drive), m_track(0), m_sector(0), m_state(0),
      m_check(true) {}

WarpCopyTask::~WarpCopyTask() {}

NetTask::Result WarpCopyTask::run(NetHost &host) {
    Result result = ERROR;

    // 1.) first time: check if drive is ready
    if (m_check) {
        bool driveReady;
        result = checkDriveReady(host, driveReady);
        if (result != READY)
            return result;
        if (!driveReady) {
            setStatusMessage(QObject::tr("Drive %1 is not ready!")
                                 .arg(QString::number(m_drive)));
            return ERROR;
        }
        m_check = false;
    }

    // 2.) handle job
    switch (m_job) {
    case EXEC_COMMAND:
        result = sendCommand(host, m_jobData);
        break;
    case READ_DIRECTORY:
        result = readDirectory(host, m_jobData);
        break;
    case READ_STATUS:
        result = readStatus(host, m_jobData);
        break;
    case READ_BLOCK:
        result = readDiskBlock(host, m_track, m_sector, m_jobData);
        break;
    case WRITE_BLOCK:
        result = writeDiskBlock(host, m_track, m_sector, m_jobData);
        break;
    case WARP_READ_BLOCKS:
        result = warpReadBlocks(host, m_tsv);
        break;
    case WARP_READ_DISK:
        result = warpReadDisk(host);
        break;
    case WARP_WRITE_DISK:
        result = warpWriteDisk(host, m_jobData);
        break;
    }
    return result;
}

// ---------- High Level Commands ----------

NetTask::Result WarpCopyTask::checkDriveReady(NetHost &host, bool &driveReady) {
    makeCommandPacket(DRIVE_READY_CMD, 1, m_drive, 0);
    char status;
    driveReady = false;
    Result result = sendPacketAndGetStatus(host, status);
    if (result != READY) {
        setStatusMessage(QObject::tr("Can't query Drive Status"));
        return result;
    }
    driveReady = (status == 0x10);
    return READY;
}

NetTask::Result WarpCopyTask::readStatus(NetHost &host, QByteArray &statusMsg) {
    // open command channel
    makeCommandPacketWithString(OPEN_CMD, 1, m_drive, 15);
    Result result = sendPacketAndCheckStatus(host);
    if (result != READY) {
        setStatusMessage(QObject::tr("Can't open command channel"));
        return result;
    }

    // read command channel
    makeCommandPacket(READ_CMD, 1);
    char status;
    result = sendPacketAndGetStatus(host, status);
    if (result != READY) {
        setStatusMessage(QObject::tr("Can't read command channel"));
        return result;
    }
    statusMsg = getBlockData(true);

    // close command channel
    makeCommandPacket(CLOSE_CMD, 1);
    result = sendPacketAndCheckStatus(host);
    if (result != READY) {
        setStatusMessage(QObject::tr("Can't close command channel"));
        return result;
    }

    return READY;
}

NetTask::Result WarpCopyTask::sendCommand(NetHost &host,
                                          const QByteArray &command) {
    // 1.) open command channel
    if (m_state == 0) {
        makeCommandPacketWithString(OPEN_CMD, 1, m_drive, 15, command);
        Result result = sendPacketAndCheckStatus(host);
        if (result != READY) {
            setStatusMessage(QObject::tr("Can't open command channel"));
            return result;
        }

        // send close command
        makeCommandPacket(CLOSE_CMD, 1);
        result = sendPacket(host);
        if (result != READY) {
            setStatusMessage(QObject::tr("Can't close command channel"));
            return result;
        }

        m_state = 1;
        return KEEP;
    }
    // 2.) close channel
    else {
        // try to get finished closed command reply
        Result result = recvBlock(host);

        // transform the retry into a keep task
        if (result == RETRY) {
            setStatusMessage(QObject::tr("waiting for command to finish"));
            return KEEP;
        }

        // check reply
        if (result == READY)
            result = checkStatus();

        if (result != READY) {
            setStatusMessage(
                QObject::tr("error waiting for command to finish"));
            return result;
        }

        // close command channel
        return READY;
    }
}

NetTask::Result WarpCopyTask::readDirectory(NetHost &host, QByteArray &rawDir) {
    // open command channel
    makeCommandPacketWithString(OPEN_CMD, 1, m_drive, 0, "$");
    Result result = sendPacketAndCheckStatus(host);
    if (result != READY) {
        setStatusMessage(QObject::tr("Can't open directory"));
        return result;
    }

    forever {
        // read data
        makeCommandPacket(READ_CMD, 1);
        char status;
        result = sendPacketAndGetStatus(host, status);
        if (result != READY) {
            setStatusMessage(QObject::tr("Can't read directory"));
            return result;
        }
        if ((status & 0x3f) != 0) {
            setStatusMessage(QObject::tr("Unknown read dir status"));
            return ERROR;
        }

        rawDir += getBlockData(false);

        if (status == 0x40)
            break;
    }

    // close command channel
    makeCommandPacket(CLOSE_CMD, 1);
    result = sendPacketAndCheckStatus(host);
    if (result != READY) {
        setStatusMessage(QObject::tr("Can't close directory"));
        return result;
    }
    return READY;
}

NetTask::Result WarpCopyTask::readDiskBlock(NetHost &host, int track,
                                            int sector, QByteArray &block) {
    // read disk block
    makeCommandPacketWithTrackSector(READ_BLOCK_CMD, 1, m_drive, 0, track,
                                     sector);

    // status is ignored here
    Result result = sendPacketAndGetPacket(host);
    if (result != READY) {
        setStatusMessage(QObject::tr("Can't read command channel"));
        return result;
    }

    if (m_block.size() < 256) {
        setStatusMessage(QObject::tr("Read result block too small!"));
        return ERROR;
    }

    // copy block
    block.resize(256);
    for (int i = 0; i < 256; i++)
        block[i] = m_block[i];

    return READY;
}

NetTask::Result WarpCopyTask::writeDiskBlock(NetHost &host, int track,
                                             int sector,
                                             const QByteArray &block) {
    // write disk block
    makeCommandPacketWithTrackSector(WRITE_BLOCK_CMD, 1, m_drive, 0, track,
                                     sector);

    // append block to packet
    m_packet += block;

    Result result = sendPacketAndGetPacket(host);
    if (result != READY) {
        setStatusMessage(QObject::tr("Can't write disk block"));
        return result;
    }

    if (m_block.size() < 256) {
        setStatusMessage(QObject::tr("Write result block too small!"));
        return ERROR;
    }

    // compare blocks
    bool ok = true;
    for (int i = 0; i < 256; i++) {
        if (m_block[i] != block[i]) {
            ok = false;
            break;
        }
    }
    if (!ok) {
        setStatusMessage(QObject::tr("Write result block differs!"));
        return ERROR;
    }

    return READY;
}

// ----- warp jobs -----

NetTask::Result WarpCopyTask::warpReadBlocks(NetHost &host,
                                             const BlockPosVector &tsv) {
    int totalBlocks = tsv.size() * 2; // 2 stages per block

    // 1.) start raw read
    if (m_state == 0) {
        // move to track 1 first
        QByteArray dummy;
        Result result = readDiskBlock(host, 1, 0, dummy);
        if (result != READY)
            return result;

        // start raw read
        makeCommandPacket(START_WARP_READ_CMD, 1, m_drive, 0);
        result = sendPacket(host);
        if (result != READY) {
            setStatusMessage(QObject::tr("Can't start warp read blocks"));
            return result;
        }
        m_state = 1;
        return KEEP;
    }
    // 2.) read blocks
    else if (m_state <= totalBlocks) {
        int blockNum = (m_state - 1) / 2;
        const BlockPos &ts = tsv[blockNum];

        m_track = ts.track();
        m_sector = ts.sector();

        if ((m_state) % 2 == 1) {
            // 2a) send read command
            // send raw read block command
            makeCommandPacket(WARP_READ_BLOCK_CMD, 0, m_track, m_sector);
            Result result = sendPacket(host);
            if (result != READY) {
                setStatusMessage(
                    QObject::tr("Can't init warp read block (%1,%2)")
                        .arg(QString::number(m_track))
                        .arg(QString::number(m_sector)));
                return result;
            }
            setRetries(5);
        } else {
            // 2b) get read block
            Result result = recvBlock(host);

            // a time out means block is missing
            if (result == RETRY) {

                // set an empty block
                memset(m_block.data(), 0, 256);
                m_jobData = m_block;

                if (retries() > 0) {
                    setRetries(retries() - 1);
                    setStatusMessage(QObject::tr("waiting for block (%1,%2)")
                                         .arg(QString::number(m_track))
                                         .arg(QString::number(m_sector)));
                    return KEEP;
                } else {
                    // abort
                    setStatusMessage(QObject::tr("missing block (%1,%2)")
                                         .arg(QString::number(m_track))
                                         .arg(QString::number(m_sector)));
                    setRetries(5);
                }
            } else if (result != READY) {
                setStatusMessage(QObject::tr("Can't warp read block (%1,%2)")
                                     .arg(QString::number(m_track))
                                     .arg(QString::number(m_sector)));
                return result;
            } else {
                // save block in job data
                m_jobData = m_block;
            }
        }
        m_state++;
        return KEEP;
    }
    // 3.) stop raw read
    else {
        // stopt raw read
        makeCommandPacket(STOP_WARP_READ_CMD, 1, m_drive, 0);
        Result result = sendPacket(host);
        if (result != READY) {
            setStatusMessage(QObject::tr("Can't stop warp read blocks"));
            return result;
        }

        // reset
        makeCommandPacket(WARP_STOP_CMD, 1, 1, 0);
        return sendPacket(host);
    }
}

NetTask::Result WarpCopyTask::warpReadDisk(NetHost &host) {
    // 1.) start warp read
    if (m_state == 0) {
        // send start warp
        makeCommandPacket(WARP_READ_DISK_CMD, 1, m_drive, 0);
        Result result = sendPacket(host);
        if (result != READY) {
            setStatusMessage(QObject::tr("Can't start warp read disk"));
            return result;
        }
        m_state = 1;
        return KEEP;
    }
    // 2.) read blocks
    else if (m_state <= m_blockCount) {
        Result result = recvBlock(host);

        // timeout means end of read if disk has errors
        if (result == RETRY)
            return READY;

        if (result != READY) {
            setStatusMessage(QObject::tr("No raw block?"));
            return result;
        }
        m_jobData = m_block;
        m_state++;
        return KEEP;
    }
    // 3.) ready
    else {
        return READY;
    }
}

NetTask::Result WarpCopyTask::warpWriteDisk(NetHost &host,
                                            const QByteArray &rawImage) {
    // 1.) start warp write
    if (m_state == 0) {
        // move to track 18 first
        QByteArray dummy;
        Result result = readDiskBlock(host, 18, 0, dummy);
        if (result != READY)
            return result;

        // send warp write disk command
        makeCommandPacket(WARP_WRITE_DISK_CMD, 1, m_drive, 0);
        result = sendPacket(host);
        if (result != READY) {
            setStatusMessage(QObject::tr("Can't start warp write disk"));
            return result;
        }
        m_state = 1;
        return KEEP;
    }
    // 2.) write raw blocks
    else if (m_state <= m_blockCount) {
        // fetch next raw block
        int blockNum = m_state - 1;
        m_packet.resize(328);
        const char *rawPtr = rawImage.constData() + blockNum * 328;
        char *packetPtr = m_packet.data();
        memcpy(packetPtr, rawPtr, 328);
        m_track = m_packet[324];
        m_sector = m_packet[322];

        // send block packet and get result
        Result result = sendPacket(host);
        if (result != READY)
            return result;

        m_state++;
        return KEEP;
    }
    // 3.) stop warp write
    else {
        // empty block finishes warp write
        m_packet.resize(328);
        memset(m_packet.data(), 0, 328);
        host.sendPacket(m_packet);

        // reset
        makeCommandPacket(WARP_STOP_CMD, 1, 1, 0);
        return sendPacket(host);
    }
}

// ---------- Low Level Communication ----------

void WarpCopyTask::makeCommandPacketWithTrackSector(char command, char fileNo,
                                                    char drive, char sec,
                                                    int track, int sector) {
    makeCommandPacket(command, fileNo, drive, sec);
    m_packet[15] = 2;
    m_packet[16] = track;
    m_packet[17] = sector;
}

void WarpCopyTask::makeCommandPacketWithString(char command, char fileNo,
                                               char drive, char sec,
                                               const QString &str) {
    makeCommandPacket(command, fileNo, drive, sec);

    int strSize = str.size();
    int packetSize = 18;
    if (strSize > 1)
        packetSize += strSize - 1;
    if (packetSize > 18)
        m_packet.resize(packetSize);

    // store length including zero byte
    m_packet[15] = strSize + 1;

    // fill in string
    QByteArray strArray = str.toUtf8();
    for (int i = 0; i < strSize; i++)
        m_packet[16 + i] = strArray[i];
    m_packet[16 + strSize] = 0;
}

void WarpCopyTask::makeCommandPacket(char command, char fileNo, char drive,
                                     char sec) {
    m_packet.resize(18);

    // header: magic+data
    m_packet[0] = 0xc9;
    m_packet[1] = 0x0f;
    m_packet[2] = 0xda;
    m_packet[3] = 0xa2;
    m_packet[4] = command;
    m_packet[5] = fileNo;
    m_packet[6] = drive;
    m_packet[7] = sec;

    m_packet[8] = 0xff; // ???? return packet size ????

    // erase packet
    for (int i = 9; i < 18; i++)
        m_packet[i] = 0;
}

NetTask::Result WarpCopyTask::sendPacketAndCheckStatus(NetHost &host) {
    Result result = sendPacketAndGetPacket(host);
    if (result == READY) {
        return checkStatus();
    }
    return result;
}

NetTask::Result WarpCopyTask::checkStatus() const {
    char status = m_block[0];
    if ((status & 0x3f) == 0) {
        return READY;
    } else {
        return ERROR;
    }
}

NetTask::Result WarpCopyTask::sendPacketAndGetStatus(NetHost &host,
                                                     char &status) {
    Result result = sendPacket(host);
    if (result == READY) {
        result = recvBlock(host);
        if (result == READY) {
            status = m_block[0];
        }
    }
    return result;
}

NetTask::Result WarpCopyTask::sendPacketAndGetPacket(NetHost &host) {
    Result result = sendPacket(host);
    if (result == READY) {
        return recvBlock(host);
    }
    return result;
}

NetTask::Result WarpCopyTask::sendPacket(NetHost &host) {
    // write packet
    if (host.sendPacket(m_packet)) {
        // read result packet
        while (host.receivePacket(m_packet)) {
            if (m_packet.size() == 18) {
                // check if its the right result packet
                if ((m_packet[0] == (char)0xc9) &&
                    (m_packet[1] == (char)0x0f) &&
                    (m_packet[2] == (char)0xda) &&
                    (m_packet[3] == (char)0xa2)) {

                    // packet status ok
                    bool ok = (m_packet[4] == (char)2);
                    if (!ok) {
                        return ERROR;
                    } else {
                        return READY;
                    }
                }
            }
        }
    }
    // time out!
    return RETRY;
}

NetTask::Result WarpCopyTask::recvBlock(NetHost &host) {
    // try 5 times the time out
    for (int i = 0; i < 5; i++) {
        if (host.receivePacket(m_block)) {
            // result blocks are always 328 bytes
            if (m_block.size() == 328) {
                return READY;
            } else {
                return ERROR;
            }
        }
    }
    // time out!
    return RETRY;
}

NetTask::Result WarpCopyTask::sendBlock(NetHost &host) {
    if (host.sendPacket(m_block))
        return READY;
    else
        return RETRY;
}

QByteArray WarpCopyTask::getBlockData(bool skipLastByte) {
    QByteArray data;
    quint8 size = (quint8)m_block[1];

    if (skipLastByte) {
        if (size <= 1)
            return data;
        size--;
    }

    data.resize(size);
    for (quint8 i = 0; i < size; i++) {
        data[i] = m_block[2 + i];
    }
    return data;
}

// ----- WarpCopyService -----

WarpCopyService::WarpCopyService() : NetService("WarpCopy") {
    // set default c64 port
    m_addrPair.c64Port = 6644;
    m_addrPair.myPort = 6644;
}

WarpCopyService::~WarpCopyService() {}

// Services:

WarpCopyTask *WarpCopyService::sendDOSCommand(int drive,
                                              const QString &command) {
    QByteArray cmd = command.toUtf8();
    WarpCopyTask *task = new WarpCopyTask(WarpCopyTask::EXEC_COMMAND, drive);
    task->setJobData(cmd);
    if (addTask(task))
        return task;
    else
        return 0;
}

WarpCopyTask *WarpCopyService::getDriveStatus(int drive) {
    WarpCopyTask *task = new WarpCopyTask(WarpCopyTask::READ_STATUS, drive);
    if (addTask(task))
        return task;
    else
        return 0;
}

WarpCopyTask *WarpCopyService::getDirectory(int drive) {
    WarpCopyTask *task = new WarpCopyTask(WarpCopyTask::READ_DIRECTORY, drive);
    if (addTask(task))
        return task;
    else
        return 0;
}

WarpCopyTask *WarpCopyService::readBlock(int drive, int track, int sector) {
    WarpCopyTask *task = new WarpCopyTask(WarpCopyTask::READ_BLOCK, drive);
    task->setTrack(track);
    task->setSector(sector);
    if (addTask(task))
        return task;
    else
        return 0;
}

WarpCopyTask *WarpCopyService::writeBlock(int drive, int track, int sector,
                                          const QByteArray &block) {
    if (block.size() != 256)
        return 0;

    WarpCopyTask *task = new WarpCopyTask(WarpCopyTask::WRITE_BLOCK, drive);
    task->setTrack(track);
    task->setSector(sector);
    task->setJobData(block);
    if (addTask(task))
        return task;
    else
        return 0;
}

WarpCopyTask *WarpCopyService::warpReadBlocks(int drive,
                                              const BlockPosVector &ts) {
    WarpCopyTask *task =
        new WarpCopyTask(WarpCopyTask::WARP_READ_BLOCKS, drive);
    task->setBlockPosVector(ts);
    if (addTask(task))
        return task;
    else
        return 0;
}

WarpCopyTask *WarpCopyService::warpReadDisk(int drive, int blockCount) {
    WarpCopyTask *task = new WarpCopyTask(WarpCopyTask::WARP_READ_DISK, drive);
    task->setBlockCount(blockCount);
    if (addTask(task))
        return task;
    else
        return 0;
}

WarpCopyTask *WarpCopyService::warpWriteDisk(int drive, const BlockMap &bm,
                                             const QByteArray &image) {
    QByteArray rawImage;
    convertImageToRawImage(bm, image, rawImage);
    int blockCount = image.size() / 256;

    WarpCopyTask *task = new WarpCopyTask(WarpCopyTask::WARP_WRITE_DISK, drive);
    task->setBlockCount(blockCount);
    task->setJobData(rawImage);
    if (addTask(task))
        return task;
    else
        return 0;
}

// Finished services:

void WarpCopyService::finishedTask(NetTask *task) {
    WarpCopyTask *wcTask = dynamic_cast<WarpCopyTask *>(task);
    if (wcTask == 0)
        return;

    WarpCopyTask::Job job = wcTask->job();
    int drive = wcTask->drive();
    const QByteArray &data = wcTask->jobData();
    int track = wcTask->track();
    int sector = wcTask->sector();

    QString msg = tr("drive %1: ").arg(QString::number(drive));

    switch (job) {
    case WarpCopyTask::EXEC_COMMAND: {
        QString cmd = QString::fromUtf8(data);
        if (wcTask->lastResult() != NetTask::READY) {
            reportEvent(msg + tr("command execution '%1' failed!!").arg(cmd));
            return;
        }

        reportEvent(msg + tr("executed command '%1'").arg(cmd));
        // read drive status
        getDriveStatus(drive);
    } break;
    case WarpCopyTask::READ_STATUS: {
        if (wcTask->lastResult() != NetTask::READY) {
            reportEvent(msg + tr("can't read status!!"));
            return;
        }

        QString status = QString::fromUtf8(data);
        reportEvent(msg + tr("status '%1'").arg(status));
        emit gotDriveStatus(drive, status);
    } break;
    case WarpCopyTask::READ_DIRECTORY: {
        if (wcTask->lastResult() != NetTask::READY) {
            reportEvent(msg + tr("can't read directory!!"));
            return;
        }

        RawDir rawDir;
        rawDir.setRawData(data);
        CBMFileList fileList;
        if (rawDir.toFileList(fileList)) {
            QString name, id;
            fileList.title(name, id);
            reportEvent(msg + tr("read directory '%1' with %2 entries")
                                  .arg(name + "," + id)
                                  .arg(fileList.size()));
            emit gotDirectory(drive, fileList);
        } else {
            reportEvent(
                msg +
                tr("error while parsing %1 directory bytes").arg(data.size()));
        }
    } break;
    case WarpCopyTask::READ_BLOCK: {
        int status = WCB_OK;
        if (wcTask->lastResult() != NetTask::READY)
            status = WCB_NET_ERROR;

        QString s = fullStatusString(track, sector, status);
        reportEvent(msg + tr("read block") + s);
        emit gotBlock(drive, track, sector, status, statusString(status), data);
    } break;
    case WarpCopyTask::WRITE_BLOCK: {
        int status = WCB_OK;
        if (wcTask->lastResult() != NetTask::READY)
            status = WCB_NET_ERROR;

        QString s = fullStatusString(track, sector, status);
        reportEvent(msg + tr("wrote block") + s);
        emit putBlock(drive, track, sector, status, statusString(status));
    } break;
    case WarpCopyTask::WARP_READ_DISK:
    case WarpCopyTask::WARP_READ_BLOCKS:
    case WarpCopyTask::WARP_WRITE_DISK: {
        bool ok = wcTask->lastResult() == NetTask::READY;
        emit finishedWarpOp(ok);
    } break;
    }
}

void WarpCopyService::keepTask(NetTask *task) {
    WarpCopyTask *wcTask = dynamic_cast<WarpCopyTask *>(task);
    if (wcTask == 0)
        return;

    WarpCopyTask::Job job = wcTask->job();
    int drive = wcTask->drive();
    const QByteArray &data = wcTask->jobData();
    int state = wcTask->state();

    QString msg = tr("drive %1: ").arg(QString::number(drive));
    switch (job) {
    case WarpCopyTask::WARP_READ_BLOCKS: {
        if (state > 1) {
            int track = wcTask->track();
            int sector = wcTask->sector();

            if (state % 2 == 0) {
                // sent block command
                emit expectBlock(drive, track, sector);
            } else {
                // got block data
                QByteArray realBlock;
                realBlock.resize(256);

                // missing block
                if ((data[0] == 0) && (data[1] == 0)) {
                    QString s = fullStatusString(track, sector, WCB_MISSING);
                    reportEvent(msg + tr("raw read block") + s);
                    emit gotBlock(drive, track, sector, WCB_MISSING,
                                  statusString(WCB_MISSING), realBlock);
                }
                // got a block
                else {
                    int trk, sec; // dummy track sector
                    int status =
                        m_tools.decodeBlock(data, realBlock, trk, sec, false);
                    QString s = fullStatusString(track, sector, status);
                    reportEvent(msg + tr("raw read block") + s);
                    emit gotBlock(drive, track, sector, status,
                                  statusString(status), realBlock);
                }
            }
        }
    } break;
    case WarpCopyTask::WARP_READ_DISK: {
        if (state > 1) {
            QByteArray realBlock;
            realBlock.resize(256);
            int track = 1, sector = -1;
            int status =
                m_tools.decodeBlock(data, realBlock, track, sector, true);
            if (sector >= 0) {
                QString s = fullStatusString(track, sector, status);
                reportEvent(msg + tr("raw read block") + s);
                emit gotBlock(drive, track, sector, status,
                              statusString(status), realBlock);
            }
        }
    } break;
    case WarpCopyTask::WARP_WRITE_DISK: {
        if (state > 1) {
            int track = wcTask->track();
            int sector = wcTask->sector();
            QString s = fullStatusString(track, sector, WCB_OK);
            reportEvent(msg + tr("raw write block") + s);
            emit putBlock(drive, track, sector, WCB_OK, statusString(WCB_OK));
        }
    }
    default:
        break;
    }
}

void WarpCopyService::convertImageToRawImage(const BlockMap &bm,
                                             const QByteArray &image,
                                             QByteArray &rawImage) {
    int blockCount = image.size() / 256;
    rawImage.resize(blockCount * 328);

    const char *imgPtr = image.constData();
    char *rawImgPtr = rawImage.data();

    // buffer for 256 byte data block
    QByteArray block;
    block.resize(256);
    char *blockPtr = block.data();

    // buffer for 328 byte raw block
    QByteArray rawBlock;
    rawBlock.resize(328);
    const char *rawBlkPtr = rawBlock.constData();

    // create interleaved raw block image
    int trackOffset = 0;
    for (int t = 0; t < bm.size(); t++) {
        int numSectors = bm[t];
        int interleave = (t < 17) ? 4 : 3; // 1..17 -> 4  18..35 -> 3
        int modOffset = (numSectors == 18) ? 1 : 0;
        int is = 0;
        for (int s = 0; s < numSectors; s++) {
            // fill block from interleaved sector
            const char *src = imgPtr + (trackOffset + is) * 256;
            memcpy(blockPtr, src, 256);

            // encode raw block
            m_tools.encodeBlock(block, rawBlock, t + 1, is);

            // copy encoded block linearly
            char *tgt = rawImgPtr + (trackOffset + s) * 328;
            memcpy(tgt, rawBlkPtr, 328);

            // next interleaved sector
            is += interleave;
            if (is >= numSectors)
                is -= numSectors - modOffset;
        }
        trackOffset += numSectors;
    }
}

QString WarpCopyService::fullStatusString(int track, int sector,
                                          int status) const {
    QString ts;
    ts.sprintf(" (t:%02d,s:%02d) ", track, sector);
    ts += statusString(status);
    return ts;
}

QString WarpCopyService::statusString(int status) const {
    switch (status) {
    case WCB_OK:
        return "ok";
    case WCB_NET_ERROR:
        return "network error";
    case WCB_NET_CHECKSUM_ERROR:
        return "network checksum error";
    case WCB_GCR_HEADER_ERROR:
        return "GCR header error";
    case WCB_GCR_DATA_ERROR:
        return "GCR data error";
    case WCB_BLOCK_CHECKSUM_ERROR:
        return "block checksum error";
    case WCB_HARD_ERROR:
        return "hard error";
    case WCB_MISSING:
        return "missing";
    default:
        return "?";
    }
}

#include "warpcopydisk.h"

WarpCopyDisk::WarpCopyDisk()
    : m_aborted(false), m_running(false), m_signalPerBlock(false) {}

WarpCopyDisk::~WarpCopyDisk() {}

void WarpCopyDisk::startCopy(int drive, WarpCopyService *service, Mode mode,
                             const BlockMap &blockMap, QByteArray *rawImage) {
    m_running = true;
    m_aborted = false;

    // get copy parameters
    m_drive = drive;
    m_service = service;
    m_mode = mode;
    m_blockMap = blockMap;
    m_rawImage = rawImage;

    // create offset map
    int sum = 0;
    int numTracks = m_blockMap.size();
    m_offsetMap.resize(numTracks);
    for (int t = 0; t < numTracks; t++) {
        m_offsetMap[t] = sum;
        sum += m_blockMap[t];
    }
    int numBlocks = sum;
    int size = numBlocks * 256;

    // init block flags
    m_blockFlags.resize(numBlocks);
    for (int b = 0; b < numBlocks; b++) {
        m_blockFlags[b] = WCB_MISSING;
    }

    // prepare image
    if (m_rawImage->size() != size) {
        m_rawImage->resize(size);
    }

    // start copier thread
    start();
}

bool WarpCopyDisk::finishCopy(int &errors) {
    // wait for copier thread to finish
    wait();

    // was aborted?
    errors = 0;
    bool ok = !m_aborted;
    if (ok) {
        // check block flags - no block missing?
        int numBlocks = m_blockFlags.size();
        for (int b = 0; b < numBlocks; b++) {
            if (m_blockFlags[b] != WCB_OK) {
                errors++;
            }
        }
    }

    m_running = false;
    m_aborted = false;
    return ok;
}

void WarpCopyDisk::abortCopy() {
    m_flagMutex.lock();
    m_aborted = true;
    m_flagMutex.unlock();
}

bool WarpCopyDisk::isAborted() {
    bool aborted = false;
    m_flagMutex.lock();
    aborted = m_aborted;
    m_flagMutex.unlock();
    return aborted;
}

bool WarpCopyDisk::isCopying() {
    bool running;
    m_flagMutex.lock();
    running = m_running;
    m_flagMutex.unlock();
    return running;
}

// ----- Thread -----

void WarpCopyDisk::run() {
    // connect to warpcopy service
    connect(m_service,
            SIGNAL(gotBlock(int, int, int, int, const QString &,
                            const QByteArray &)),
            this,
            SLOT(gotBlock(int, int, int, int, const QString &,
                          const QByteArray &)));
    connect(m_service, SIGNAL(putBlock(int, int, int, int, const QString &)),
            this, SLOT(putBlock(int, int, int, int, const QString &)));
    connect(m_service, SIGNAL(expectBlock(int, int, int)), this,
            SLOT(expectBlock(int, int, int)));
    connect(m_service, SIGNAL(finishedWarpOp(bool)), this,
            SLOT(finishedWarpOp(bool)));

    // perform copy action
    switch (m_mode) {
    case READ_DISK_SLOW:
        readDiskSlow();
        break;
    case WRITE_DISK_SLOW:
        writeDiskSlow();
        break;
    case READ_DISK_WARP:
        readDiskWarp();
        break;
    case WRITE_DISK_WARP:
        writeDiskWarp();
        break;
    }

    // disconnect from warpcopy service
    disconnect(m_service,
               SIGNAL(gotBlock(int, int, int, int, const QString &,
                               const QByteArray &)),
               this,
               SLOT(gotBlock(int, int, int, int, const QString &,
                             const QByteArray &)));
    disconnect(m_service, SIGNAL(putBlock(int, int, int, int, const QString &)),
               this, SLOT(putBlock(int, int, int, int, const QString &)));
    disconnect(m_service, SIGNAL(expectBlock(int, int, int)), this,
               SLOT(expectBlock(int, int, int)));
    disconnect(m_service, SIGNAL(finishedWarpOp(bool)), this,
               SLOT(finishedWarpOp(bool)));

    // clear running flag
    m_flagMutex.lock();
    m_running = false;
    m_flagMutex.unlock();
}

// ---------- slow operations -----------------------------------------------

//#define DUMMY

void WarpCopyDisk::readDiskSlow() {
    m_signalPerBlock = true;

#ifdef DUMMY
    QFile file("test.d64");
    file.open(QIODevice::ReadOnly);
    QByteArray block;
#endif

    bool abort = false;
    int numTracks = m_blockMap.size();
    for (int t = 0; t < numTracks; t++) {
        int numSectors = m_blockMap[t];
        for (int s = 0; s < numSectors; s++) {

            // user aborted copy!
            abort = isAborted();
            if (abort)
                break;

#ifdef DUMMY
            block = file.read(256);
            msleep(50);
            gotBlock(m_drive, t + 1, s, WCB_OK, block);
#else
            // no signal arrived
            resetSignal();

            // do read block task - emits gotBlock!
            int track = t + 1;
            WarpCopyTask *wct = m_service->readBlock(m_drive, track, s);
            if (wct == 0) {
                abortCopy();
                break;
            }

            // wait for block signal
            waitSignal();
#endif
        }
        if (abort)
            break;
    }

#ifdef DUMMY
    file.close();
#endif
}

void WarpCopyDisk::writeDiskSlow() {
    m_signalPerBlock = true;

    QByteArray block;
    block.resize(256);
    const char *data = m_rawImage->constData();

#ifdef DUMMY
    QFile file("test.d64");
    file.open(QIODevice::WriteOnly);
#endif

    bool abort = false;
    int numTracks = m_blockMap.size();
    for (int t = 0; t < numTracks; t++) {
        int numSectors = m_blockMap[t];
        for (int s = 0; s < numSectors; s++) {

            // user aborted?
            abort = isAborted();
            if (abort)
                break;

            // get raw block
            char *ptr = block.data();
            memcpy(ptr, data, 256);
            data += 256;

#ifdef DUMMY
            file.write(block);
            msleep(50);
            putBlock(m_drive, t + 1, s, WCB_OK);
#else
            // no signal arrived
            resetSignal();

            // do write block task - emits putBlock!
            int track = t + 1;
            WarpCopyTask *wct = m_service->writeBlock(m_drive, track, s, block);
            if (wct == 0) {
                abortCopy();
                break;
            }

            // if no signal arrived yet then wait
            waitSignal();
#endif
        }
        if (abort)
            break;
    }

#ifdef DUMMY
    file.close();
#endif
}

// ---------- warp operations -----------------------------------------------

void WarpCopyDisk::readDiskWarp() {
    // 1.) warp read disk
    resetSignal();
    WarpCopyTask *task = m_service->warpReadDisk(m_drive, m_blockFlags.size());
    if (task == 0) {
        abortCopy();
        return;
    }
    waitSignal();
    if (isAborted())
        return;

    // 2.) need retry for some blocks?
    BlockPosVector bpv;
    for (int i = 0; i < 5; i++) {
        if (getRetryBlocks(bpv)) {
            // retry reading blocks
            resetSignal();
            WarpCopyTask *task = m_service->warpReadBlocks(m_drive, bpv);
            if (task == 0) {
                abortCopy();
                return;
            }
            waitSignal();
            if (isAborted())
                break;
        } else
            break;
    }
}

bool WarpCopyDisk::getRetryBlocks(BlockPosVector &bpv) {
    bpv.clear();
    int block = 0;
    for (int t = 0; t < m_blockMap.size(); t++) {
        int numSectors = m_blockMap[t];
        for (int s = 0; s < numSectors; s++) {
            int status = m_blockFlags[block];
            if (status != WCB_OK) {
                bpv.push_back(BlockPos(t + 1, s));
            }
            block++;
        }
    }
    return !bpv.isEmpty();
}

void WarpCopyDisk::writeDiskWarp() {
    resetSignal();
    WarpCopyTask *task =
        m_service->warpWriteDisk(m_drive, m_blockMap, *m_rawImage);
    if (task == 0) {
        abortCopy();
        return;
    }
    waitSignal();
    if (!isAborted())
        abortCopy();
}

// ---------- feedback from warpcopy ----------------------------------------

void WarpCopyDisk::gotBlock(int /*drive*/, int track, int sector, int status,
                            const QString &statusMessage,
                            const QByteArray &block) {
    if (status == WCB_NET_ERROR) {
        abortCopy();
    } else {
        int t = track - 1;
        int blockOffset = m_offsetMap[t] + sector;

        // store block in raw image
        char *imgPtr = m_rawImage->data() + blockOffset * 256;
        const char *blkPtr = block.constData();
        memcpy(imgPtr, blkPtr, 256);

        // update block flags
        m_blockFlags[blockOffset] = status;

        // report block
        emit procBlock(t, sector, status, statusMessage);
    }

    // signal to thread
    if (m_signalPerBlock)
        wakeUpSignal();
}

void WarpCopyDisk::putBlock(int /*drive*/, int track, int sector, int status,
                            const QString &statusMessage) {
    if (status == WCB_NET_ERROR) {
        abortCopy();
    } else {
        int t = track - 1;
        int blockOffset = m_offsetMap[t] + sector;

        // update block flags
        m_blockFlags[blockOffset] = status;

        // report block
        emit procBlock(t, sector, status, statusMessage);
    }

    // signal to thread
    if (m_signalPerBlock)
        wakeUpSignal();
}

void WarpCopyDisk::expectBlock(int /*drive*/, int track, int sector) {
    emit prepareBlock(track - 1, sector);
}

void WarpCopyDisk::finishedWarpOp(bool ok) {
    if (!ok)
        abortCopy();

    // signal to thread
    wakeUpSignal();
}

// ----- Threading ----------------------------------------------------------

void WarpCopyDisk::resetSignal() {
    m_signalMutex.lock();
    m_gotSignalFlag = false;
    m_signalMutex.unlock();
}

void WarpCopyDisk::waitSignal() {
    m_signalMutex.lock();
    if (!m_gotSignalFlag) {
        m_gotSignalFlag = true;
        m_signalWaitCond.wait(&m_signalMutex);
    }
    m_signalMutex.unlock();
}

void WarpCopyDisk::wakeUpSignal() {
    m_signalMutex.lock();
    bool wakeUp = false;
    if (!m_gotSignalFlag)
        m_gotSignalFlag = true;
    else
        wakeUp = true;
    m_signalMutex.unlock();
    if (wakeUp)
        m_signalWaitCond.wakeAll();
}

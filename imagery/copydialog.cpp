#include "copydialog.h"

CopyDialog::CopyDialog(QWidget *parent) : QDialog(parent), m_copier(nullptr) {
    setWindowTitle(tr("Copy Disk"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_titleLabel = new QLabel(this);
    layout->addWidget(m_titleLabel, 0);
    m_blockMapWidget = new BlockMapWidget(this);
    layout->addWidget(m_blockMapWidget, 1);
    m_statusLabel = new QLabel(this);
    layout->addWidget(m_statusLabel, 0);
    QHBoxLayout *hlayout = new QHBoxLayout;
    layout->addLayout(hlayout);
    m_timeLabel = new QLabel(this);
    hlayout->addWidget(m_timeLabel, 0);
    hlayout->addStretch(1);
    m_abortButton = new QPushButton(tr("Abort"), this);
    hlayout->addWidget(m_abortButton, 0);

    connect(m_abortButton, SIGNAL(clicked()), this, SLOT(abort()));
    connect(this, SIGNAL(rejected()), this, SLOT(abort()));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateTime()));
}

CopyDialog::~CopyDialog() {}

bool CopyDialog::copyDisk(int drive, WarpCopyService *service,
    WarpCopyDisk::Mode mode, DImage *image) {

    // init block map widget and title
    BlockMap blockMap = image->blockMap();
    QString title;

    m_blockMapWidget->setBlockMap(blockMap);
    switch (mode) {
        case WarpCopyDisk::READ_DISK_SLOW:
            title = tr("Reading Disk (Slow)");
            break;
        case WarpCopyDisk::WRITE_DISK_SLOW:
            title = tr("Writing Disk (Slow)");
            break;
        case WarpCopyDisk::READ_DISK_WARP:
            title = tr("Reading Disk (Warp)");
            break;
        case WarpCopyDisk::WRITE_DISK_WARP:
            title = tr("Writing Disk (Warp)");
            break;
    }
    m_titleLabel->setText(title);

    // fire up dialog
    setModal(true);
    show();
    activateWindow();
    raise();

    // prepare raw image
    QByteArray rawImage;
    if ((mode == WarpCopyDisk::WRITE_DISK_SLOW) ||
        (mode == WarpCopyDisk::WRITE_DISK_WARP)) {
        // read in raw image for write operation
        image->putToRawImage(rawImage);
    }

    m_startTime = QDateTime::currentDateTime();
    m_timer.start(500);

    // create copier
    m_copier = new WarpCopyDisk;
    connect(m_copier, SIGNAL(procBlock(int, int, int, const QString &)), this,
        SLOT(procBlock(int, int, int, const QString &)));
    connect(m_copier, SIGNAL(prepareBlock(int, int)), this,
        SLOT(prepareBlock(int, int)));

    // start copier thread!
    m_copier->startCopy(drive, service, mode, blockMap, &rawImage);

    // handle event loop of gui main thread while copier is copying
    while (m_copier->isCopying())
        QCoreApplication::processEvents(QEventLoop::AllEvents, 500);

    // finish copier thread
    int numBlockErrors;
    bool ok = m_copier->finishCopy(numBlockErrors);

    // remove copier
    disconnect(m_copier, SIGNAL(procBlock(int, int, int, const QString &)),
        this, SLOT(procBlock(int, int, int, const QString &)));
    disconnect(m_copier, SIGNAL(prepareBlock(int, int)), this,
        SLOT(prepareBlock(int, int)));

    delete m_copier;
    m_copier = nullptr;
    m_timer.stop();

    // was not aborted
    if (ok) {
        if ((mode == WarpCopyDisk::READ_DISK_SLOW) ||
            (mode == WarpCopyDisk::READ_DISK_WARP)) {

            if (numBlockErrors > 0) {
                if (QMessageBox::question(
                    this, tr("Read Disk Result"),
                    tr("%1 read Blocks still have Errors!\nKeep Image?")
                    .arg(QString::number(numBlockErrors)),
                    QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {

                    ok = false;
                }
            }

            // write out raw image from read operation
            if (ok) image->getFromRawImage(rawImage);
        } else {
            if (numBlockErrors > 0) {
                QMessageBox::warning(this, tr("Write Disk Result"),
                    tr("%1 wrote Blocks still have Errors!")
                    .arg(QString::number(numBlockErrors)));
            }
        }
    }

    // hide dialog
    hide();

    // mac needs events handled here to hide menu
    QCoreApplication::processEvents();

    // return finished copy state
    return ok;
}

void CopyDialog::abort() {
    // signal copier thread to stop
    if (m_copier != nullptr) m_copier->abortCopy();
}

void CopyDialog::procBlock(int track, int sector, int status, const QString &statusMessage) {
    // set color block
    QColor color;
    switch (status) {
        case WCB_OK:
            color = Qt::green;
            break;
        case WCB_NET_ERROR:
            color = Qt::darkGray;
            break;
        case WCB_NET_CHECKSUM_ERROR:
            color = Qt::gray;
            break;
        case WCB_GCR_HEADER_ERROR:
            color = Qt::cyan;
            break;
        case WCB_GCR_DATA_ERROR:
            color = Qt::yellow;
            break;
        case WCB_BLOCK_CHECKSUM_ERROR:
            color = Qt::magenta;
            break;
        case WCB_HARD_ERROR:
            color = Qt::red;
            break;
        default:
            color = Qt::black;
            break;
    }
    m_blockMapWidget->setBlockColor(track, sector, color);

    // set status
    QString msg;
    msg = QString("Track %02d, Sector %02d ").arg(track + 1).arg(sector);
    msg += statusMessage;
    m_statusLabel->setText(msg);
}

void CopyDialog::prepareBlock(int track, int sector) {
    m_blockMapWidget->setBlockColor(track, sector, Qt::darkBlue);
}

void CopyDialog::updateTime() {
    QDateTime now = QDateTime::currentDateTime();
    int total = int(m_startTime.secsTo(now));
    int secs = total % 60;
    total /= 60;
    int mins = total % 60;
    total /= 60;
    QString time;
    time = QString("%02d:%02d:%02d").arg(total).arg(mins).arg(secs);
    m_timeLabel->setText(time);
}

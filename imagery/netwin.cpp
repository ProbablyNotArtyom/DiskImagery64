#include "netwin.h"
#include "codenet.h"
#include "preferences.h"
#include "version.h"

#include <QGridLayout>
#include <QLabel>
#include <QTextEdit>
#include <QToolButton>

NetWin::NetWin(QWidget *parent) : QDialog(parent, Qt::Tool), m_copyDialog(nullptr) {
    setWindowTitle("DiskImagery64: " + tr("Network"));
    QGridLayout *layout = new QGridLayout(this);

    // log view
    m_logView = new QTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setLineWrapMode(QTextEdit::NoWrap);
    QFont smallFont("Monaco");
	smallFont.setPointSize(FONT_POINT_LOG);
    m_logView->setFont(smallFont);

    // packet counter
    m_packetsInLabel = new QLabel(this);
    m_packetsOutLabel = new QLabel(this);

    m_stopButton = new QToolButton(this);
    m_stopButton->setText(tr("Abort"));
    connect(m_stopButton, SIGNAL(clicked()), this, SLOT(stopNet()));

    layout->addWidget(m_logView, 0, 0, 1, 3);
    layout->addWidget(m_packetsInLabel, 1, 0);
    layout->addWidget(m_packetsOutLabel, 1, 1);
    layout->addWidget(m_stopButton, 1, 2);
    layout->setMargin(2);

    resetCounters();
    loadSettings();

    logService(&m_codeNet);
    logService(&m_netDrive);
    logService(&m_warpCopy);
}

NetWin::~NetWin() {
    if (m_copyDialog != nullptr) delete m_copyDialog;

    saveSettings();
    stopNet();
}

// ----- Settings -----

void NetWin::saveSettings() {
    QSettings settings;
    settings.beginGroup("NetWin");

    // geometry
    settings.setValue("geometry", geometry());
    // visible
    settings.setValue("visible", isVisible());

    settings.endGroup();
}

void NetWin::loadSettings() {
    QSettings settings;
    settings.beginGroup("NetWin");

    // geometry
    QRect rect = settings.value("geometry", QRect(400, 100, 200, 100)).toRect();
    move(rect.topLeft());
    resize(rect.size());
    // visible
    bool visible = settings.value("visible", false).toBool();
    if (visible) show();
    else hide();

    settings.endGroup();
}

// ----- UI -----

void NetWin::logMessage(const QString &msg) {
    QDateTime now = QDateTime::currentDateTime();
    QLocale locale;
    QString header = locale.toString(now.date(), QLocale::ShortFormat) + " " +
        locale.toString(now.time(), QLocale::ShortFormat) + " ";

    m_logView->append(header + msg);
}

void NetWin::logService(NetService *service) {
    connect(service, SIGNAL(networkEvent(const QString &)), this, SLOT(logMessage(const QString &)));
    connect(service, SIGNAL(sentPacket(int)), this, SLOT(reportOutPacket(int)));
    connect(service, SIGNAL(receivedPacket(int)), this, SLOT(reportInPacket(int)));
}

void NetWin::resetCounters() {
    m_packetsIn = 0;
    m_packetsInSize = 0;
    m_packetsOut = 0;
    m_packetsOutSize = 0;
    updateCounters();
}

void NetWin::updateCounters() {
    QString inMsg;
    inMsg.asprintf("In: %d/%d bytes", m_packetsIn, m_packetsInSize);
    m_packetsInLabel->setText(inMsg);
    QString outMsg;
    outMsg.asprintf("Out: %d/%d bytes", m_packetsOut, m_packetsOutSize);
    m_packetsOutLabel->setText(outMsg);
}

void NetWin::reportInPacket(int size) {
    m_packetsIn++;
    m_packetsInSize += size;
    updateCounters();
}

void NetWin::reportOutPacket(int size) {
    m_packetsOut++;
    m_packetsOutSize += size;
    updateCounters();
}

// ----- Commands -----

void NetWin::stopNet() {
    if (m_codeNet.isServiceAvailable()) m_codeNet.stopService();
    if (m_netDrive.isServiceAvailable()) m_netDrive.stopService();
    if (m_warpCopy.isServiceAvailable()) m_warpCopy.stopService();
    resetCounters();
}

void NetWin::runProgram(quint16 addr, const QByteArray &data) {
    if (!startCodeNet()) return;

    m_codeNet.sendData(addr, data);
    if (addr == 0x0801) m_codeNet.execRun();
    else m_codeNet.execJump(addr);
}

void NetWin::shareFiles(const CBMFileList &files) {
    if (!startNetDrive()) return;

    m_netDrive.shareFiles(files);
}

// ----- warp copy -----

bool NetWin::warpReadDisk(DImage *image) {
    return copyDisk(WarpCopyDisk::READ_DISK_WARP, image);
}

bool NetWin::warpWriteDisk(DImage *image) {
    return copyDisk(WarpCopyDisk::WRITE_DISK_WARP, image);
}

bool NetWin::readDisk(DImage *image) {
    return copyDisk(WarpCopyDisk::READ_DISK_SLOW, image);
}

bool NetWin::writeDisk(DImage *image) {
    return copyDisk(WarpCopyDisk::WRITE_DISK_SLOW, image);
}

bool NetWin::copyDisk(WarpCopyDisk::Mode mode, DImage *image) {
    if (!startWarpCopy()) return false;
    if (m_copyDialog == nullptr) m_copyDialog = new CopyDialog;
    return m_copyDialog->copyDisk(8, &m_warpCopy, mode, image);
}

// ----- other wc ops -----

void NetWin::formatDisk(const QString &name, const QString &id) {
    QString cmd = "N:" + name;
    if (id != "") cmd += "," + id;
    if (!startWarpCopy()) return;

    m_warpCopy.sendDOSCommand(8, cmd);
}

void NetWin::verifyDisk() {
    if (!startWarpCopy()) return;
    m_warpCopy.sendDOSCommand(8, "V");
}

void NetWin::sendDOSCommand(const QString &cmd) {
    if (!startWarpCopy()) return;
    m_warpCopy.sendDOSCommand(8, cmd);
}

void NetWin::getDriveStatus() {
    if (!startWarpCopy()) return;
    m_warpCopy.getDriveStatus(8);
}

// ---------- start services ----------

bool NetWin::startCodeNet() {
    if (!m_codeNet.isServiceAvailable()) {
        AddrPair addrPair;
        Preferences::getNetworkDefaults(addrPair);
        return m_codeNet.startService(addrPair);
    }
    return true;
}

bool NetWin::startNetDrive() {
    if (!m_netDrive.isServiceAvailable()) {
        AddrPair addrPair;
        Preferences::getNetworkDefaults(addrPair);
        return m_netDrive.startService(addrPair);
    }
    return true;
}

bool NetWin::startWarpCopy() {
    if (!m_warpCopy.isServiceAvailable()) {
        AddrPair addrPair;
        Preferences::getNetworkDefaults(addrPair);
        return m_warpCopy.startService(addrPair);
    }
    return true;
}

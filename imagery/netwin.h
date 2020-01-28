#ifndef NETWIN_H
#define NETWIN_H

#include <QtGui/QtGui>
#include "codenet.h"
#include "netdrive.h"
#include "warpcopy.h"
#include "copydialog.h"

#include <QToolButton>
#include <QTextEdit>

class NetWin : public QDialog
{
    Q_OBJECT

public:
    NetWin(QWidget *parent=0);
    ~NetWin();

    void runProgram(quint16 addr,const QByteArray &data);
    void shareFiles(const CBMFileList &files);

    bool warpReadDisk(DImage *image);
    bool warpWriteDisk(DImage *image);
    bool readDisk(DImage *image);
    bool writeDisk(DImage *image);

    void formatDisk(const QString &name,const QString &id="");
    void verifyDisk();
    void sendDOSCommand(const QString &cmd);
    void getDriveStatus();

    void saveSettings();
    void loadSettings();

public slots:
    void stopNet();

protected slots:
    void logMessage(const QString &msg);
    void reportInPacket(int size);
    void reportOutPacket(int size);

protected:
    QTextEdit *m_logView;
    QLabel    *m_packetsInLabel;
    QLabel    *m_packetsOutLabel;
    QToolButton *m_stopButton;

    int m_packetsIn;
    int m_packetsOut;
    int m_packetsInSize;
    int m_packetsOutSize;

    CodeNetService  m_codeNet;
    NetDriveService m_netDrive;
    WarpCopyService m_warpCopy;

    CopyDialog *m_copyDialog;

    void logService(NetService *service);
    void updateCounters();
    void resetCounters();

    bool startCodeNet();
    bool startNetDrive();
    bool startWarpCopy();

    bool copyDisk(WarpCopyDisk::Mode mode,DImage *image);
};

#endif

#ifndef OPENCBMWIN_H
#define OPENCBMWIN_H

#include <QtGui/QtGui>
#include <QDialog>

#include "cbmfile.h"
#include "dimage.h"

class OpenCBMWin : public QDialog {
    Q_OBJECT

public:
    OpenCBMWin(QWidget *parent = nullptr);
    ~OpenCBMWin();

    enum DriveType { C1541, C1571, C1581, UNKNOWN, NONE };
    typedef struct {
        int         id;     // Device ID
        DriveType   type;   // Drive model
    } CBMDrive;
    typedef QVector<CBMDrive> DriveList;

    bool writeFiles(const CBMFileList &files);
    bool writeDisk(DImage *image);
    bool readDisk(DImage *image);
    bool formatDisk(const QString &name, const QString &id = "");

    bool cbmctrlReset();
    void cbmctrlDetect(DriveList &list);
    void cbmctrlWaitForChange(CBMDrive &drive);
    QString cbmctrlGetDriveStatus(CBMDrive &drive);
    void cbmctrlReadDirectory(CBMFileList &files);
    void cbmctrlSendCommand(const QString &cmd);

protected:

};

#endif // OPENCBMWIN_H

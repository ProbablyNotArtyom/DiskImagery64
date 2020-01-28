#ifndef OPENCBMWIN_H
#define OPENCBMWIN_H

#include <QtGui/QtGui>
#include <QDialog>
#include <opencbm.h>
#include <cbmcopy.h>

#include "cbmfile.h"
#include "dimage.h"
#include "petscii.h"

class OpenCBMWin : public QDialog {
    Q_OBJECT

public:
    OpenCBMWin(QWidget *parent = nullptr);
    ~OpenCBMWin();

    typedef cbm_device_type_e DriveType;
    typedef struct {
        unsigned char   unit;   // IEC unit
        DriveType       type;   // Drive model
        intptr_t        driver; // Device file of cable
    } CBMDrive;
    typedef QVector<CBMDrive> DriveList;

    bool writeFiles(const CBMFileList &files);
    bool writeDisk(DImage *image);
    bool readDisk(DImage *image);
    bool formatDisk(const QString &name, const QString &id = "");

    bool cbmctrlReset();
    bool cbmctrlDetect(DriveList &list);
    bool cbmctrlWaitForChange(CBMDrive &drive);
    QString cbmctrlGetDriveStatus(CBMDrive &drive);
    QByteArray cbmctrlReadDirectory(QString filename);
    void cbmctrlSendCommand(const QString &cmd);

protected:

};

#endif // OPENCBMWIN_H

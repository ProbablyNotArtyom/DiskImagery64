#include "opencbmwin.h"
#include "dimage.h"
#include "petscii.h"

OpenCBMWin::OpenCBMWin(QWidget *parent) : QDialog(parent) {

}

OpenCBMWin::~OpenCBMWin() {}



/* -------------------------------- */

bool OpenCBMWin::writeFiles(const CBMFileList &files) {

    return true;
}

bool OpenCBMWin::writeDisk(DImage *image) {

    return true;
}

bool OpenCBMWin::readDisk(DImage *image) {

    return true;
}

bool OpenCBMWin::formatDisk(const QString &name, const QString &id) {

    return true;
}

bool OpenCBMWin::cbmctrlReset() {

    return true;
}

bool OpenCBMWin::cbmctrlDetect(DriveList &list) {

}

bool OpenCBMWin::cbmctrlWaitForChange(CBMDrive &drive) {
    return false;
}

QString OpenCBMWin::cbmctrlGetDriveStatus(CBMDrive &drive) {
    char buf[40];
    cbm_device_status(drive.driver, drive.unit, buf, sizeof(buf));
    return QString::fromUtf8(buf);
}

QByteArray OpenCBMWin::cbmctrlReadDirectory(QString filename) {
    static QStringList fileTypes = (QStringList() << "del" << "seq" << "prg" << "usr" << "rel" << "cbm" << "dir" << "???");
    char mode[3] = "rb";
    unsigned char dir[2] = "$";
    unsigned char buffer[254];
    DiskImage *diskImage;
    ImageFile *dh;
    char name[17], id[6];
    int type, size, offset;

    if ((diskImage = di_load_image(filename.toLocal8Bit().data())) == nullptr) {
        qDebug() << "Load image failed";
        return QByteArray();
    }
    if ((dh = di_open(diskImage, dir, T_PRG, mode)) == nullptr) {
        qDebug() << "Couldn't open directory";
        return QByteArray();
    }

    di_name_from_rawname(name, di_title(diskImage));
    memcpy(id, di_title(diskImage) + 18, 5);
    id[5] = 0;

    QByteArray output;
    QString label = QString("0 \"%1\" %2")
        .arg(Petscii::convertToAscii(QString(name), Petscii::ONLY_CASE), -16)
        .arg(Petscii::convertToAscii(QString(id), Petscii::ONLY_CASE));

    output.append("label|").append(label).append('\n');

    if (di_read(dh, buffer, 254) != 254) {
        qDebug() << "BAM read failed";
        di_close(dh);
        di_free_image(diskImage);
        return output;
    }

    QByteArray dirEntry;

    /* Read directory blocks */
    while (di_read(dh, buffer, 254) == 254) {
        dirEntry.clear();
        for (offset = -2; offset < 254; offset += 32) {
            if (buffer[offset+2]) {
                /* If file type != 0 */
                di_name_from_rawname(name, buffer + offset + 5);
                type = buffer[offset + 2] & 7;
                size = buffer[offset + 31]<<8 | buffer[offset + 30];
                qDebug() << name;
                output.append("entry|").append(QString::number(size)).append('|').append(name).append('|').append(fileTypes.at(type)).append('\n');
            }
        }
    }

    output.append("bfree|").append(QString::number(diskImage->blocksfree)).append(" blocks free");

    di_close(dh);
    di_free_image(diskImage);

    return output;
}

void OpenCBMWin::cbmctrlSendCommand(const QString &cmd) {

}

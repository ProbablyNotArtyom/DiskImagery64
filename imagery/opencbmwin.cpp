#include "opencbmwin.h"
#include "cbmfile.h"
#include "dimage.h"

OpenCBMWin::OpenCBMWin(QWidget *parent) : QDialog(parent) {

}

OpenCBMWin::~OpenCBMWin() {}



/* -------------------------------- */

bool OpenCBMWin::writeFiles(const CBMFileList &files) {

}

bool OpenCBMWin::writeDisk(DImage *image) {

}

bool OpenCBMWin::readDisk(DImage *image) {

}

bool OpenCBMWin::formatDisk(const QString &name, const QString &id) {

}

bool OpenCBMWin::cbmctrlReset() {

}

void OpenCBMWin::cbmctrlDetect(DriveList &list) {

}

void OpenCBMWin::cbmctrlWaitForChange(CBMDrive &drive) {

}

QString OpenCBMWin::cbmctrlGetDriveStatus(CBMDrive &drive) {

}

void OpenCBMWin::cbmctrlReadDirectory(CBMFileList &files) {

}

void OpenCBMWin::cbmctrlSendCommand(const QString &cmd) {

}

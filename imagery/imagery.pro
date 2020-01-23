# project.pro
# qt project file for DiskImagery64
# written by Christian Vogelgsang <chris@vogelgsang.org>

include(../common.pri)

TEMPLATE = app
TARGET   = DiskImagery64
CONFIG += qt create_prl link_prl
QT += network

HEADERS += dimagemodel.h   dimagewin.h   filewin.h   filemodel.h
SOURCES += dimagemodel.cpp dimagewin.cpp filewin.cpp filemodel.cpp
HEADERS += netwin.h   blockmapwidget.h   copydialog.h
SOURCES += netwin.cpp blockmapwidget.cpp copydialog.cpp
HEADERS += mainwin.h   preferences.h   app.h   version.h
SOURCES += mainwin.cpp preferences.cpp app.cpp main.cpp

RESOURCES = ../icons/icons.qrc

INCLUDEPATH += ../base ../net
HEADERS += ../base/petscii.h ../base/cbmfile.h ../base/dimage.h
HEADERS += ../base/rawdir.h
HEADERS += ../net/nethost.h ../net/netservice.h ../net/codenet.h
HEADERS += ../net/netdrive.h ../net/netdrivedevice.h

LIBS += -L../BUILD -lnet -lbase
POST_TARGETDEPS += ../BUILD/libbase.a ../BUILD/libnet.a

mac {
 ICON = ../icons/imagery/imagery.icns 
 QMAKE_BUNDLE_DATA = DISK_ICONS
 DISK_ICONS.files = ../icons/d64/d64.icns ../icons/d71/d71.icns ../icons/d81/d81.icns
 DISK_ICONS.path = Contents/Resources
 QMAKE_INFO_PLIST = mac/Info.plist
}
win32 {
 RC_FILE = ../icons/imagery/imagery.rc
}
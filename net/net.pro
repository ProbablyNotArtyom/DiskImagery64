# net.pro
# qt project file for DiskImagery64
# written by Christian Vogelgsang <chris@vogelgsang.org>

include(../common.pri)

TEMPLATE = lib
TARGET   = net
CONFIG  += staticlib create_prl
QT -= gui
QT += network

INCLUDEPATH += ../base
HEADERS += ../base/petscii.h ../base/cbmfile.h ../base/dimage.h

HEADERS += nethost.h    netservice.h   codenet.h
SOURCES += nethost.cpp  netservice.cpp codenet.cpp
HEADERS += netdrive.h   netdrivedevice.h
SOURCES += netdrive.cpp netdrivedevice.cpp
HEADERS += warpcopytools.h   warpcopy.h   warpcopydisk.h
SOURCES += warpcopytools.cpp warpcopy.cpp warpcopydisk.cpp
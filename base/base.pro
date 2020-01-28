# base.pro
# qt project file for DiskImagery64
# written by Christian Vogelgsang <chris@vogelgsang.org>

TEMPLATE = lib
TARGET   = base
QMAKE_CLEAN += libbase.a libbase.prl
CONFIG  += qt staticlib create_prl
QT -= gui
QT += widgets

HEADERS += petscii.h   cbmfile.h   dimage.h   diskimage.h
SOURCES += petscii.cpp cbmfile.cpp dimage.cpp diskimage.c
HEADERS += rawdir.h
SOURCES += rawdir.cpp

# base.pro
# qt project file for DiskImagery64
# written by Christian Vogelgsang <chris@vogelgsang.org>

include(../common.pri)

TEMPLATE = lib
TARGET   = base
CONFIG  += staticlib create_prl
QT -= gui

HEADERS += petscii.h   cbmfile.h   dimage.h   ../diskimage-0.95/diskimage.h
SOURCES += petscii.cpp cbmfile.cpp dimage.cpp ../diskimage-0.95/diskimage.c
HEADERS += rawdir.h
SOURCES += rawdir.cpp

# base.pro
# qt project file for DiskImagery64
# written by Christian Vogelgsang <chris@vogelgsang.org>

include(../common.pri)

TEMPLATE = lib
TARGET   = widgets
CONFIG  += staticlib create_prl
QT += gui

HEADERS += ColorPickerButton.h
SOURCES += ColorPickerButton.cpp
HEADERS += ColorPickerActionWidget.h

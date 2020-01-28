# base.pro
# qt project file for DiskImagery64
# written by Christian Vogelgsang <chris@vogelgsang.org>

TEMPLATE = lib
TARGET   = widgets
QMAKE_CLEAN += libwidgets.a libwidgets.prl
CONFIG  += qt staticlib create_prl
QT += widgets gui

HEADERS += ColorPickerButton.h ColorPickerActionWidget.h
SOURCES += ColorPickerButton.cpp

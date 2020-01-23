# config.pri
# common project definitions

CONFIG += qt 
QT += widgets

# universal build
CONFIG(release) {
  mac {
	message(Mac Universal Binary)
    CONFIG += ppc x86
    QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
  }
}

INCLUDEPATH += ../diskimage-0.95

OBJECTS_DIR = ../BUILD
MOC_DIR = ../BUILD
RCC_DIR = ../BUILD
DESTDIR = ../BUILD

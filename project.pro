# project.pro
# main qt project file for DiskImagery64
# written by Christian Vogelgsang <chris@vogelgsang.org>

TEMPLATE = subdirs
SUBDIRS = base net widgets imagery
imagery.depends = base net widgets

win32:CONFIG(release, debug|release): \
    LIBS += -L$$OUT_PWD/base/release/ -lbase \
    LIBS += -L$$OUT_PWD/widgets/release/ -lwidgets \
    LIBS += -L$$OUT_PWD/net/release/ -lnet
else:win32:CONFIG(debug, debug|release): \
    LIBS += -L$$OUT_PWD/base/release/ -lbase \
    LIBS += -L$$OUT_PWD/widgets/release/ -lwidgets \
    LIBS += -L$$OUT_PWD/net/release/ -lnet
else:unix: \
    LIBS += -L/usr/lib/ -lopencbm \
    LIBS += -L$$OUT_PWD/base/ -lbase \
    LIBS += -L$$OUT_PWD/widgets/ -lwidgets \
    LIBS += -L$$OUT_PWD/net/ -lnet

win32-g++:CONFIG(release, debug|release): \
    PRE_TARGETDEPS += $$OUT_PWD/base/release/libbase.a \
		      $$OUT_PWD/widgets/release/libwidgets.a \
		      $$OUT_PWD/net/release/libnet.a
else:win32-g++:CONFIG(debug, debug|release): \
    PRE_TARGETDEPS += $$OUT_PWD/base/debug/libbase.a \
		      $$OUT_PWD/widgets/debug/libwidgets.a \
		      $$OUT_PWD/net/debug/libnet.lib
else:win32:!win32-g++:CONFIG(release, debug|release): \
    PRE_TARGETDEPS += $$OUT_PWD/base/release/base.lib \
		      $$OUT_PWD/widgets/release/widgets.lib \
		      $$OUT_PWD/net/release/net.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): \
    PRE_TARGETDEPS += $$OUT_PWD/base/debug/base.lib \
		      $$OUT_PWD/widgets/debug/widgets.lib \
		      $$OUT_PWD/net/debug/net.lib
else:unix: \
    PRE_TARGETDEPS += $$OUT_PWD/base/libbase.a \
		      $$OUT_PWD/widgets/libwidgets.a \
		      $$OUT_PWD/net/libnet.a

# universal build
mac:CONFIG(release) {
    mac {
	message(Mac Universal Binary)
	CONFIG += ppc x86
	QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
    }
}

OBJECTS_DIR = ../BUILD
MOC_DIR = ../BUILD
RCC_DIR = ../BUILD
DESTDIR = ../BUILD

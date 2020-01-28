# project.pro
# main qt project file for DiskImagery64
# written by Christian Vogelgsang <chris@vogelgsang.org>

TEMPLATE = subdirs
SUBDIRS = base net widgets imagery

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/base/release/ -lbase
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/base/debug/ -lbase
else:unix: LIBS += -L$$OUT_PWD/base/ -lbase

INCLUDEPATH += $$PWD/widgets
DEPENDPATH += $$PWD/widgets

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/base/release/libbase.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/base/debug/libbase.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/base/release/base.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/base/debug/base.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/base/libbase.a

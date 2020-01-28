# project.pro
# qt project file for DiskImagery64
# written by Christian Vogelgsang <chris@vogelgsang.org>

TEMPLATE = app
TARGET   = diskimagery64
QMAKE_CLEAN += diskimagery64
CONFIG += qt create_prl link_prl
QT += widgets network

HEADERS += dimagemodel.h dimagewin.h filewin.h filemodel.h opencbmwin.h
SOURCES += dimagemodel.cpp dimagewin.cpp filewin.cpp filemodel.cpp opencbmwin.cpp
HEADERS += netwin.h blockmapwidget.h copydialog.h
SOURCES += netwin.cpp blockmapwidget.cpp copydialog.cpp
HEADERS += mainwin.h preferences.h app.h version.h
SOURCES += mainwin.cpp preferences.cpp app.cpp main.cpp

RESOURCES = ../icons/icons.qrc

INCLUDEPATH += ../base ../net ../widgets
HEADERS += ../base/petscii.h ../base/cbmfile.h ../base/dimage.h ../base/rawdir.h
HEADERS += ../net/nethost.h ../net/netservice.h ../net/codenet.h ../net/netdrive.h ../net/netdrivedevice.h
HEADERS += ../widgets/ColorPickerButton.h ../widgets/ColorPickerActionWidget.h

LIBS += -L../net -lnet -L../base -lbase -L../widgets -lwidgets
POST_TARGETDEPS += ../base/libbase.a ../net/libnet.a ../widgets/libwidgets.a

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

unix {
    icon.path = /usr/share/pixmaps
    icon.files = ../icons/imagery/imagery.xpm
    icon.uninstall = @echo "uninstall"
    desktop.path = /usr/share/applications
    desktop.files = ../icons/imagery/diskimagery64.desktop
    desktop.uninstall = @echo "uninstall"
    target.path = /usr/local/bin
    target.files = diskimagery64
    target.uninstall = @echo "uninstall"

    INSTALLS += icon desktop target
}

#-------------------------------------------------
#
# Project created by QtCreator 2014-04-29T11:18:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Spotifyserver
TEMPLATE = app


SOURCES += main.cpp

HEADERS  +=

FORMS    +=

# system libraries
win32: LIBS += -lMfplat -lMfuuid -lWs2_32 -lWinmm -lOle32

#OpenSSL
win32: LIBS += -llibeay32MD -lssleay32MD -L$$PWD/../../../spotifyserver_deps/lib/OpenSSL/OpenSSL-Win32/lib/VC
win32:INCLUDEPATH += $$PWD/../../../spotifyserver_deps/lib/OpenSSL/include
win32:DEPENDPATH += $$PWD/../../../spotifyserver_deps/lib/OpenSSL/include

# UI
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QTWidgetUI/release/ -lQTWidgetUI
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QTWidgetUI/debug/ -lQTWidgetUI
else:unix: LIBS += -L$$OUT_PWD/../QTWidgetUI/ -lQTWidgetUI

INCLUDEPATH += $$PWD/../QTWidgetUI
DEPENDPATH += $$PWD/../QTWidgetUI

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QTWidgetUI/release/libQTWidgetUI.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QTWidgetUI/debug/libQTWidgetUI.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QTWidgetUI/release/QTWidgetUI.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QTWidgetUI/debug/QTWidgetUI.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../QTWidgetUI/libQTWidgetUI.a

# libspotify
win32: LIBS += -L$$PWD/../../../spotifyserver_deps/lib/libspotify-12.1.51-win32-release/lib/ -llibspotify

win32:INCLUDEPATH += $$PWD/../../../spotifyserver_deps/lib/libspotify-12.1.51-win32-release/include
win32:DEPENDPATH += $$PWD/../../../spotifyserver_deps/lib/libspotify-12.1.51-win32-release/include

# visual studio libs
win32:CONFIG(release, debug|release): LIBS += -L"$$PWD/../../workspaces/Visual Studio/release/" -lCommon -lServerLib
else:win32:CONFIG(debug, debug|release): LIBS += -L"$$PWD/../../workspaces/Visual Studio/debug/" -lCommon -lServerLib

INCLUDEPATH += $$PWD/../../common += $$PWD/../../src
DEPENDPATH += $$PWD/../../common += $$PWD/../../src

win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += "$$PWD/../../workspaces/Visual Studio/release/Common.lib"  "$$PWD/../../workspaces/Visual Studio/release/ServerLib.lib"
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += "$$PWD/../../workspaces/Visual Studio/debug/Common.lib"  "$$PWD/../../workspaces/Visual Studio/debug/ServerLib.lib"

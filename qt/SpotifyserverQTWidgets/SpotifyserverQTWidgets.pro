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

# UI
win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-QTWidgetUI-Desktop_Qt_5_2_1_MSVC2012_32bit-Debug/debug -lQTWidgetUI
else:win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-QTWidgetUI-Desktop_Qt_5_2_1_MSVC2012_32bit-Release/release -lQTWidgetUI

INCLUDEPATH += $$PWD/../QTWidgetUI

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

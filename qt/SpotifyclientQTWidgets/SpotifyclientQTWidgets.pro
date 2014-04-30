#-------------------------------------------------
#
# Project created by QtCreator 2014-04-29T11:18:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Spotifyclient
TEMPLATE = app


SOURCES += main.cpp

HEADERS  +=

FORMS    +=

# system libraries
win32: LIBS += -lMfplat -lMfuuid -lWs2_32 -lWinmm -lOle32

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

# visual studio libs
win32:CONFIG(release, debug|release): LIBS += -L"$$PWD/../../clients/cpp/workspaces/VisualStudio/release/" -lClientLib
else:win32:CONFIG(debug, debug|release): LIBS += -L"$$PWD/../../clients/cpp/workspaces/VisualStudio/debug/" -lClientLib
win32:CONFIG(release, debug|release): LIBS += -L"$$PWD/../../workspaces/Visual Studio/release/" -lCommon
else:win32:CONFIG(debug, debug|release): LIBS += -L"$$PWD/../../workspaces/Visual Studio/debug/" -lCommon

INCLUDEPATH += $$PWD/../../clients/cpp/src $$PWD/../../common += $$PWD/../../src
DEPENDPATH += $$PWD/../../clients/cpp/src $$PWD/../../common += $$PWD/../../src

win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += "$$PWD/../../clients/cpp/workspaces/VisualStudio/release/ClientLib.lib"  "$$PWD/../../workspaces/Visual Studio/release/Common.lib"
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += "$$PWD/../../clients/cpp/workspaces/VisualStudio/debug/ClientLib.lib"  "$$PWD/../../workspaces/Visual Studio/debug/Common.lib"



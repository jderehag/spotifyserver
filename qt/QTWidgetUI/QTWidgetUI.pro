#-------------------------------------------------
#
# Project created by QtCreator 2014-04-29T10:23:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QTWidgetUI
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$PWD/../../../spotifyserver_deps/lib/libspotify-12.1.51-win32-release/include
DEPENDPATH += $$PWD/../../../spotifyserver_deps/lib/libspotify-12.1.51-win32-release/include

INCLUDEPATH += $$PWD/../../common += $$PWD/../../src
DEPENDPATH += $$PWD/../../common += $$PWD/../../src

SOURCES += \
    mainwindow.cpp \
    TrackListModel.cpp \
    AlbumEntry.cpp \
    GlobalActionInterface.cpp

HEADERS += \
    mainwindow.h \
    TrackListModel.h \
    AlbumEntry.h \
    GlobalActionInterface.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    mainwindow.ui \
    AlbumEntry.ui

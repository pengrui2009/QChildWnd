#-------------------------------------------------
#
# Project created by QtCreator 2023-03-14T22:35:24
#
#-------------------------------------------------

QT       += core gui
CONFIG   += C++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QChildWnd
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

INCLUDEPATH += $$PWD/camerawidget
INCLUDEPATH += $$PWD/videowidget
INCLUDEPATH += $$PWD/signalrssi

include ($$PWD/videowidget/videowidget.pri)
include ($$PWD/camerawidget/camerawidget.pri)
include ($$PWD/signalrssi/signalrssi.pri)

RESOURCES += \
    other/main.qrc \
    other/qss.qrc

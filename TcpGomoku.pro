#-------------------------------------------------
#
# Project created by QtCreator 2015-09-01T18:05:21
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TcpGomoku
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    gamedata.cpp \
    gamearea.cpp \
    setupserverdialog.cpp

HEADERS  += mainwindow.h \
    gamedata.h \
    gamearea.h \
    setupserverdialog.h

FORMS    += mainwindow.ui \
    gamearea.ui \
    setupserverdialog.ui

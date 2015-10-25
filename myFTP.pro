#-------------------------------------------------
#
# Project created by QtCreator 2011-10-26T11:44:11
#
#-------------------------------------------------

QT       += core gui sql xml
QT += network

TARGET = myFTP
TEMPLATE = app
#window下添加此宏，linux下去掉
DEFINES += Q_OS_WIN

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

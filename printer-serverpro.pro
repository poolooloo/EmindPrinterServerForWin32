#-------------------------------------------------
#
# Project created by QtCreator 2016-11-02T14:24:01
#
#-------------------------------------------------

QT       += core gui
QT += printsupport
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = printer-server
TEMPLATE = app


SOURCES += main.cpp\
        printerdialog.cpp \   
    winserver.cpp \
    printerserver.cpp \
    printerthread.cpp

HEADERS  += printerdialog.h \
    winserver.h \
    printerserver.h \
    printerthread.h

FORMS    += printerdialog.ui


win32: LIBS += -lwinspool


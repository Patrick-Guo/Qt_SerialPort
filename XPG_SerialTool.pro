#-------------------------------------------------
#
# Project created by QtCreator 2015-02-06T11:11:47
#
#-------------------------------------------------

QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = XPG_SerialTool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    doublewindow.cpp \
    xpgserialport.cpp \
    mcuwindow.cpp \
    xpganalyze.cpp \
    xpgtool.cpp \
    xpgframer.cpp \
    xpgjson.cpp \
    toolwindow.cpp \
    about.cpp \
    mcusettingwindow.cpp \
    mcucmdwindow.cpp \
    uploaddata.cpp

HEADERS  += mainwindow.h \
    doublewindow.h \
    xpgserialport.h \
    mcuwindow.h \
    xpganalyze.h \
    xpgtool.h \
    xpgframer.h \
    xpgjson.h \
    toolwindow.h \
    about.h \
    mcusettingwindow.h \
    mcucmdwindow.h \
    uploaddata.h

FORMS    += mainwindow.ui \
    doublewindow.ui \
    mcuwindow.ui \
    toolwindow.ui \
    about.ui \
    mcusettingwindow.ui \
    mcucmdwindow.ui

RC_FILE = image/ico.rc

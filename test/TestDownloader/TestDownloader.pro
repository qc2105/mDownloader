#-------------------------------------------------
#
# Project created by QtCreator 2015-06-04T16:23:01
#
#-------------------------------------------------

QT       += widgets network testlib

TARGET = tst_testdownloader
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_testdownloader.cc \
    hashworker.cpp \
    ../../downloader.cpp \
    ../../advio.cpp \
    ../../block.cpp \
    ../../debug.cpp \
    ../../ftp.cpp \
    ../../ftpparser.cpp \
    ../../ftpplugin.cpp \
    ../../header.cpp \
    ../../http.cpp \
    ../../httpplugin.cpp \
    ../../plugin.cpp \
    ../../progressbar.cpp \
    ../../proxy.cpp \
    ../../task.cpp \
    ../../url.cpp \
    ../../utils.cpp \
    ../../status.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    hashworker.h \
    ../../downloader.h \
    ../../advio.h \
    ../../block.h \
    ../../debug.h \
    ../../ftp.h \
    ../../ftpparser.h \
    ../../ftpplugin.h \
    ../../header.h \
    ../../http.h \
    ../../httpplugin.h \
    ../../macro.h \
    ../../myget.h \
    ../../plugin.h \
    ../../progressbar.h \
    ../../proxy.h \
    ../../task.h \
    ../../url.h \
    ../../utils.h \
    ../../status.h

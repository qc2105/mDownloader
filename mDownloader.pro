#-------------------------------------------------
#
# Project created by QtCreator 2014-01-26T14:49:20
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mDownloader
TEMPLATE = app


SOURCES += main.cpp\
    advio.cpp \
    block.cpp \
    debug.cpp \
    downloader.cpp \
    ftp.cpp \
    ftpparser.cpp \
    ftpplugin.cpp \
    header.cpp \
    http.cpp \
    httpplugin.cpp \
    plugin.cpp \
    progressbar.cpp \
    proxy.cpp \
    task.cpp \
    url.cpp \
    utils.cpp \
    status.cpp \
    ui/newtask.cpp \
    ui/mainwindow.cpp \
    ui/jobview.cc \
    ui/errormessagebox.cpp

HEADERS  +=   advio.h \
    block.h \
    debug.h \
    downloader.h \
    ftp.h \
    ftpparser.h \
    ftpplugin.h \
    header.h \
    http.h \
    httpplugin.h \
    macro.h \
    myget.h \
    plugin.h \
    progressbar.h \
    proxy.h \
    task.h \
    url.h \
    utils.h \
    status.h \
    ui/newtask.h \
    ui/mainwindow.h \
    ui/jobview.h \
    ui/errormessagebox.h

FORMS    += \
    ui/newtask.ui


RESOURCES += \
    mDownloader.qrc

TRANSLATIONS = mDownloader_zh_CN.ts

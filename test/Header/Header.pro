#-------------------------------------------------
#
# Project created by QtCreator 2015-06-27T10:03:35
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_headertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_headertest.cpp \
    ../../header.cpp \
    ../../utils.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../header.h \
    ../../utils.h

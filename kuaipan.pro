#-------------------------------------------------
#
# Project created by QtCreator 2013-06-01T18:31:59
#
#-------------------------------------------------

QT       += core gui network sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kuaipan
TEMPLATE = app


SOURCES += main.cpp\
        logindialog.cpp \
    kuaipanoauth.cpp \
    kuaipanhttp.cpp \
    accountdialog.cpp

HEADERS  += logindialog.h \
    kuaipanoauth.h \
    kuaipanhttp.h \
    accountdialog.h

FORMS    += logindialog.ui \
    accountdialog.ui

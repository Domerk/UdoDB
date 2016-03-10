#-------------------------------------------------
#
# Project created by QtCreator 2016-02-09T11:44:14
#
#-------------------------------------------------

QT       += core gui sql axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KcttDB
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    searchdialog.cpp \
    connectiondialog.cpp

HEADERS  += mainwindow.h \
    searchdialog.h \
    connectiondialog.h

FORMS    += mainwindow.ui \
    searchdialog.ui \
    connectiondialog.ui

QMAKE_CXXFLAGS += -std=gnu++0x

RESOURCES += \
    sources.qrc


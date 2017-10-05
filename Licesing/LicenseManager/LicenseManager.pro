#-------------------------------------------------
#
# Project created by QtCreator 2017-05-15T12:06:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LicenseManager
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mainwindowSQL.cpp \
    ../SQLite/sqlite3.c

HEADERS  += mainwindow.h \
    mainwindowSQL.h \
    ../SQLite/sqlite3.h

FORMS    += mainwindow.ui

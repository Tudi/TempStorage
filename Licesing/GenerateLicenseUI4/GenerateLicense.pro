#-------------------------------------------------
#
# Project created by QtCreator 2017-04-11T09:53:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GenerateLicense
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mainwindowSQL.cpp \
    mainwindowlic.cpp \
    ../SQLite/sqlite3.c

HEADERS  += mainwindow.h \
    mainwindowSQL.h \
    mainwindowlic.h \
    ../SQLite/sqlite3.h

FORMS    += mainwindow.ui

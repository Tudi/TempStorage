QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CurlInterface.cpp \
    SessionStore.cpp \
    dialoglogin.cpp \
    dialogmemberarea.cpp \
    frameprojectdetailslistview.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    CurlInterface.h \
    SessionStore.h \
    StdAfx.h \
    curl/curl.h \
    curl/curlver.h \
    curl/easy.h \
    curl/mprintf.h \
    curl/multi.h \
    curl/options.h \
    curl/stdcheaders.h \
    curl/system.h \
    curl/typecheck-gcc.h \
    curl/urlapi.h \
    dialoglogin.h \
    dialogmemberarea.h \
    frameprojectdetailslistview.h \
    mainwindow.h

FORMS += \
    dialoglogin.ui \
    dialogmemberarea.ui \
    frameprojectdetailslistview.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    curl/curl-ca-bundle.crt \
    curl/libcrypto-1_1-x64.dll \
    curl/libcurl-x64.def \
    curl/libcurl-x64.dll \
    curl/libcurl.a \
    curl/libcurl.dll.a \
    curl/libssl-1_1-x64.dll

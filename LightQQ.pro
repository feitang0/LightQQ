#-------------------------------------------------
#
# Project created by QtCreator 2014-12-22T23:20:43
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LightQQ
TEMPLATE = app


SOURCES += main.cpp\
        loginwindow.cpp \
    frienditem.cpp \
    mainpanel.cpp \
    SmartQQ/friend.cpp \
    SmartQQ/networkcookiejar.cpp \
    SmartQQ/smartqq.cpp \
    getface.cpp

HEADERS  += loginwindow.h \
    frienditem.h \
    mainpanel.h \
    SmartQQ/friend.h \
    SmartQQ/networkcookiejar.h \
    SmartQQ/smartqq.h \
    getface.h

FORMS    += loginwindow.ui \
    frienditem.ui \
    mainpanel.ui

RESOURCES += \
    face.qrc \
    icon.qrc

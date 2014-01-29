#-------------------------------------------------
#
# Project created by QtCreator 2013-10-29T14:54:04
#
#-------------------------------------------------

QT       += core gui testlib network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Metron_Motion_Generator
TEMPLATE = app

CONFIG += console


SOURCES += main.cpp\
        mainwindow.cpp \
    Motion_Controller.cpp \
    Script_Reader.cpp \
    Status_Thread.cpp \
    circular_thread.cpp \
    external_thread.cpp

HEADERS  += mainwindow.h \
    Motion_Controller.h \
    Script_Reader.h \
    Status_Thread.h \
    circular_thread.h \
    external_thread.h \
    QtSingleApplication

FORMS    += mainwindow.ui


QMAKE_LIBDIR += "C:/Users/Sunny/Documents/Visual Studio 2010/Projects/Metron_Motion_Generator/Metron_Motion_Generator" \


LIBS += gts.lib QtSolutions_SingleApplication-head.lib


TRANSLATIONS	= Metron_Motion_Generator_zh_CN.ts


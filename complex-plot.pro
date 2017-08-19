#-------------------------------------------------
#
# Project created by QtCreator 2017-08-11T11:54:39
#
#-------------------------------------------------

CONFIG += c++11 strict_c++

QT += core widgets

TARGET = complex-plot
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += $$PWD/src/

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/plotwidget.cpp \
    src/coloring.cpp \
    src/function.cpp

HEADERS += \
    src/mainwindow.h \
    src/plotdata.h \
    src/plotwidget.h \
    src/coloring.h \
    src/function.h

FORMS += \
    src/mainwindow.ui

DESTDIR = build
OBJECTS_DIR = build/.obj
MOC_DIR = build/.moc
RCC_DIR = build/.rcc
UI_DIR = build/.ui

#-------------------------------------------------
#
# Project created by QtCreator 2013-12-07T18:02:02
#
#-------------------------------------------------

QT += core gui
QT += network
QT += widgets
TARGET = audio
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    mythread.cpp

HEADERS  += dialog.h \
    global.h \
    mythread.h

FORMS    += dialog.ui
LIBS += -LD:WINMM1.LIB







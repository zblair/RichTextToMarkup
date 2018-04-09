#-------------------------------------------------
#
# Project created by QtCreator 2013-04-06T10:51:52
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RichTextToMarkup
TEMPLATE = app


SOURCES += main.cpp \
    converterDialog.cpp

HEADERS  += \
    converterDialog.h

FORMS    += \
    converterDialog.ui

RESOURCES += \
    resources.qrc

CXXFLAGS += -DQT_NO_TRANSLATION

#-------------------------------------------------
#
# Project created by QtCreator 2017-12-20T09:21:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VeriFace_Com
TEMPLATE = app

#add debug message
QMAKE_CXXFLAGS += -g

#remote path setting
target.path= /root
INSTALLS +=target

SOURCES += main.cpp\
        mainwindow.cpp \
    posix_qextserialport.cpp \
    qextserialbase.cpp \
    crc16.cpp \
    inputnum_widget.cpp \
    mylineedit.cpp \
    hardware.cpp

HEADERS  += mainwindow.h \
    posix_qextserialport.h \
    qextserialbase.h \
    crc16.h \
    inputnum_widget.h \
    mylineedit.h \
    hardware.h

FORMS    += mainwindow.ui \
    inputnum_widget.ui

RESOURCES += \
    image.qrc

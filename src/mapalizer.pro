#-------------------------------------------------
#
# Project created by QtCreator 2015-05-05T14:32:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mapalizer
TEMPLATE = app

CONFIG += c++11 warn_on
*-g++*|*-clang*: QMAKE_CXXFLAGS += -Wextra

win32:RC_FILE = mapalizer.rc

# debug output only in debug builds (see debug_cout.h)
CONFIG(debug, debug|release) {
    DEFINES += DEBUGCOUT
    DEFINES += DEBUGON
}

SOURCES += main.cpp \
    mainwindow.cpp \
    ConfFile.cpp \
    AppInterface.cpp \
    debug_cout.cpp \
    map_entry.cpp \
    map_loader.cpp \
    map_models.cpp \
    utils.cpp \
    st_map.cpp \
    iar_map.cpp

HEADERS  += mainwindow.h \
    ConfFile.h \
    AppInterface.h \
    debug_cout.h \
    map_entry.h \
    map_loader.h \
    map_models.h \
    utils.h \
    st_map.h \
    iar_map.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    mapalizer.rc

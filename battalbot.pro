# -------------------------------------------------
# Project created by QtCreator 2010-02-11T13:38:25
# -------------------------------------------------
QT += network \
    xml
TARGET = battalbot
TEMPLATE = app
LIBS += -lqwt
SOURCES += main.cpp \
    mainwindow.cpp \
    irc.cpp \
    reader.cpp \
    battle.cpp
HEADERS += mainwindow.h \
    irc.hpp \
    reader.hpp \
    battle.hpp
FORMS += mainwindow.ui
RESOURCES += images.qrc

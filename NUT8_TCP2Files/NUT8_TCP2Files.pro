TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
        tcpserver.cpp

HEADERS += \
    tcpserver.h

LIBS += -lws2_32
#LIBS += -lpthread
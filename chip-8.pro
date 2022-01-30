TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt #will add Qt later :)

#LIBS += -lsfml-graphics
#LIBS += -lsfml-window
#LIBS += -lsfml-system

#LIBS += -lSDL2

#chip-8 emyulator

SOURCES += main.cpp \
    chip8.cpp

HEADERS += \
    chip8.h

DISTFILES += \
    README.md \
    assets/opcodechip8.png \
    assets/test_opcode.ch8


TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt #will add Qt later :)

#LIBS += -lsfml-graphics
#LIBS += -lsfml-window
#LIBS += -lsfml-system

LIBS += -lSDL2

#chip-8 emyulator

SOURCES += main.cpp \
    chip8.cpp

HEADERS += \
    chip8.h

DISTFILES += \
    README.md \
    assets/Screenshot_2022-01-31_08-58-12.png \
    assets/consolememgfxdmp.png \
    assets/dump.txt \
    assets/opcodechip8.png \
    assets/pixels1pass.png \
    assets/test_opcode.ch8


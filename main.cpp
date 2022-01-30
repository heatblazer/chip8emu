#include "chip8.h"
// anisc
#include <stdio.h>

#define SCREENW 640
#define SCREENH 480

int main(int argc, char** argv) {
    chip8 test;
    test.init();
    //hardocded test - will do later
    if (test.loadApplication("/home/ilian/gitprojects/chip8emu/assets/test_opcode.ch8")) {
        //ivz test.emulatetest(); //after loading - test opcodes
        test.romtest();
    } else {
        printf("Failed to load rom\r\n");
    }
    return 0;
}

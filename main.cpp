#include "chip8.h"
// anisc
#include <stdio.h>

#define SCREENW 640
#define SCREENH 480

int main(int argc, char** argv) {
    chip8 test;
    test.init();
//    test.emulatetest();
    //hardocded test - will do later
#if 1

// like a text draw
    test.blinkred();
    test.dumpgfx();
// end test draw


    if (test.loadApplication("/home/ilian/gitprojects/chip8emu/assets/test_opcode.ch8")) {
        //test.romtest(); //after loading - test opcodes
        test.start();
    } else {
        printf("Failed to load rom\r\n");
    }
#else
    test.emulatetest();
#endif
    return 0;
}

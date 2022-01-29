#include "chip8.h"
// anisc
#include <stdio.h>

#define SCREENW 640
#define SCREENH 480

int main(int argc, char** argv) {
    chip8 test;
    test.init();
    test.emulatetest();

    return 0;
}

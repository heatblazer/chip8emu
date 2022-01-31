#include "chip8.h"
// anisc
#include <stdio.h>
#include <SDL2/SDL.h>
#define SCREENW 640
#define SCREENH 480


void DrawChip8(chip8* pChip);



int main(int argc, char** argv) {
    chip8 test;
    test.init();
//    test.emulatetest();
    //hardocded test - will do later
#if 1

// like a text draw
    puts("Test graphics");
/*
    test.blinkred();
    test.testPutXY(1,1, 0);
    test.testPutXY(2,2, 0);
    test.testPutXY(10,10, 0);
    test.testPutXY(20,20, 0);
    test.testPutXY(60,30, 0);
    test.dumpgfx();
*/
// end test draw

    puts("Test rom loading");
    if (test.loadApplication("/home/ilian/gitprojects/chip8emu/assets/test_opcode.ch8")) {
        //test.romtest(); //after loading - test opcodes
        puts("Test emulation");
//        test.start();
        DrawChip8(&test);
    } else {
        printf("Failed to load rom\r\n");
    }
#else
    test.emulatetest();
#endif
    return 0;
}


void DrawChip8(chip8 *pChip)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Test", 0,0,640,480, 0);
    if (window == NULL)
    {
        return ;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (renderer == NULL)
    {
        return ;
    }

    /* Create texture for display */
    SDL_Texture *display = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 64, 32);

    while(1)
    {
        uint16_t op = pChip->fetch();
        pChip->decode(op);
        // update screen

        SDL_UpdateTexture(display, nullptr, pChip->m_mem.gfx, chip8::Width);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, display, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }


    SDL_Quit();
    return;
}

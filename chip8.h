#ifndef CHIP8
#define CHIP8
#include <stdint.h>

#define SCREEN_SIZE 64*32
#define MAX_KEYS 16

class chip8
{
public:
    chip8();
    ~chip8();

    bool    m_drawFlag;

    void init(void);
    void emulateCycle(void);
    void debugRender(void);
    bool loadApplication(const char* fname);
    void start();


#if 1 //dbg
    void emulatetest();

    void romtest();
#endif

private:

    uint16_t fetch();

    void decode(uint16_t op); // will redo it later...if needed to return some data

    void decode8nX(uint16_t);

    uint16_t pc;         // program counter
    uint16_t opcode;     // opcode
    union {
        uint16_t value;
        uint8_t data[sizeof(uint16_t)];
    } I; // Index register

    uint8_t sp;         // stack pointer
    uint16_t stack[16];

    struct {
        uint8_t keys[MAX_KEYS]; //has 16 keys A-F + 0-9
        uint8_t V[16];      // V-regs V0 - VF
        uint8_t memory[4096]; // max memory 4K
        uint8_t gfx[SCREEN_SIZE];
    } m_mem;

    struct {
        uint8_t delay;  // delay timer
        uint8_t sound;  // sound timer
    } m_timers; //TBIMPL

};


#endif // CHIP8


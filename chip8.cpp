#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const unsigned int START_ADDRESS = 0x200; //as from papers

static const uint8_t chip8_fontset[] = //as from papers
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
}; // the color of fonts that are shown as sprites kind of



#define OPCODE8nN(X) ( ((X) & 0xF000) >> 8u) | ((X) & 0x000F)


chip8::chip8() : pc{0x00}, opcode{0}, sp{0}
{

}

chip8::~chip8()
{

}

//well, let's init all step by step .. will redo it
void chip8::init()
{
    pc = START_ADDRESS;
    memset(&m_mem, 0, sizeof(m_mem));
    memset(&m_timers, 0, sizeof(m_timers));
    for(size_t i=0x50; i < 0x9F; ++i) { //suggested place to put fonts
        // first 80 cells in memory are for fontset
        m_mem.memory[i] = chip8_fontset[i];
    }

    m_drawFlag = true;
    // seed the random number
    srand(time(NULL)); //will do Nasir way later with some lookup fixed rnd :)
}

//this will be official
void chip8::emulateCycle()
{
    // fetch the opcode
    opcode = fetch();
    decode(opcode);
}

//the rule of loop-fetch-decode-exec-update
void chip8::start()
{
    //loop
        //fetch
        //decode
        //execute
    // update
}

//the unit test for various opcodes, will be removed later
void chip8::emulatetest()
{
    // fetch the opcode
    opcode = fetch();
    //ok let's try to add
    decode(0x7542); //add (7 op) in 5 reg - value 0x42 - test
    decode(0x7610); // add in 6 reg - 0x10

    //decode(0x00E0); //will clear screen... duh, they are 2 types of ops...

    decode(0x2FF1); //call FF1

    decode(0x00EE); //return from FF1

#if 0 // dbf
    //will do some test helper macro for future calling corsponding opcode
    auto x0 = OPCODE8nN(0x8FF0);
    auto x1 = OPCODE8nN(0x8FF1);
    auto x2 = OPCODE8nN(0x8FF2);
    auto x3 = OPCODE8nN(0x8FF3);
    auto x4 = OPCODE8nN(0x8FF4);
    auto x5 = OPCODE8nN(0x8FF5);
    auto x6 = OPCODE8nN(0x8FF6);
    auto x7 = OPCODE8nN(0x8FF7);
    auto x8 = OPCODE8nN(0x8FF8);
    auto x9 = OPCODE8nN(0x8FF9);
    auto xA = OPCODE8nN(0x8FFA);
    auto xB = OPCODE8nN(0x8FFB);
    auto xC = OPCODE8nN(0x8FFC);
    auto xD = OPCODE8nN(0x8FFD);
    auto xE = OPCODE8nN(0x8FFE);
#endif
    // tst
    decode(0x61AF); // store in 1
    decode(0x620A); // store in 1
    decode(0x8124); // add 1 and 2
    printf("test print: ....\r\n");
    // load in 3 and 4 w/carry
    decode(0x63FF); // store in 1
    decode(0x641A); // store in 1
    decode(0x8344); // add 1 and 2


    return;

}

//nothing to see here
uint16_t chip8::fetch()
{
    uint16_t op = m_mem.memory[pc] << 8 | m_mem.memory[pc+1];
    pc += 2; //always increment PC by 2
    return op;
}

//decode all stuff
void chip8::decode(uint16_t op)
{
    //for clear screen and return from call
    uint8_t op0 = (op & 0x00FF);
    switch (op0) {
    case 0x00E0: {
        printf("Clear scrn\r\n");
        //cleaer screen
        memset(&m_mem.gfx, 0, sizeof(m_mem.gfx));
        break;
    }
    case 0x00EE: {
        printf("return from sub\r\n");
        pc = stack[--sp]; //pop the stack
        break;
    }
    default:
        break;
    }

    uint8_t code = (op >> 12u) & 0xF;
    switch (code) {
    case 0x1: {
        //jump to NNN address
        pc = (op & 0x0FFFu); // override the +2??/
        break;
    }
    case 0x2: {
        // call func - use sp and PC...
        uint16_t addr = (op & 0x0FFFu);
        stack[++sp] = pc; // sooo ... put the program counter to the stack
                          // kind of restore point??? TB(Researched)
        pc = addr; // then set the new value to the address we are calling...
        break;
    }
    case 0x6: {
        //assing
//        6XNN 	Const 	Vx = N 	Sets VX to NN.
        uint8_t reg = (op & 0x0F00) >> 8;
        uint8_t data = (op & 0x00FF);
        m_mem.V[reg] = data;
        break;
    }
    case 0x7: {
        printf("ADD\r\n");
        uint8_t reg = (op & 0x0F00) >> 8;
        uint8_t data = (op & 0x00FF);
        m_mem.V[reg] += data;
        break;
    }
    case 0x8: // can't do it as is - will use helper and call the coresponding opcode
    {         //since last 4 bits tells the call
        decode8nX(op); //helper
        break;
    }
    default:
        break;
    }
}

void chip8::decode8nX(uint16_t opc8)
{
    uint8_t op8 = OPCODE8nN(opc8);
    uint8_t regX=0, regY=0;
    if (op8 < 128 || op8 > 142)
        return;
    regX = (opc8 & 0x0F00u) >> 8u;
    regY = (opc8 & 0x000F0u) >> 4u;

    switch (op8) {
    case 128: { //wiki: 8XY0 	Assig 	Vx = Vy 	Sets VX to the value of VY.
        m_mem.V[regX] = m_mem.V[regY];
        break;
    }
    case 129: { //8XY1 	BitOp 	Vx |= Vy 	Sets VX to VX or VY. (Bitwise OR operation);
        m_mem.V[regX] |= m_mem.V[regY];
        break;
    }
    case 130: { //8XY2 	BitOp 	Vx &= Vy 	Sets VX to VX and VY. (Bitwise AND operation);
        m_mem.V[regX] &= m_mem.V[regY];
        break;
    }
    case 131: { //8XY3[a] 	BitOp 	Vx ^= Vy 	Sets VX to VX xor VY.
        m_mem.V[regX] ^= m_mem.V[regY];
        break;
    }
    case 132: { //8XY4 	Math 	Vx += Vy 	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there is not.
        uint16_t summ = m_mem.V[regX] + m_mem.V[regY];
        if (summ > 0xFFu) {
            m_mem.V[0xF] = 1;//carry
        } else {
            m_mem.V[0xF] = 0;
        }
        m_mem.V[regX] = summ & 0xFFu;
        break;
    }
    case 133: { //8XY5 	Math 	Vx -= Vy 	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there is not.
        if (m_mem.V[regX] > m_mem.V[regY]) {
            m_mem.V[0xF] = 1;
        } else {
            m_mem.V[0xF] = 0;
        }
        m_mem.V[regX] -= m_mem.V[regY];
        break;
    }
    case 134: { //8XY6[a] 	BitOp 	Vx >>= 1 	Stores the least significant bit of VX in VF and then shifts VX to the right by 1.[b]
        m_mem.V[0xF] = m_mem.V[regX] & 0x01;
        m_mem.V[regX] >>= 1;
        break;
    }
    case 135: {

        break;
    }
    default:
        break;
    }
}



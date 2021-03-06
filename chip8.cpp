#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <chrono>
#include <thread>

#define OP_8XY0 0x80
#define OP_8XY1 0x81
#define OP_8XY2 0x82
#define OP_8XY3 0x83
#define OP_8XY4 0x84
#define OP_8XY5 0x85
#define OP_8XY6 0x86
#define OP_8XY7 0x87
/* well - not present in the spec...
#define OP_8XY8 0x88
#define OP_8XY9 0x89
#define OP_8XYA 0x8A
#define OP_8XYB 0x8B
#define OP_8XYC 0x8C
#define OP_8XYD 0x8D
*/
#define OP_8XYE 0x8E

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

chip8::chip8() : m_loadedFileSz{0}, pc{0x00}, opcode{0}, sp{0}
{

}

chip8::~chip8()
{

}

//well, let's init all step by step .. will redo it
void chip8::init()
{
    Running = true;
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

bool chip8::loadApplication(const char *fname)
{
    uint8_t* pBegin = &m_mem.memory[START_ADDRESS+1];
    FILE* fp = fopen(fname, "rb");
    if (!fp)
        return false;
    fseek(fp, 0, SEEK_END);
    size_t size = m_loadedFileSz = ftell(fp);
    rewind(fp);
    fread(pBegin,size, sizeof(uint8_t), fp);
    fclose(fp);
    return true;
}

//the rule of loop-fetch-decode-exec-update
void chip8::start()
{
    //loop
        //fetch
        //decode
        //execute

        //<ext> update

    while (Running.load()) {
        opcode = fetch();
        decode(opcode);
        std::this_thread::sleep_for(std::chrono::nanoseconds(10)); // sleep for 1 second
    }
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

    // tst
    decode(0x61AF); // store in 1
    decode(0x620A); // store in 2
    decode(0x8124); // add 1 and 2
    printf("test print: ....\r\n");
    // load in 3 and 4 w/carry
    decode(0x63FF); // store in 3
    decode(0x641A); // store in 4
    decode(0x8344); // add 3 and 4

    decode(0x65FA); // store 15 in 5th
    decode(0x85F8); // test msbit

    decode(0x9430); // skip next if X ne Y

    decode(0xAA1F); // load in I the 01F for example

    decode(0xBAA0);

    return;

}


void chip8::dumpgfx()
{
    for(int x=0; x < chip8::Width; x++) {
        for (int y=0; y < chip8::Height; y++) {
            printf("[%02x]", m_mem.gfx[x * chip8::Height + y]);
        }
        puts("....................................................................................");
    }

}

void chip8::blinkred()
{
    for(int x=0; x < chip8::Width; x++) {
        for (int y=0; y < chip8::Height; y++) {
            m_mem.gfx[x * chip8::Height + y] = 0xFA;
        }

    }
}

void chip8::testPutXY(uint8_t x, uint8_t y, uint8_t color)
{
    m_mem.gfx[x * chip8::Height + y] = color;
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
        //cleaer screen
        puts("00E0");
        memset(&m_mem.gfx, 0, sizeof(m_mem.gfx));
        break;
    }
    case 0x00EE: {
        puts("00EE");
        pc = stack[--sp]; //pop the stack
        break;
    }
    default:
        break;
    }

    uint8_t code = (op >> 12u) & 0xF;
    switch (code) {
    case 0x1: {
        puts("1NNN");
        //jump to NNN address
        pc = (op & 0x0FFFu); // override the +2??/
        break;
    }
    case 0x2: {
        puts("2NNN");
        // call func - use sp and PC...
        uint16_t addr = (op & 0x0FFFu);
        stack[++sp] = pc; // sooo ... put the program counter to the stack
                          // kind of restore point??? TB(Researched)
        pc = addr; // then set the new value to the address we are calling...
        break;
    }
    //if x != y ...
    case 0x3: {//3XNN 	Cond 	if (Vx == NN) 	Skips the next instruction if VX equals NN. (
                //Usually the next instruction is a jump to skip a code block);
        puts("3XNN");
        uint8_t regX = (op & 0x0F00u) >> 8u;
        uint8_t NN = (op & 0x00FF);
        if (m_mem.V[regX] == NN)
            pc +=2; //skip
        break;
    }
    case 0x4: {//4XNN 	Cond 	if (Vx != NN) 	Skips the next instruction if VX does not equal NN.
        //(Usually the next instruction is a jump to skip a code block);
        puts("4XNN");
        uint8_t regX = (op & 0x0F00u) >> 8u;
        uint8_t NN = (op & 0x00FF);
        if (m_mem.V[regX] != NN)
            pc +=2; //skip
        break;
    }
    case 0x5: {//5XY0 	Cond 	if (Vx == Vy) 	Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block);
        puts("5XY0");
        uint8_t regX = (op & 0x0F00u) >> 8u;
        uint8_t regY = (op & 0x00F0) >> 4u;
        if (m_mem.V[regX] == m_mem.V[regY])
            pc +=2; //skip
        break;
    }
    case 0x6: {
        puts("6XNN");
        //assing
//        6XNN 	Const 	Vx = N 	Sets VX to NN.
        uint8_t reg = (op & 0x0F00) >> 8;
        uint8_t data = (op & 0x00FF);
        m_mem.V[reg] = data;
        break;
    }
    case 0x7: {
        puts("7XNN");
        uint8_t reg = (op & 0x0F00) >> 8;
        uint8_t data = (op & 0x00FF);
        m_mem.V[reg] += data;
        break;
    }
    case 0x8: {// can't do it as is - will use helper and call the coresponding opcode
             //since last 4 bits tells the call
        decode8nX(op); //helper
        break;
    }
    case 0x9: {//9XY0 	Cond 	if (Vx != Vy) 	Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block);
        puts("9XY0");
        uint8_t regX = (op & 0x0F00) >> 8u;
        uint8_t regY = (op & 0x00F0) >> 4u;
        if (m_mem.V[regX] != m_mem.V[regY]) {
            pc += 2; //skip next, just inc the PC (always by 2)
        }
        break;
    }
    case 0xA: {//ANNN 	MEM 	I = NNN 	Sets I to the address NNN.
        puts("ANNN");
        I.value = (op  & 0x0FFF);
        break;
    }
    case 0xB: {//BNNN 	Flow 	PC = V0 + NNN 	Jumps to the address NNN plus V0.
        puts("BNNN");
        pc = m_mem.V[0] + (op & 0x0FFF);
        break;
    }
    case 0xC: {//CXNN 	Rand 	Vx = rand() & NN 	Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
        puts("CXNN");
        uint8_t regX = (op & 0x0F00u) >> 8u;
        m_mem.V[regX] = rand() & (0x00FF & op);
        break;
    }
    case 0xD: {
        puts("DXYN");
        //DXYN 	Display 	draw(Vx, Vy, N) 	Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
        //Each row of 8 pixels is read as bit-coded starting from memory location I;
        //I value does not change after the execution of this instruction.
        //As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
        //and to 0 if that does not happen
        uint8_t regX = (op & 0x0F00u) >> 8u;
        uint8_t regY  = (op & 0x00F0u) >> 4u;
        uint8_t heightN = (op & 0x000Fu);
        int xpos = m_mem.V[regX] % chip8::Width;
        int ypos = m_mem.V[regY] % chip8::Height;
        //we will read I
        for(uint8_t i = 0; i < heightN; i++) {
            uint8_t sprite = m_mem.memory[I.value + i];            
            for(uint8_t j=0; j < 8; j++) {
                uint8_t pixel = sprite & (0x80u >> i);
#if 1 // mmm not ok...
                uint32_t* vmemptr = (uint32_t*)&m_mem.gfx[(ypos + i) * chip8::Width + (xpos + j)];
                // set pixel at screen ...
                if (pixel) {
                    if (*vmemptr == 0xFFFFFFFFu)
                        m_mem.V[0xF] = 1;
                    *vmemptr ^= ~(0);
                }
#endif
            }
        }
        break;
    }
    case 0x000:
    default:
        break;
    }
}

void chip8::decode8nX(uint16_t opc8)
{
    uint8_t op8 = OPCODE8nN(opc8);
    uint8_t regX=0, regY=0;
    if (op8 < OP_8XY0 || op8 > OP_8XYE)
        return;
    regX = (opc8 & 0x0F00u) >> 8u;
    regY = (opc8 & 0x000F0u) >> 4u;

    switch (op8) {
    case OP_8XY0: { //wiki: 8XY0 	Assig 	Vx = Vy 	Sets VX to the value of VY.
        puts("8XY0");
        m_mem.V[regX] = m_mem.V[regY];
        break;
    }
    case OP_8XY1: { //8XY1 	BitOp 	Vx |= Vy 	Sets VX to VX or VY. (Bitwise OR operation);
        puts("8XY1");
        m_mem.V[regX] |= m_mem.V[regY];
        break;
    }
    case OP_8XY2: { //8XY2 	BitOp 	Vx &= Vy 	Sets VX to VX and VY. (Bitwise AND operation);
        puts("8XY2");
        m_mem.V[regX] &= m_mem.V[regY];
        break;
    }
    case OP_8XY3: { //8XY3[a] 	BitOp 	Vx ^= Vy 	Sets VX to VX xor VY.
        puts("8XY3");
        m_mem.V[regX] ^= m_mem.V[regY];
        break;
    }
    case OP_8XY4: { //8XY4 	Math 	Vx += Vy 	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there is not.
        puts("8XY4");
        uint16_t summ = m_mem.V[regX] + m_mem.V[regY];
        if (summ > 0xFFu) {
            m_mem.V[0xF] = 1;//carry
        } else {
            m_mem.V[0xF] = 0;
        }
        m_mem.V[regX] = summ & 0xFFu;
        break;
    }
    case OP_8XY5: { //8XY5 	Math 	Vx -= Vy 	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there is not.
        puts("8XY5");
        if (m_mem.V[regX] > m_mem.V[regY]) {
            m_mem.V[0xF] = 1;
        } else {
            m_mem.V[0xF] = 0;
        }
        m_mem.V[regX] -= m_mem.V[regY];
        break;
    }
    case OP_8XY6: { //8XY6[a] 	BitOp 	Vx >>= 1 	Stores the least significant bit of VX in VF and then shifts VX to the right by 1.[b]
        puts("8XY6");
        m_mem.V[0xF] = m_mem.V[regX] & 0x01;
        m_mem.V[regX] >>= 1;
        break;
    }
    case OP_8XY7: { //8XY7[a] 	Math 	Vx = Vy - Vx 	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there is not.
        puts("8XY7");
        if (m_mem.V[regY] < m_mem.V[regX]) {
            m_mem.V[0xF] = 1;
        } else {
            m_mem.V[0xF] = 0;
        }
        m_mem.V[regX] = m_mem.V[regY] - m_mem.V[regX];
        break;
    }
    case OP_8XYE: {// 8XYE[a] 	BitOp 	Vx <<= 1 	Stores the most significant bit of VX in VF and then shifts VX to the left by 1.[b]
        puts("8XYE");
        m_mem.V[0xF] = (m_mem.V[regX] & 0x80u) >> 7u;
        m_mem.V[regX] <<= 1;
        break;
    }
    default:
        break;
    }
}



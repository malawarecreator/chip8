#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define MEMORY_SIZE 4096
#define REGISTER_COUNT 16
#define STACK_SIZE 16
#define GFX_WIDTH 64
#define GFX_HEIGHT 32
#define SCALE 10
#define WINDOW_WIDTH (GFX_WIDTH * SCALE)
#define WINDOW_HEIGHT (GFX_HEIGHT * SCALE)

uint8_t memory[MEMORY_SIZE];
uint8_t V[REGISTER_COUNT];
uint16_t I;
uint16_t PC;
uint8_t delayTimer;
uint8_t soundTimer;
uint16_t stack[STACK_SIZE];
uint8_t SP;
uint8_t gfx[GFX_WIDTH * GFX_HEIGHT];
uint8_t key[REGISTER_COUNT];

SDL_Window* window;
SDL_Renderer* renderer;

uint8_t chip8_fontset[80] = {
    0xF0,0x90,0x90,0x90,0xF0,
    0x20,0x60,0x20,0x20,0x70,
    0xF0,0x10,0xF0,0x80,0xF0,
    0xF0,0x10,0xF0,0x10,0xF0,
    0x90,0x90,0xF0,0x10,0x10,
    0xF0,0x80,0xF0,0x10,0xF0,
    0xF0,0x80,0xF0,0x90,0xF0,
    0xF0,0x10,0x20,0x40,0x40,
    0xF0,0x90,0xF0,0x90,0xF0,
    0xF0,0x90,0xF0,0x10,0xF0
};

void initialize() {
    PC = 0x200;
    I = 0;
    SP = 0;
    for (int i = 0; i < REGISTER_COUNT; ++i) V[i] = 0;
    for (int i = 0; i < MEMORY_SIZE; ++i) memory[i] = 0;
    for (int i = 0; i < STACK_SIZE; ++i) stack[i] = 0;
    for (int i = 0; i < GFX_WIDTH * GFX_HEIGHT; ++i) gfx[i] = 0;
    for (int i = 0; i < REGISTER_COUNT; ++i) key[i] = 0;
    delayTimer = 0;
    soundTimer = 0;
    for (int i = 0; i < 80; ++i) memory[i] = chip8_fontset[i];

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return;
    window = SDL_CreateWindow("CHIP-8",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

void drawGraphics() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = 0; y < GFX_HEIGHT; ++y) {
        for (int x = 0; x < GFX_WIDTH; ++x) {
            if (gfx[x + y * GFX_WIDTH]) {
                SDL_Rect rect = { x * SCALE, y * SCALE, SCALE, SCALE };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

int mapKey(SDL_Keycode kc) {
    switch (kc) {
        case SDLK_x: return 0x0;
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_q: return 0x4;
        case SDLK_w: return 0x5;
        case SDLK_e: return 0x6;
        case SDLK_a: return 0x7;
        case SDLK_s: return 0x8;
        case SDLK_d: return 0x9;
        case SDLK_z: return 0xA;
        case SDLK_c: return 0xB;
        case SDLK_4: return 0xC;
        case SDLK_r: return 0xD;
        case SDLK_f: return 0xE;
        case SDLK_v: return 0xF;
    }
    return -1;
}

int main(int argc, char** argv) {
    if (argc < 2) return 1;
    FILE* rom = fopen(argv[1], "rb");
    if (!rom) return 1;
    initialize();
    fread(&memory[0x200], 1, MEMORY_SIZE - 0x200, rom);
    fclose(rom);
    int drawFlag = 0;
    int running = 1;
    SDL_Event e;

    while (running) {
        uint16_t opcode = memory[PC] << 8 | memory[PC + 1];
        switch (opcode & 0xF000) {
            case 0x0000:
                switch (opcode & 0x00FF) {
                    case 0x00E0:
                        for (int i = 0; i < GFX_WIDTH * GFX_HEIGHT; ++i) gfx[i] = 0;
                        drawFlag = 1;
                        PC += 2;
                        break;
                    case 0x00EE:
                        PC = stack[--SP] + 2;
                        break;
                }
                break;
            case 0x1000:
                PC = opcode & 0x0FFF;
                break;
            case 0x2000:
                stack[SP++] = PC;
                PC = opcode & 0x0FFF;
                break;
            case 0x3000:
                if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) PC += 4;
                else PC += 2;
                break;
            case 0x4000:
                if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) PC += 4;
                else PC += 2;
                break;
            case 0x5000:
                if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) PC += 4;
                else PC += 2;
                break;
            case 0x6000:
                V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
                PC += 2;
                break;
            case 0x7000:
                V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
                PC += 2;
                break;
            case 0x8000:
                switch (opcode & 0x000F) {
                    case 0x0: V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4]; break;
                    case 0x1: V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4]; break;
                    case 0x2: V[(opcode & 0x0F00)>>8] &= V[(opcode & 0x00F0)>>4]; break;
                    case 0x3: V[(opcode & 0x0F00)>>8] ^= V[(opcode & 0x00F0)>>4]; break;
                    case 0x4: {uint16_t sum = V[(opcode & 0x0F00)>>8] + V[(opcode & 0x00F0)>>4]; V[0xF] = sum > 255; V[(opcode & 0x0F00)>>8] = sum & 0xFF;} break;
                    case 0x5: V[0xF] = V[(opcode & 0x0F00)>>8] > V[(opcode & 0x00F0)>>4]; V[(opcode & 0x0F00)>>8] -= V[(opcode & 0x00F0)>>4]; break;
                    case 0x6: V[0xF] = V[(opcode & 0x0F00)>>8] & 1; V[(opcode & 0x0F00)>>8] >>= 1; break;
                    case 0x7: V[0xF] = V[(opcode & 0x00F0)>>4] > V[(opcode & 0x0F00)>>8]; V[(opcode & 0x0F00)>>8] = V[(opcode & 0x00F0)>>4] - V[(opcode & 0x0F00)>>8]; break;
                    case 0xE: V[0xF] = V[(opcode & 0x0F00)>>8] >> 7; V[(opcode & 0x0F00)>>8] <<= 1; break;
                }
                PC += 2;
                break;
            case 0x9000:
                if (V[(opcode & 0x0F00)>>8] != V[(opcode & 0x00F0)>>4]) PC += 4;
                else PC += 2;
                break;
            case 0xA000:
                I = opcode & 0x0FFF;
                PC += 2;
                break;
            case 0xB000:
                PC = (opcode & 0x0FFF) + V[0];
                break;
            case 0xC000:
                V[(opcode & 0x0F00)>>8] = (rand()%256) & (opcode & 0x00FF);
                PC += 2;
                break;
            case 0xD000:
                {uint8_t x = V[(opcode & 0x0F00)>>8] % GFX_WIDTH;
                uint8_t y = V[(opcode & 0x00F0)>>4] % GFX_HEIGHT;
                uint8_t h = opcode & 0x000F;
                V[0xF] = 0;
                for (int row=0; row<h; ++row) {
                    uint8_t spr = memory[I+row];
                    for (int col=0; col<8; ++col) {
                        if (spr & (0x80 >> col)) {
                            int idx = x+col + (y+row)*GFX_WIDTH;
                            if (gfx[idx]) V[0xF] = 1;
                            gfx[idx] ^= 1;
                        }
                    }
                }
                drawFlag = 1;
                PC += 2;}
                break;
            case 0xE000:
                if ((opcode & 0x00FF)==0x9E) { if(key[V[(opcode&0x0F00)>>8]]) PC+=4; else PC+=2; }
                else if ((opcode & 0x00FF)==0xA1) { if(!key[V[(opcode&0x0F00)>>8]]) PC+=4; else PC+=2; }
                break;
            case 0xF000:
                switch(opcode&0x00FF) {
                    case 0x07: V[(opcode&0x0F00)>>8]=delayTimer; PC+=2; break;
                    case 0x0A: { int kp=-1; while(kp<0) { while(SDL_PollEvent(&e)) if(e.type==SDL_KEYDOWN) kp=mapKey(e.key.keysym.sym); } V[(opcode&0x0F00)>>8]=kp; PC+=2;} break;
                    case 0x15: delayTimer=V[(opcode&0x0F00)>>8]; PC+=2; break;
                    case 0x18: soundTimer=V[(opcode&0x0F00)>>8]; PC+=2; break;
                    case 0x1E: I+=V[(opcode&0x0F00)>>8]; PC+=2; break;
                    case 0x29: I=V[(opcode&0x0F00)>>8]*5; PC+=2; break;
                    case 0x33: memory[I]=V[(opcode&0x0F00)>>8]/100; memory[I+1]=(V[(opcode&0x0F00)>>8]/10)%10; memory[I+2]=V[(opcode&0x0F00)>>8]%10; PC+=2; break;
                    case 0x55: for(int i=0;i<=((opcode&0x0F00)>>8);++i) memory[I+i]=V[i]; PC+=2; break;
                    case 0x65: for(int i=0;i<=((opcode&0x0F00)>>8);++i) V[i]=memory[I+i]; PC+=2; break;
                }
                break;
        }
        while(SDL_PollEvent(&e)) {
            if(e.type==SDL_QUIT) running=0;
            if(e.type==SDL_KEYDOWN){int k=mapKey(e.key.keysym.sym); if(k>=0) key[k]=1;}
            if(e.type==SDL_KEYUP){int k=mapKey(e.key.keysym.sym); if(k>=0) key[k]=0;}
        }
        if(drawFlag){ drawGraphics(); drawFlag=0; }
        if(delayTimer>0) --delayTimer;
        if(soundTimer>0) --soundTimer;
        SDL_Delay(1);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

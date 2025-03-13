// latest release by benjamin liu, Thursday 3/13/25 

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t memory[4096] = {0};

uint8_t V[16] = {0};
uint16_t I = 0;
uint16_t PC = 0x200;
uint8_t SP = 0;
uint16_t stack[16];
uint8_t VF = 0;
uint8_t delay = 0;
uint8_t sound = 0;

int main(int argc, char** argv) {
    for (int i = 0; i < 16; i++) {
        V[i] = 0;
    }

    char* filename = argv[1];
    FILE *rom = fopen(filename , "rb");
    fread(&memory[0x200], 1, 3584, rom);
    fclose(rom);

    while (1) {
        uint16_t opcode = (memory[PC] << 8) | memory[PC + 1];
        printf("PC: 0x%04X, Opcode: 0x%04X\n", PC, opcode);
        if (opcode == 0x0000) {
            printf("Program halted (0x0000 encountered).\n");
            break;
        }
        if ((opcode & 0xF000) == 0x1000) {
            uint16_t addr = opcode & 0x0FFF;
            PC = addr;
            printf("Jump to address 0x%04X\n", PC);
        }
        if ((opcode & 0xF000) == 0x2000) {
            uint16_t addr = opcode & 0xFFF;
            SP++;
            stack[SP] = PC + 2;
            PC = addr;
            printf("Call subroutine at 0x%04X\n", PC);
        }
        if (opcode == 0x00EE) {
            PC = stack[SP];
            SP--;
            printf("Return from subroutine to 0x%04X\n", PC);
        }
        if ((opcode & 0xF000) == 0x3000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            if (V[X] == NN) {
                PC += 4;
                printf("Skip next instruction (V[%d] == 0x%02X)\n", X, NN);
            } else {
                PC += 2;
            }
        }
        if ((opcode & 0xF000) == 0x4000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            if (V[X] != NN) {
                PC += 4;
                printf("Skip next instruction (V[%d] != 0x%02X)\n", X, NN);
            } else {
                PC += 2;
            }
        }
        if ((opcode & 0xF000) == 0x5000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            if (V[X] == V[Y]) {
                PC += 4;
                printf("Skip next instruction (V[%d] == V[%d])\n", X, Y);
            } else {
                PC += 2;
            }
        }
        if ((opcode & 0xF000) == 0x6000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            V[X] = NN;
            PC += 2;
            printf("LD V[%d], 0x%02X\n", X, NN);
        } else if ((opcode & 0xF000) == 0x7000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            V[X] += NN;
            PC += 2;
            printf("ADD V[%d], 0x%02X\n", X, NN);
        } else if ((opcode & 0xF000) == 0x8000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;

            switch (opcode & 0x000F) {
                case 0x0:
                    V[X] = V[Y];
                    break;
                case 0x1:
                    V[X] = V[X] | V[Y];
                    break;
                case 0x2:
                    V[X] = V[X] & V[Y];
                    break;
                case 0x3:
                    V[X] = V[X] ^ V[Y];
                    break;
                case 0x4:
                    V[X] = V[X] + V[Y];
                    if (V[X] < V[Y]) {
                        VF = 1;
                    } else {
                        VF = 0;
                    }
                    break;
                case 0x5:
                    if (V[X] > V[Y]) {
                        VF = 1;
                    } else {
                        VF = 0;
                    }
                    V[X] = V[X] - V[Y];
                    break;
                case 0x6:
                    VF = V[X] & 0x01;
                    V[X] = V[X] >> 1;
                    break;
                case 0x7:
                    if (V[Y] > V[X]) {
                        VF = 1;
                    } else {
                        VF = 0;
                    }
                    V[X] = V[Y] - V[X];
                    break;
                case 0xE:
                    VF = V[X] >> 7;
                    V[X] = V[X] << 1;
                    break;
                default:
                    break;
            }
            PC += 2;
            printf("8XY operation\n");
        } else if ((opcode & 0xF000) == 0x9000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            if (V[X] != V[Y]) {
                PC += 4;
                printf("Skip next instruction (V[%d] != V[%d])\n", X, Y);
            } else {
                PC += 2;
            }
        } else if ((opcode & 0xF000) == 0xA000) {
            I = opcode & 0x0FFF;
            PC += 2;
            printf("LD I, 0x%04X\n", I);
        } else if ((opcode & 0xF000) == 0xB000) {
            uint16_t addr = opcode & 0x0FFF;
            PC = addr + V[0];
            printf("JP V0 + 0x%04X\n", addr);
        } else if ((opcode & 0xF000) == 0xC000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            V[X] = (rand() % 256) & NN;
            PC += 2;
            printf("RND V[%d], 0x%02X\n", X, NN);
        } else if ((opcode & 0xF000) == 0xD000) {
            PC += 2;
            printf("DRW\n");
        } else if ((opcode & 0xF000) == 0xF000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            switch(opcode & 0x00FF){
                case 0x07:
                    V[X] = delay;
                    PC+=2;
                    printf("LD V[%d], delay\n", X);
                    break;
                case 0x15:
                    delay = V[X];
                    PC+=2;
                    printf("LD delay, V[%d]\n", X);
                    break;
                case 0x18:
                    sound = V[X];
                    PC+=2;
                    printf("LD sound, V[%d]\n", X);
                    break;
                case 0x1E:
                    I += V[X];
                    PC+=2;
                    printf("ADD I, V[%d]\n", X);
                    break;
                case 0x29:
                    I = V[X] * 5;
                    PC+=2;
                    printf("LD I, font address V[%d]\n", X);
                    break;
                case 0x33:
                    memory[I] = V[X] / 100;
                    memory[I+1] = (V[X] / 10) % 10;
                    memory[I+2] = V[X] % 10;
                    PC+=2;
                    printf("LD BCD V[%d]\n", X);
                    break;
                case 0x55:
                    for (int i = 0; i <= X; i++){
                        memory[I + i] = V[i];
                    }
                    PC+=2;
                    printf("LD [I], V0...V[%d]\n", X);
                    break;
                case 0x65:
                    for (int i = 0; i <= X; i++){
                        V[i] = memory[I + i];
                    }
                    PC+=2;
                    printf("LD V0...V[%d], [I]\n", X);
                    break;
                default:
                    PC+=2;
                    printf("F operation\n");
                    break;
            }
        } else {
            PC += 2;
            printf("Unknown opcode\n");
        }
    }
    printf("V0: %d, V1: %d, V2: %d\n",V[0], V[1], V[2]);
    return 0;
}
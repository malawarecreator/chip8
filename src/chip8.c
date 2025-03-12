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
    // Initialize V registers to 0
    for (int i = 0; i < 16; i++) {
        V[i] = 0;
    }

    char* filename = argv[1];
    FILE *rom = fopen(filename , "rb");
    fread(&memory[0x200], 1, 3584, rom);
    fclose(rom);

    while (1) {
        uint16_t opcode = (memory[PC] << 8) | memory[PC + 1];
        if (opcode == 0x0000) {
            printf("Program halted (0x0000 encountered).\n");
            break;
        }

        if ((opcode & 0xF000) == 0x6000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            V[X] = NN;
            PC += 2;
        } else if ((opcode & 0xF000) == 0x7000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            V[X] += NN;
            PC += 2;
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
        } else if ((opcode & 0xF000) == 0x9000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            if (V[X] != V[Y]) {
                PC += 4;
            } else {
                PC += 2;
            }
        } else if ((opcode & 0xF000) == 0xA000) {
            I = opcode & 0x0FFF;
            PC += 2;
        } else if ((opcode & 0xF000) == 0xB000) {
            uint16_t addr = opcode & 0x0FFF;
            PC = addr + V[0];
        } else if ((opcode & 0xF000) == 0xC000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            V[X] = (rand() % 256) & NN;
            PC += 2;
        } else if ((opcode & 0xF000) == 0xD000) {
            PC += 2;
        } else if ((opcode & 0xF000) == 0xF000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            switch(opcode & 0x00FF){
                case 0x07:
                    V[X] = delay;
                    PC+=2;
                    break;
                case 0x15:
                    delay = V[X];
                    PC+=2;
                    break;
                case 0x18:
                    sound = V[X];
                    PC+=2;
                    break;
                case 0x1E:
                    I += V[X];
                    PC+=2;
                    break;
                case 0x29:
                    I = V[X] * 5; //font address calculation
                    PC+=2;
                    break;
                case 0x33:
                    memory[I] = V[X] / 100;
                    memory[I+1] = (V[X] / 10) % 10;
                    memory[I+2] = V[X] % 10;
                    PC+=2;
                    break;
                case 0x55:
                    for (int i = 0; i <= X; i++){
                        memory[I + i] = V[i];
                    }
                    PC+=2;
                    break;
                case 0x65:
                    for (int i = 0; i <= X; i++){
                        V[i] = memory[I + i];
                    }
                    PC+=2;
                    break;
                default:
                    PC+=2;
                    break;
            }
        } else {
            PC += 2;
        }
    }
    printf("V0: %d, V1: %d, V2: %d\n",V[0], V[1], V[2]);
    return 0;
}
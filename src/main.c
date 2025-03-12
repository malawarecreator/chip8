#include <stdint.h>

#include <stdio.h>
uint8_t memory[4096] = {0};

uint8_t V[16] = {0}; // registers
uint16_t I = 0; // index register
uint16_t PC = 0x200; // program counter
uint8_t SP = 0; // Stack pointer
uint16_t stack[16];
uint8_t VF = 0;
uint8_t delay = 0;
uint8_t sound = 0;
int main(int argc, char** argv) {

    // Read the rom dude
    char* filename = argv[1];
    FILE *rom = fopen(filename , "rb");
    fread(&memory[0x200], 1, 3584, rom);

    fclose(rom);

    while (1)
    {
        uint16_t opcode = (memory[PC] << 8 | memory[PC + 1]);
        if ((opcode & 0xF000) == 0xA000) {
            I = opcode & 0x0FFF;
            PC += 2;
            
        }
        if ((opcode & 0xF000) == 0x6000) {
            uint8_t X  = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            V[X] = NN;
            PC += 2;
        }
        if ((opcode & 0xF000) == 0x7000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            V[X] += NN;
            PC += 2;
        }
        if ((opcode & 0xF000) == 0x8000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            switch (opcode & 0x000F)
            {
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
                
                break;
            case 0x5:
                V[X] = V[X] - V[Y];
                
                break;
            case 0x6:
                VF = V[X] & 0x01;
                V[X] = V[X] >> 1;
            case 0x7:
                V[X] = V[Y] - V[X];
            case 0xE:

                VF = V[X] >> 7;
                V[X] = V[X] << 1;

            default:
                break;


            
            }
            PC += 2;
        }
        if ((opcode & 0xF000) == 0x9000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            if (V[X] != V[Y]) {
                PC += 4;
            }  else {
                PC += 2;
            }
        }

        if ((opcode & 0xF000) == 0xA000) {
            I = opcode & 0x0FFF;
            PC += 2;
        }

        if ((opcode & 0xF000) == 0xB000) {
            uint16_t addr  = opcode & 0x0FFF;
            PC = addr + V[0];
        }
        if ((opcode & 0xF000) == 0xC000) {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = (opcode & 0x00F0);
            V[X] = (rand() % 256) & NN;
        }

        if ((opcode & 0xF000) == 0xD000) {
            // To be implemented. Needed vars: X Y N
            /*
            uint8_t X = (opcode & 0x0F00) >> 8;  
            uint8_t Y = (opcode & 0x00F0) >> 4;
            uint8_t N = opcode & 0x000F;         
            VF = 0;
            
            
            */
            
        }


        // 0xE000 and 0xF000 are to be implemented


        
        
    }
    






}
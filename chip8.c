//
// Created by Jonathan on 01.01.2026.
//

#include "chip8.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

unsigned char chip8_fontset[80] =
        {
                0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                0x20, 0x60, 0x20, 0x20, 0x70, // 1
                0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };


struct chip8 *init_chip8() {
    struct chip8 *cpu = malloc(sizeof(struct chip8));

    return cpu;
}

void initialize(struct chip8 *chip) {
    chip->pc = TEXT_SEGMENT_START;
    chip->opcode = 0;
    chip->index_reg = 0;
    chip->sp = 0;

    for (int i = 0; i < 80; ++i)
        chip->memory[i] = chip8_fontset[i];

    srand(time(NULL));
}

void load_program(char *filename, struct chip8 *chip) {
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        printf("File could not be opened! %d\n", ferror(file));
        exit(1);
    }

    size_t result = fread(chip->memory+TEXT_SEGMENT_START, 1, MEM_SIZE-TEXT_SEGMENT_START, file);

    if (!result)
        puts("Did not read anything");

    if (ferror(file))
        puts("Error encountered");
}

void emulate_cycle(struct chip8 *chip) {

    chip->opcode = FETCH_OPCODE(chip->pc, chip->memory);
    chip->pc += 2;
    printf("Executing opcode 0x%04x with pc %x\n", chip->opcode, chip->pc);

    int reg_x = GET_X(chip->opcode);
    int reg_y = GET_Y(chip->opcode);
    int nnn = NNN(chip->opcode);
    int nn = NN(chip->opcode);
    int n = N(chip->opcode);

    switch (chip->opcode & BIT_16) {
        case CLEAR_EXIT:
            switch (nn) {
                case CLEAR:
                    for (int i = 0; i < GFX_SIZE; i++) {
                        chip->gfx[i] = 0x0;
                    }
                    break;
                case EXIT:
                    chip->pc = chip->stack[chip->sp];
                    chip->sp -= 1;
                    break;
                default:
                    printf("DEFAULT CASE opcode: %d, pc: %x\n", chip->opcode, chip->pc);
                    print_debug(chip);
                    WaitTime(20);
                    exit(1);
            }
        case LDI:
            printf("Assign Index register with value %x\n", nnn);
            chip->index_reg = nnn;
            break;
        case JMP:
            chip->pc = nnn;
            break;
        case CALL:
            printf("called %x with pc %x", nnn, chip->pc);
            chip->sp += 1;
            chip->stack[chip->sp] = chip->pc;
            chip->pc = nnn;
            break;
        case SE:
            if (chip->V[reg_x] == nn) chip->pc += 2;

            break;
        case SNE:
            if (chip->V[reg_x] != nn) chip->pc += 2;

            break;
        case SE_R:
            if (chip->V[reg_x] == chip->V[reg_y])
                chip->pc += 2;
            break;
        case ASSIGN_R:
            printf("Assigning register %x with value %d\n", reg_x, nn);
            chip->V[reg_x] = nn;
            break;
        case ADD_R:
            chip->V[reg_x] += nn;
            break;
        case OPS_R:
            switch (chip->opcode & LAST_4_BITS) {
                case ASSIGN:
                    chip->V[reg_x] = chip->V[reg_y];
                    break;
                case OR:
                    chip->V[reg_x] |= chip->V[reg_y];
                    break;
                case AND:
                    chip->V[reg_x] &= chip->V[reg_y];
                    break;
                case XOR:
                    chip->V[reg_x] ^= chip->V[reg_y];
                    break;
                case ADD:
                    chip->V[VF] = 0;
                    chip->V[VF] = chip->V[reg_x];
                    chip->V[reg_x] += chip->V[reg_y];

                    if (((int) (chip->V[VF] + chip->V[reg_y])) > UINT8_MAX)
                        chip->V[VF] = 1;
                    break;
                case SUB:
                    chip->V[VF] = 0;
                    chip->V[VF] = (chip->V[reg_x] > chip->V[reg_y]);
                    chip->V[reg_x] -= chip->V[reg_y];
                    break;
                case SHR:
                    chip->V[VF] = 0;
                    chip->V[VF] = chip->V[reg_x] & 0b1;
                    chip->V[reg_x] >>= chip->V[reg_y];
                    break;
                case SHL:
                    chip->V[VF] = 0;
                    chip->V[VF] = chip->V[reg_y] & 0b10000000;
                    chip->V[reg_x] <<= chip->V[reg_y];
                    break;
                case SUBN:
                    chip->V[VF] = 0;
                    chip->V[VF] = chip->V[reg_y] > chip->V[reg_x];
                    chip->V[reg_x] = chip->V[reg_y] - chip->V[reg_x];
                    break;
                default:
                    printf("UNIMPLEMENTED REGISTER OPCODE %d\n", chip->opcode & LAST_4_BITS);
            }
            break;
        case SNE_R:
            if (chip->V[reg_x] != chip->V[reg_y])
                chip->pc += 2;
            break;
        case JMP_R:
            chip->pc = nnn + chip->V[V0];
            break;
        case RAND:
            chip->V[reg_x] = (rand() % UCHAR_MAX) ^ nn;
            break;
        case DRW: {
            // This section was generated by gemini
            uint8_t x_coord = chip->V[reg_x];
            uint8_t y_coord = chip->V[reg_y];
            uint8_t height = n;
            chip->V[VF] = 0;

            printf("DRAW at x: %d, y: %d, with height %d\n", x_coord, y_coord, height);

            for (int row = 0; row < height; row++) {
                uint8_t sprite_byte = chip->memory[chip->index_reg + row];
                for (int col = 0; col < 8; col++) {
                    if ((sprite_byte & (0x80 >> col)) != 0) {
                        int screen_x = (x_coord + col) % 64;
                        int screen_y = (y_coord + row) % 32;

                        int screen_index = screen_x + (screen_y * 64);

                        if (chip->gfx[screen_index] == 1) {
                            chip->V[VF] = 1;
                        }

                        chip->gfx[screen_index] ^= 1;
                    }
                }
            }

            chip->draw_flag = 1;
            //Gemini generated section ends here
        }
            break;
        case OPS_K:
            if ((chip->opcode & LAST_8_BITS) == KEY_NOT_PRESSED)
                if (chip->key[chip->V[reg_x]])
                    chip->pc += 2;
            if ((chip->opcode & LAST_8_BITS) != KEY_PRESSED)
                if (!chip->key[chip->V[reg_x]])
                    chip->pc += 2;
            break;

        case OPS_M:
            switch (nn) {
                case LD_X_DT:
                    chip->V[reg_x] = chip->delay_timer;
                    break;
                case LD_K_W: {
                    int key = GetKeyPressed();

                    if (!key) {
                        chip->pc -= 2;
                    } else {
                        if (key_code_to_key_pad(key) != INVALID_KEY) {
                            chip->V[reg_x] = key_code_to_key_pad(key);
                        } else {
                            chip->pc -= 2;
                        }
                    }
                }
                    break;
                case LD_DT_X:
                    chip->delay_timer = chip->V[reg_x];
                    break;
                case LD_ST_X:
                    chip->sound_timer = chip->V[reg_x];
                    break;
                case ADD_I_X:
                    chip->index_reg += chip->V[reg_x];
                    break;
                case SET_I_SP:
                    chip->index_reg = reg_x * 5;
                    break;
                case BCD:
                    chip->memory[chip->index_reg] = chip->V[reg_x] / 100;
                    chip->memory[chip->index_reg + 1] = (chip->V[reg_x] / 10) % 10;
                    chip->memory[chip->index_reg + 2] = (chip->V[reg_x] % 10);
                    break;
                case SAVE:
                    for (int i = 0; i < chip->V[reg_x]; i++) {
                        chip->memory[chip->index_reg + i] = chip->V[i];
                    }
                    break;
                case LOAD:
                    for (int i = 0; i < chip->V[reg_x]; i++) {
                        chip->V[i] = chip->memory[chip->index_reg + i];
                    }
                    break;
                default:
                    printf("UNIMPLEMENTED MISC OPERATION 0x%X\n", nn);
            }

        default:
            printf("UNIMPLEMENTED OPCODE 0x%X\n", chip->opcode);
    };

    if (chip->delay_timer > 0)
        chip->delay_timer -= 1;

    if (chip->sound_timer > 0) {
        if (chip->sound_timer == 1)
            printf("BEEP!\n");
        chip->sound_timer -= 1;
    }
}


void print_debug(struct chip8 *chip) {
    printf("STATE: \n");
    printf("PC:               %x\n", chip->pc);
    printf("INDEX REGISTER:   %x\n", chip->index_reg);
    printf("OPCODE:           %x\n", chip->opcode);
    printf("STACK POINTER:    %x\n", chip->sp);
    printf("DELAY TIMER:      %x\n", chip->delay_timer);
    printf("SOUND TIMER:      %x\n", chip->sound_timer);
    printf("DRAW FLAG:        %x\n", chip->draw_flag);

    printf("MEMORY CONTENT:     \n");

    for (int i = 0; i < MEM_SIZE; i++) {
        if (i % 8 == 0 && i != 0)
            printf("\n");
        if (i % 8 == 0)
            printf("0x%03x ", i);
        printf("0x%02x ", chip->memory[i]);
    }
    printf("\n");

    printf("STACK CONTENT:  \n");
    for (int i = 0; i < STACK_SIZE; i++) {
        printf("0x%04x\n", chip->stack[i]);
    }
    printf("\n");

    printf("GFX BUFFER CONTENT: \n");

    for (int i = 0; i < GFX_SIZE; i++) {
        if (i % 8 == 0 && i != 0)
            printf("\n");
        if (i % 8 == 0)
            printf("0x%03x ", i);
        printf("0x%02x ", chip->gfx[i]);
    }
    printf("\n");
}

void draw_graphics(struct chip8 *chip, RenderTexture2D target) {
    printf("DRAWING \n");
    BeginTextureMode(target);
    ClearBackground(BLACK);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (chip->gfx[x + (y*64)] == 1)
                DrawPixel(x , y, WHITE);
        }
    }

    EndTextureMode();
    chip->draw_flag = 0;
}

void set_keys(struct chip8 *chip) {
    for (int i = 0; i < 16; i++) {
        chip->key[i] = IsKeyDown(key_pad_to_key_code(i));
    }
}

void execute_program(struct chip8 *chip) {
    InitWindow(WIDTH * SCALE, HEIGHT * SCALE, "chip-8 emulator");

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        emulate_cycle(chip);

        if (chip->draw_flag)
            draw_graphics(chip, target);

        set_keys(chip);

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(target.texture,
                       (Rectangle) {0, 0, (float) target.texture.width, (float) -target.texture.height},
                       (Rectangle) {0, 0, (float) GetScreenWidth(), (float) GetScreenHeight()},
                       (Vector2) {0, 0}, 0.00, WHITE);
        EndDrawing();
    }
}


int key_pad_to_key_code(unsigned char key_pad) {
    switch (key_pad) {
        // Row 1
        case KEY_1_P:
            return KEY_KP_1;
        case KEY_2_P:
            return KEY_KP_2;
        case KEY_3_P:
            return KEY_KP_3;
        case KEY_4_P:
            return KEY_KP_4;

        // Row 2
        case KEY_Q_P:
            return KEY_Q;
        case KEY_W_P:
            return KEY_W;
        case KEY_E_P:
            return KEY_E;
        case KEY_R_P:
            return KEY_R;

        // Row 3
        case KEY_A_P:
            return KEY_A;
        case KEY_S_P:
            return KEY_S;
        case KEY_D_P:
            return KEY_D;
        case KEY_F_P:
            return KEY_F;

        // Row 4
        case KEY_Z_P:
            return KEY_Z;
        case KEY_X_P:
            return KEY_X;
        case KEY_C_P:
            return KEY_C;
        case KEY_V_P:
            return KEY_V;
        default:
            printf("UNKNOWN KEY PAD ENCOUNTERED %d\n", key_pad);
    }

    return -1;
}


unsigned char key_code_to_key_pad(int key_code) {
    switch (key_code) {
        // Row 1
        case KEY_ONE:
            return KEY_1_P;
        case KEY_TWO:
            return KEY_2_P;
        case KEY_THREE:
            return KEY_3_P;
        case KEY_FOUR:
            return KEY_4_P;

            // Row 2
        case KEY_Q:
            return KEY_Q_P;
        case KEY_W:
            return KEY_W_P;
        case KEY_E:
            return KEY_E_P;
        case KEY_R:
            return KEY_R_P;

            // Row 3
        case KEY_A:
            return KEY_A_P;
        case KEY_S:
            return KEY_S_P;
        case KEY_D:
            return KEY_D_P;
        case KEY_F:
            return KEY_F_P;

            // Row 4
        case KEY_Z:
            return KEY_Z_P;
        case KEY_X:
            return KEY_X_P;
        case KEY_C:
            return KEY_C_P;
        case KEY_V:
            return KEY_V_P;

        default:
            return INVALID_KEY;
    }
}
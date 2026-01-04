//
// Created by Jonathan on 01.01.2026.
//

#include "chip8.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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


Instruction_Handler main_table[16] = {
        [CLEAR_EXIT] =  &op_0x,
        [JMP] =&op_jmp,
        [CALL] =&op_call,
        [SE] =&op_se,
        [SNE] =&op_sne,
        [SE_R] =&op_se_r,
        [ASSIGN_R] =&op_assign_r,
        [ADD_R] =&op_add_r,
        [OPS_R] =&op_ops_r,
        [SNE_R] =&op_sne_r,
        [LDI] =&op_ldi,
        [JMP_R] =&op_jmp_r,
        [RAND] =&op_rand,
        [DRW] =&op_drw,
        [OPS_K] =&op_ops_k,
        [OPS_M] =&op_ops_m
};
Instruction_Handler table_8[16] = {
        [ASSIGN] = &op_assign,
        [OR] = &op_or,
        [AND] = &op_and,
        [XOR] = &op_xor,
        [ADD] = &op_add,
        [SUB] = &op_sub,
        [SHR] = &op_shr,
        [SUBN] = &op_subn,
        [SHL] = &op_shl,
};

Instruction_Handler table_f[256] = {
        [LD_X_DT] = &op_load_x_delay_timer,
        [LD_K_W] = &op_load_key_wait,
        [LD_DT_X] = &op_load_delay_timer_x,
        [LD_ST_X] = &op_load_sound_timer_x,
        [ADD_I_X] = &op_add_i_x,
        [SET_I_SP] = &op_set_i_sp,
        [BCD] = &op_bcd,
        [SAVE] = &op_save,
        [LOAD] = &op_load,
};

struct chip8 *init_chip8() {
    struct chip8 *cpu = malloc(sizeof(struct chip8));

    return cpu;
}

void update_timers(struct chip8 *chip);

void initialize(struct chip8 *chip, char *mode) {
    chip->pc = TEXT_SEGMENT_START;
    chip->opcode = 0;
    chip->index_reg = 0;
    chip->sp = 0;

    for (int i = 0; i < 80; ++i)
        chip->memory[i] = chip8_fontset[i];

    srand(time(NULL));

    if (!strcmp("chip-8", mode)) {
        chip->quirks.shift_quirk = true;
        chip->quirks.load_store_quirk = true;
        chip->quirks.jump_quirk = false;
        chip->quirks.clipping_quirk = true;
        chip->quirks.display_wait = true;
    }
}

void load_program(char *filename, struct chip8 *chip) {
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        printf("File could not be opened! %d\n", ferror(file));
        exit(1);
    }

    size_t result = fread(chip->memory + TEXT_SEGMENT_START, 1, MEM_SIZE - TEXT_SEGMENT_START, file);

    if (!result)
        puts("Did not read anything");

    if (ferror(file))
        puts("Error encountered");
}

void emulate_cycle(struct chip8 *chip) {
    chip->opcode = FETCH_OPCODE(chip->pc, chip->memory);
    chip->pc += 2;

    uint8_t first_nibble = NXXX(chip->opcode);
    main_table[first_nibble](chip);
}

void update_timers(struct chip8 *chip) {
    if (chip->delay_timer > 0)
        chip->delay_timer -= 1;

    if (chip->sound_timer > 0) {
        if (chip->sound_timer == 1)
            printf("BEEP!\n");
        chip->sound_timer -= 1;
    }
}


void op_0x(struct chip8 *chip) {
    switch (NN(chip->opcode)) {
        case CLEAR:
            for (int i = 0; i < GFX_SIZE; i++) {
                chip->gfx[i] = 0x0;
            }
            chip->draw_flag = 1;
            break;
        case EXIT:
            chip->pc = chip->stack[chip->sp];
            chip->sp -= 1;
            break;
        default:
            printf("DEFAULT CASE opcode: %d, pc: %x\n", chip->opcode, chip->pc);
    }
}


void op_jmp(struct chip8 *chip) {
    chip->pc = NNN(chip->opcode);
}

void op_call(struct chip8 *chip) {
    chip->sp += 1;
    chip->stack[chip->sp] = chip->pc;
    chip->pc = NNN(chip->opcode);
}

void op_se(struct chip8 *chip) {
    if (chip->V[GET_X(chip->opcode)] == NN(chip->opcode))
        chip->pc += 2;
}

void op_sne(struct chip8 *chip) {
    if (chip->V[GET_X(chip->opcode)] != NN(chip->opcode))
        chip->pc += 2;
}

void op_se_r(struct chip8 *chip) {
    if (chip->V[GET_X(chip->opcode)] == chip->V[GET_Y(chip->opcode)])
        chip->pc += 2;
}

void op_assign_r(struct chip8 *chip) {
    chip->V[GET_X(chip->opcode)] = NN(chip->opcode);
}

void op_add_r(struct chip8 *chip) {
    chip->V[GET_X(chip->opcode)] += NN(chip->opcode);
}

void op_ops_r(struct chip8 *chip) {
    table_8[N(chip->opcode)](chip);
}

void op_assign(struct chip8 *chip) {
    chip->V[GET_X(chip->opcode)] = chip->V[GET_Y(chip->opcode)];
}

void op_or(struct chip8 *chip) {
    chip->V[GET_X(chip->opcode)] |= chip->V[GET_Y(chip->opcode)];
    chip->V[VF] = 0;
}

void op_and(struct chip8 *chip) {
    chip->V[GET_X(chip->opcode)] &= chip->V[GET_Y(chip->opcode)];
    chip->V[VF] = 0;
}

void op_xor(struct chip8 *chip) {
    chip->V[GET_X(chip->opcode)] ^= chip->V[GET_Y(chip->opcode)];
    chip->V[VF] = 0;
}

void op_add(struct chip8 *chip) {
    uint8_t flag = ((int) (chip->V[GET_X(chip->opcode)] + chip->V[GET_Y(chip->opcode)])) > UINT8_MAX;
    chip->V[GET_X(chip->opcode)] += chip->V[GET_Y(chip->opcode)];
    chip->V[VF] = flag;
}

void op_sub(struct chip8 *chip) {
    uint8_t flag = (chip->V[GET_X(chip->opcode)] >= chip->V[GET_Y(chip->opcode)]);

    chip->V[GET_X(chip->opcode)] -= chip->V[GET_Y(chip->opcode)];
    chip->V[VF] = flag;
}

void op_shr(struct chip8 *chip) {
    if (chip->quirks.shift_quirk) {
        chip->V[GET_X(chip->opcode)] = chip->V[GET_Y(chip->opcode)];
    }

    uint8_t flag = chip->V[GET_X(chip->opcode)] & 0b1;
    chip->V[GET_X(chip->opcode)] >>= 1;
    chip->V[VF] = flag;
}

void op_subn(struct chip8 *chip) {
    uint8_t x = GET_X(chip->opcode);
    uint8_t y = GET_Y(chip->opcode);

    uint8_t flag = (chip->V[y] >= chip->V[x]);

    chip->V[x] = chip->V[y] - chip->V[x];
    chip->V[VF] = flag;
}

void op_shl(struct chip8 *chip) {
    if (chip->quirks.shift_quirk) {
        chip->V[GET_X(chip->opcode)] = chip->V[GET_Y(chip->opcode)];
    }

    uint8_t flag = chip->V[GET_X(chip->opcode)] & 0b10000000;
    chip->V[GET_X(chip->opcode)] <<= 1;
    chip->V[VF] = flag / 128;
}


void op_sne_r(struct chip8 *chip) {
    if (chip->V[GET_X(chip->opcode)] != chip->V[GET_Y(chip->opcode)])
        chip->pc += 2;
}


void op_ldi(struct chip8 *chip) {
    chip->index_reg = NNN(chip->opcode);
}

void op_jmp_r(struct chip8 *chip) {
    chip->pc = NNN(chip->opcode) + chip->V[V0];
}

void op_rand(struct chip8 *chip) {
    chip->V[GET_X(chip->opcode)] = (rand() % UCHAR_MAX) ^ NN(chip->opcode);
}

void op_drw(struct chip8 *chip) {
    // This section was generated by gemini
    uint8_t x_coord = chip->V[GET_X(chip->opcode)];
    uint8_t y_coord = chip->V[GET_Y(chip->opcode)];


    if (chip->quirks.clipping_quirk && (x_coord > WIDTH))
        x_coord %= WIDTH;
    if (chip->quirks.clipping_quirk && (y_coord > HEIGHT))
        y_coord %= HEIGHT;

    uint8_t height = N(chip->opcode);
    chip->V[VF] = 0;

    for (int row = 0; row < height; row++) {
        uint8_t sprite_byte = chip->memory[chip->index_reg + row];
        for (int col = 0; col < 8; col++) {


            if ((sprite_byte & (0x80 >> col)) != 0) {
                int screen_x = x_coord + col;
                int screen_y = y_coord + row;

                if (chip->quirks.clipping_quirk && (screen_x > WIDTH || screen_y > HEIGHT)) {
                    continue;
                } else {
                    screen_x %= WIDTH;
                    screen_y %= HEIGHT;
                }


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

void op_ops_k(struct chip8 *chip) {
    switch (NN(chip->opcode)) {
        case KEY_PRESSED:
            if (chip->key[chip->V[GET_X(chip->opcode)]])
                chip->pc += 2;
            break;
        case KEY_NOT_PRESSED:
            if (!chip->key[chip->V[GET_X(chip->opcode)]])
                chip->pc += 2;
            break;
    }
}

void op_ops_m(struct chip8 *chip) {
    table_f[NN(chip->opcode)](chip);
}

void op_load_x_delay_timer(struct chip8 *chip) {
    chip->V[GET_X(chip->opcode)] = chip->delay_timer;
}


void op_load_key_wait(struct chip8 *chip) {
    int key = GetKeyPressed();

    if (!key) {
        chip->pc -= 2;
    } else {
        if (key_code_to_key_pad(key) != INVALID_KEY) {
            chip->V[GET_X(chip->opcode)] = key_code_to_key_pad(key);
        } else {
            chip->pc -= 2;
        }
    }
}


void op_load_delay_timer_x(struct chip8 *chip) {
    chip->delay_timer = chip->V[GET_X(chip->opcode)];
}


void op_load_sound_timer_x(struct chip8 *chip) {
    chip->sound_timer = chip->V[GET_X(chip->opcode)];
}


void op_add_i_x(struct chip8 *chip) {
    chip->index_reg += chip->V[GET_X(chip->opcode)];
}


void op_set_i_sp(struct chip8 *chip) {
    chip->index_reg = GET_X(chip->opcode) * 5;
}


void op_bcd(struct chip8 *chip) {
    chip->memory[chip->index_reg] = chip->V[GET_X(chip->opcode)] / 100;
    chip->memory[chip->index_reg + 1] = (chip->V[GET_X(chip->opcode)] / 10) % 10;
    chip->memory[chip->index_reg + 2] = (chip->V[GET_X(chip->opcode)] % 10);
}

void op_save(struct chip8 *chip) {
    for (int i = 0; i <= GET_X(chip->opcode); i++) {
        chip->memory[chip->index_reg + i] = chip->V[i];
    }

    if (chip->quirks.load_store_quirk)
        chip->index_reg += GET_X(chip->opcode) + 1;

}

void op_load(struct chip8 *chip) {
    for (int i = 0; i <= GET_X(chip->opcode); i++) {
        chip->V[i] = chip->memory[chip->index_reg + i];
    }

    if (chip->quirks.load_store_quirk)
        chip->index_reg += GET_X(chip->opcode) + 1;
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
    BeginTextureMode(target);
    ClearBackground(BLACK);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (chip->gfx[x + (y * 64)] == 1)
                DrawPixel(x, y, WHITE);
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
    double last_time = GetTime();

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
    BeginDrawing();
    SetTargetFPS(FPS);

    while (!WindowShouldClose()) {

        for (int i = 0; i < 10; i++) {
            emulate_cycle(chip);
            if (chip->draw_flag) {
                draw_graphics(chip, target);
                if (chip->quirks.display_wait)
                    break;
            }
            set_keys(chip);
        }
        update_timers(chip);


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
            return KEY_ONE;
        case KEY_2_P:
            return KEY_TWO;
        case KEY_3_P:
            return KEY_THREE;
        case KEY_4_P:
            return KEY_FOUR;

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
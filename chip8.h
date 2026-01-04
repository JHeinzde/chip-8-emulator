//
// Created by Jonathan on 01.01.2026.
//

#pragma once

#include "raylib.h"
#include <stdint.h>
#include <stdbool.h>

#define WIDTH 64
#define HEIGHT 32
#define SCALE 10
#define FPS 60

#define GFX_SIZE 2048
#define MEM_SIZE 4096
#define REG_SIZE 16
#define STACK_SIZE 16
#define KEY_SIZE 16

#define TEXT_SEGMENT_START 0x200
#define FETCH_OPCODE(pc, mem) ((mem)[(pc)] << 8 | (mem)[(pc) + 1])

typedef enum {
    V0 = 0,
    V1 = 1,
    V2 = 2,
    V3 = 3,
    V4 = 4,
    V5 = 5,
    V6 = 6,
    V7 = 7,
    V8 = 8,
    V9 = 9,
    VA = 10,
    VB = 11,
    VC = 12,
    VD = 13,
    VE = 14,
    VF = 15,
} register_refs;

typedef enum {
    CLEAR_EXIT = 0x0,
    JMP = 0x1,
    CALL = 0x2,
    SE = 0x3,
    SNE = 0x4,
    SE_R = 0x5,
    ASSIGN_R = 0x6,
    ADD_R = 0x7,
    OPS_R = 0x8,
    SNE_R = 0x9,
    LDI = 0xa,
    JMP_R = 0xb,
    RAND = 0xc,
    DRW = 0xd,
    OPS_K = 0xe,
    OPS_M = 0xf,
} chip8_opcodes;

typedef enum {
    CLEAR = 0x00e0,
    EXIT = 0x00ee,

} clear_exit;

typedef enum {
    ASSIGN = 0x0,
    OR = 0x1,
    AND = 0x2,
    XOR = 0x3,
    ADD = 0x4,
    SUB = 0x5,
    SHR = 0x6,
    SUBN = 0x7,
    SHL = 0xE,
} register_opcodes;

typedef enum {
    KEY_NOT_PRESSED = 0xa1,
    KEY_PRESSED = 0x9e,
} key_ops;

typedef enum {
    LD_X_DT = 0x07,
    LD_K_W = 0x0a,
    LD_DT_X = 0x15,
    LD_ST_X = 0x18,
    ADD_I_X = 0x1e,
    SET_I_SP = 0x29,
    BCD = 0x33,
    SAVE = 0x55,
    LOAD = 0x65,
} misc_operations;

typedef enum {
    KEY_1_P = 1,
    KEY_2_P = 2,
    KEY_3_P = 3,
    KEY_4_P = 0xc,
    KEY_Q_P = 4,
    KEY_W_P = 5,
    KEY_E_P = 6,
    KEY_R_P = 0xd,
    KEY_A_P = 7,
    KEY_S_P = 8,
    KEY_D_P = 9,
    KEY_F_P = 0xe,
    KEY_Z_P = 0xa,
    KEY_X_P = 0,
    KEY_C_P = 0xb,
    KEY_V_P = 0xf,
} keys;

typedef struct {
    bool shift_quirk;
    bool load_store_quirk;
    bool jump_quirk;
    bool clipping_quirk;
    bool display_wait;
} chip8_quirks;

#define LAST_12_BITS 0x0fff
#define LAST_8_BITS 0x00ff
#define LAST_4_BITS 0x000f

#define BIT_16 0xf000
#define BIT_16_SHIFT 12

#define BIT_12 0x0f00
#define BIT_12_SHIFT 8

#define BIT_8  0x00f0
#define BIT_8_SHIFT 4

#define GET_X(opcode) (((opcode) & BIT_12) >> BIT_12_SHIFT)
#define GET_Y(opcode) (((opcode) & BIT_8) >> BIT_8_SHIFT)

#define NNN(opcode) ((opcode) & LAST_12_BITS)
#define NN(opcode) ((opcode) & LAST_8_BITS)
#define N(opcode) ((opcode) & LAST_4_BITS)
#define NXXX(opcode) (((opcode) & BIT_16) >> BIT_16_SHIFT)

#define INVALID_KEY 255


struct chip8 {
    uint8_t memory[MEM_SIZE];
    uint8_t V[REG_SIZE];
    uint8_t gfx[GFX_SIZE];

    uint16_t stack[STACK_SIZE];
    uint16_t sp;

    uint8_t key[KEY_SIZE];

    uint16_t pc;
    uint16_t opcode;

    uint16_t index_reg;

    uint8_t delay_timer;
    uint8_t sound_timer;

    bool draw_flag;

    chip8_quirks quirks;
};


struct chip8 *init_chip8();

void load_program(char *filename, struct chip8 *chip);

void initialize(struct chip8 *chip, char* mode);

void execute_program(struct chip8 *chip);

void emulate_cycle(struct chip8 *chip);

void draw_graphics(struct chip8 *chip, RenderTexture2D target);

void set_keys(struct chip8 *chip);

void print_debug(struct chip8 *chip);

int key_pad_to_key_code(unsigned char key_pad);

unsigned char key_code_to_key_pad(int key);

typedef void (*Instruction_Handler)(struct chip8 *chip);

void op_0x(struct chip8 *chip);

void op_ldi(struct chip8 *chip);

void op_jmp(struct chip8 *chip);

void op_call(struct chip8 *chip);

void op_se(struct chip8 *chip);

void op_sne(struct chip8 *chip);

void op_se_r(struct chip8 *chip);

void op_assign_r(struct chip8 *chip);

void op_add_r(struct chip8 *chip);

void op_ops_r(struct chip8 *chip);

void op_assign(struct chip8 *chip);

void op_or(struct chip8 *chip);

void op_and(struct chip8 *chip);

void op_xor(struct chip8 *chip);

void op_add(struct chip8 *chip);

void op_sub(struct chip8 *chip);

void op_shr(struct chip8 *chip);

void op_subn(struct chip8 *chip);

void op_shl(struct chip8 *chip);

void op_sne_r(struct chip8 *chip);

void op_jmp_r(struct chip8 *chip);

void op_rand(struct chip8 *chip);

void op_drw(struct chip8 *chip);

void op_ops_k(struct chip8 *chip);

void op_ops_m(struct chip8 *chip);

void op_load_x_delay_timer(struct chip8 *chip);

void op_load_key_wait(struct chip8 *chip);

void op_load_delay_timer_x(struct chip8 *chip);

void op_load_sound_timer_x(struct chip8 *chip);

void op_add_i_x(struct chip8 *chip);

void op_set_i_sp(struct chip8 *chip);

void op_bcd(struct chip8 *chip);

void op_save(struct chip8 *chip);

void op_load(struct chip8 *chip);

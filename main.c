#include <stdio.h>
#include <limits.h>
#include "chip8.h"
#include "raylib.h"

#define DEBUG 1

int main() {
   struct chip8* chip = init_chip8();
    initialize(chip, "chip-8");
    load_program("C:\\Users\\Jonathan\\CLionProjects\\untitled\\6-keypad.ch8", chip);
    execute_program(chip);

    return 0;
}

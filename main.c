#include <stdio.h>
#include <limits.h>
#include "chip8.h"
#include "raylib.h"

#define DEBUG 1

int main() {
   struct chip8* chip = init_chip8();
    initialize(chip);
    //load_program("C:\\Users\\Jonathan\\CLionProjects\\untitled\\test_opcode.ch8", chip);
    //load_program("C:\\Users\\Jonathan\\CLionProjects\\untitled\\1-chip8-logo.ch8", chip);
    //load_program("C:\\Users\\Jonathan\\CLionProjects\\untitled\\3-corax+.ch8", chip);
    //load_program("C:\\Users\\Jonathan\\CLionProjects\\untitled\\2-ibm-logo.ch8", chip);
    load_program("C:\\Users\\Jonathan\\CLionProjects\\untitled\\4-flags.ch8", chip);
    //load_program("C:\\Users\\Jonathan\\CLionProjects\\untitled\\5-quirks.ch8", chip);
    execute_program(chip);

    return 0;
}

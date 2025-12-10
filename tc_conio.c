#include "tc_conio.h"
#include "sdltest/sdl_shim.h"

void gotoxy(bbcuint i, uint16_t i1) {
    sdl_gotoxy((uint16_t) i, i1);
}

void putch(ubyte iobyte) {
    sdl_putch(iobyte);
}

int coniogetch() {
    return sdl_coniogetch();
}

void _setcursortype(int cursortype) {
    sdl_setcursortype(cursortype);
}

int kbhit(void) {
    return sdl_kbhit();
}

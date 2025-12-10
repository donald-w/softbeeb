#include "tc_dos.h"
#include "sdltest/sdl_shim.h"

void nosound() {
    sdl_nosound();
}

void sound(double totfreq) {
    sdl_sound(totfreq);
}

void delay(int delay) {
    sdl_delay(delay);
}

char peekb(int segment, int offset) {
    return sdl_peekb(segment, offset);
}

void pokeb(int segment, int offset, char value) {
    sdl_pokeb(segment, offset, value);
}
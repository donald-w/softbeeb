#include "tc_bios.h"
#include "sdltest/sdl_shim.h"

ubyte _bios_keybrd(int shiftstatus) {
    return sdl_bios_keybrd(shiftstatus);
}
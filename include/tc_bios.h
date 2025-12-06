#pragma once

#include "header.h"

extern ubyte _bios_keybrd(int shiftstatus);

#define _KEYBRD_SHIFTSTATUS 1
#define _KEYBRD_READY 2
#define _KEYBRD_READ 3

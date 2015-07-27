#ifndef SOFTBEEB_TC_BIOS_H
#define SOFTBEEB_TC_BIOS_H

#include "header.h"

extern ubyte _bios_keybrd(int shiftstatus);

#define _KEYBRD_SHIFTSTATUS 0
#define _KEYBRD_READY 0
#define _KEYBRD_READ 0

#endif //SOFTBEEB_TC_BIOS_H

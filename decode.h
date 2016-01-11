#pragma once

#include "header.h"

extern void non_opcode(void);  // prototype for function occurring when
// an illegal opcode is executed

extern void update_flags(void);

extern void update_dyn_p(void);       // builds a valid status byte for
// stack pushing
//extern void pushbyte(ubyte); // push sub instruction
//extern ubyte popbyte(void);   // pop sub instruction


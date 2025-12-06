#pragma once

#include <stdio.h>
#include "header.h"

extern void gotoxy(bbcuint i, bbcuint i1);

extern void putch(ubyte iobyte);

extern int coniogetch();

extern int kbhit(void);

extern void _setcursortype(int cursortype);

#define _NOCURSOR 0
#define _NORMALCURSOR 1

#pragma once

#include "tc_graphics.h"
#include "tc_conio.h"
#include "tc_dos.h"
#include "tc_bios.h"

// SDL-backed replacements for the Turbo C shims. All functions mirror the
// original signatures but are prefixed with sdl_ to avoid clashes with the
// no-op stubs in the main project.

// Graphics
void sdl_getimage(int x1, int y1, int x2, int y2, ubyte ram[]);
void sdl_putimage(int x1, int y1, ubyte ram[], int mode);
void sdl_putpixel(uint16_t x, uint16_t y, int color);
void sdl_setgraphmode(int vgamed);
void sdl_restorecrtmode(void);
void sdl_clrscr(void);
void sdl_textcolor(int c);
void sdl_textbackground(int c);
void sdl_textmode(int mode);
int  sdl_registerfarbgidriver(int farptr);
void sdl_setpalette(ubyte logical, ubyte physical);
void sdl_setvisualpage(int page);
void sdl_setactivepage(int page);
void sdl_cprintf(char *string);
void sdl_closegraph(void);
void sdl_initgraph(int *driver, int *mode, char *path);
void sdl_setrgbpalette(int idx, int r, int g, int b);
bbcuint sdl_getcolor(void);
void sdl_setbkcolor(int c);
void sdl_cleardevice(void);
void sdl_settextstyle(int font, int dir, int size);
void sdl_setcolor(int c);
void sdl_outtextxy(int x, int y, char *string);

// Console
void sdl_gotoxy(uint16_t x, uint16_t y);
void sdl_putch(ubyte ch);
int  sdl_coniogetch(void);
int  sdl_kbhit(void);
void sdl_setcursortype(int cursortype);

// BIOS / DOS
ubyte sdl_bios_keybrd(int shiftstatus);
void sdl_nosound(void);
void sdl_sound(double totfreq);
void sdl_delay(int ms);
char sdl_peekb(int segment, int offset);
void sdl_pokeb(int segment, int offset, char value);

// Helper
int sdl_quit_requested(void);

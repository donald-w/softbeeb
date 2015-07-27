#ifndef SOFTBEEB_TC_GRAPHICS_H
#define SOFTBEEB_TC_GRAPHICS_H

#include "header.h"

extern void getimage(int i, int i1, int i2, int i3, ubyte ram[]);
extern void putimage(int i, int i1, ubyte ram[], int i2);

extern void putpixel(uint i, uint cord, int i1);

extern void setgraphmode(int vgamed);

extern void restorecrtmode();

extern void clrscr();

extern void textcolor(int white);

extern void textbackground(int i);

extern void textmode(int i);

#define VGAMED 0
#define VGA 0
#define C40 0
#define C80 0
#define RED 11
#define BLACK 12
#define WHITE 13
#define BLUE 14
#define YELLOW 15

#define EGAVGA_driver_far 0
extern int registerfarbgidriver(int far);

extern void setpalette(ubyte c, ubyte i);

#define EGA_WHITE	0
#define EGA_CYAN	1
#define EGA_MAGENTA	2
#define EGA_BLUE	3
#define EGA_YELLOW	4
#define EGA_GREEN	5
#define EGA_RED		6
#define EGA_BLACK	7
#define EGA_LIGHTRED	8


extern void setvisualpage(int i);
extern void setactivepage(int i);
extern void cprintf(char *string);

extern void closegraph();

extern void initgraph(int *pInt, int *pInt1, char *string);

extern void setrgbpalette(int i, int i1, int i2, int i3);

extern uint getcolor();

extern void setbkcolor(int i);

extern void cleardevice();

extern void settextstyle(int i, int i1, int i2);

extern void setcolor(int i);

extern void outtextxy(int i, int i1, char *string);

#define DEFAULT_FONT 0
#define HORIZ_DIR 0

#endif //SOFTBEEB_TC_GRAPHICS_H

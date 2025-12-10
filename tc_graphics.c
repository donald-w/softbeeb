#include "tc_graphics.h"
#include "sdltest/sdl_shim.h"

void getimage(int i, int i1, int i2, int i3, ubyte ram[]) {
    sdl_getimage(i, i1, i2, i3, ram);
}

void putimage(int i, int i1, ubyte ram[], int i2) {
    sdl_putimage(i, i1, ram, i2);
}

void putpixel(bbcuint i, bbcuint cord, int i1) {
    sdl_putpixel(i, cord, i1);
}

void setgraphmode(int vgamed) {
    sdl_setgraphmode(vgamed);
}

void restorecrtmode() {
    sdl_restorecrtmode();
}

void textcolor(int white) {
    sdl_textcolor(white);
}

int registerfarbgidriver(int far) {
    return sdl_registerfarbgidriver(far);
}

void setpalette(ubyte c, ubyte i) {
    sdl_setpalette(c, i);
}

void clrscr() {
    sdl_clrscr();
}

void textmode(int i) {
    sdl_textmode(i);
}

void textbackground(int i) {
    sdl_textbackground(i);
}

void setvisualpage(int i) {
    sdl_setvisualpage(i);
}

void setactivepage(int i) {
    sdl_setactivepage(i);
}

void cprintf(char *string) {
    sdl_cprintf(string);
}

void closegraph() {
    sdl_closegraph();
}

void initgraph(int *pInt, int *pInt1, char *string) {
    sdl_initgraph(pInt, pInt1, string);
}

void setrgbpalette(int i, int i1, int i2, int i3) {
    sdl_setrgbpalette(i, i1, i2, i3);
}

bbcuint getcolor() {
    return sdl_getcolor();
}

void setbkcolor(int i) {
    sdl_setbkcolor(i);
}

void cleardevice() {
    sdl_cleardevice();
}

void settextstyle(int i, int i1, int i2) {
    sdl_settextstyle(i, i1, i2);
}

void setcolor(int i) {
    sdl_setcolor(i);
}

void outtextxy(int i, int i1, char *string) {
    sdl_outtextxy(i, i1, string);
}
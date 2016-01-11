#include <stdio.h>
#include <stdlib.h>
#include "tc_graphics.h"
#include "header.h"
#include "screen.h"
#include "reg_name.h"
#include "io.h"

void (*wsheila[256])(ubyte);  // array of pointers to functions for io-write
// decoding

void flash(void);             // prototype for flash function

ubyte adc_control = 0;          // analogue digital control register
ubyte vid_con_reg = 0;          // video ULA control register

ubyte crt_sel = 0;              // crt register select register
ubyte crt_regs[18];           // array of crt register

ubyte romsel = 0;               // paged rom select register

ubyte s_via[16];              // array of system via registers
ubyte s_via_latch[8];         // the system via latch

ubyte vidpal[16];             // the video palette


/********************************************************
---------------------------------------------------------
********************************************************/


void donald(void) {
    puts("Donald");
}


void nullwrite(ubyte ignored) {
}

void write_crt(ubyte iobyte)   // crt controller write decode function
{
    if (crt_sel <= 15)               // if a valid register for writing to
    {
        crt_regs[crt_sel] = iobyte;   // write to the register
    }

    switch (crt_sel)               // take relevant action depending on
        // register action
    {
        case 1:
            set_colour_bits();

        case 12:
            break;
        case 13:
            if (teletext) {
                screen_start = crt_regs[12] * 0x100 + crt_regs[13];
                screen_start = 0x7400 + (screen_start ^ 0x2000);
                update_screen();
            }
            else {
                screen_start = crt_regs[12] * 0x100 + crt_regs[13];
                screen_start <<= 3;
                update_screen();
            }
            break;
        case 14:
            break;
        case 15:
            update_cursor();
            break; // if the cursor address has changed,
    }                               // update the cursor.

}

void crt_select(ubyte iobyte)   // crt register select function
{
    crt_sel = iobyte;
}

void vid_ULA(ubyte iobyte)      // Video ULA control register
{
    ubyte tempbits;
    ubyte flashstate;

    flashstate = vid_con_reg & 1;       // initial flash state
    vid_con_reg = iobyte;             // update control register

    if (!(iobyte & 2) && teletext)      // if 1-0 transition on the teletext select
    {                               // initialise graphics
        teletext = 0;
        pixel_vid_init();
    }

    if ((iobyte & 2) && (!teletext))    // if 0-1 transition on the teletext select
    {                               // initialise teletext
        teletext = 1;
        teletext_init();
    }

    tempbits = disp_chars;            // initial number of display characters

    switch (vid_con_reg & 0xC) {
        case 0x00:
            disp_chars = 10;
            break;
        case 0x04:
            disp_chars = 20;
            break;
        case 0x08:
            disp_chars = 40;
            break;
        case 0x0C:
            disp_chars = 80;
            break;
    }

    // if displayed characters change, set_colour_bits()
    if (tempbits != disp_chars) set_colour_bits();
    // if flash state has changed, flash()
    if ((flashstate != (vid_con_reg & 1)) && (!teletext)) flash();

}

void vidpalset(ubyte iobyte)     // video palette update functions
{
    ubyte logical_colour;
    ubyte actual_colour;
    ubyte pc_colour;

    logical_colour = iobyte / 0x10;   // calculate logical colour
    actual_colour = iobyte & 0xF;     // calculate actual programmed colour

    vidpal[logical_colour] = actual_colour;  // update vidpal copy of palette

    switch (actual_colour ^ 0x7)              // convert actual colour to
    {                                      // pc colour
        case 0:
            pc_colour = EGA_BLACK;
            break;
        case 1:
            pc_colour = EGA_RED;
            break;
        case 2:
            pc_colour = EGA_GREEN;
            break;
        case 3:
            pc_colour = EGA_YELLOW;
            break;
        case 4:
            pc_colour = EGA_BLUE;
            break;
        case 5:
            pc_colour = EGA_MAGENTA;
            break;
        case 6:
            pc_colour = EGA_CYAN;
            break;
        case 7:
            pc_colour = EGA_WHITE;
            break;

        case 8:
            pc_colour = EGA_BLACK;
            break;
        case 9:
            pc_colour = EGA_RED;
            break;
        case 10:
            pc_colour = EGA_GREEN;
            break;
        case 11:
            pc_colour = EGA_YELLOW;
            break;
        case 12:
            pc_colour = EGA_BLUE;
            break;
        case 13:
            pc_colour = EGA_MAGENTA;
            break;
        case 14:
            pc_colour = EGA_CYAN;
            break;
        case 15:
            pc_colour = EGA_WHITE;
            break;

    }

    setpalette(logical_colour, pc_colour); // update pc palette
}

void promsel(ubyte iobyte)               // paged rom select write function
{
    romsel = iobyte;
}

/******************************************************
******************************************************/

/* Functions of the form "void sw_via_?(ubyte)"
	are the system via register decodes functions
*/


void sw_via_0(ubyte iobyte) {
    s_via[ORB] = iobyte;
    current_latch = s_via_opb = iobyte & s_via[DDRB];
    if (iobyte == 0)
        iobyte = 0;
    s_via_latch[s_via_opb & 7] = s_via_opb & 8;

    iobyte &= 7;
    if ((iobyte == 4) || (iobyte == 5)) if (!teletext) set_screen_start();
    if (iobyte == 0) sound_byte(s_via_opa);
}

void sw_via_1(ubyte iobyte) {
    s_via[ORA] = iobyte;
    s_via_opa = iobyte & s_via[DDRA];
    if (s_via_latch[3] == 0) if (current_key) if ((current_key & 0xF) == (s_via_opa & 0xF)) s_via[IFR] |= 1;
}

void sw_via_2(ubyte iobyte) {
    s_via[DDRB] = iobyte;
    s_via_opb = iobyte & s_via[ORB];
}

void sw_via_3(ubyte iobyte) {
    s_via[DDRA] = iobyte;
    s_via_opa = iobyte & s_via[ORA];
}

void sw_via_4(ubyte iobyte) {
    s_via[T1C_L] = iobyte;
}

void sw_via_5(ubyte iobyte) {
    s_via[T1C_H] = iobyte;
}

void sw_via_6(ubyte iobyte) {
    s_via[T1L_L] = iobyte;
}

void sw_via_7(ubyte iobyte) {
    s_via[T1L_H] = iobyte;
}

void sw_via_8(ubyte iobyte) {
    s_via[T2C_L] = iobyte;
}

void sw_via_9(ubyte iobyte) {
    s_via[T2C_H] = iobyte;
}

void sw_via_A(ubyte iobyte) {
    s_via[SR] = iobyte;
}

void sw_via_B(ubyte iobyte) {
    s_via[ACR] = iobyte;
}

void sw_via_C(ubyte iobyte) {
    s_via[PCR] = iobyte;
}

void sw_via_D(ubyte iobyte) {
    s_via[IFR] = ((s_via[IFR] | iobyte) ^ iobyte);
}

void sw_via_E(ubyte iobyte) {
    (iobyte & 0x80) ? (s_via[IER] |= iobyte) : (s_via[IER] = ((s_via[IER] | iobyte) ^ iobyte));
    s_via[IER] |= 0x80;
}

void sw_via_F(ubyte iobyte) {
    sw_via_1(iobyte);
}


/******************************************************
******************************************************/

void adc_con_reg(ubyte iobyte)   // adc_control is never used
{                                // but a copy is kept for debugging
    adc_control = iobyte;              // purposes
}

void winit_io(void)              // Called to do any io_write initialising
{
    unsigned int c; // misc loop counter

    printf("Init write IO system...");  // Display pretty startup info

    for (c = 0; c <= 17; c++) crt_regs[c] = 0;   // initialise the arrays to 0
    for (c = 0; c <= 15; c++) s_via[c] = 0;
    for (c = 0; c <= 7; c++) s_via_latch[c] = 0;

    for (c = 0x00; c <= 0xFF; c++)          // initialise wsheila to point to a
    {                                 // dummy function
        wsheila[c] = nullwrite;
    }

    for (c = 0x00; c <= 0x07; c += 2) {
        wsheila[c] = crt_select;
        wsheila[c + 1] = write_crt;
    }

    for (c = 0x20; c <= 0x2F; c += 0x02) {
        wsheila[c] = vid_ULA;
        wsheila[c + 1] = vidpalset;
    }

    for (c = 0x30; c <= 0x3F; c++) {
        wsheila[c] = promsel;
    }

    wsheila[0x50] = wsheila[0x40] = sw_via_0;
    wsheila[0x51] = wsheila[0x41] = sw_via_1;
    wsheila[0x52] = wsheila[0x42] = sw_via_2;
    wsheila[0x53] = wsheila[0x43] = sw_via_3;
    wsheila[0x54] = wsheila[0x44] = sw_via_4;
    wsheila[0x55] = wsheila[0x45] = sw_via_5;
    wsheila[0x56] = wsheila[0x46] = sw_via_6;
    wsheila[0x57] = wsheila[0x47] = sw_via_7;
    wsheila[0x58] = wsheila[0x48] = sw_via_8;
    wsheila[0x59] = wsheila[0x49] = sw_via_9;
    wsheila[0x5A] = wsheila[0x4A] = sw_via_A;
    wsheila[0x5B] = wsheila[0x4B] = sw_via_B;
    wsheila[0x5C] = wsheila[0x4C] = sw_via_C;
    wsheila[0x5D] = wsheila[0x4D] = sw_via_D;
    wsheila[0x5E] = wsheila[0x4E] = sw_via_E;
    wsheila[0x5F] = wsheila[0x4F] = sw_via_F;

    for (c = 0xC0; c <= 0xDF; c += 0x04) {
        wsheila[c] = adc_con_reg;
    }

    printf("Done\n");   // display pretty startup status
}


void flash(void) {  // flash function, alternates the flash colours
    ubyte c;         // depending on the video ULA control register
    ubyte d;

    for (c = 0; c < 16; c++)         // loop through each logical colour
        if (vidpal[c] & 8) {
            if (vid_con_reg & 1) {
                switch ((vidpal[c] & 7) ^ 7) {
                    case 0:
                        d = EGA_WHITE;
                        break;
                    case 1:
                        d = EGA_CYAN;
                        break;
                    case 2:
                        d = EGA_MAGENTA;
                        break;
                    case 3:
                        d = EGA_BLUE;
                        break;
                    case 4:
                        d = EGA_YELLOW;
                        break;
                    case 5:
                        d = EGA_GREEN;
                        break;
                    case 6:
                        d = EGA_RED;
                        break;
                    case 7:
                        d = EGA_BLACK;
                        break;
                }
                setpalette(c, d);        // update pc palette
            }
            else {
                switch ((vidpal[c] & 7) ^ 7) {
                    case 0:
                        d = EGA_BLACK;
                        break;
                    case 1:
                        d = EGA_RED;
                        break;
                    case 2:
                        d = EGA_GREEN;
                        break;
                    case 3:
                        d = EGA_YELLOW;
                        break;
                    case 4:
                        d = EGA_BLUE;
                        break;
                    case 5:
                        d = EGA_MAGENTA;
                        break;
                    case 6:
                        d = EGA_CYAN;
                        break;
                    case 7:
                        d = EGA_WHITE;
                        break;
                }
                setpalette(c, d);        // update pc palette
            }

        }
}
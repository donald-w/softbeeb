#include <stdio.h>
#include <stdlib.h>
#include "tc_conio.h"
#include "tc_graphics.h"
#include "tc_dos.h"
#include "header.h"
#include "screen.h"
#include "io.h"

void update_full_text_screen(void);

void (*update_screen)(void)=update_full_text_screen; // initialise update
																	  // screen function

uint screen_start=0x7C00;      	// init screen variables for teletext
uint ram_screen_start=0x7C00;    //
ubyte teletext=1;                //

											// Declare graphics variables..
ubyte disp_chars;                // displayed characters per row
ubyte colour_bits;               // bits per pixel


/*
	Teletext screen write function.
*/
void text_screen_byte(ubyte iobyte,uint address)
{
ubyte column=1;                          // init column (first column=1)
ubyte row=1;                             // init row    (first row=1)

if (address<screen_start) address+=0x400;// deal with hardware wrap-around
address-=screen_start;                   //

column+=address%40;                      // calculate column
row+=address/40;                         // calculate row
if (column==40&&row==25)      // if bottom right, return since, any putchar
	return;                    // in this position will scroll the screen

gotoxy(column,row);           // move pc cursor to correct position


switch(iobyte) {                 // convert pc characters to teletext
											// characters
	case '_':	iobyte='#'   ;break;
	case '[':   iobyte='\x1B';break; // some of these conversion are not
	case '\\':	iobyte='?';   break; // stricly correct, since no corresponding
	case ']':   iobyte='\x1A';break; // character in the pc set exists
	case '^':   iobyte='\x18';break; //
	case '`':	iobyte='?'   ;break; // notably the } characters is not
	case '#':	iobyte='?'   ;break; // converted to the proper 3/4 symbol
	case '{':	iobyte='?'   ;break;
	case '|':	iobyte='?'   ;break;
	case '}':	iobyte='}'   ;break;
	case '~':	iobyte='?'   ;break;
	case 0x7F:	iobyte='?'   ;break; // This conversion takes place because
											// 0x7F is used for the cursor when
											// copying
}

putch(iobyte);							// Ouput the character

update_cursor();                 // update to the correct position
											// as specified in the BBC hardware
}

/*
This function is used for graphics modes with 1 bit per pixel
*/
void screen_byte_1(ubyte iobyte,uint address)
{
uint x_cord=0;                 // init real x cordinate variable
uint y_cord=0;                 // init real y cordinate variable
uint virt_y_cord=0;            // init actual pc output y coordinate
uint bitsperline;              // variable holding bits per line

										 // deal with hardware wrap-around
if (address<screen_start) address+=(0x8000-ram_screen_start);
address-=screen_start;
bitsperline=crt_regs[1]<<3;    // calculate bits per line

y_cord=address/(bitsperline);  // calculate character row of address
y_cord*=8;                     // convert to pixel row
y_cord+=(address%8);           // add fraction of character to pixel row

virt_y_cord+=y_cord+y_cord/4;  // scale output to pc screen

x_cord=address%(bitsperline);  // calculate x coordinate
x_cord&=0xFFF8;                // mask off lower 3 bits

if (bitsperline==640)          // if an 80 character mode
{
putpixel(x_cord+0,virt_y_cord,(iobyte&0x80)?8:0);
putpixel(x_cord+1,virt_y_cord,(iobyte&0x40)?8:0);
putpixel(x_cord+2,virt_y_cord,(iobyte&0x20)?8:0);
putpixel(x_cord+3,virt_y_cord,(iobyte&0x10)?8:0);
putpixel(x_cord+4,virt_y_cord,(iobyte&0x08)?8:0);
putpixel(x_cord+5,virt_y_cord,(iobyte&0x04)?8:0);
putpixel(x_cord+6,virt_y_cord,(iobyte&0x02)?8:0);
putpixel(x_cord+7,virt_y_cord,(iobyte&0x01)?8:0);
}

else                           // if not an 80 character mode
{
x_cord<<=1;                    // scale output to fit pc screen
putpixel(x_cord+0,virt_y_cord,(iobyte&0x80)?8:0);
putpixel(x_cord+1,virt_y_cord,(iobyte&0x80)?8:0);
putpixel(x_cord+2,virt_y_cord,(iobyte&0x40)?8:0);
putpixel(x_cord+3,virt_y_cord,(iobyte&0x40)?8:0);
putpixel(x_cord+4,virt_y_cord,(iobyte&0x20)?8:0);
putpixel(x_cord+5,virt_y_cord,(iobyte&0x20)?8:0);
putpixel(x_cord+6,virt_y_cord,(iobyte&0x10)?8:0);
putpixel(x_cord+7,virt_y_cord,(iobyte&0x10)?8:0);
putpixel(x_cord+8,virt_y_cord,(iobyte&0x08)?8:0);
putpixel(x_cord+9,virt_y_cord,(iobyte&0x08)?8:0);
putpixel(x_cord+10,virt_y_cord,(iobyte&0x04)?8:0);
putpixel(x_cord+11,virt_y_cord,(iobyte&0x04)?8:0);
putpixel(x_cord+12,virt_y_cord,(iobyte&0x02)?8:0);
putpixel(x_cord+13,virt_y_cord,(iobyte&0x02)?8:0);
putpixel(x_cord+14,virt_y_cord,(iobyte&0x01)?8:0);
putpixel(x_cord+15,virt_y_cord,(iobyte&0x01)?8:0);
}


if (!(y_cord&3))               // if the vertical coordinate has been scaled
{
virt_y_cord--;                 // file in the extra pixels

if (bitsperline==640)          // if an 80 character mode
{
putpixel(x_cord+0,virt_y_cord,(iobyte&0x80)?8:0);
putpixel(x_cord+1,virt_y_cord,(iobyte&0x40)?8:0);
putpixel(x_cord+2,virt_y_cord,(iobyte&0x20)?8:0);
putpixel(x_cord+3,virt_y_cord,(iobyte&0x10)?8:0);
putpixel(x_cord+4,virt_y_cord,(iobyte&0x08)?8:0);
putpixel(x_cord+5,virt_y_cord,(iobyte&0x04)?8:0);
putpixel(x_cord+6,virt_y_cord,(iobyte&0x02)?8:0);
putpixel(x_cord+7,virt_y_cord,(iobyte&0x01)?8:0);
}

else                           // if not an 80 character mode
{
putpixel(x_cord+0,virt_y_cord,(iobyte&0x80)?8:0);
putpixel(x_cord+1,virt_y_cord,(iobyte&0x80)?8:0);
putpixel(x_cord+2,virt_y_cord,(iobyte&0x40)?8:0);
putpixel(x_cord+3,virt_y_cord,(iobyte&0x40)?8:0);
putpixel(x_cord+4,virt_y_cord,(iobyte&0x20)?8:0);
putpixel(x_cord+5,virt_y_cord,(iobyte&0x20)?8:0);
putpixel(x_cord+6,virt_y_cord,(iobyte&0x10)?8:0);
putpixel(x_cord+7,virt_y_cord,(iobyte&0x10)?8:0);
putpixel(x_cord+8,virt_y_cord,(iobyte&0x08)?8:0);
putpixel(x_cord+9,virt_y_cord,(iobyte&0x08)?8:0);
putpixel(x_cord+10,virt_y_cord,(iobyte&0x04)?8:0);
putpixel(x_cord+11,virt_y_cord,(iobyte&0x04)?8:0);
putpixel(x_cord+12,virt_y_cord,(iobyte&0x02)?8:0);
putpixel(x_cord+13,virt_y_cord,(iobyte&0x02)?8:0);
putpixel(x_cord+14,virt_y_cord,(iobyte&0x01)?8:0);
putpixel(x_cord+15,virt_y_cord,(iobyte&0x01)?8:0);
}

}

}

/*
Screen output function for 2 bits per pixels (4 colour) modes.
*/
void screen_byte_2(ubyte iobyte,uint address)
{
uint x_cord=0;         // general x coordinate variable
uint y_cord=0;         // general y coordinate variable
uint virt_y_cord=0;    // vertically scaled pc y coordinate
uint bitsperline;      // bits per line variable
ubyte colpix1;         // colour of pixel 1
ubyte colpix2;         // colour of pixel 2
ubyte colpix3;         // colour of pixel 3
ubyte colpix4;         // colour of pixel 4

							  // deal with hardware wraparound
if (address<screen_start) address+=(0x8000-ram_screen_start);
address-=screen_start;

bitsperline=8*crt_regs[1];  // calculate bits per scanline

y_cord=address/bitsperline; // calculate character row
y_cord*=8;                  // convert to pixel row
y_cord+=(address%8);        // add fraction of character offset

virt_y_cord+=y_cord+y_cord/4; // scale output to fit pc screen

x_cord=address%bitsperline;  // calculate x character column
x_cord&=0xFFF8;              // mask off lower 3 bits

switch(iobyte&0x88)                // calculate colour of pixel 1
{
	case 0x00: colpix1=0x00;break;
	case 0x08: colpix1=0x02;break;
	case 0x80: colpix1=0x08;break;
	case 0x88: colpix1=0x0A;break;
}

switch(iobyte&0x44)                // calculate colour of pixel 2
{
	case 0x00: colpix2=0x00;break;
	case 0x04: colpix2=0x02;break;
	case 0x40: colpix2=0x08;break;
	case 0x44: colpix2=0x0A;break;
}

switch(iobyte&0x22)                // calculate colour of pixel 3
{
	case 0x00: colpix3=0x00;break;
	case 0x02: colpix3=0x02;break;
	case 0x20: colpix3=0x08;break;
	case 0x22: colpix3=0x0A;break;
}

switch(iobyte&0x11)                // calcalate colour of pixel 4
{
	case 0x00: colpix4=0x00;break;
	case 0x01: colpix4=0x02;break;
	case 0x10: colpix4=0x08;break;
	case 0x11: colpix4=0x0A;break;
}


if (bitsperline==640)              // if this is a 40 character mode
{
putpixel(x_cord+0,virt_y_cord,colpix1);
putpixel(x_cord+1,virt_y_cord,colpix1);
putpixel(x_cord+2,virt_y_cord,colpix2);
putpixel(x_cord+3,virt_y_cord,colpix2);
putpixel(x_cord+4,virt_y_cord,colpix3);
putpixel(x_cord+5,virt_y_cord,colpix3);
putpixel(x_cord+6,virt_y_cord,colpix4);
putpixel(x_cord+7,virt_y_cord,colpix4);
}
else                               // if this is not a 40 character mode
{
x_cord<<=1;                        // scale the x coorinate to fit the scren
putpixel(x_cord+0,virt_y_cord,colpix1);
putpixel(x_cord+1,virt_y_cord,colpix1);
putpixel(x_cord+2,virt_y_cord,colpix1);
putpixel(x_cord+3,virt_y_cord,colpix1);
putpixel(x_cord+4,virt_y_cord,colpix2);
putpixel(x_cord+5,virt_y_cord,colpix2);
putpixel(x_cord+6,virt_y_cord,colpix2);
putpixel(x_cord+7,virt_y_cord,colpix2);
putpixel(x_cord+8,virt_y_cord,colpix3);
putpixel(x_cord+9,virt_y_cord,colpix3);
putpixel(x_cord+10,virt_y_cord,colpix3);
putpixel(x_cord+11,virt_y_cord,colpix3);
putpixel(x_cord+12,virt_y_cord,colpix4);
putpixel(x_cord+13,virt_y_cord,colpix4);
putpixel(x_cord+14,virt_y_cord,colpix4);
putpixel(x_cord+15,virt_y_cord,colpix4);
}

if (!(y_cord&3))                      // if the y coordinate was scaled,
{
virt_y_cord--;                        // put the pixels on the original line

if (bitsperline==640)                 // if this is a 40 character mode
{
putpixel(x_cord+0,virt_y_cord,colpix1);
putpixel(x_cord+1,virt_y_cord,colpix1);
putpixel(x_cord+2,virt_y_cord,colpix2);
putpixel(x_cord+3,virt_y_cord,colpix2);
putpixel(x_cord+4,virt_y_cord,colpix3);
putpixel(x_cord+5,virt_y_cord,colpix3);
putpixel(x_cord+6,virt_y_cord,colpix4);
putpixel(x_cord+7,virt_y_cord,colpix4);
}
else                                  // if this is not a 40 character mode
{
putpixel(x_cord+0,virt_y_cord,colpix1);
putpixel(x_cord+1,virt_y_cord,colpix1);
putpixel(x_cord+2,virt_y_cord,colpix1);
putpixel(x_cord+3,virt_y_cord,colpix1);
putpixel(x_cord+4,virt_y_cord,colpix2);
putpixel(x_cord+5,virt_y_cord,colpix2);
putpixel(x_cord+6,virt_y_cord,colpix2);
putpixel(x_cord+7,virt_y_cord,colpix2);
putpixel(x_cord+8,virt_y_cord,colpix3);
putpixel(x_cord+9,virt_y_cord,colpix3);
putpixel(x_cord+10,virt_y_cord,colpix3);
putpixel(x_cord+11,virt_y_cord,colpix3);
putpixel(x_cord+12,virt_y_cord,colpix4);
putpixel(x_cord+13,virt_y_cord,colpix4);
putpixel(x_cord+14,virt_y_cord,colpix4);
putpixel(x_cord+15,virt_y_cord,colpix4);
}

}

}

/*
Screen output function for 16 colour modes
*/
void screen_byte_4(ubyte iobyte,uint address)
{
uint x_cord=0;                       // general x coordinate
uint y_cord=0;                       // general y coordinate
uint virt_y_cord=0;                  // scaled pc y coordinate
uint bitsperline;                    // bits per scaline

ubyte colpix1;                       // holds the colour of pixel 1
ubyte colpix2;                       // holds the colour of pixel 2

bitsperline=8*crt_regs[1];           // calculate bits per line

												 // deal with hardware wraparound
if (address<screen_start) address+=(0x8000-ram_screen_start);
address-=screen_start;
y_cord=address/bitsperline;          // calculate the character row
y_cord*=8;                           // convert to the pixel row
y_cord+=(address%8);                 // add fraction of character offset
virt_y_cord+=y_cord+y_cord/4;        // scale output to fit screen


x_cord=address%bitsperline;          // calcalate the character column
x_cord&=0xFFF8;                      // mask off the lower 3 bits

switch(iobyte&0xAA)						 // calculate the colour of pixel 1
{
	case  0x00: colpix1=0x0; break;
	case  0x02: colpix1=0x1; break;
	case  0x08: colpix1=0x2; break;
	case  0x0A: colpix1=0x3; break;
	case  0x20: colpix1=0x4; break;
	case  0x22: colpix1=0x5; break;
	case  0x28: colpix1=0x6; break;
	case  0x2A: colpix1=0x7; break;
	case  0x80: colpix1=0x8; break;
	case  0x82: colpix1=0x9; break;
	case  0x88: colpix1=0xA; break;
	case  0x8A: colpix1=0xB; break;
	case  0xA0: colpix1=0xC; break;
	case  0xA2: colpix1=0xD; break;
	case  0xA8: colpix1=0xE; break;
	case  0xAA: colpix1=0xF; break;
}

switch(iobyte&0x55)                 // calculate the colour of pixel 2
{
	case  0x00: colpix2=0x0; break;
	case  0x01: colpix2=0x1; break;
	case  0x04: colpix2=0x2; break;
	case  0x05: colpix2=0x3; break;
	case  0x10: colpix2=0x4; break;
	case  0x11: colpix2=0x5; break;
	case  0x14: colpix2=0x6; break;
	case  0x15: colpix2=0x7; break;
	case  0x40: colpix2=0x8; break;
	case  0x41: colpix2=0x9; break;
	case  0x44: colpix2=0xA; break;
	case  0x45: colpix2=0xB; break;
	case  0x50: colpix2=0xC; break;
	case  0x51: colpix2=0xD; break;
	case  0x54: colpix2=0xE; break;
	case  0x55: colpix2=0xF; break;
}
														// only a twenty column mode is
														// officially supported,
														// so no need to scale output
putpixel(x_cord+0,virt_y_cord,colpix1);
putpixel(x_cord+1,virt_y_cord,colpix1);
putpixel(x_cord+2,virt_y_cord,colpix1);
putpixel(x_cord+3,virt_y_cord,colpix1);
putpixel(x_cord+4,virt_y_cord,colpix2);
putpixel(x_cord+5,virt_y_cord,colpix2);
putpixel(x_cord+6,virt_y_cord,colpix2);
putpixel(x_cord+7,virt_y_cord,colpix2);

if (!(y_cord&3))                          // if the y_cord was scaled,
														// fill in the pixels for the
{                                         // original line
virt_y_cord--;
putpixel(x_cord+0,virt_y_cord,colpix1);
putpixel(x_cord+1,virt_y_cord,colpix1);
putpixel(x_cord+2,virt_y_cord,colpix1);
putpixel(x_cord+3,virt_y_cord,colpix1);
putpixel(x_cord+4,virt_y_cord,colpix2);
putpixel(x_cord+5,virt_y_cord,colpix2);
putpixel(x_cord+6,virt_y_cord,colpix2);
putpixel(x_cord+7,virt_y_cord,colpix2);
}
}



void update_cursor(void)
{
uint cursor_add;                   // cursor address
uint cursor_wid;                   // cursor width in bytes
static uint old_cursor_add;        // static variable to hold the
static uint old_cursor_wid;        // previous cursor data,
											  // so that it may be erased

if (teletext) {                    // if in teletext mode
	cursor_add=crt_regs[14]*0x100+crt_regs[15];
	cursor_add^=0x2000;
	cursor_add+=0x7400;             // calculate actual address
											  // move the cursor to the relevant place
	if (cursor_add<screen_start) cursor_add+=0x400;// deal with hardware wraparound
	cursor_add-=screen_start;

	gotoxy(cursor_add%40+1,cursor_add/40+1);
}

else	{									  // if in a graphics mode
	cursor_add=0x100*crt_regs[14]+crt_regs[15];
	cursor_add<<=3;                 // calculte the actual address
											  // deal with hardware wrap around
	if (cursor_add>=0x8000) cursor_add-=0x8000-ram_screen_start;

	switch (vid_con_reg&0x60) {     // get the cursor width in bytes
		case 0x60: cursor_wid=4;break;
		case 0x40: cursor_wid=2;break;
		case 0x20: puts("Invalid Cursor");exit(1);break;
		case 0x00: cursor_wid=1;break;
	}


	switch (old_cursor_wid) {       // remove the old cursor

		case 4:	screen_byte_P(RAM[old_cursor_add+0x1F],old_cursor_add+0x1F);
					screen_byte_P(RAM[old_cursor_add+0x17],old_cursor_add+0x17);

		case 2: 	screen_byte_P(RAM[old_cursor_add+0x0F],old_cursor_add+0x0F);
		case 1:	screen_byte_P(RAM[old_cursor_add+0x07],old_cursor_add+0x07);
	}

	switch (cursor_wid) {           // position the new cursor

		case 4: 	screen_byte_P(0xFF,cursor_add+0x1F);
					screen_byte_P(0xFF,cursor_add+0x17);
		case 2:	screen_byte_P(0xFF,cursor_add+0x0F);
		case 1:	screen_byte_P(0xFF,cursor_add+0x07);
	}


	old_cursor_wid=cursor_wid;      // store a copy of the current cursor
	old_cursor_add=cursor_add;      //
}

}


// The name says it all really, used to update
// the teletext screen 	after a hardware scroll
void update_full_text_screen(void)
{
uint c;
uint d;

_setcursortype(_NOCURSOR);     // turn off the cursor to stop flickering

c=screen_start;

for(d=0;d<999;d++,c++) {       // work through every byte
	if (c==0x8000) c=0x7C00;    // deal with hardware wraparound
	text_screen_byte(RAM[c],c); // output the current byte
}

_setcursortype(_NORMALCURSOR); // turn the cursor back on again
}

/*
This function does not really do anywork, it calls the relevant function
*/
void update_graphics_screen(void)
{
switch(colour_bits) {
	case 1: mono_update();break;
	case 2: four_update();break;
	case 4: sixt_update();break;
	default: puts("Error, unknown number of colours");exit(1);
	}
}


/*
	Called upon a negative transition of the teletext
	select bit in the video ULA Sheila Address 0xFE20
*/
void pixel_vid_init(void)
{
setgraphmode(VGAMED);   // Change mode from text to graphics
set_colour_bits();      // initialise for the relevant colour mode
set_screen_start();     // initialise the screen start
update_screen=update_graphics_screen; // point the screen update function
return;
}

/*
	Called upon a positive transition of the teletext
	select bit in the video ULA
*/
void teletext_init(void)
{
restorecrtmode();        // Change to a text mode
ram_screen_start=0x7C00; // Set screen start
screen_byte_P=text_screen_byte; // Point screen access function
update_screen=update_full_text_screen; // point screen update function
textcolor(WHITE);                      // set text to white colour
return;
}

/*
Called only once, during system initialisation
*/
void graphics_init(void)
{
signed int errorcode;
printf("Init graphics system...");       // display pretty startup info
errorcode=registerfarbgidriver(EGAVGA_driver_far); // initialise the graphics
if (errorcode<0)                                   // system. check for error
{
	printf("\nUnable to Initialise Graphics\n");		// if unable to init,
	printf("This software requires VGA/EGA");       // print error
	exit(1);                                        // and exit
}

screen_byte_P=text_screen_byte;     // point screen access function for text
printf("Done\n");                   // display pretty startup info

}

void set_colour_bits(void)          // initialise colour functions
{
if (!teletext)                               // only if in graphics mode
switch(colour_bits=crt_regs[1]/disp_chars)   // calc bits per pixel
{
	case 1: screen_byte_P=screen_byte_1; break; // point screen access func.
	case 2: screen_byte_P=screen_byte_2; break;
	case 4: screen_byte_P=screen_byte_4; break;
}

}

/*
Update ram screen start, used in putbyte() (to decide if output is directed
at the screen) and elsewhere
*/
void set_screen_start(void)
{
ubyte screen_latch=0;

	screen_latch=s_via_latch[4]+(2*s_via_latch[5]); // read from the system
																	// via latch

	// There is a possibly bug here:  the system via latch does not
	// correspond to values given in the Advanced User Guide or the
	// Advanced Master Series Guide.
	// However, inaccuracies would probably have gone unnoticed, since
	// these bits are only ever altered by the OS and cannot be read by the
	// user

	switch (screen_latch)
	{
		 case 0x000:	ram_screen_start=0x4000; break;
		 case 0x008:	ram_screen_start=0x6000; break;
		 case 0x018:	ram_screen_start=0x5800; break;
		 case 0x010:	ram_screen_start=0x3000; break;
			 default:	puts("Fatal error in detecting screen boundary");
							exit(1);
	}
}

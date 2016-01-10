#include <stdio.h>
#include <stdlib.h>
#include "tc_dos.h"
#include "tc_conio.h"
#include "tc_graphics.h"
#include "tc_alloc.h"
#include "header.h"
#include "screen.h"
#include "mnemonic.h"


ubyte debuginfo=0;

ubyte pic_temp_ram[0x8000];


//char far *graph_ptr;
char *graph_ptr;

void titlepic(void);
FILE *output;

void system_init(void)
{
	int gdriver=VGA,gmode=VGAMED;
	output=fopen("./output.dat","wb");

//	unsigned int c;
//	unsigned char test[]={END};

	textmode(C80);     	// intialise the screen mode
	clrscr();            // clear the screen

	textbackground(BLUE);//display pretty startup screen
	textcolor(YELLOW);
	cprintf(" Softbeeb Initialisation \n\n\r");
	textbackground(BLACK);
	textbackground(WHITE);
	init_decode();       // initialise the 6502 processor emulator
	init_mem();          // initialise the memory
	rinit_io();				// initialize the read io system
	winit_io();          // initialise the write io system
	graphics_init();		// initialise the graphics system
	textmode(C40);
	initgraph(&gdriver,&gmode,"");
	titlepic();
	restorecrtmode();
	textcolor(WHITE);
	printf("_________________________\n\n"); // display end of startup line
	clrscr();
/*
	// instructions for testing purposes

	pc=0x4000;
	for (c=0;(c<sizeof(test)+1);c++) putbyte(test[c2],pc+c2);

	// ***********  SETUP FOR TESTS GOES HERE *********

	// ************************************************
*/

}

void show_regs(void)      // debugging procedure to display misc status
								  // and registers.
{
fprintf(output,"\nPC=%4X A =%02X ",pc,acc);
fprintf(output,"X=%02X Y=%02X S=%02X:%02X ",x_reg,y_reg,sp,RAM[sp+0x101]);
fprintf(output,"n%dv%db%dd%d",((except)||(result_f&0x80))?1:0,ovr_f==0?0:1,brk_f==0?0:1,dec_f==0?0:1);
fprintf(output,"id%dz%dc%d",intd_f==0?0:1,(except||result_f)?0:1,carry_f==0?0:1);
fprintf(output," %18s D=%2X%02X",mnemonic[getbyte(pc)],getbyte(pc+2),getbyte(pc+1));
fprintf(output," %02X",getbyte(pc));
}

void show_screen(void)           // only used in the waitkey function
{
unsigned int c;

for (c=0x7C00;c<0x8000;c++)
	{
	if (!((c+16)%40)) printf("\n");
	putchar(RAM[c]);
	}
}

/*
Waits for a keypress, then discards it. Useful as a pause procedure.
*/
void waitkey(void)
{
int c;
if (debuginfo)             // Check if debugging is enabled
{
	c=getchar();
	switch (c)
	{
	case 'i': irq(); break;           // if "I" is pressed, generated an IRQ
	case 'r': debuginfo=0;break;      // turn off debugging and run freely
	case 's': show_screen(); break;   // if "s", display a mode 7 screen
	case 'x': decode[0xFF](); break;  // if "x", exit!
	}
}
}



void titlepic(void)	{                     // the title picture display
														 // display function
	int horz,vert;
	int vvert=0;
	uint c,d=0;
	uint red=63,green=0,blue=0;

	FILE *title;

	if ((title=fopen("data\\title.bmp","rb"))==NULL) {
		printf("Titlepic fileopen Error");
		coniogetch();                            // open the picture file
		exit(1);
	};

	pokeb(0x0040,0x0017,peekb(0x0040,0x0017)|0x40); // turn on capslock

	for (c=0x8000;c;c--) {                          // load the picture
	  pic_temp_ram[c-1]=getc(title);
	}

	for (c=0;c<16;c++) {                            // initialise the palette
		setrgbpalette(c,c<<2,c<<2,c<<2);             // for greyscale
		setpalette(c,c);
	}

	setrgbpalette(15,63,0,0);                       // set the red colour
	setpalette(15,15);

	for (vert=0,c=650;vert<200;vert++) {            // loop through each
		vvert++;                                     // pixel row

		if (vert&0x3) vvert++;                       // scale output to fit
																	// screen
		for (horz=640;horz;horz-=4,c++) {

			putpixel(horz+0,vvert,pic_temp_ram[c]>>4);
			putpixel(horz+1,vvert,pic_temp_ram[c]>>4);
			putpixel(horz+2,vvert,pic_temp_ram[c]&0xF);
			putpixel(horz+3,vvert,pic_temp_ram[c]&0xF);

			if (vert&0x3) { // if y cord has been scaled, draw on original line
				vvert--;
				putpixel(horz+0,vvert,pic_temp_ram[c]>>4);
				putpixel(horz+1,vvert,pic_temp_ram[c]>>4);
				putpixel(horz+2,vvert,pic_temp_ram[c]&0xF);
				putpixel(horz+3,vvert,pic_temp_ram[c]&0xF);
				vvert++;
			}
		if (kbhit()) {coniogetch(); return;}  // wait for keypress
		}
	}

	for(;;)                             // cycle colours until
		for (c=0;c<=4;c++)
			for (d=0;d<15;d++) {
				switch(c) {
					case 0:green+=4;	break;
					case 1:red-=4;		break;
					case 2:blue+=4;	break;
					case 3:green-=4;	break;
					case 4:blue-=4;red+=4;break;
				}
			setrgbpalette(15,red,green,blue);
			setpalette(15,15);
			delay(10);                        // delay deliberately
			if (kbhit()) {coniogetch(); return;}   // until keypressed
			}
}
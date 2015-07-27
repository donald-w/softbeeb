#include <stdio.h>
#include "tc_conio.h"
#include "tc_dos.h"
#include <stdlib.h>
#include "tc_graphics.h"
#include <ctype.h>
#include "header.h"
#include "io.h"
#include "screen.h"


void monitor(void);

void monitor_call(void)  {            // Is called when home is pressed

	if (teletext) {
		setgraphmode(VGAMED);
		monitor();
		flash();
		restorecrtmode();
		update_screen();
	}
	if (!teletext) {
		setvisualpage(1);
		setactivepage(1);
		monitor();
		flash();
		setvisualpage(0);
		setactivepage(0);
		}

}

void  monitor(void) {
	uint init_colour;
	uint c=0;  // misc counters
	uint d=0;  // misc counters

	init_colour=getcolor();

	setpalette(11,EGA_LIGHTRED);
	setpalette(12,EGA_BLACK);
	setpalette(13,EGA_WHITE);
	setpalette(14,EGA_BLUE);
	setpalette(15,EGA_YELLOW);
	setbkcolor(BLACK);
	cleardevice();

	settextstyle(DEFAULT_FONT,HORIZ_DIR,3);
	setcolor(YELLOW);
	outtextxy(10,10,"Options");
	setcolor(RED);
	outtextxy(12,12,"Options");
	setcolor(BLUE);
	outtextxy(14,14,"Options");

	setcolor(WHITE);

	settextstyle(DEFAULT_FONT,HORIZ_DIR,1);

/* The area 0,300,0,350 is to be left clear for further option prompts */

/*****************************************
******************************************
*****************************************/

	outtextxy(0,60,"X : Quit Softbeeb");
	outtextxy(0,70,"B : Simulate Break Key (Reset)");
	outtextxy(0,80,"R : Redraw Graphics Screen");
	outtextxy(0,90,"S : PC Speaker Sound Emulation");
	outtextxy(0,110,"Press the key of your choice");
	outtextxy(50,200," Press `return' to continue the Emulation");

	while(kbhit()) coniogetch();

	for(;c!='\r';c=coniogetch()) {
		switch(tolower(c)) {

			case 'x':
						 setcolor(YELLOW);
						 outtextxy(0,305,"Are you sure you want to quit? Y/N");
						 if(tolower(coniogetch())=='y') quit_prog();
						 else {
							setcolor(BLACK);
							outtextxy(0,305,"Are you sure you want to quit? Y/N");
							}
						 setcolor(WHITE);
						 break;
			case 'r':
						 if (!teletext) {
							setcolor(YELLOW);
							outtextxy(0,305,"Redrawing Graphics Screen...");
							d=screen_start;
							setactivepage(0);
							for(c=0;c<(0x8000-ram_screen_start);c++) {
								if (d==0x8000) d=ram_screen_start;
								screen_byte_P(RAM[d],d);
								d++;
								update_cursor();
							}
							setactivepage(1);
						 }
						 setcolor(BLACK);
						 outtextxy(0,305,"Redrawing Graphics Screen...");
						 setcolor(WHITE);
						 break;
			case 'b': setcolor(YELLOW);
						 outtextxy(0,305,"Press Y to confirm reset");
						 if(tolower(coniogetch())=='y')
							pc=0xD9CD;
						 setcolor(BLACK);
						 outtextxy(0,305,"Press Y to confirm reset");
						 setcolor(WHITE);
						 break;

			case 's': setcolor(YELLOW);
						 outtextxy(0,305,"Do you want sound? Y/N");
						 if(tolower(coniogetch())=='y') {
						 soundyesno=1;
						 update_sound();
						 }
						 else {
						 soundyesno=0;
						 nosound();
						 }
						 setcolor(BLACK);
						 outtextxy(0,305,"Do you want sound? Y/N");
						 setcolor(WHITE);
						 break;


		}


	}
	setbkcolor(0);
	setcolor(init_colour);
}


void quit_prog() {

closegraph();
textmode(C80);
pokeb(0,0x417,peekb(0,0x417)&0xBF);
puts("Thankyou for using `SOFTBEEB'\n");
puts("Have a nice day {;-)\n");
exit(0);
}
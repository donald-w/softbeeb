#include <stdio.h>
#include <stdlib.h>
#include "tc_conio.h"
#include "tc_graphics.h"
#include "tc_dos.h"
#include "header.h"
#include "screen.h"
#include "io.h"


// Used to scroll a mono screen up 1 line


void mono_update(void) {
	getimage(0,10,639,96,pic_temp_ram);
	putimage(0,0,pic_temp_ram,0);

	getimage(0,97,639,184,pic_temp_ram);
	putimage(0,87,pic_temp_ram,0);

	getimage(0,185,639,271,pic_temp_ram);
	putimage(0,175,pic_temp_ram,0);

	getimage(0,272,639,350,pic_temp_ram);
	putimage(0,262,pic_temp_ram,0);
}

// Used to scroll a four colour screen up 1 line


void four_update(void) {
	getimage(0,10,639,96,pic_temp_ram);
	putimage(0,0,pic_temp_ram,0);

	getimage(0,97,639,184,pic_temp_ram);
	putimage(0,87,pic_temp_ram,0);

	getimage(0,185,639,271,pic_temp_ram);
	putimage(0,175,pic_temp_ram,0);

	getimage(0,272,639,350,pic_temp_ram);
	putimage(0,262,pic_temp_ram,0);
}


// Used to scroll a sixteen colour screen up 1 line

void sixt_update(void) {
	getimage(0,10,639,96,pic_temp_ram);
	putimage(0,0,pic_temp_ram,0);

	getimage(0,97,639,184,pic_temp_ram);
	putimage(0,87,pic_temp_ram,0);

	getimage(0,185,639,271,pic_temp_ram);
	putimage(0,175,pic_temp_ram,0);

	getimage(0,272,639,350,pic_temp_ram);
	putimage(0,262,pic_temp_ram,0);
}
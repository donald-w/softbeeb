#include <stdio.h>
#include <stdlib.h>
#include "header.h"
#include "screen.h"

unsigned char lang_ROM[0x4000]; // reserve 16K for the language ROM data
unsigned char OS_ROM[0x4000];   // reserve 16K for the OS_ROM data

void (*screen_byte_P)(ubyte,uint);


/*
This function is _always_ used to access an address unless it is in the
zero page or the stack, then it may be directly accessed in the RAM[]
*/
ubyte getbyte(uint address)
{

if (address<0x8000) return RAM[address];
					// If address is below the 32K mark access the RAM


if (address<0xC000) return (romsel==0xC?lang_ROM[address-0x8000]:(ubyte)0xFF);
					// If address is below the 48K mark access the language ROM
if ((address>0xFEFF)||(address<0xFC00)) return OS_ROM[address-0xC000];
					// If address is outside the 3 pages of memmapped io,
					// access the OS_ROM
return ((address&0xFF00)==0xFE00)?rsheila[(char)(address)]():(ubyte)0xff;

}					// the address must be in the 3 pages of memmapped io.


/*
This functions is always used to write to an address unless it is
known for certain to exist with the zeropage/stack range.
(when it may be accessed directly through the RAM[]).
*/

void putbyte(ubyte byte, uint address)
{
if (address<0x8000)
	{
	if (address>=ram_screen_start)  // if screen RAM
		screen_byte_P(byte,address); // output screen data

	RAM[address]=byte;              // update RAM
	}

else if ((address&0xFF00)==0xFE00) wsheila[(char)(address)](byte);

else if ((address<0xFC00)||(address>0xFEFF))
	{
	// Since this is an impossible situation, a crash probably has occured
	// therefore display error and exit
	printf("\nROM write attempt at %X",address);
	exit(1);
	}
}




void init_mem(void)     // initialise the 6502 memory space
{
	FILE *fp1;           // misc files pointers
	FILE *fp2;
	unsigned int c;      // misc loop counter

	for (c=0;c<0x8000;c++)
	{
		RAM[c]=0;         // intiialise the 6502 emulator RAM
	}

	printf("Opening ROM Image files..."); // display pretty startup status
	if ((fp1=fopen("roms/os.bin","rb"))==NULL)
	{
		printf("\nError: Could not open OS_ROM image.");
		exit(1);    // exit abnormally
	}
	if ((fp2=fopen("roms/basic.bin","rb"))==NULL)
	{
		printf("\nError: Could not open lang_ROM image.");
		exit(1);    // exit abnormally
	}
	printf("Done\n"); // display pretty status status


	printf("Reading ROM Image data..."); // display pretty startup status
	for (c=0;c<0x4000;c++)   // initialise the ROM blocks
	{
		OS_ROM[c]=(ubyte)getc(fp1);    // read data from the image files
		lang_ROM[c]=(ubyte)getc(fp2);  //

		if ((ferror(fp2))||(ferror(fp1)))
		{
			printf("\nFatal Error:- Rom image file error");
			exit(1);          // exit abnormally if there is a file error
		}
	}
	printf("Done\n");    // display pretty startup status

	printf("Closing ROM Image files...");
	fclose(fp1);
	fclose(fp2);          // close the image files
	printf("Done\n");

}

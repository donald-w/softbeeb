#include <stdio.h>
#include "header.h"
#include "io.h"
#include "reg_name.h"

#define CLKFREQ 850
						  // compile time instructions between interrupt counter

/*

Softbeeb- A BBC Model B Microcomputer for the PC
Code begun on 16th December 1995
Designed for Compilation on Turbo C++ For DOS V3.0


Written for CSYS Computing Studies
Copyright Donald Walker

*/

ubyte RAM[0x8000];  			 // the BBC's ram
uint pc=0xD9CD;     			 // where the processor starts execution

ubyte acc =0;               //
ubyte x_reg =0;             //  initialise registers.
ubyte y_reg =0;             //

ubyte sp =0xFF;             // initialise the stack pointer.
ubyte dyn_p =0;         	 // the dynamic processor status
									 // update_dyn_p() must be executed
									 // before use of this variable

/* update_dyn_p uses these flags to construct the
	full processor status byte when required i.e.
	during BRKs, IRQs or PHP's */

ubyte result_f=0;          // dual purpose flag holding neg and zero.
ubyte ovr_f=0;             // sign overflow flag
ubyte brk_f=1;             // set to zero when an IRQ occurs
ubyte dec_f=0;             // flag indicating decimal mode operation
									// (current unsupported)
ubyte intd_f=0;            // set when interrupts are disabled
ubyte carry_f=0;           // arithmetic carry flag
ubyte except=0;            // set for a negative zero condition


int main()
{
uint clock;								// local clock counter
ubyte ir=0;								// holds current instruction

system_init();                   // initialise everything.

for (clock=CLKFREQ;;clock--)		// interrupt generation timer
	{
		ir=getbyte(pc++);				// fetch instruction
		decode[ir]();              // decode and execute the instructon


		if (!clock)                // if an interrupt is due
		{
			if (!intd_f)            // and if interrupts are enabled,
			{
			gen_irq();              // call the interrupt  handler
			clock=CLKFREQ;          // and reset interrupt generation timer
			}
			else(clock++);          // if interrupts are disabled,
											// increment the clock timer so that
											// an interrupt occurs when enabled
		}
	}
}
===========================================
-------------------------------------------

-------------------------------------------
===========================================
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

unsigned char getbyte(unsigned int address)
{

if (address<0x8000) return RAM[address];
					// If address is below the 32K mark access the RAM


if (address<0xC000) return (romsel==0xC?lang_ROM[address-0x8000]:0xFF);
					// If address is below the 48K mark access the language ROM
if ((address>0xFEFF)||(address<0xFC00)) return OS_ROM[address-0xC000];
					// If address is outside the 3 pages of memmapped io,
					// access the OS_ROM
return ((address&0xFF00)==0xFE00)?rsheila[(char)(address)]():0xff;

}					// the address must be in the 3 pages of memmapped io.


/*
This functions is always used to write to an address unless it is
known for certain to exist with the zeropage/stack range.
(when it may be accessed directly through the RAM[]).
*/

void putbyte (unsigned char byte, unsigned int address)
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
	if ((fp1=fopen("data\\os.bin","rb"))==NULL)
	{
		printf("\nError: Could not open OS_ROM image.");
		exit(1);    // exit abnormally
	}
	if ((fp2=fopen("data\\basic.bin","rb"))==NULL)
	{
		printf("\nError: Could not open lang_ROM image.");
		exit(1);    // exit abnormally
	}
	printf("Done\n"); // display pretty status status


	printf("Reading ROM Image data..."); // display pretty startup status
	for (c=0;c<0x4000;c++)   // initialise the ROM blocks
	{
		OS_ROM[c]=getc(fp1);    // read data from the image files
		lang_ROM[c]=getc(fp2);  //

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
===========================================
-------------------------------------------

-------------------------------------------
===========================================
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "header.h"

#define popbyte() (RAM[0x100+(++sp)])
#define pushbyte(A) (RAM[0x100+sp--]=(A))

#define CLE except=0

void non_opcode(void);  		 // prototype for function occuring when
										 // an illegal opcode is executed
void update_flags(void);
void update_dyn_p(void);       // builds a valid status byte for
										 // stack pushing

ubyte temp1 =0;       //
ubyte temp2 =0;       // Misc scratchpad variables for use in
ubyte temp3 =0;       // decode functions
uint tempint=0;

void (*decode[256])(void); // array of pointers to decode functions

/*

The decode logic for individual instructions follows
they are not necessarily individually commented
since they are generally self explanatory.

However some details are common.
they take no arguments and return no values
operating on global data.
However they make extensive use of subfunctions.
The name of each function is constructed of the
BBC Basic mnemonic and the opcode in hex

Due to the possibility of the 'BIT' instruction creating a negative zero
condition, and both the negative and zero flags being contained in
one variable, in which this condition is unrepresentable, a pseudo flag
called except exists. Any instruction which alters the negative and
zero flags must clear the except flag using the CLE #define. All
instructions which test the negative or zero flags must also check except
(where a non zero value indicates a negative zero condition

*/


void brk_00(void)
{
brk_f=1;
pc++;
pushbyte(pc/0x100);
pushbyte(pc%0x100);
update_dyn_p();
pushbyte(dyn_p);
intd_f=1;
pc=getbyte(0xFFFE)+(0x100*getbyte(0xFFFF));
}

void ora_01(void)
{
CLE;
temp1=(getbyte(pc)+x_reg);
acc|=getbyte(RAM[temp1]+0x100*RAM[(char)temp1+1]);
result_f=acc;
pc++;
}

void ora_05(void)
{
CLE;
acc|=RAM[getbyte(pc)];
result_f=acc;
pc++;
}

void asl_06(void)
{
CLE;
result_f=RAM[temp2=getbyte(pc)];
carry_f=result_f&0x80;
result_f<<=1;
RAM[temp2]=result_f;
pc++;
}

void php_08(void)
{
update_dyn_p();
pushbyte(dyn_p);
}

void ora_09(void)
{
CLE;
acc|=getbyte(pc);
result_f=acc;
pc++;
}

void asl_0A(void)
{
CLE;
carry_f=acc&0x80;
acc<<=1;
result_f=acc;
}

void ora_0D(void)
{
CLE;
result_f=acc|=getbyte(getbyte(pc)+0x100*getbyte(pc+1));
pc+=2;
}

void asl_0E(void)
{
CLE;
tempint=getbyte(pc)+0x100*getbyte(pc+1);
result_f=getbyte(tempint);

carry_f=result_f&0x80;
result_f<<=1;
putbyte(result_f,tempint);
pc+=2;
}

void bpl_10(void)
{
pc++;
if (except) return;
if (!(result_f&0x80)) pc+=(signed char)(getbyte(pc-1));
}

void ora_11(void)
{
CLE;
temp1=getbyte(pc);

acc|=getbyte((RAM[temp1]+0x100*RAM[temp1+1])+(ubyte)y_reg);
result_f=acc;
pc++;
}

void ora_15(void)
{
CLE;
acc|=RAM[(ubyte)(getbyte(pc)+(ubyte)x_reg)];
result_f=acc;
pc++;
}

void asl_16(void)
{
CLE;
temp2=(getbyte(pc)+(ubyte)x_reg);
result_f=RAM[temp2];

carry_f=result_f&0x80;
result_f<<=1;

RAM[temp2]=result_f;
pc++;
}

void clc_18(void)
{
carry_f=0;
}

void ora_19(void)
{
CLE;
acc|=getbyte(getbyte(pc)+(0x100*getbyte(pc+1))+y_reg);
result_f=acc;
pc+=2;
}

void ora_1D(void)
{
CLE;
acc|=getbyte(getbyte(pc)+(0x100*getbyte(pc+1))+x_reg);
result_f=acc;
pc+=2;
}

void asl_1E(void)
{
CLE;
tempint=getbyte(pc)+0x100*getbyte(pc+1)+x_reg;
result_f=getbyte(tempint);

carry_f=result_f&0x80;
result_f<<=1;

putbyte(result_f,tempint);
pc+=2;
}

void jsr_20(void)
{
pc++;
pushbyte(pc/0x100);
pushbyte((ubyte)(pc));
pc=(getbyte(pc-1))+(0x100*(getbyte(pc)));
}


void and_21(void)
{
CLE;
temp1=(getbyte(pc)+x_reg);
acc&=getbyte(getbyte(temp1)+0x100*getbyte(temp1+1));
result_f=acc;
pc++;
}

void bit_24(void)
{
temp1=RAM[getbyte(pc)];
result_f=(acc&temp1);
result_f|=(temp1&0x80);
ovr_f=temp1&0x40;
if ((!(acc&temp1))&&(temp1&0x80)) except=1;
pc++;
}

void and_25(void)
{
CLE;
acc&=RAM[getbyte(pc)];
result_f=acc;
pc++;
}

void rol_26(void)
{

ubyte oldcar;
oldcar=carry_f;
CLE;
result_f=RAM[temp2=getbyte(pc)];

	carry_f=result_f&0x80;
	result_f<<=1;
	if (oldcar) result_f++;

RAM[temp2]=result_f;
pc++;
}

void plp_28(void)
{
dyn_p=popbyte();
update_flags();
}


void and_29(void)
{
CLE;
acc&=getbyte(pc);
result_f=acc;
pc++;
}

void rol_2A(void)
{
ubyte oldcar;
CLE;
oldcar=carry_f;

carry_f=acc&0x80;
acc<<=1;
if (oldcar) acc++;
result_f=acc;
}

void bit_2C(void)
{
temp1=getbyte(getbyte(pc)+(0x100*getbyte(pc+1)));
result_f=(acc&temp1);
result_f|=(temp1&0x80);
ovr_f=temp1&0x40;
if ((!(acc&temp1))&&(temp1&0x80)) except=1;
pc+=2;
}

void and_2D(void)
{
CLE;
acc&=getbyte((getbyte(pc))+(0x100*getbyte(pc+1)));
result_f=acc;
pc+=2;
}

void rol_2E(void)
{

ubyte oldcar;
CLE;
oldcar=carry_f;

tempint=getbyte(pc)+0x100*getbyte(pc+1);
result_f=getbyte(tempint);

carry_f=result_f&0x80;
result_f<<=1;
if (oldcar) result_f++;

putbyte(result_f,tempint);
pc+=2;
}

void bmi_30(void)
{
pc++;
if (except) {pc+=(signed char)(getbyte(pc-1)); return; }
if (result_f&0x80) pc+=(signed char)(getbyte(pc-1));
}

void and_31(void)
{
CLE;
temp1=getbyte(pc);

acc&=getbyte((RAM[temp1]+0x100*RAM[temp1+1])+(ubyte)y_reg);
result_f=acc;
pc++;
}

void and_35(void)
{
CLE;
acc&=RAM[(ubyte)(getbyte(pc)+(ubyte)x_reg)];
result_f=acc;
pc++;
}

void rol_36(void)
{
ubyte oldcar;
CLE;
oldcar=carry_f;

result_f=RAM[temp2=getbyte(pc)+x_reg];
carry_f=result_f&0x80;
result_f<<=1;

if (oldcar) result_f++;

RAM[temp2]=result_f;
pc++;
}

void sec_38(void)
{
carry_f=1;
}

void and_39(void)
{
CLE;
acc&=getbyte(getbyte(pc)+(0x100*getbyte(pc+1))+y_reg);
result_f=acc;

pc+=2;
}

void and_3D(void)
{
CLE;
acc&=getbyte(getbyte(pc)+0x100*getbyte(pc+1)+y_reg);

result_f=acc;
pc+=2;
}

void rol_3E(void)
{
ubyte oldcar;
CLE;
oldcar=carry_f;

result_f=getbyte(tempint=(getbyte(pc))+(0x100*getbyte(pc+1))+x_reg);

carry_f=result_f&0x80;
result_f<<=1;
if (oldcar) result_f++;

putbyte(result_f,tempint);
pc+=2;
}

void rti_40(void)
{
dyn_p=popbyte();
update_flags();
brk_f=1;
pc=popbyte();
pc+=popbyte()*0x100;
}


void eor_41(void)
{
CLE;
temp1=getbyte(pc)+x_reg;
acc^=getbyte(RAM[temp1]+0x100*RAM[temp1+1]);
result_f=acc;
pc++;
}

void eor_45(void)
{
CLE;
acc^=RAM[getbyte(pc)];
result_f=acc;
pc++;
}

void lsr_46(void)
{
CLE;
result_f=RAM[temp1=getbyte(pc)];
carry_f=result_f&1;
result_f>>=1;
RAM[temp1]=result_f;
pc++;
}

void pha_48(void)
{
pushbyte(acc);
}

void eor_49(void)
{
CLE;
acc^=getbyte(pc);
result_f=acc;
pc++;
}

void lsr_4A(void)
{
CLE;
carry_f=(acc&1);
acc>>=1;
result_f=acc;
}

void jmp_4C(void)
{
pc=getbyte(pc)+0x100*getbyte(pc+1);
}

void eor_4D(void)
{
CLE;
acc^=getbyte(getbyte(pc)+0x100*getbyte(pc+1));
result_f=acc;
pc+=2;
}

void lsr_4E(void)
{
CLE;
result_f=getbyte(tempint=getbyte(pc)+0x100*getbyte(pc+1));
carry_f=result_f&1;
result_f>>=1;
putbyte(result_f,tempint);
pc+=2;
}

void bvc_50(void)
{
pc++;
if (ovr_f==0) pc+=(signed char)(getbyte(pc-1));
}

void eor_51(void)
{
CLE;
temp1=getbyte(pc);

acc^=getbyte((RAM[temp1]+0x100*RAM[temp1+1])+(ubyte)y_reg);
result_f=acc;
pc++;
}

void eor_55(void)
{
CLE;
acc^=RAM[(ubyte)(getbyte(pc)+(ubyte)x_reg)];
result_f=acc;
pc++;
}

void lsr_56(void)
{
CLE;
result_f=RAM[temp2=getbyte(pc)+x_reg];
carry_f=result_f&1;
result_f>>=1;
RAM[temp2]=result_f;
pc++;
}

void cli_58(void)
{
intd_f=0;
}

void eor_59(void)
{
CLE;
acc^=getbyte(getbyte(pc)+0x100*getbyte(pc+1)+y_reg);
result_f=acc;
pc+=2;
}

void eor_5D(void)
{
CLE;
acc^=getbyte(getbyte(pc)+0x100*getbyte(pc+1)+x_reg);
result_f=acc;
pc+=2;
}

void lsr_5E(void)
{
CLE;
result_f=getbyte(tempint=getbyte(pc)+0x100*getbyte(pc+1)+x_reg);
carry_f=result_f&1;
result_f>>=1;
putbyte(result_f,tempint);
pc+=2;
}

void rts_60(void)
{
pc=popbyte();
pc+=0x100*popbyte();
pc++;
}

void adc_61(void)
{
uint flags;
uint answer;

CLE;

temp2=getbyte(pc)+x_reg;
temp1=getbyte(RAM[temp2]+0x100*RAM[temp2+1]);

asm push ax
asm CLC
if (carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm adc AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=acc=answer;
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=flags&0x01;
pc++;
}

void adc_65(void)
{
uint flags;
uint answer;

CLE;

temp1=RAM[getbyte(pc)];

asm push ax

asm CLC
if (carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm adc AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=acc=answer;
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=flags&0x01;

pc++;
}


void ror_66(void)
{
ubyte oldcar;

CLE;
oldcar=carry_f;
result_f=RAM[temp2=getbyte(pc)];

carry_f=result_f&1;

result_f>>=1;

if (oldcar) result_f|=0x80;

RAM[temp2]=result_f;
pc++;
}

void pla_68(void)
{
result_f=acc=popbyte();
}

void adc_69(void)
{
uint flags;
uint answer;

CLE;
temp1=getbyte(pc);

asm push ax
asm CLC

if (carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm adc AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=acc=answer;
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=flags&0x01;

pc++;
}

void ror_6A(void)
{
ubyte oldcar;

CLE;
oldcar=carry_f;


carry_f=acc&1;
acc>>=1;

if (oldcar) acc|=0x80;
result_f=acc;
}

void jmp_6C(void)
{
tempint=getbyte(pc)+0x100*getbyte(pc+1);
pc=getbyte(tempint)+0x100*getbyte(tempint+1);
}

void adc_6D(void)
{
uint flags;
uint answer;

CLE;

temp1=getbyte(getbyte(pc)+0x100*getbyte(pc+1));

asm push ax
asm CLC

if (carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm adc AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=(acc=answer);
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=flags&0x01;

pc+=2;
}

void ror_6E(void)
{
ubyte oldcar;
CLE;

oldcar=carry_f;
result_f=getbyte(tempint=getbyte(pc)+0x100*getbyte(pc+1));

carry_f=result_f&1;
result_f>>=1;
if (oldcar) result_f|=0x80;

putbyte(result_f,tempint);
pc+=2;
}

void bvs_70(void)
{
pc++;
if (ovr_f) pc+=(signed char)(getbyte(pc-1));
}

void adc_71(void)
{
uint flags;
uint answer;

CLE;

temp2=getbyte(pc);
temp1=getbyte(RAM[temp2]+0x100*RAM[temp2+1]+y_reg);

asm push ax
asm CLC

if (carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm adc AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=(acc=answer);
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=flags&0x01;

pc++;
}

void adc_75(void)
{
uint flags;
uint answer;

CLE;
temp1=RAM[(ubyte)(getbyte(pc)+x_reg)];

asm push ax
asm CLC

if (carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm adc AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=(acc=answer);
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=flags&0x01;

pc++;
}

void ror_76(void)
{
ubyte oldcar;

CLE;
oldcar=carry_f;
result_f=RAM[temp2=getbyte(pc)+x_reg];

carry_f=result_f&1;
result_f>>=1;
if (oldcar) result_f|=0x80;

RAM[temp2]=result_f;
pc++;
}

void sei_78(void)
{
intd_f=1;
}

void adc_79(void)
{
uint flags;
uint answer;
CLE;
temp1=getbyte(getbyte(pc)+0x100*getbyte(pc+1)+y_reg);

asm push ax

asm CLC
if (carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm adc AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=(acc=answer);
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=flags&1;

pc+=2;
}

void adc_7D(void)
{
uint flags;
uint answer;
CLE;
temp1=getbyte(getbyte(pc)+0x100*getbyte(pc+1)+x_reg);

asm push ax

asm CLC
if (carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm adc AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=acc=answer;
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=flags&0x01;

pc+=2;
}

void ror_7E(void)
{
ubyte oldcar;
CLE;
oldcar=carry_f;
result_f=getbyte(tempint=getbyte(pc)+0x100*getbyte(pc+1)+x_reg);

carry_f=result_f&1;
result_f>>=1;
if (oldcar) result_f|=0x80;

putbyte(result_f,tempint);
pc+=2;
}

void sta_81(void)
{
temp1=getbyte(pc)+x_reg;
putbyte(acc,RAM[temp1]+0x100*RAM[(ubyte)(temp1+1)]);
pc++;
}

void sty_84(void)
{
RAM[getbyte(pc)]=y_reg;
pc++;
}

void sta_85(void)
{
RAM[getbyte(pc)]=acc;
pc++;
}

void stx_86(void)
{
RAM[getbyte(pc)]=x_reg;
pc++;
}

void dey_88(void)
{
CLE;
--y_reg;
result_f=y_reg;
}

void txa_8A(void)
{
CLE;
result_f=(acc=x_reg);
}

void sty_8C(void)
{
putbyte(y_reg,getbyte(pc)+0x100*getbyte(pc+1));
pc+=2;
}

void sta_8D(void)
{
putbyte(acc,getbyte(pc)+0x100*getbyte(pc+1));
pc+=2;
}

void stx_8E(void)
{
putbyte(x_reg,getbyte(pc)+0x100*getbyte(pc+1));
pc+=2;
}

void bcc_90(void)
{
pc++;
if (!carry_f) pc+=(signed char)(getbyte(pc-1));
}

void sta_91(void)
{
temp1=getbyte(pc);
putbyte(acc,RAM[temp1]+0x100*RAM[(ubyte)(temp1+1)]+y_reg);
pc++;
}

void sty_94(void)
{
RAM[(ubyte)(getbyte(pc)+x_reg)]=y_reg;
pc++;
}

void sta_95(void)
{
RAM[(ubyte)(getbyte(pc)+x_reg)]=acc;
pc++;
}

void stx_96(void)
{
RAM[(unsigned char)(getbyte(pc)+y_reg)]=x_reg;
pc++;
}

void tya_98(void)
{
CLE;
result_f=(acc=y_reg);
}

void sta_99(void)
{
putbyte(acc,getbyte(pc)+0x100*getbyte(pc+1)+y_reg);
pc+=2;
}

void txs_9A(void)
{
sp=x_reg;
}

void sta_9D(void)
{
putbyte(acc,getbyte(pc)+0x100*getbyte(pc+1)+x_reg);
pc+=2;
}

void ldy_A0(void)
{
CLE;
result_f=(y_reg=getbyte(pc));
pc++;
}

void lda_A1(void)
{
CLE;
temp1=getbyte(pc)+x_reg;
result_f=(acc=getbyte(RAM[temp1]+0x100*RAM[temp1+1]));
pc++;
}

void ldx_A2(void)
{
CLE;
result_f=(x_reg=getbyte(pc));
pc++;
}

void ldy_A4(void)
{
CLE;
result_f=(y_reg=RAM[getbyte(pc)]);
pc++;
}

void lda_A5(void)
{
CLE;
result_f=(acc=RAM[getbyte(pc)]);
pc++;
}

void ldx_A6(void)
{
CLE;
result_f=(x_reg=RAM[getbyte(pc)]);
pc++;
}

void tay_A8(void)
{
CLE;
result_f=(y_reg=acc);
}

void lda_A9(void)
{
CLE;
result_f=(acc=getbyte(pc));
pc++;
}

void tax_AA(void)
{
CLE;
result_f=(x_reg=acc);
}

void ldy_AC(void)
{
CLE;
result_f=(y_reg=getbyte(getbyte(pc)+0x100*getbyte(pc+1)));
pc+=2;
}

void lda_AD(void)
{
CLE;
result_f=(acc=getbyte(getbyte(pc)+0x100*getbyte(pc+1)));
pc+=2;
}

void ldx_AE(void)
{
CLE;
result_f=(x_reg=getbyte(getbyte(pc)+0x100*getbyte(pc+1)));
pc+=2;
}

void bcs_B0(void)
{
pc++;
if (carry_f) pc+=(signed char)(getbyte(pc-1));
}

void lda_B1(void)
{
CLE;
temp1=getbyte(pc);
result_f=(acc=getbyte(RAM[temp1]+0x100*RAM[temp1+1]+y_reg));
pc++;
}

void ldy_B4(void)
{
CLE;
result_f=(y_reg=RAM[(ubyte)(getbyte(pc)+x_reg)]);
pc++;
}

void lda_B5(void)
{
CLE;
result_f=(acc=RAM[(ubyte)(getbyte(pc)+x_reg)]);
pc++;
}

void ldx_B6(void)
{
CLE;
result_f=(x_reg=RAM[(ubyte)(getbyte(pc)+y_reg)]);
pc++;
}

void clv_B8(void)
{
ovr_f=0;
}

void lda_B9(void)
{
CLE;
result_f=(acc=getbyte(getbyte(pc)+0x100*getbyte(pc+1)+y_reg));
pc+=2;
}

void tsx_BA(void)
{
CLE;
result_f=(x_reg=sp);
}

void ldy_BC(void)
{
CLE;
result_f=(y_reg=getbyte(getbyte(pc)+0x100*getbyte(pc+1)+x_reg));
pc+=2;
}

void lda_BD(void)
{
CLE;
result_f=(acc=getbyte(getbyte(pc)+0x100*getbyte(pc+1)+x_reg));
pc+=2;
}

void ldx_BE(void)
{
CLE;
result_f=(x_reg=getbyte(getbyte(pc)+0x100*getbyte(pc+1)+y_reg));
pc+=2;
}

void cpy_C0(void)
{
uint flags;
CLE;

temp1=getbyte(pc);

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=y_reg;
_AH=temp1;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc++;
}

void cmp_C1(void)
{
uint flags;
CLE;
temp2=getbyte(pc)+x_reg;
temp1=getbyte(RAM[temp2]+0x100*RAM[(ubyte)(temp2+1)]);

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc++;
}

void cpy_C4(void)
{
uint flags;
ubyte neartemp;
CLE;
neartemp=RAM[getbyte(pc)];

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=y_reg;
_AH=neartemp;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc++;
}

void cmp_C5(void)
{
uint flags;
CLE;
temp1=RAM[getbyte(pc)];

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc++;
update_dyn_p();
}

void dec_C6(void)
{
CLE;
RAM[temp1=getbyte(pc)]--;
result_f=RAM[temp1];
pc++;
}

void iny_C8(void)
{
CLE;
result_f=++y_reg;
}

void cmp_C9(void)
{
uint flags;
CLE;
temp1=getbyte(pc);

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc++;
}

void dex_CA(void)
{
CLE;
x_reg--;
result_f=x_reg;
}

void cpy_CC(void)
{
uint flags;
CLE;
temp1=getbyte(getbyte(pc)+0x100*getbyte(pc+1));

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=y_reg;
_AH=temp1;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc+=2;

}

void cmp_CD(void)
{
uint flags;
CLE;
temp1=getbyte(getbyte(pc)+0x100*getbyte(pc+1));

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc+=2;
}

void dec_CE(void)
{
CLE;
result_f=getbyte(tempint=getbyte(pc)+0x100*getbyte(pc+1));
result_f--;
putbyte(result_f,tempint);
pc+=2;
}

void bne_D0(void)
{
pc++;
if (except) return;
if (result_f) pc+=(signed char)(getbyte(pc-1));
}

void cmp_D1(void)
{
uint flags;
CLE;
temp2=getbyte(pc);
temp1=getbyte(RAM[temp2]+0x100*RAM[temp2+1]+y_reg);

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc++;

}


void cmp_D5(void)
{
uint flags;
CLE;
temp1=RAM[(ubyte)(getbyte(pc)+x_reg)];

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc++;
}

void dec_D6(void)
{
CLE;
result_f=RAM[temp1=getbyte(pc)+x_reg];
result_f--;
RAM[temp1]=result_f;
pc++;
}

void cld_D8(void)
{
dec_f=0;
}

void cmp_D9(void)
{
uint flags;
CLE;
temp1=getbyte(getbyte(pc)+0x100*getbyte(pc+1)+y_reg);

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc+=2;
}

void cmp_DD(void)
{
uint flags;
CLE;
temp1=getbyte(getbyte(pc)+0x100*getbyte(pc+1)+x_reg);

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc+=2;
}

void dec_DE(void)
{
CLE;
result_f=getbyte(tempint=getbyte(pc)+0x100*getbyte(pc+1)+x_reg);
result_f--;
putbyte(result_f,tempint);
pc+=2;
}

void cpx_E0(void)
{
uint flags;
CLE;
temp1=getbyte(pc);

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=x_reg;
_AH=temp1;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc++;
}

void sbc_E1(void)
{
uint flags;
uint answer;
CLE;
temp2=getbyte(pc)+x_reg;
temp1=getbyte(RAM[temp2]+0x100*RAM[(ubyte)(temp2+1)]);

asm push ax
asm CLC

if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm sbb AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=acc=answer;
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=(flags&1)^1;

pc++;
}

void cpx_E4(void)
{
uint flags;
CLE;
temp1=RAM[getbyte(pc)];

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=x_reg;
_AH=temp1;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc++;
}

void sbc_E5(void)
{
uint flags;
uint answer;

CLE;
temp1=RAM[getbyte(pc)];

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm sbb AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=acc=answer;
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=(flags&1)^1;

pc++;
}

void inc_E6(void)
{
CLE;
temp1=getbyte(pc);
RAM[temp1]++;
result_f=RAM[temp1];
pc++;
}

void inx_E8(void)
{
CLE;
x_reg++;
result_f=x_reg;
}

void sbc_E9(void)
{
uint flags;
uint answer;
CLE;
temp1=getbyte(pc);

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm sbb AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=acc=answer;
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=(flags&1)^1;

pc++;
}

void nop_EA(void)
{
}

void cpx_EC(void)
{
uint flags;
CLE;
temp1=getbyte(getbyte(pc)+0x100*getbyte(pc+1));

asm push ax

asm CLC
if (!carry_f) asm STC;

_AL=x_reg;
_AH=temp1;

asm cmp AL,AH
asm pushf
asm pop flags
asm pop AX

carry_f=(flags&1)^1;
result_f=flags&0xC0;
result_f^=0x40;

pc+=2;

}


void sbc_ED(void)
{
uint flags;
uint answer;

asm push ax
CLE;
temp1=getbyte(getbyte(pc)+0x100*getbyte(pc+1));

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm sbb AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=acc=answer;
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=(flags&1)^1;

pc+=2;
}

void inc_EE(void)
{
CLE;
result_f=getbyte(tempint=getbyte(pc)+0x100*getbyte(pc+1));
result_f++;
putbyte(result_f,tempint);
pc+=2;
}

void beq_F0(void)
{
pc++;
if (except) {pc+=(signed char)(getbyte(pc-1));return;}
if (!result_f) pc+=(signed char)(getbyte(pc-1));
}

void sbc_F1(void)
{
uint flags;
uint answer;
CLE;
asm push ax

temp2=getbyte(pc);
temp1=getbyte(RAM[temp2]+0x100*RAM[(ubyte)(temp2+1)]+y_reg);

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm sbb AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=acc=answer;
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=(flags&0x01)^1;

pc++;
}

void sbc_F5(void)
{
uint flags;
uint answer;
CLE;
asm push ax

temp1=RAM[(ubyte)(getbyte(pc)+x_reg)];

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm sbb AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=acc=answer;
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=(flags&1)^1;

pc++;
}

void inc_F6(void)
{
CLE;
RAM[temp1=getbyte(pc)+x_reg]++;
result_f=RAM[temp1];
pc++;
}

void sed_F8(void)
{
dec_f=1;
puts("Decimal Mode Not Supported!");
exit(1);
}

void sbc_F9(void)
{
uint flags;
uint answer;
CLE;
asm push ax

temp1=getbyte(getbyte(pc)+0x100*getbyte(pc+1)+y_reg);

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm sbb AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=acc=answer;
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=(flags&1)^1;

pc+=2;
}

void sbc_FD(void)
{
uint flags;
uint answer;

CLE;
asm push ax

temp1=getbyte(getbyte(pc)+0x100*getbyte(pc+1)+x_reg);

asm CLC
if (!carry_f) asm STC;

_AL=acc;
_AH=temp1;

asm sbb AL,AH
asm pushf
asm pop flags
asm push AX
asm pop answer
asm pop AX

result_f=acc=answer;
(flags&0x800)?(ovr_f=1):(ovr_f=0);
carry_f=(flags&1)^1;

pc+=2;
}

void inc_FE(void)
{
CLE;
result_f=getbyte(tempint=getbyte(pc)+0x100*getbyte(pc+1)+x_reg);
result_f++;
putbyte(result_f,tempint);
pc+=2;
}


/*

Not a 6502 instruction, but is used in place of opcode 0xFF,
which is undefined. Thus when this is excuted it ends execution.
This is useful for writing small test programs, and means that
the exact number of instructions to be executed doens't have
to be calculated.

*/

void exitprog_FF(void)
{
puts("\n_____________________________________\n");
puts("\nExecution Terminated with opcode 0xFF");
exit(0);
}


void init_decode(void)     // Initialises the array of decode functions
{
	int c=0;                // misc counter

	printf("Init Decode Array...");  // display pretty startup status.
	for (;c<256;c++)                 //
	{                                // Initialise all pointers to a
		decode[c]=non_opcode;         // function dealing with illegal
	}                                // opcodes

	// initialise specific locations in the array with a pointer
	// to the relevant function.


	decode[0x00]=brk_00;
	decode[0x01]=ora_01;
	decode[0x05]=ora_05;
	decode[0x06]=asl_06;
	decode[0x08]=php_08;
	decode[0x09]=ora_09;
	decode[0x0A]=asl_0A;
	decode[0x0D]=ora_0D;
	decode[0x0E]=asl_0E;
	decode[0x10]=bpl_10;
	decode[0x11]=ora_11;
	decode[0x15]=ora_15;
	decode[0x16]=asl_16;
	decode[0x18]=clc_18;
	decode[0x19]=ora_19;
	decode[0x1D]=ora_1D;
	decode[0x1E]=asl_1E;
	decode[0x20]=jsr_20;
	decode[0x21]=and_21;
	decode[0x24]=bit_24;
	decode[0x25]=and_25;
	decode[0x26]=rol_26;
	decode[0x28]=plp_28;
	decode[0x29]=and_29;
	decode[0x2A]=rol_2A;
	decode[0x2C]=bit_2C;
	decode[0x2D]=and_2D;
	decode[0x2E]=rol_2E;
	decode[0x30]=bmi_30;
	decode[0x31]=and_31;
	decode[0x35]=and_35;
	decode[0x36]=rol_36;
	decode[0x38]=sec_38;
	decode[0x39]=and_39;
	decode[0x3D]=and_3D;
	decode[0x3E]=rol_3E;
	decode[0x40]=rti_40;
	decode[0x41]=eor_41;
	decode[0x45]=eor_45;
	decode[0x46]=lsr_46;
	decode[0x48]=pha_48;
	decode[0x49]=eor_49;
	decode[0x4A]=lsr_4A;
	decode[0x4C]=jmp_4C;
	decode[0x4D]=eor_4D;
	decode[0x4E]=lsr_4E;
	decode[0x50]=bvc_50;
	decode[0x51]=eor_51;
	decode[0x55]=eor_55;
	decode[0x56]=lsr_56;
	decode[0x59]=eor_59;
	decode[0x5D]=eor_5D;
	decode[0x5E]=lsr_5E;
	decode[0x58]=cli_58;
	decode[0x60]=rts_60;
	decode[0x61]=adc_61;
	decode[0x65]=adc_65;
	decode[0x66]=ror_66;
	decode[0x68]=pla_68;
	decode[0x69]=adc_69;
	decode[0x6A]=ror_6A;
	decode[0x6C]=jmp_6C;
	decode[0x6D]=adc_6D;
	decode[0x6E]=ror_6E;
	decode[0x70]=bvs_70;
	decode[0x71]=adc_71;
	decode[0x75]=adc_75;
	decode[0x76]=ror_76;
	decode[0x78]=sei_78;
	decode[0x79]=adc_79;
	decode[0x7D]=adc_7D;
	decode[0x7E]=ror_7E;
	decode[0x81]=sta_81;
	decode[0x84]=sty_84;
	decode[0x85]=sta_85;
	decode[0x86]=stx_86;
	decode[0x88]=dey_88;
	decode[0x8A]=txa_8A;
	decode[0x8C]=sty_8C;
	decode[0x8D]=sta_8D;
	decode[0x8E]=stx_8E;
	decode[0x90]=bcc_90;
	decode[0x91]=sta_91;
	decode[0x94]=sty_94;
	decode[0x95]=sta_95;
	decode[0x96]=stx_96;
	decode[0x98]=tya_98;
	decode[0x99]=sta_99;
	decode[0x9A]=txs_9A;
	decode[0x9D]=sta_9D;
	decode[0xA0]=ldy_A0;
	decode[0xA1]=lda_A1;
	decode[0xA2]=ldx_A2;
	decode[0xA4]=ldy_A4;
	decode[0xA5]=lda_A5;
	decode[0xA6]=ldx_A6;
	decode[0xA8]=tay_A8;
	decode[0xA9]=lda_A9;
	decode[0xAA]=tax_AA;
	decode[0xAC]=ldy_AC;
	decode[0xAD]=lda_AD;
	decode[0xAE]=ldx_AE;
	decode[0xB0]=bcs_B0;
	decode[0xB1]=lda_B1;
	decode[0xB4]=ldy_B4;
	decode[0xB5]=lda_B5;
	decode[0xB6]=ldx_B6;
	decode[0xB8]=clv_B8;
	decode[0xB9]=lda_B9;
	decode[0xBA]=tsx_BA;
	decode[0xBC]=ldy_BC;
	decode[0xBD]=lda_BD;
	decode[0xBE]=ldx_BE;
	decode[0xC0]=cpy_C0;
	decode[0xC1]=cmp_C1;
	decode[0xC4]=cpy_C4;
	decode[0xC5]=cmp_C5;
	decode[0xC6]=dec_C6;
	decode[0xC8]=iny_C8;
	decode[0xC9]=cmp_C9;
	decode[0xCA]=dex_CA;
	decode[0xCC]=cpy_CC;
	decode[0xCD]=cmp_CD;
	decode[0xCE]=dec_CE;
	decode[0xD0]=bne_D0;
	decode[0xD1]=cmp_D1;
	decode[0xD5]=cmp_D5;
	decode[0xD6]=dec_D6;
	decode[0xD8]=cld_D8;
	decode[0xD9]=cmp_D9;
	decode[0xDD]=cmp_DD;
	decode[0xDE]=dec_DE;
	decode[0xE0]=cpx_E0;
	decode[0xE1]=sbc_E1;
	decode[0xE4]=cpx_E4;
	decode[0xE5]=sbc_E5;
	decode[0xE6]=inc_E6;
	decode[0xE8]=inx_E8;
	decode[0xEA]=nop_EA;
	decode[0xEC]=cpx_EC;
	decode[0xED]=sbc_ED;
	decode[0xEE]=inc_EE;
	decode[0xE9]=sbc_E9;
	decode[0xF0]=beq_F0;
	decode[0xF1]=sbc_F1;
	decode[0xF5]=sbc_F5;
	decode[0xF6]=inc_F6;
	decode[0xF8]=sed_F8;
	decode[0xF9]=sbc_F9;
	decode[0xFD]=sbc_FD;
	decode[0xFE]=inc_FE;

	decode[0xFF]=exitprog_FF;

	printf("Done\n");   // display pretty startup status
}

void non_opcode(void)   // functions to deal with illegal opcode
{
	printf("\nInvalid Opcode Error.\nProgram Execution terminated.");
	printf("\nOpcode=%2X  Address=%4X",getbyte(pc-1),pc-1);
	exit(1);
}

void update_dyn_p(void)        // build valid processor status byte
										 // for pushing
{
dyn_p=0x20;                    // bit 5 is always set in the status byte
dyn_p|=(result_f&0x80);        //
if (ovr_f) dyn_p|=0x40;        //
if (brk_f) dyn_p|=0x10;        //
if (dec_f) dyn_p|=0x08;        // set the relevant bit in the status byte
if (intd_f) dyn_p|=0x04;       // for each flag
if (!result_f) dyn_p|=0x02;       //
if (carry_f) dyn_p|=0x01;      //
if (except) dyn_p|=0x82;
}


void update_flags(void)        // update the flags depending on dyn_p
{
result_f=(dyn_p&0x82)^2;
carry_f=dyn_p&0x01;
intd_f=dyn_p&0x04;
dec_f=dyn_p&0x08;
brk_f=dyn_p&0x10;
ovr_f=dyn_p&0x40;
if ((dyn_p&0x82)==0x82) except=1;
}


void irq(void) {

	brk_f=0;
	update_dyn_p();
	pushbyte(pc/0x100);
	pushbyte((ubyte)(pc));
	pushbyte(dyn_p);
	intd_f=1;
	pc=getbyte(0xFFFE)+0x100*getbyte(0xFFFF);
}===========================================
-------------------------------------------

-------------------------------------------
===========================================
#include<stdio.h>
#include "header.h"
#include "reg_name.h"
#include "io.h"

ubyte (*rsheila[256])(void);   // array of pointers to i/o handling funcs

ubyte current_latch;           // last value sent to latch

ubyte s_via_opa=0;            // system via output port a
ubyte s_via_ipa=0;            // system via input  port a
ubyte s_via_opb=0;            // system via output port b
ubyte s_via_ipb=0;            // system via input  port b


/*
The following functions { of the form s??loc()   } return
the value ??. These are used in memory mapped locations
which always return a constant value
(to the best of my knowledge).
*/

ubyte s00loc()
{
return 0x00;
}

ubyte s02loc()
{
return 0x02;
}

ubyte s10loc()
{
return 0x10;
}

ubyte s20loc()
{
return 0x20;
}

ubyte s28loc()
{
return 0x28;
}

ubyte s9Dloc()
{
return 0x9D;
}

ubyte sABloc()
{
return 0xAB;
}

ubyte sB7loc()
{
return 0xB7;
}

ubyte sFEloc()
{
return 0xFE;
}

ubyte sFFloc()
{
return 0xFF;
}

// Return the values of crt_regs 14,15,16,17 decimal else return 0

ubyte crt_reg(void)
{
if ((crt_sel>17)||(crt_sel<14)) return 0;
return crt_regs[crt_sel];
}


/******************************************************
******************************************************/


/*
Functions of the form ubyte sr_via_?(void) are used when reading from
the system via. with ? representing the register in hexadecimal.
*/

ubyte sr_via_0(void)
{
return 0xF0|current_latch;   // return the last value sent to latch
}

ubyte sr_via_1(void)
{
if ((current_key)&&(s_via_opa==current_key)) return (current_key|0x80);
if ((s_via_opa==0)&&(current_shift)) return 0x80;
if ((s_via_opa==1)&&(current_control)) return 0x81;
return s_via_opa;
}

ubyte sr_via_2(void)
{
return s_via[DDRB];
}

ubyte sr_via_3(void)
{
return s_via[DDRA];
}

ubyte sr_via_4(void)
{
return 0;
}

ubyte sr_via_5(void)
{
return 0;
}

ubyte sr_via_6(void)
{
return 0;
}

ubyte sr_via_7(void)
{
return 0;
}

ubyte sr_via_8(void)
{
return 0;
}

ubyte sr_via_9(void)
{
return 0;
}

ubyte sr_via_A(void)
{
return 0;
}

ubyte sr_via_B(void)
{
return 0;
}

ubyte sr_via_C(void)
{
return 0;
}

ubyte sr_via_D(void)
{
return (s_via[IFR]);
}

ubyte sr_via_E(void)
{
return (s_via[IER]|0x80);
}

ubyte sr_via_F(void)
{
return sr_via_1();
}


/*
This function initialises all the pointers to functions for
the io read function.
First it fills all locations with a warning that no
return function is defined.
Then fills the locations with suitable i/o handling
functions relevant to the specific address.
*/

void rinit_io(void)
{
unsigned int c;

printf("Init read IO system...");    // display pretty startup status

for (c=0;c<=0xFF;c++)
{
	rsheila[c]=s00loc;
}

for (c=0x00;c<=0x07;c+=2)
{
	rsheila[c]=s00loc;
	rsheila[c+1]=crt_reg;
}

for (c=0x08;c<=0xF;c+=2)
{
	rsheila[c]=s02loc;
	rsheila[c+1]=sFFloc;
}

for (c=0x10;c<=0x17;c++)
{
	rsheila[c]=s00loc;
}

for (c=0x18;c<=0x1F;c++)
{
	rsheila[c]=sB7loc;
}

for (c=0x20;c<=0x3F;c++)
{
	rsheila[c]=sFEloc;
}

rsheila[0x50]=rsheila[0x40]=sr_via_0;
rsheila[0x51]=rsheila[0x41]=sr_via_1;
rsheila[0x52]=rsheila[0x42]=sr_via_2;
rsheila[0x53]=rsheila[0x43]=sr_via_3;
rsheila[0x54]=rsheila[0x44]=sr_via_4;
rsheila[0x55]=rsheila[0x45]=sr_via_5;
rsheila[0x56]=rsheila[0x46]=sr_via_6;
rsheila[0x57]=rsheila[0x47]=sr_via_7;
rsheila[0x58]=rsheila[0x48]=sr_via_8;
rsheila[0x59]=rsheila[0x49]=sr_via_9;
rsheila[0x5A]=rsheila[0x4A]=sr_via_A;
rsheila[0x5B]=rsheila[0x4B]=sr_via_B;
rsheila[0x5C]=rsheila[0x4C]=sr_via_C;
rsheila[0x5D]=rsheila[0x4D]=sr_via_D;
rsheila[0x5E]=rsheila[0x4E]=sr_via_E;
rsheila[0x5F]=rsheila[0x4F]=sr_via_F;


for (c=0x60;c<=0x7F;c++)
{
	rsheila[c]=s00loc;
}

for (c=0x80;c<=0x9F;c+=8)
{
	rsheila[c+0]=s00loc;
	rsheila[c+1]=sFFloc;
	rsheila[c+2]=s9Dloc;
	rsheila[c+3]=sFFloc;
	rsheila[c+4]=sABloc;
	rsheila[c+5]=sABloc;
	rsheila[c+6]=sABloc;
	rsheila[c+7]=sABloc;
}

for (c=0xA0;c<=0xBF;c+=4)
{
	rsheila[c+0]=s10loc;
	rsheila[c+1]=s20loc;
	rsheila[c+2]=s28loc;
	rsheila[c+3]=s28loc;
}

for (c=0xC0;c<=0xDF;c++)
{
	rsheila[c]=s00loc;
}

for (c=0xE0;c<=0xFF;c++)
{
	rsheila[c]=sFEloc;
}

printf("Done\n");     // display pretty startup status

}
===========================================
-------------------------------------------

-------------------------------------------
===========================================
#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include "header.h"
#include "screen.h"
#include "reg_name.h"
#include "io.h"

void (*wsheila[256])(ubyte);  // array of pointers to functions for io-write
										// decoding

void flash(void);             // prototype for flash function

ubyte adc_control=0;          // analogue digital control register
ubyte vid_con_reg=0;          // video ULA control register

ubyte crt_sel=0;              // crt register select register
ubyte crt_regs[18];           // array of crt register

ubyte romsel=0;               // paged rom select register

ubyte s_via[16];              // array of system via registers
ubyte s_via_latch[8];         // the system via latch

ubyte vidpal[16];             // the video palette


/********************************************************
---------------------------------------------------------
********************************************************/


void donald(void)
{
puts("Donald");
}


void nullwrite()
{
}

void write_crt(ubyte iobyte)   // crt controller write decode function
{
if (crt_sel<=15)               // if a valid register for writing to
	{
	crt_regs[crt_sel]=iobyte;   // write to the register
	}

switch (crt_sel)               // take relevant action depending on
										 // register action
{
case  1: set_colour_bits();

case 12:	break;
case 13:	if (teletext)
			{
			screen_start=crt_regs[12]*0x100+crt_regs[13];
			screen_start=0x7400+(screen_start^0x2000);
			update_screen();
			}
			else
			{
			screen_start=crt_regs[12]*0x100+crt_regs[13];
			screen_start<<=3;
			update_screen();
			}
			break;
case 14: break;
case 15: update_cursor();break; // if the cursor address has changed,
}                               // update the cursor.

}

void crt_select(ubyte iobyte)   // crt register select function
{
crt_sel=iobyte;
}

void vid_ULA(ubyte iobyte)      // Video ULA control register
{
ubyte tempbits;
ubyte flashstate;

flashstate=vid_con_reg&1;       // initial flash state
vid_con_reg=iobyte;             // update control register

if (!(iobyte&2)&&teletext)      // if 1-0 transition on the teletext select
{                               // initialise graphics
	teletext=0;
	pixel_vid_init();
}

if ((iobyte&2)&&(!teletext))    // if 0-1 transition on the teletext select
{                               // initialise teletext
	teletext=1;
	teletext_init();
}

tempbits=disp_chars;            // initial number of display characters

switch (vid_con_reg&0xC)
{
	case 0x00:	disp_chars=10 ; break;
	case 0x04:  disp_chars=20 ; break;
	case 0x08:  disp_chars=40 ; break;
	case 0x0C:  disp_chars=80 ; break;
}

	// if displayed characters change, set_colour_bits()
if (tempbits!=disp_chars) set_colour_bits();
	// if flash state has changed, flash()
if ((flashstate!=(vid_con_reg&1))&&(!teletext)) flash();

}

void vidpalset(ubyte iobyte)     // video palette update functions
{
	ubyte logical_colour;
	ubyte actual_colour;
	ubyte pc_colour;

	logical_colour=iobyte/0x10;   // calculate logical colour
	actual_colour=iobyte&0xF;     // calculate actual programmed colour

	vidpal[logical_colour]=actual_colour;  // update vidpal copy of palette

	switch(actual_colour^0x7)              // convert actual colour to
	{                                      // pc colour
	case 0: pc_colour=EGA_BLACK	;break;
	case 1: pc_colour=EGA_RED		;break;
	case 2: pc_colour=EGA_GREEN   ;break;
	case 3: pc_colour=EGA_YELLOW  ;break;
	case 4: pc_colour=EGA_BLUE    ;break;
	case 5: pc_colour=EGA_MAGENTA ;break;
	case 6: pc_colour=EGA_CYAN    ;break;
	case 7: pc_colour=EGA_WHITE	;break;

	case 8: pc_colour=EGA_BLACK	;break;
	case 9: pc_colour=EGA_RED		;break;
	case 10: pc_colour=EGA_GREEN  ;break;
	case 11: pc_colour=EGA_YELLOW ;break;
	case 12: pc_colour=EGA_BLUE   ;break;
	case 13: pc_colour=EGA_MAGENTA;break;
	case 14: pc_colour=EGA_CYAN   ;break;
	case 15: pc_colour=EGA_WHITE	;break;

	}

	setpalette(logical_colour,pc_colour); // update pc palette
}

void promsel(ubyte iobyte)               // paged rom select write function
{
romsel=iobyte;
}

/******************************************************
******************************************************/

/* Functions of the form "void sw_via_?(ubyte)"
	are the system via register decodes functions
*/


void sw_via_0(ubyte iobyte)
{
s_via[ORB]=iobyte;
current_latch=s_via_opb=iobyte&s_via[DDRB];
if (iobyte==0)
	iobyte=0;
s_via_latch[s_via_opb&7]=s_via_opb&8;

iobyte&=7;
if ((iobyte==4)||(iobyte==5))
	if (!teletext) set_screen_start();
if (iobyte==0) sound_byte(s_via_opa);
}

void sw_via_1(ubyte iobyte)
{
s_via[ORA]=iobyte;
s_via_opa=iobyte&s_via[DDRA];
if (s_via_latch[3]==0)
	if (current_key)
		if ((current_key&0xF)==(s_via_opa&0xF)) s_via[IFR]|=1;
}

void sw_via_2(ubyte iobyte)
{
s_via[DDRB]=iobyte;
s_via_opb=iobyte&s_via[ORB];
}

void sw_via_3(ubyte iobyte)
{
s_via[DDRA]=iobyte;
s_via_opa=iobyte&s_via[ORA];
}

void sw_via_4(ubyte iobyte)
{
s_via[T1C_L]=iobyte;
}

void sw_via_5(ubyte iobyte)
{
s_via[T1C_H]=iobyte;
}

void sw_via_6(ubyte iobyte)
{
s_via[T1L_L]=iobyte;
}

void sw_via_7(ubyte iobyte)
{
s_via[T1L_H]=iobyte;
}

void sw_via_8(ubyte iobyte)
{
s_via[T2C_L]=iobyte;
}

void sw_via_9(ubyte iobyte)
{
s_via[T2C_H]=iobyte;
}

void sw_via_A(ubyte iobyte)
{
s_via[SR]=iobyte;
}

void sw_via_B(ubyte iobyte)
{
s_via[ACR]=iobyte;
}

void sw_via_C(ubyte iobyte)
{
s_via[PCR]=iobyte;
}

void sw_via_D(ubyte iobyte)
{
s_via[IFR]=((s_via[IFR]|iobyte)^iobyte);
}

void sw_via_E(ubyte iobyte)
{
(iobyte&0x80)?(s_via[IER]|=iobyte):(s_via[IER]=((s_via[IER]|iobyte)^iobyte));
s_via[IER]|=0x80;
}

void sw_via_F(ubyte iobyte)
{
	sw_via_1(iobyte);
}


/******************************************************
******************************************************/

void adc_con_reg(ubyte iobyte)   // adc_control is never used
{                                // but a copy is kept for debugging
adc_control=iobyte;              // purposes
}

void winit_io(void)              // Called to do any io_write initialising
{
unsigned int c; // misc loop counter

printf("Init write IO system...");  // Display pretty startup info

for (c=0;c<=17;c++) crt_regs[c]=0;   // initialise the arrays to 0
for (c=0;c<=15;c++) s_via[c]=0;
for (c=0;c<=7;c++) s_via_latch[c]=0;

for (c=0x00;c<=0xFF;c++)          // initialise wsheila to point to a
{                                 // dummy function
	wsheila[c]=nullwrite;
}

for (c=0x00;c<=0x07;c+=2)
{
	wsheila[c]=crt_select;
	wsheila[c+1]=write_crt;
}

for (c=0x20;c<=0x2F;c+=0x02)
{
	wsheila[c]=vid_ULA;
	wsheila[c+1]=vidpalset;
}

for (c=0x30;c<=0x3F;c++)
{
	wsheila[c]=promsel;
}

wsheila[0x50]=wsheila[0x40]=sw_via_0;
wsheila[0x51]=wsheila[0x41]=sw_via_1;
wsheila[0x52]=wsheila[0x42]=sw_via_2;
wsheila[0x53]=wsheila[0x43]=sw_via_3;
wsheila[0x54]=wsheila[0x44]=sw_via_4;
wsheila[0x55]=wsheila[0x45]=sw_via_5;
wsheila[0x56]=wsheila[0x46]=sw_via_6;
wsheila[0x57]=wsheila[0x47]=sw_via_7;
wsheila[0x58]=wsheila[0x48]=sw_via_8;
wsheila[0x59]=wsheila[0x49]=sw_via_9;
wsheila[0x5A]=wsheila[0x4A]=sw_via_A;
wsheila[0x5B]=wsheila[0x4B]=sw_via_B;
wsheila[0x5C]=wsheila[0x4C]=sw_via_C;
wsheila[0x5D]=wsheila[0x4D]=sw_via_D;
wsheila[0x5E]=wsheila[0x4E]=sw_via_E;
wsheila[0x5F]=wsheila[0x4F]=sw_via_F;

for (c=0xC0;c<=0xDF;c+=0x04)
{
wsheila[c]=adc_con_reg;
}

printf("Done\n");   // display pretty startup status
}


void flash(void) {  // flash function, alternates the flash colours
	ubyte c;         // depending on the video ULA control register
	ubyte d;

	for (c=0;c<16;c++)         // loop through each logical colour
		if (vidpal[c]&8) {
			if (vid_con_reg&1) {
				switch ((vidpal[c]&7)^7) {
					case 0: d=EGA_WHITE		;break;
					case 1: d=EGA_CYAN		;break;
					case 2: d=EGA_MAGENTA	;break;
					case 3: d=EGA_BLUE		;break;
					case 4: d=EGA_YELLOW		;break;
					case 5: d=EGA_GREEN		;break;
					case 6: d=EGA_RED			;break;
					case 7: d=EGA_BLACK		;break;
				}
				setpalette(c,d);        // update pc palette
			}
			else {
				switch ((vidpal[c]&7)^7) {
					case 0: d=EGA_BLACK		;break;
					case 1: d=EGA_RED			;break;
					case 2: d=EGA_GREEN		;break;
					case 3: d=EGA_YELLOW		;break;
					case 4: d=EGA_BLUE		;break;
					case 5: d=EGA_MAGENTA	;break;
					case 6: d=EGA_CYAN		;break;
					case 7: d=EGA_WHITE		;break;
				}
				setpalette(c,d);        // update pc palette
			}

		}
}===========================================
-------------------------------------------

-------------------------------------------
===========================================
#include <stdio.h>
#include <conio.h>
#include <bios.h>
#include <dos.h>
#include "header.h"
#include "io.h"
#include "reg_name.h"

#define SHIFT current_shift=1
#define CLEAR current_shift=0

ubyte current_key=0;
ubyte current_shift=0;
ubyte current_control=0;

void vert_sync(void);
void getkey(void);

void gen_irq(void) {  		// Main interrupt generation and handler routine
	static type_counter=0;

	getkey();               // Check for key input and convert pc
									// keypress to BBC internal keynumber

	if (!intd_f)            // if interrupts are enabled
	{
		type_counter++;      // step through different interrupt types
		switch(type_counter)
		{
			case 1:
			case 2: if (s_via[IER]&0x40) {s_via[IFR]|=0xC0; irq();}
					  else s_via[IFR]|=0x40;
					  break;                  // 100 Hz interrupt

			case 3: if (s_via[IER]&0x02) {s_via[IFR]|=0x82; irq();}
					  else s_via[IFR]|=0x02;
					  type_counter=0;         // 50 Hz vertical blank interrupt
					  break;
		}
	}
}


void getkey(void) {

	uint pc_scan_code;
	static uint old_scan_code;
	static ubyte pressed=0;
													  // If no key is pressed,
	if (!pressed) {                       // get the shift status
		current_shift=(_bios_keybrd(_KEYBRD_SHIFTSTATUS)&3);
	}

	if (_bios_keybrd(_KEYBRD_READY)) {    // if a keystroke is waiting
		if (!pressed) {                    // and if not key is being processed

													  // Synchronise BBC and PC capslock

			switch((peekb(0,0x417)&0x40)|s_via_latch[6]) {
				case 0x48:
				case 0x00: current_key=0x40; goto caps_lock_pressed;
			}


														// get pc keystroke
			pc_scan_code=_bios_keybrd(_KEYBRD_READ);
			pressed=7;                       // hold the key for 7 interrupts

			switch (pc_scan_code&0xff) {

/**********************************************

		Keyboard Translation statements

**********************************************/

				case 0x8:	current_key=0x59;CLEAR;break;
				case 0x9:	current_key=0x60;CLEAR;break;
				case 0xD:	current_key=0x49;CLEAR;break;
				case 0x1B:	current_key=0x70;CLEAR;break;
				case ' ': 	current_key=0x62;CLEAR;break;
				case '!': 	current_key=0x30;SHIFT;break;
				case '"': 	current_key=0x31;SHIFT;break;
				case '#': 	current_key=0x11;SHIFT;break;
				case '$': 	current_key=0x12;SHIFT;break;
				case '%': 	current_key=0x13;SHIFT;break;
				case '&': 	current_key=0x34;SHIFT;break;
				case '\'': 	current_key=0x24;SHIFT;break;
				case '(': 	current_key=0x15;SHIFT;break;
				case ')': 	current_key=0x26;SHIFT;break;
				case '*':	current_key=0x48;SHIFT;break;
				case '+':   current_key=0x57;SHIFT;break;
				case ',':	current_key=0x66;CLEAR;break;
				case '-':	current_key=0x17;CLEAR;break;
				case '.':	current_key=0x67;CLEAR;break;
				case '/':	current_key=0x68;CLEAR;break;
				case '0':	current_key=0x27;CLEAR;break;
				case '1':   current_key=0x30;CLEAR;break;
				case '2':   current_key=0x31;CLEAR;break;
				case '3':   current_key=0x11;CLEAR;break;
				case '4':   current_key=0x12;CLEAR;break;
				case '5':   current_key=0x13;CLEAR;break;
				case '6':   current_key=0x34;CLEAR;break;
				case '7':   current_key=0x24;CLEAR;break;
				case '8':   current_key=0x15;CLEAR;break;
				case '9':   current_key=0x26;CLEAR;break;
				case ':':	current_key=0x48;CLEAR;break;
				case ';':	current_key=0x57;CLEAR;break;
				case '<':	current_key=0x66;SHIFT;break;
				case '=':	current_key=0x17;SHIFT;break;
				case '>':	current_key=0x67;SHIFT;break;
				case '?':	current_key=0x68;SHIFT;break;
				case '@':	current_key=0x47;CLEAR;break;
				case 'A':	current_key=0x41;SHIFT;break;
				case 'B':	current_key=0x64;SHIFT;break;
				case 'C':	current_key=0x52;SHIFT;break;
				case 'D':	current_key=0x32;SHIFT;break;
				case 'E':	current_key=0x22;SHIFT;break;
				case 'F':	current_key=0x43;SHIFT;break;
				case 'G':	current_key=0x53;SHIFT;break;
				case 'H':	current_key=0x54;SHIFT;break;
				case 'I':	current_key=0x25;SHIFT;break;
				case 'J':	current_key=0x45;SHIFT;break;
				case 'K':	current_key=0x46;SHIFT;break;
				case 'L':	current_key=0x56;SHIFT;break;
				case 'M':	current_key=0x65;SHIFT;break;
				case 'N':	current_key=0x55;SHIFT;break;
				case 'O':	current_key=0x36;SHIFT;break;
				case 'P':	current_key=0x37;SHIFT;break;
				case 'Q':	current_key=0x10;SHIFT;break;
				case 'R':	current_key=0x33;SHIFT;break;
				case 'S':	current_key=0x51;SHIFT;break;
				case 'T':	current_key=0x23;SHIFT;break;
				case 'U':	current_key=0x35;SHIFT;break;
				case 'V':	current_key=0x63;SHIFT;break;
				case 'W':	current_key=0x21;SHIFT;break;
				case 'X':	current_key=0x42;SHIFT;break;
				case 'Y':	current_key=0x44;SHIFT;break;
				case 'Z':	current_key=0x61;SHIFT;break;
				case '[':	current_key=0x38;CLEAR;break;
				case '\\':	current_key=0x78;CLEAR;break;
				case ']':	current_key=0x58;CLEAR;break;
				case '^':	current_key=0x18;CLEAR;break;
				case '_':   current_key=0x28;CLEAR;break;
				case '`':   current_control=1;break;
				case 'a':	current_key=0x41;CLEAR;break;
				case 'b':	current_key=0x64;CLEAR;break;
				case 'c':	current_key=0x52;CLEAR;break;
				case 'd':	current_key=0x32;CLEAR;break;
				case 'e':	current_key=0x22;CLEAR;break;
				case 'f':	current_key=0x43;CLEAR;break;
				case 'g':	current_key=0x53;CLEAR;break;
				case 'h':	current_key=0x54;CLEAR;break;
				case 'i':	current_key=0x25;CLEAR;break;
				case 'j':	current_key=0x45;CLEAR;break;
				case 'k':	current_key=0x46;CLEAR;break;
				case 'l':	current_key=0x56;CLEAR;break;
				case 'm':	current_key=0x65;CLEAR;break;
				case 'n':	current_key=0x55;CLEAR;break;
				case 'o':	current_key=0x36;CLEAR;break;
				case 'p':	current_key=0x37;CLEAR;break;
				case 'q':	current_key=0x10;CLEAR;break;
				case 'r':	current_key=0x33;CLEAR;break;
				case 's':	current_key=0x51;CLEAR;break;
				case 't':	current_key=0x23;CLEAR;break;
				case 'u':	current_key=0x35;CLEAR;break;
				case 'v':	current_key=0x63;CLEAR;break;
				case 'w':	current_key=0x21;CLEAR;break;
				case 'x':	current_key=0x42;CLEAR;break;
				case 'y':	current_key=0x44;CLEAR;break;
				case 'z':	current_key=0x61;CLEAR;break;
				case '{':	current_key=0x38;SHIFT;break;
				case '|':	current_key=0x78;SHIFT;break;
				case '}':	current_key=0x58;SHIFT;break;
				case '~':	current_key=0x18;SHIFT;break;
				case 'œ':	current_key=0x28;SHIFT;break;

				case 0: switch(pc_scan_code/0x100) {

					case 0x3B: current_key=0x71;break;
					case 0x3C: current_key=0x72;break;
					case 0x3D: current_key=0x73;break;
					case 0x3E: current_key=0x14;break;
					case 0x3F: current_key=0x74;break;
					case 0x40: current_key=0x75;break;
					case 0x41: current_key=0x16;break;
					case 0x42: current_key=0x76;break;
					case 0x43: current_key=0x77;break;
					case 0x44: current_key=0x20;break;
					case 0x47: monitor_call();return;   // The home key
					case 0x48: current_key=0x39;break;
					case 0x4B: current_key=0x19;break;
					case 0x4D: current_key=0x79;break;
					case 0x4F: current_key=0x69;break;
					case 0x50: current_key=0x29;break;
				}
			}


/**********************************************
**********************************************/

			caps_lock_pressed:

			if (s_via[IER]&0x01)	{    // if relevant interrupts are enabled
				s_via[IFR]|=0x81;      // generate the interrupt
				irq();
			}
			else 	s_via[IFR]|=0x01;
			old_scan_code=pc_scan_code; // store a copy of the current key
		}
												 // discard key repeats of the same key
		else if (_bios_keybrd(_KEYBRD_READY)==old_scan_code) getch();
	}

	if (!pressed) {

		asm IN AL,0x60
		asm push AX
		asm pop pc_scan_code;

		if (pc_scan_code&0x80) {      // detect if no keys are physically
			current_key=0;             // pressed
			current_shift=0;
			current_control=0;
		}
	}


	if (pressed) pressed--;				// decrease the current key counter
}
===========================================
-------------------------------------------

-------------------------------------------
===========================================
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <graphics.h>
#include <dos.h>
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
	case '\\':	iobyte='«';   break; // stricly correct, since no corresponding
	case ']':   iobyte='\x1A';break; // character in the pc set exists
	case '^':   iobyte='\x18';break; //
	case '`':	iobyte='Ä'   ;break; // notably the } characters is not
	case '#':	iobyte='œ'   ;break; // converted to the proper 3/4 symbol
	case '{':	iobyte='¬'   ;break;
	case '|':	iobyte='º'   ;break;
	case '}':	iobyte='}'   ;break;
	case '~':	iobyte='ö'   ;break;
	case 0x7F:	iobyte='Û'   ;break; // This conversion takes place because
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
===========================================
-------------------------------------------

-------------------------------------------
===========================================
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <graphics.h>
#include <dos.h>
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
}===========================================
-------------------------------------------

-------------------------------------------
===========================================
#include <stdio.h>
#include <dos.h>
#include "header.h"
#include "io.h"



void get_vol(ubyte iobyte);       // prototypes for sound functions
void get_freq(ubyte iobyte);

uint freqbits[4];                 // arrays holding frequencys
ubyte vol[4];                     // and volumes for each channel
ubyte soundyesno=1;

void sound_byte(ubyte iobyte) {      // if a noise program byte, return
	if ((iobyte&0xE0)==0xE0) return;

	if ((iobyte&0x90)==0x90) get_vol(iobyte);  // if a volume byte
	if ((iobyte&0x90)==0x80) get_freq(iobyte); // if a frequency byte
	if (!(iobyte&0x80)) get_freq(iobyte);      // if a frequency byte
	if (soundyesno) update_sound();            // update sound

}


void get_vol(ubyte iobyte) {    // used to extract volume data from a byte

	ubyte channel;

	switch (iobyte&0x60) {            // select channel
		case 0x00: channel=3;break;
		case 0x20: channel=2;break;
		case 0x40: channel=1;break;
	}

	vol[channel]=15-(iobyte&(0xF));   // update channel
}

void get_freq(ubyte iobyte) {      // used to extract frequency data from
											  // a byte
	static ubyte channel=0xFF;

	if (iobyte&0x80) {              // select channel

		switch (iobyte&0x60) {
			case 0x00: channel=3; break;
			case 0x20: channel=2;break;
			case 0x40: channel=1;break;
		}

		freqbits[channel]&=0;             // update high order bits of channel
		freqbits[channel]|=(iobyte&0xF);
	}

	if (!(iobyte&0x80)) {          // if a `low order' frequency byte

		freqbits[channel]&=0x000F;
		freqbits[channel]|=(((uint)(iobyte&0x3F))<<4); // update low order bits
	}
}

/*
This function has the fairly difficult task of selecting a suitable single
frequency, which best describes the combined sound of three
variable frequency, variable volume frequencies.

*/

void update_sound(void) {

	uint c,totvol=0;
	double totfreq=0;

	for (c=1;c<4;c++) {    // step through each channel
		totvol+=vol[c];     // keep a running volume count
		if (freqbits[c]==0) continue;   // check to avoid divide by zero

		totfreq+=vol[c]*(4000000./(32.*freqbits[c]));
	}

  if (totvol==0) {        // if no channels are turned on, switch off
		nosound();          // all sound
		return;
  }

	totfreq/=totvol;       // normalise sound
	sound(totfreq);        // and generate it
}
===========================================
-------------------------------------------

-------------------------------------------
===========================================
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <graphics.h>
#include <alloc.h>
#include "header.h"
#include "screen.h"
#include "mnemonic.h"


ubyte debuginfo=0;

ubyte pic_temp_ram[0x8000];

char far *graph_ptr;

void titlepic(void);
FILE *output;

void system_init(void)
{
	int gdriver=VGA,gmode=VGAMED;
	output=fopen("C:\\tc\\source\\softbeeb\\output.dat","wb");

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
		getch();                            // open the picture file
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
		if (kbhit()) {getch(); return;}  // wait for keypress
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
			if (kbhit()) {getch(); return;}   // until keypressed
			}
}===========================================
-------------------------------------------

-------------------------------------------
===========================================
#include <stdio.h>
#include "header.h"

/*
	This file provides names for the instructions, and the data here is
	only ever used by show_regs(). It may be removed before the `final
	release'
*/

char *mnemonic[]={
"BRK",
"ORA (indirect , X)",
"- ",
"-",
"-",
"ORA-Zero Page",
"ASL-Zero Page",
"-",
"PHP",
"ORA-Immediate",
"ASL-Accumulator",
"-",
"-",
"ORA-Absolute",
"ASL-Absolute",
"-",
"BPL",
"ORA (Indirect) , Y",
"-",
"-",
"-",
"ORA-Zero Page , X",
"ASL-Zero Page , X",
"-",
"CLC",
"ORA-Absolute ,Y",
"-",
"-",
"-",
"ORA-Absolute , X",
"ASL  Absolute , X",
"-",
"JSR",
"AND-(Indirect , X)",
"-",
"-",
"BIT-Zero Page",
"AND-Zero Page",
"ROL-Zero Page",
"-",
"PLP",
"AND-Immediate",
"ROL-Accumulator",
"-",
"BIT-Absolute",
"AND-Absolute",
"ROL-Absolute",
"-",
"BMI",
"AND (Indirect) , Y",
"-",
"-",
"-",
"AND-Zero Page , X",
"ROL-Zero Page , X",
"-",
"SEC",
"AND-Absolute , Y",
"-",
"-",
"-",
"AND-Absolute , X",
"ROL-Absolute , X",
"-",
"RTI",
"EOR (Indirect , X)",
"-",
"-",
"-",
"EOR-Zero Page",
"LSR-Zero Page",
"-",
"PHA",
"EOR-Immediate",
"LSR-Accumulator",
"-",
"JMP-Absolute",
"EOR-Absolute",
"LSR-Absolute",
"-",
"BVC",
"EOR (Indirect) , Y",
"-",
"-",
"-",
"EOR-Zero Page , X",
"LSR-Zero Page , X",
"-",
"CLI",
"EOR-Absolute ,Y",
"-",
"-",
"-",
"EOR-Absolute , X",
"LSR-Absolute , X",
"-",
"RTS",
"ADC (Indirect , X)",
"-",
"-",
"-",
"ADC-Zero Page",
"ROR-Zero Page",
"-",
"PLA",
"ADC-Immediate",
"ROR-Accumulator",
"-",
"JMP-Indirect",
"ADC-Absolute",
"ROR-Absolute",
"-",
"BVS",
"ADC (Indirect) , Y",
"-",
"-",
"-",
"ADC-Zero Page , X",
"ROR-Zero Page , X",
"-",
"SEI",
"ADC-Absolute , Y",
"-",
"-",
"-",
"ADC-Absolute , X",
"ROR-Absolute , X",
"-",
"-",
"STA (Indirect , X)",
"-",
"-",
"STY-Zero Page",
"STA-Zero Page",
"STX-Zero Page",
"-",
"DEY",
"-",
"TXA",
"-",
"STY-Absolute",
"STA-Absolute",
"STX-Absolute",
"-",
"BCC",
"STA-(Indirect) , Y",
"-",
"-",
"STY-Zero Page , X",
"STA-Zero Page , X",
"STX-Zero Page, Y",
"-",
"TYA",
"STA-Absolute , Y",
"TXS",
"-",
"-",
"STA-Absolute , X",
"-",
"-",
"LDY-Immediate",
"LDA (Indirect , X)",
"LDX-Immediate",
"-",
"LDY-Zero Page",
"LDA-Zero Page",
"LDX-Zero PAge",
"-",
"TAY",
"LDA-Immediate",
"TAX",
"-",
"LDY-Absolute",
"LDA-Absolute",
"LDX-Absolute",
"-",
"BCS",
"LDA (Indirect) , Y",
"-",
"-",
"LDY-Zero Page , X",
"LDA-Zero Page , X",
"LDX-Zero Page, Y",
"-",
"CLV",
"LDA-Absolute , Y",
"TSX",
"-",
"LDY- Absolute , X",
"LDA-Absolute , X",
"LDX-Absolute ,Y",
"-",
"CPY-Immediate",
"CMP (Indirect , X)",
"-",
"-",
"CPY-Zero Page",
"CMP-Zero Page",
"DEC-Zero Page",
"-",
"INY",
"CMP-Immediate",
"DEX",
"-",
"CPY-Absolute",
"CMP-Absolute",
"DEC-Absolute",
"-",
"BNE",
"CMP (Indirect) , Y",
"-",
"-",
"-",
"CMP-Zero Page , X",
"DEC-Zero Page , X",
"-",
"CLD",
"CMP-Absolute , Y",
"-",
"-",
"-",
"CMP-Absolute , X",
"DEC-Absolute , X",
"-",
"CPX-Immediate",
"SBC (Indirect , X)",
"-",
"-",
"CPX-Zero Page",
"SBC-Zero Page",
"INC-Zero Page",
"-",
"INX",
"SBC-Immediate",
"NOP",
"-",
"CPX-Absolute",
"SBC-Absolute",
"INC-Absolute",
"-",
"BEQ",
"SBC (Indirect) , Y",
"-",
"-",
"-",
"SBC-Zero Page , X",
"INC-Zero Page , X",
"-",
"SED",
"SBC-Absolute , Y",
"-",
"-",
"-",
"SBC-Absolute , X",
"INC-Absolute , X",
"-",
};
===========================================
-------------------------------------------

-------------------------------------------
===========================================
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <graphics.h>
#include <ctype.h>
#include "header.h"
#include "io.h"
#include "screen.h"


#define RED 11
#define BLACK 12
#define WHITE 13
#define BLUE 14
#define YELLOW 15


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

	while(kbhit()) getch();

	for(;c!='\r';c=getch()) {
		switch(tolower(c)) {

			case 'x':
						 setcolor(YELLOW);
						 outtextxy(0,305,"Are you sure you want to quit? Y/N");
						 if(tolower(getch())=='y') quit_prog();
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
						 if(tolower(getch())=='y')
							pc=0xD9CD;
						 setcolor(BLACK);
						 outtextxy(0,305,"Press Y to confirm reset");
						 setcolor(WHITE);
						 break;

			case 's': setcolor(YELLOW);
						 outtextxy(0,305,"Do you want sound? Y/N");
						 if(tolower(getch())=='y') {
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
}===========================================
-------------------------------------------

-------------------------------------------
===========================================
// Standard type definitions

	typedef unsigned char ubyte;
	typedef unsigned int uint;
	typedef signed char sbyte;
// __________________________


// Miscellaneous control functions / data

	extern FILE *output;
	extern ubyte debuginfo;
	extern char *mnemonic[];
	extern void show_regs(void);
	extern void monitor_call(void);
	extern void show_screen(void);
	extern void waitkey(void);
	extern void quit_prog(void);

// ____________________


// Startup Initialisation Routines

	extern void system_init(void);
	extern void graphics_init(void);
	extern void init_decode(void);
	extern void init_mem(void);
	extern void rinit_io(void);
	extern void winit_io(void);
	extern void titlepic();
// __________________________

// Interrupt Handling Functions

	extern void irq(void);
	extern void gen_irq(void);
// _____________________________

// Video Output Control Functions / Data

	extern void (*screen_byte_P)(ubyte,uint);
	extern void text_screen_byte(ubyte,uint);
	extern void teletext_init(void);
	extern void pixel_vid_init(void);
	extern void update_cursor(void);
	extern void set_colour_bits(void);
	extern void set_screen_start(void);
	extern void (*update_screen)(void);
	extern uint ram_screen_start;
	extern ubyte teletext;
	extern ubyte disp_chars;
	extern uint screen_start;
	extern ubyte vidpal[16];

// ___________________________________-


// Address Decoding Functions / Data

	extern ubyte getbyte(uint);
	extern void putbyte (ubyte byte, uint address);
	extern void (*wsheila[256])(ubyte);
	extern ubyte (*rsheila[256])(void);
	extern ubyte romsel;
// _______________________________________________


// Processor Control Data

	extern void (*decode[256])(void);

	extern ubyte RAM[0x8000];          //
	extern ubyte OS_ROM[0x4000];       // Memory Arrays
	extern ubyte lang_ROM[0x4000];     //

	extern uint pc;                    //
	extern ubyte acc;                  // 6502 Internal Registers
	extern ubyte x_reg;                //
	extern ubyte y_reg;                //

	extern ubyte sp;                   //
	extern ubyte ir;                   //
	extern ubyte dyn_p;                //
	extern ubyte result_f;             //
	extern ubyte ovr_f;                // 6502 Status Flags
	extern ubyte brk_f;                //
	extern ubyte dec_f;                //
	extern ubyte intd_f;               //
	extern ubyte carry_f;              //
	extern ubyte except;
// _______________________
===========================================
-------------------------------------------

-------------------------------------------
===========================================
extern void non_opcode(void);  // prototype for function occuring when
										 // an illegal opcode is executed

extern void update_flags(void);
extern void update_dyn_p(void);       // builds a valid status byte for
												  // stack pushing
extern void pushbyte(unsigned char ); // push sub instruction
extern unsigned char popbyte(void);   // pop sub instruction

===========================================
-------------------------------------------

-------------------------------------------
===========================================
/* Graphics Related Declarations. */

extern ubyte crt_regs[18];
extern ubyte crt_sel;
extern void update_cursor(void);
extern ubyte vid_con_reg;
extern ubyte vid_ula;
/*********************************/

/* System Via Related Declarations */

extern ubyte s_via[16];
extern ubyte s_via_latch[8];
extern ubyte s_via_opa;
extern ubyte s_via_ipa;
extern ubyte s_via_opb;
extern ubyte s_via_ipb;
extern ubyte current_latch;
extern ubyte current_key;
extern ubyte current_shift;
extern ubyte current_control;
/**********************************/


/* Sound Related Declarations */

extern void sound_byte(ubyte);
extern void update_sound(void);
extern uint freqbits[4];
extern ubyte vol[4];
extern ubyte soundyesno;
/**********************************/

/* Miscellaneous Declarations */

extern ubyte adc_control;
/**********************************/
===========================================
-------------------------------------------

-------------------------------------------
===========================================
// Video Output Control Functions / Data

	extern void (*screen_byte_P)(ubyte,uint);
	extern void text_screen_byte(ubyte,uint);
	extern void teletext_init(void);
	extern void pixel_vid_init(void);
	extern void update_cursor(void);
	extern void set_colour_bits(void);
	extern void set_screen_start(void);
	extern void (*update_screen)(void);
	extern void flash(void);
	extern uint ram_screen_start;
	extern ubyte teletext;
	extern ubyte disp_chars;
	extern uint screen_start;
	extern ubyte vidpal[16];

	extern void mono_update(void);
	extern void four_update(void);
	extern void sixt_update(void);

	extern ubyte pic_temp_ram[];===========================================
-------------------------------------------

-------------------------------------------
===========================================
#define BRK 0x00
#define PHP 0x08
#define ORA 0x09
#define ASL 0x0A
#define BPL 0x10
#define CLC 0x18
#define JSR 0x20
#define PLP 0x28
#define AND 0x29
#define ROL 0x2A
#define BIT 0x2C
#define BMI 0x30
#define SEC 0x38
#define RTI 0x40
#define PHA 0x48
#define EOR 0x49
#define LSR 0x4A
#define JMP 0x4C
#define BVC 0x50
#define CLI 0x58
#define RTS 0x60
#define PLA 0x68
#define ADC 0x69
#define ROR 0x6A
#define BVS 0x70
#define SEI 0x78
#define DEY 0x88
#define TXA 0x8A
#define STY 0x8C
#define STA 0x8D
#define STX 0x8E
#define BCC 0x90
#define TYA 0x98
#define TXS 0x9A
#define LDY 0xA0
#define LDX 0xA2
#define TAY 0xA8
#define LDA 0xA9
#define TAX 0xAA
#define BCS 0xBO
#define CLV 0xB8
#define TSX 0xBA
#define CPY 0xC0
#define INY 0xC8
#define CMP 0xC9
#define DEX 0xCA
#define DEC 0xCE
#define BNE 0xD0
#define CLD 0xD8
#define CPX 0xE0
#define INX 0xE8
#define SBC 0xE9
#define NOP 0xEA
#define INC 0xEE
#define BEQ 0xF0
#define SED 0xF8
#define END 0xFF
===========================================
-------------------------------------------

-------------------------------------------
===========================================
#define ORB 0
#define ORA 1
#define DDRB 2
#define DDRA 3
#define T1C_L 4
#define T1C_H 5
#define T1L_L 6
#define T1L_H 7
#define T2C_L 8
#define T2C_H 9
#define SR 10
#define ACR 11
#define PCR 12
#define IFR 13
#define IER 14
#define ORA_15 15
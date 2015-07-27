#include <stdio.h>
#include "tc_conio.h"
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
}
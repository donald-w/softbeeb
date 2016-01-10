#pragma once

#include <stdio.h>
#include <stdint.h>

//  Placeholder for asm support
    extern void asm(char *);


// Standard type definitions

	typedef uint8_t ubyte;
	typedef uint16_t uint;
	typedef int8_t sbyte;
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


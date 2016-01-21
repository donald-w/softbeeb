#include <stdio.h>
#include "header.h"
#include "io.h"
#include "reg_name.h"

#define CLKFREQ 8500
// compile time instructions between interrupt counter

/*

Softbeeb- A BBC Model B Microcomputer for the PC
Code begun on 16th December 1995
Designed for Compilation on Turbo C++ For DOS V3.0

Written for CSYS Computing Studies
Copyright Donald Walker

*/

ubyte RAM[0x8000];             // the BBC's ram
uint pc = 0xD9CD;                 // where the processor starts execution

ubyte acc = 0;               //
ubyte x_reg = 0;             //  initialise registers.
ubyte y_reg = 0;             //

ubyte sp = 0xFF;             // initialise the stack pointer.
ubyte dyn_p = 0;             // the dynamic processor status
// update_dyn_p() must be executed
// before use of this variable

/* update_dyn_p uses these flags to construct the
	full processor status byte when required i.e.
	during BRKs, IRQs or PHP's */

ubyte result_f = 0;          // dual purpose flag holding neg and zero.
ubyte ovr_f = 0;             // sign overflow flag
ubyte brk_f = 1;             // set to zero when an IRQ occurs
ubyte dec_f = 0;             // flag indicating decimal mode operation
// (current unsupported)
ubyte intd_f = 0;            // set when interrupts are disabled
ubyte carry_f = 0;           // arithmetic carry flag
ubyte except = 0;            // set for a negative zero condition


int main() {
    uint clock;                                // local clock counter
    ubyte ir = 0;                                // holds current instruction

    system_init();                   // initialise everything.

    for (clock = CLKFREQ; ; clock--)        // interrupt generation timer
    {
        ir = getbyte(pc++);                // fetch instruction
        decode[ir]();              // decode and execute the instructon

        //show_regs();

        if (!clock)                // if an interrupt is due
        {
            if (!intd_f)            // and if interrupts are enabled,
            {
                gen_irq();              // call the interrupt  handler
                clock = CLKFREQ;          // and reset interrupt generation timer
            }
            else(clock++);          // if interrupts are disabled,
            // increment the clock timer so that
            // an interrupt occurs when enabled
        }
    }
}

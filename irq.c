#include <stdio.h>
#include "tc_conio.h"
#include "tc_bios.h"
#include "tc_dos.h"
#include "header.h"
#include "io.h"
#include "reg_name.h"

#define SHIFT current_shift=1
#define CLEAR current_shift=0

ubyte current_key = 0;
ubyte current_shift = 0;
ubyte current_control = 0;

void vert_sync(void);

void getkey(void);

void gen_irq(void) {        // Main interrupt generation and handler routine
    // static type_counter=0; // Added to support c99
    static int type_counter = 0;

    getkey();               // Check for key input and convert pc
    // keypress to BBC internal keynumber

    if (!intd_f)            // if interrupts are enabled
    {
        type_counter++;      // step through different interrupt types
        switch (type_counter) {
            case 1:
            case 2:
                if (s_via[IER] & 0x40) {
                    s_via[IFR] |= 0xC0;
                    irq();
                }
                else s_via[IFR] |= 0x40;
                break;                  // 100 Hz interrupt

            case 3:
                if (s_via[IER] & 0x02) {
                    s_via[IFR] |= 0x82;
                    irq();
                }
                else s_via[IFR] |= 0x02;
                type_counter = 0;         // 50 Hz vertical blank interrupt
                break;
        }
    }
}


void getkey(void) {

    uint pc_scan_code;
    static uint old_scan_code;
    static ubyte pressed = 0;
    // If no key is pressed,
    if (!pressed) {                       // get the shift status
        current_shift = (_bios_keybrd(_KEYBRD_SHIFTSTATUS) & 3);
    }

    if (_bios_keybrd(_KEYBRD_READY)) {    // if a keystroke is waiting
        if (!pressed) {                    // and if not key is being processed

            // Synchronise BBC and PC capslock

            switch ((peekb(0, 0x417) & 0x40) | s_via_latch[6]) {
                case 0x48:
                case 0x00:
                    current_key = 0x40;
                    goto caps_lock_pressed;
            }


            // get pc keystroke
            pc_scan_code = _bios_keybrd(_KEYBRD_READ);
            pressed = 7;                       // hold the key for 7 interrupts

            switch (pc_scan_code & 0xff) {

/**********************************************

		Keyboard Translation statements

**********************************************/

                case 0x8:
                    current_key = 0x59;
                    CLEAR;
                    break;
                case 0x9:
                    current_key = 0x60;
                    CLEAR;
                    break;
                case 0xD:
                    current_key = 0x49;
                    CLEAR;
                    break;
                case 0x1B:
                    current_key = 0x70;
                    CLEAR;
                    break;
                case ' ':
                    current_key = 0x62;
                    CLEAR;
                    break;
                case '!':
                    current_key = 0x30;
                    SHIFT;
                    break;
                case '"':
                    current_key = 0x31;
                    SHIFT;
                    break;
                case '#':
                    current_key = 0x11;
                    SHIFT;
                    break;
                case '$':
                    current_key = 0x12;
                    SHIFT;
                    break;
                case '%':
                    current_key = 0x13;
                    SHIFT;
                    break;
                case '&':
                    current_key = 0x34;
                    SHIFT;
                    break;
                case '\'':
                    current_key = 0x24;
                    SHIFT;
                    break;
                case '(':
                    current_key = 0x15;
                    SHIFT;
                    break;
                case ')':
                    current_key = 0x26;
                    SHIFT;
                    break;
                case '*':
                    current_key = 0x48;
                    SHIFT;
                    break;
                case '+':
                    current_key = 0x57;
                    SHIFT;
                    break;
                case ',':
                    current_key = 0x66;
                    CLEAR;
                    break;
                case '-':
                    current_key = 0x17;
                    CLEAR;
                    break;
                case '.':
                    current_key = 0x67;
                    CLEAR;
                    break;
                case '/':
                    current_key = 0x68;
                    CLEAR;
                    break;
                case '0':
                    current_key = 0x27;
                    CLEAR;
                    break;
                case '1':
                    current_key = 0x30;
                    CLEAR;
                    break;
                case '2':
                    current_key = 0x31;
                    CLEAR;
                    break;
                case '3':
                    current_key = 0x11;
                    CLEAR;
                    break;
                case '4':
                    current_key = 0x12;
                    CLEAR;
                    break;
                case '5':
                    current_key = 0x13;
                    CLEAR;
                    break;
                case '6':
                    current_key = 0x34;
                    CLEAR;
                    break;
                case '7':
                    current_key = 0x24;
                    CLEAR;
                    break;
                case '8':
                    current_key = 0x15;
                    CLEAR;
                    break;
                case '9':
                    current_key = 0x26;
                    CLEAR;
                    break;
                case ':':
                    current_key = 0x48;
                    CLEAR;
                    break;
                case ';':
                    current_key = 0x57;
                    CLEAR;
                    break;
                case '<':
                    current_key = 0x66;
                    SHIFT;
                    break;
                case '=':
                    current_key = 0x17;
                    SHIFT;
                    break;
                case '>':
                    current_key = 0x67;
                    SHIFT;
                    break;
                case '?':
                    current_key = 0x68;
                    SHIFT;
                    break;
                case '@':
                    current_key = 0x47;
                    CLEAR;
                    break;
                case 'A':
                    current_key = 0x41;
                    SHIFT;
                    break;
                case 'B':
                    current_key = 0x64;
                    SHIFT;
                    break;
                case 'C':
                    current_key = 0x52;
                    SHIFT;
                    break;
                case 'D':
                    current_key = 0x32;
                    SHIFT;
                    break;
                case 'E':
                    current_key = 0x22;
                    SHIFT;
                    break;
                case 'F':
                    current_key = 0x43;
                    SHIFT;
                    break;
                case 'G':
                    current_key = 0x53;
                    SHIFT;
                    break;
                case 'H':
                    current_key = 0x54;
                    SHIFT;
                    break;
                case 'I':
                    current_key = 0x25;
                    SHIFT;
                    break;
                case 'J':
                    current_key = 0x45;
                    SHIFT;
                    break;
                case 'K':
                    current_key = 0x46;
                    SHIFT;
                    break;
                case 'L':
                    current_key = 0x56;
                    SHIFT;
                    break;
                case 'M':
                    current_key = 0x65;
                    SHIFT;
                    break;
                case 'N':
                    current_key = 0x55;
                    SHIFT;
                    break;
                case 'O':
                    current_key = 0x36;
                    SHIFT;
                    break;
                case 'P':
                    current_key = 0x37;
                    SHIFT;
                    break;
                case 'Q':
                    current_key = 0x10;
                    SHIFT;
                    break;
                case 'R':
                    current_key = 0x33;
                    SHIFT;
                    break;
                case 'S':
                    current_key = 0x51;
                    SHIFT;
                    break;
                case 'T':
                    current_key = 0x23;
                    SHIFT;
                    break;
                case 'U':
                    current_key = 0x35;
                    SHIFT;
                    break;
                case 'V':
                    current_key = 0x63;
                    SHIFT;
                    break;
                case 'W':
                    current_key = 0x21;
                    SHIFT;
                    break;
                case 'X':
                    current_key = 0x42;
                    SHIFT;
                    break;
                case 'Y':
                    current_key = 0x44;
                    SHIFT;
                    break;
                case 'Z':
                    current_key = 0x61;
                    SHIFT;
                    break;
                case '[':
                    current_key = 0x38;
                    CLEAR;
                    break;
                case '\\':
                    current_key = 0x78;
                    CLEAR;
                    break;
                case ']':
                    current_key = 0x58;
                    CLEAR;
                    break;
                case '^':
                    current_key = 0x18;
                    CLEAR;
                    break;
                case '_':
                    current_key = 0x28;
                    CLEAR;
                    break;
                case '`':
                    current_control = 1;
                    break;
                case 'a':
                    current_key = 0x41;
                    CLEAR;
                    break;
                case 'b':
                    current_key = 0x64;
                    CLEAR;
                    break;
                case 'c':
                    current_key = 0x52;
                    CLEAR;
                    break;
                case 'd':
                    current_key = 0x32;
                    CLEAR;
                    break;
                case 'e':
                    current_key = 0x22;
                    CLEAR;
                    break;
                case 'f':
                    current_key = 0x43;
                    CLEAR;
                    break;
                case 'g':
                    current_key = 0x53;
                    CLEAR;
                    break;
                case 'h':
                    current_key = 0x54;
                    CLEAR;
                    break;
                case 'i':
                    current_key = 0x25;
                    CLEAR;
                    break;
                case 'j':
                    current_key = 0x45;
                    CLEAR;
                    break;
                case 'k':
                    current_key = 0x46;
                    CLEAR;
                    break;
                case 'l':
                    current_key = 0x56;
                    CLEAR;
                    break;
                case 'm':
                    current_key = 0x65;
                    CLEAR;
                    break;
                case 'n':
                    current_key = 0x55;
                    CLEAR;
                    break;
                case 'o':
                    current_key = 0x36;
                    CLEAR;
                    break;
                case 'p':
                    current_key = 0x37;
                    CLEAR;
                    break;
                case 'q':
                    current_key = 0x10;
                    CLEAR;
                    break;
                case 'r':
                    current_key = 0x33;
                    CLEAR;
                    break;
                case 's':
                    current_key = 0x51;
                    CLEAR;
                    break;
                case 't':
                    current_key = 0x23;
                    CLEAR;
                    break;
                case 'u':
                    current_key = 0x35;
                    CLEAR;
                    break;
                case 'v':
                    current_key = 0x63;
                    CLEAR;
                    break;
                case 'w':
                    current_key = 0x21;
                    CLEAR;
                    break;
                case 'x':
                    current_key = 0x42;
                    CLEAR;
                    break;
                case 'y':
                    current_key = 0x44;
                    CLEAR;
                    break;
                case 'z':
                    current_key = 0x61;
                    CLEAR;
                    break;
                case '{':
                    current_key = 0x38;
                    SHIFT;
                    break;
                case '|':
                    current_key = 0x78;
                    SHIFT;
                    break;
                case '}':
                    current_key = 0x58;
                    SHIFT;
                    break;
                case '~':
                    current_key = 0x18;
                    SHIFT;
                    break;
                    //case '�':	current_key=0x28;SHIFT;break; // TODO DOS / ASCII mismatch

                case 0:
                    switch (pc_scan_code / 0x100) {

                        case 0x3B:
                            current_key = 0x71;
                            break;
                        case 0x3C:
                            current_key = 0x72;
                            break;
                        case 0x3D:
                            current_key = 0x73;
                            break;
                        case 0x3E:
                            current_key = 0x14;
                            break;
                        case 0x3F:
                            current_key = 0x74;
                            break;
                        case 0x40:
                            current_key = 0x75;
                            break;
                        case 0x41:
                            current_key = 0x16;
                            break;
                        case 0x42:
                            current_key = 0x76;
                            break;
                        case 0x43:
                            current_key = 0x77;
                            break;
                        case 0x44:
                            current_key = 0x20;
                            break;
                            /*	case 0x47: monitor_call();return;   The home key */
                        case 0x48:
                            current_key = 0x39;
                            break;
                        case 0x4B:
                            current_key = 0x19;
                            break;
                        case 0x4D:
                            current_key = 0x79;
                            break;
                        case 0x4F:
                            current_key = 0x69;
                            break;
                        case 0x50:
                            current_key = 0x29;
                            break;
                    }
            }


/**********************************************
**********************************************/

            caps_lock_pressed:

            if (s_via[IER] & 0x01) {    // if relevant interrupts are enabled
                s_via[IFR] |= 0x81;      // generate the interrupt
                irq();
            }
            else s_via[IFR] |= 0x01;
            old_scan_code = pc_scan_code; // store a copy of the current key
        }
            // discard key repeats of the same key
        else if (_bios_keybrd(_KEYBRD_READY) == old_scan_code) coniogetch();
    }

    if (!pressed) {

        /* TODO this is an x86 keyboard read
        asm("IN AL,0x60"); */
        asm("push AX");
        asm("pop pc_scan_code");

        if (pc_scan_code & 0x80) {      // detect if no keys are physically
            current_key = 0;             // pressed
            current_shift = 0;
            current_control = 0;
        }
    }


    if (pressed) pressed--;                // decrease the current key counter
}

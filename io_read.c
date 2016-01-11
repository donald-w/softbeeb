#include<stdio.h>
#include "header.h"
#include "reg_name.h"
#include "io.h"

ubyte (*rsheila[256])(void);   // array of pointers to i/o handling funcs

ubyte current_latch;           // last value sent to latch

ubyte s_via_opa = 0;            // system via output port a
ubyte s_via_ipa = 0;            // system via input  port a
ubyte s_via_opb = 0;            // system via output port b
ubyte s_via_ipb = 0;            // system via input  port b


/*
The following functions { of the form s??loc()   } return
the value ??. These are used in memory mapped locations
which always return a constant value
(to the best of my knowledge).
*/

ubyte s00loc() {
    return 0x00;
}

ubyte s02loc() {
    return 0x02;
}

ubyte s10loc() {
    return 0x10;
}

ubyte s20loc() {
    return 0x20;
}

ubyte s28loc() {
    return 0x28;
}

ubyte s9Dloc() {
    return 0x9D;
}

ubyte sABloc() {
    return 0xAB;
}

ubyte sB7loc() {
    return 0xB7;
}

ubyte sFEloc() {
    return 0xFE;
}

ubyte sFFloc() {
    return 0xFF;
}

// Return the values of crt_regs 14,15,16,17 decimal else return 0

ubyte crt_reg(void) {
    if ((crt_sel > 17) || (crt_sel < 14)) return 0;
    return crt_regs[crt_sel];
}


/******************************************************
******************************************************/


/*
Functions of the form ubyte sr_via_?(void) are used when reading from
the system via. with ? representing the register in hexadecimal.
*/

ubyte sr_via_0(void) {
    return 0xF0 | current_latch;   // return the last value sent to latch
}

ubyte sr_via_1(void) {
    if ((current_key) && (s_via_opa == current_key)) return (current_key | 0x80);
    if ((s_via_opa == 0) && (current_shift)) return 0x80;
    if ((s_via_opa == 1) && (current_control)) return 0x81;
    return s_via_opa;
}

ubyte sr_via_2(void) {
    return s_via[DDRB];
}

ubyte sr_via_3(void) {
    return s_via[DDRA];
}

ubyte sr_via_4(void) {
    return 0;
}

ubyte sr_via_5(void) {
    return 0;
}

ubyte sr_via_6(void) {
    return 0;
}

ubyte sr_via_7(void) {
    return 0;
}

ubyte sr_via_8(void) {
    return 0;
}

ubyte sr_via_9(void) {
    return 0;
}

ubyte sr_via_A(void) {
    return 0;
}

ubyte sr_via_B(void) {
    return 0;
}

ubyte sr_via_C(void) {
    return 0;
}

ubyte sr_via_D(void) {
    return (s_via[IFR]);
}

ubyte sr_via_E(void) {
    return (s_via[IER] | 0x80);
}

ubyte sr_via_F(void) {
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

void rinit_io(void) {
    unsigned int c;

    printf("Init read IO system...");    // display pretty startup status

    for (c = 0; c <= 0xFF; c++) {
        rsheila[c] = s00loc;
    }

    for (c = 0x00; c <= 0x07; c += 2) {
        rsheila[c] = s00loc;
        rsheila[c + 1] = crt_reg;
    }

    for (c = 0x08; c <= 0xF; c += 2) {
        rsheila[c] = s02loc;
        rsheila[c + 1] = sFFloc;
    }

    for (c = 0x10; c <= 0x17; c++) {
        rsheila[c] = s00loc;
    }

    for (c = 0x18; c <= 0x1F; c++) {
        rsheila[c] = sB7loc;
    }

    for (c = 0x20; c <= 0x3F; c++) {
        rsheila[c] = sFEloc;
    }

    rsheila[0x50] = rsheila[0x40] = sr_via_0;
    rsheila[0x51] = rsheila[0x41] = sr_via_1;
    rsheila[0x52] = rsheila[0x42] = sr_via_2;
    rsheila[0x53] = rsheila[0x43] = sr_via_3;
    rsheila[0x54] = rsheila[0x44] = sr_via_4;
    rsheila[0x55] = rsheila[0x45] = sr_via_5;
    rsheila[0x56] = rsheila[0x46] = sr_via_6;
    rsheila[0x57] = rsheila[0x47] = sr_via_7;
    rsheila[0x58] = rsheila[0x48] = sr_via_8;
    rsheila[0x59] = rsheila[0x49] = sr_via_9;
    rsheila[0x5A] = rsheila[0x4A] = sr_via_A;
    rsheila[0x5B] = rsheila[0x4B] = sr_via_B;
    rsheila[0x5C] = rsheila[0x4C] = sr_via_C;
    rsheila[0x5D] = rsheila[0x4D] = sr_via_D;
    rsheila[0x5E] = rsheila[0x4E] = sr_via_E;
    rsheila[0x5F] = rsheila[0x4F] = sr_via_F;


    for (c = 0x60; c <= 0x7F; c++) {
        rsheila[c] = s00loc;
    }

    for (c = 0x80; c <= 0x9F; c += 8) {
        rsheila[c + 0] = s00loc;
        rsheila[c + 1] = sFFloc;
        rsheila[c + 2] = s9Dloc;
        rsheila[c + 3] = sFFloc;
        rsheila[c + 4] = sABloc;
        rsheila[c + 5] = sABloc;
        rsheila[c + 6] = sABloc;
        rsheila[c + 7] = sABloc;
    }

    for (c = 0xA0; c <= 0xBF; c += 4) {
        rsheila[c + 0] = s10loc;
        rsheila[c + 1] = s20loc;
        rsheila[c + 2] = s28loc;
        rsheila[c + 3] = s28loc;
    }

    for (c = 0xC0; c <= 0xDF; c++) {
        rsheila[c] = s00loc;
    }

    for (c = 0xE0; c <= 0xFF; c++) {
        rsheila[c] = sFEloc;
    }

    printf("Done\n");     // display pretty startup status

}

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
extern void non_opcode(void);  // prototype for function occuring when
										 // an illegal opcode is executed

extern void update_flags(void);
extern void update_dyn_p(void);       // builds a valid status byte for
												  // stack pushing
extern void pushbyte(unsigned char ); // push sub instruction
extern unsigned char popbyte(void);   // pop sub instruction


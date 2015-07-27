#ifndef SOFTBEEB_TC_DOS_H
#define SOFTBEEB_TC_DOS_H

extern void nosound();
extern void sound(double totfreq);
extern void delay(int delay);
extern char peekb(int segment,int offset);
extern void pokeb(int segment, int offset, char value);

#endif //SOFTBEEB_TC_DOS_H



#include "header.h"
#include "io.h"
#include "tc_dos.h"


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
		default: channel=0; // Added to prevent compiler warning.  Check behaviour. 2016
	}

	vol[channel]=((ubyte)15)-(iobyte&((ubyte)0xF));   // update channel
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
	    nosound();        // all sound
		return;
  }

	totfreq/=totvol;       // normalise sound
	sound(totfreq);        // and generate it
}

#include "util.h"

#include <stdint.h>
#include <stdio.h>
#include <conio.h>

#define PB0 ((char*)0xC061)
#define PB1 ((char*)0xC062)
char read_applekey(void) { return ((*PB0) | (*PB1)) & 0x80; }

void wait(char frames) { 
	char i;
	for (i = 0; i < frames; i++) {
		unsigned int l;
		for (l = 0; *VBL < 0 && l < 2500; l++);
		for (l = 0; *VBL >= 0 && l < 2500; l++);
	}
}

#define SPIN_HALFCYCLES 3
#define SPIN_FRAMESPERCHAR 4
void spin(uint8_t x, uint8_t y) { 
	char i;

	// Sync to frame before starting
	wait(1);

	// Wait and animate spinner.
	// Spin_half
	for (i = 0; i < SPIN_HALFCYCLES; i++) {
		char j;
		for (j = 0; j < 4; j++) {
			char spinchar;
			// Assign spinner char based on j
			switch (j) {
				case 0: spinchar = '\\'; break;
				case 1: spinchar = '|'; break;
				case 2: spinchar = '/'; break;
				case 3: spinchar = '-'; break;
				default: spinchar = '-'; break;
			}

			// Write it to screen
			gotoxy(x, y);
			putchar(spinchar);

			// Wait specificed number of frames
			wait(SPIN_FRAMESPERCHAR);
		}
	}

	// Wait a frame when finished
	wait(1);
}

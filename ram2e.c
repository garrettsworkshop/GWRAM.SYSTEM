#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#define true 1
#define false 0

static char read_applekey(void) {
	/*__asm__("lda $C061");
	__asm__("ora $C062");
	__asm__("and #$80");
	__asm__("ldx #0");*/
	return (*((char*)0xC061) | *((char*)0xC062)) & 0x80;
}

static void set_mask_temp(char mask) {
	__asm__("ldx %o", mask);

	__asm__("lda #$FF");
	__asm__("sta $C073");

	__asm__("lda #$00");
	__asm__("sta $C073");

	__asm__("lda #$55");
	__asm__("sta $C073");

	__asm__("lda #$AA");
	__asm__("sta $C073");

	__asm__("lda #$C1");
	__asm__("sta $C073");

	__asm__("lda #$AD");
	__asm__("sta $C073");

	__asm__("lda #$E0");
	__asm__("sta $C073");

	__asm__("stx $C073");
}

static void ufm_bitbang(char bitbang) {
	__asm__("ldx %o", bitbang);

	__asm__("lda #$FF");
	__asm__("sta $C073");

	__asm__("lda #$00");
	__asm__("sta $C073");

	__asm__("lda #$55");
	__asm__("sta $C073");

	__asm__("lda #$AA");
	__asm__("sta $C073");

	__asm__("lda #$C1");
	__asm__("sta $C073");

	__asm__("lda #$AD");
	__asm__("sta $C073");

	__asm__("lda #$EA");
	__asm__("sta $C073");

	__asm__("stx $C073");
}

static void set_nvm(char mask) {
	int i;
	// Shift mask into UFMD twice
	for (i = 0; i < 2; i++) {
		char maskpart;

		maskpart = 0x80 | ((mask >> 1) & 0x40);
		ufm_bitbang(maskpart);

		maskpart = 0x80 | ((mask     ) & 0x40);
		ufm_bitbang(maskpart);

		maskpart = 0x80 | ((mask << 1) & 0x40);
		ufm_bitbang(maskpart);

		maskpart = 0x80 | ((mask << 2) & 0x40);
		ufm_bitbang(maskpart);

		maskpart = 0x80 | ((mask << 3) & 0x40);
		ufm_bitbang(maskpart);

		maskpart = 0x80 | ((mask << 4) & 0x40);
		ufm_bitbang(maskpart);

		maskpart = 0x80 | ((mask << 5) & 0x40);
		ufm_bitbang(maskpart);

		maskpart = 0x80 | ((mask << 6) & 0x40);
		ufm_bitbang(maskpart);
	}
}

static void menu(void)
{
	gotoxy(5, 1);
	cputs("-- RAM2E Capacity Settings --");

	gotoxy(1, 4);
	cputs("Select desired memory capacity:");

	gotoxy(4, 6);
	cputs("1. 64 kilobytes");
	gotoxy(4, 8);
	cputs("2. 512 kilobytes");
	gotoxy(4, 10);
	cputs("3. 1 megabyte");
	gotoxy(4, 12);
	cputs("4. 4 megabytes");
	gotoxy(4, 14);
	cputs("5. 8 megabytes");

	gotoxy(1, 17);
	cputs("Capacity will be saved until power-off.");

	gotoxy(1, 19);
	cputs("To remember capacity setting in");
	gotoxy(1, 20);
	cputs("nonvolatile memory, press Apple+number.");

	gotoxy(1, 22);
	cputs("Press [Q] to quit without saving.");
}

static void spin(uint8_t x, uint8_t y) { 
	int i;
	const uint8_t spin_big = 10;
	const uint8_t spin_small = 150;

	for (i = 0; i < spin_big; i++) {
		int j;
		for (j = 0; j < spin_small; j++) {
			__asm__("lda $C000");
			__asm__("lda $C100");
			gotoxy(x, y);
			__asm__("lda $C000");
			__asm__("lda $C100");
			cputs("-");
			__asm__("lda $C000");
			__asm__("lda $C100");
		}
		for (j = 0; j < spin_small; j++) {
			__asm__("lda $C000");
			__asm__("lda $C100");
			gotoxy(x, y);
			__asm__("lda $C000");
			__asm__("lda $C100");
			cputs("\\");
			__asm__("lda $C000");
			__asm__("lda $C100");
		}
		for (j = 0; j < spin_small; j++) {
			__asm__("lda $C000");
			__asm__("lda $C100");
			gotoxy(x, y);
			__asm__("lda $C000");
			__asm__("lda $C100");
			cputs("|");
			__asm__("lda $C000");
			__asm__("lda $C100");
		}
		for (j = 0; j < spin_small; j++) {
			__asm__("lda $C000");
			__asm__("lda $C100");
			gotoxy(x, y);
			__asm__("lda $C000");
			__asm__("lda $C100");
			cputs("/");
			__asm__("lda $C000");
			__asm__("lda $C100");
		}
	}
}

int main(void)
{
	char key;
	char mask;
	char nvm;

	clrscr();

	// Make sure we are running on an Apple IIe
	if((get_ostype() & 0xF0) != APPLE_IIE) {
		gotoxy(0, 8);
		cputs(" THIS PROGRAM REQUIRES AN APPLE IIE.");
		gotoxy(0, 10);
		cputs(" PRESS ANY KEY TO QUIT.");
		cgetc();
		clrscr();
		return EXIT_SUCCESS;
	}

	// Show menu
	menu();

	// Get user choice
	mask = 0;
	nvm = 0;
	while (true) {
		key = toupper(cgetc());

		// Set capacity mask or quit according to keypress
		if (key == 'Q') {
			// Quit
			clrscr();
			return EXIT_SUCCESS;
		}
		else if (key == '1') { mask = 0x00; }
		else if (key == '2') { mask = 0x07; }
		else if (key == '3') { mask = 0x0F; }
		else if (key == '4') { mask = 0x3F; }
		else if (key == '5') { mask = 0x7F; }
		else { continue; }

		// Check if pressed with apple key
		if (read_applekey()) { nvm = true; }
		break;
	}

	// Set capacity in volatile memory
	set_mask_temp(mask);

	clrscr();
	if (nvm) { // Save in NVM if requested
		set_nvm(mask);

		gotoxy(1, 8);
		cputs("Saving RAM2E capacity setting.");
		gotoxy(1, 9);
		cputs("Do not turn off your Apple.");

		spin(32, 8);

		clrscr();

		gotoxy(1, 8);
		cputs("RAM2E capacity saved successfully.");
	} else { // Print success message
		gotoxy(1, 8);
		cputs("RAM2E capacity set successfully.");
	}

	gotoxy(1, 10);
	cputs("Press any key to quit.");
	cgetc();

	// Quit
	clrscr();
	return EXIT_SUCCESS;
}

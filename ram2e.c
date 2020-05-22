#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#define true 1
#define false 0


#define VBL ((signed char*)0xC019)
const uint8_t SPIN_HALFCYCLES = 3;
const uint8_t SPIN_FRAMESPERCHAR = 4;

#define PB0 ((char*)0xC061)
#define PB1 ((char*)0xC062)
static char read_applekey(void) { return (*PB0 | *PB1) & 0x80; }

char _cmd;
char _arg;
/* ram2e_cmd(...) issues a coded command+argument sequence to the RAM2E */
static void ram2e_cmd(char cmd, char arg) {
	// Load operation and data bytes into X and Y registers
	// in preparation for command sequence
	_cmd = cmd;
	_arg = arg;
	__asm__("ldx %v", _cmd); // X = command
	__asm__("ldy %v", _arg); // Y = argument

	// First, reset command sequence just in case it,
	// for some reason, has not timed out. (e.g. crazy fast accelerator?)
	// Write 0 twice because command and argument steps always advance seq.
	__asm__("lda #0");
	__asm__("sta $C073");
	__asm__("sta $C073");

	// Command sequence
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
	// Command
	__asm__("stx $C073");
	// Argument
	__asm__("sty $C073");

	// Reset RAMWorks bank register just in case
	__asm__("lda #0");
	__asm__("sta $C073");
}

/* set_mask_temp(...) sends the "Set RAMWorks Capacity Mask" to the RAM2E */
static void set_mask_temp(char mask) { ram2e_cmd(0xE0, mask); }

/* ufm_bitbang(...) sends the "Set UFM Bitbang Outputs" to the RAM2E */
static void ufm_bitbang(char bitbang) { ram2e_cmd(0xEA, bitbang); }

/* ufm_program(...) sends the "UFM Program Once" command to the RAM2E */
static void ufm_program() { ram2e_cmd(0xEF, 0x00); }

/* ufm_erase(...) sends the "UFM Erase Once" command to the RAM2E */
static void ufm_erase() { ram2e_cmd(0xEE, 0x00); }

/* set_mask_temp(...) sends the "Set RAMWorks Capacity Mask" */
static void set_nvm(char mask) {
	int i;
	// Shift mask OR'd with data register clock pulse trigger into UFMD twice
	for (i = 0; i < 2; i++) {
		ufm_bitbang(0x80 | ((mask >> 1) & 0x40));
		ufm_bitbang(0x80 | ((mask >> 0) & 0x40));
		ufm_bitbang(0x80 | ((mask << 1) & 0x40));
		ufm_bitbang(0x80 | ((mask << 2) & 0x40));
		ufm_bitbang(0x80 | ((mask << 3) & 0x40));
		ufm_bitbang(0x80 | ((mask << 4) & 0x40));
		ufm_bitbang(0x80 | ((mask << 5) & 0x40));
		ufm_bitbang(0x80 | ((mask << 6) & 0x40));
	}
	// Program UFM
	ufm_program();
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
	char i;

	// Sync to frame before starting
	while (*VBL >= 0);

	// Wait and animate spinner.
	// Spin_half
	for (i = 0; i < SPIN_HALFCYCLES; i++) {
		char j;
		for (j = 0; j < 4; j++) {
			char spinchar;
			char k;

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
			for (k = 0; k < SPIN_FRAMESPERCHAR; k++) {
				while (*VBL < 0);
				while (*VBL >= 0);
			}
		}
	}

	// Wait a frame when finished
	while (*VBL < 0);
	while (*VBL >= 0);
}

int main(void)
{
	char mask;
	char nvm;
	int reset_count;

	// First clear screen
	clrscr();

	// Make sure we are running on an Apple IIe
	if((get_ostype() & 0xF0) != APPLE_IIE) {
		// If not on Apple IIe, show an error message and quit
		gotoxy(0, 8);
		cputs(" THIS PROGRAM REQUIRES AN APPLE IIE.");
		gotoxy(0, 10);
		cputs(" PRESS ANY KEY TO QUIT.");
		cgetc(); // Wait for key
		clrscr(); // Clear screen before quitting
		return EXIT_SUCCESS;
	}

	// Show menu
	menu();

	// Get user choice from menu
	mask = 0;
	nvm = 0;
	reset_count = 0;
	while (true) {
		// Set capacity mask or quit according to keypress.
		switch (toupper(cgetc())) {
			case 'Q' : {
				clrscr();
				return EXIT_SUCCESS;
			}
			case '1': mask = 0x00; break;
			case '2': mask = 0x07; break;
			case '3': mask = 0x0F; break;
			case '4': mask = 0x3F; break;
			case '5': mask = 0x7F; break;
			case 'R': {
				if (reset_count > 127) {
					ufm_erase();
					reset_count = 0;
				} else { reset_count++; }
			} default: continue;
		}

		// Check if pressed with apple key.
		// If so, save to nonvolatile memory.
		if (read_applekey()) { nvm = true; }
		break;
	}

	// Set capacity in volatile memory.
	set_mask_temp(mask);

	// Clear screen in preparation to show saving or success message.
	clrscr();

	if (nvm) { // Save in NVM if requested.
		// Show message about saving.
		gotoxy(1, 8);
		cputs("Saving RAM2E capacity setting.");
		gotoxy(1, 9);
		cputs("Do not turn off your Apple.");

		// Save capacity in nonvolatile memory.
		set_nvm(mask);

		// Wait for >= 500ms on even the fastest systems.
		spin(32, 8);

		// Clear screen again.
		clrscr();

		gotoxy(1, 8);
		cputs("RAM2E capacity saved successfully.");
	} else { // Print success message if not saving in NVM.
		gotoxy(1, 8);
		cputs("RAM2E capacity set successfully.");
	}
	gotoxy(1, 10);
	cputs("Press any key to quit.");
	gotoxy(1, 11);
	cputs("You may also turn off your Apple.");
	cgetc();

	// Quit
	clrscr();
	return EXIT_SUCCESS;
}

#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "util.h"
#include "gwconio.h"

static char _rwsave[256];
static char _rwsave0_1;
static char _rwsave0_2;
static char _rwsave0_3;
static void ramworks_save() {
	__asm__("sta $C009"); // Store in ALTZP
	
	// Save address 0x0000 in every bank
	__asm__("ldx #0");
	saveloop:
	__asm__("stx $C073");
	__asm__("lda $00,X");
	__asm__("sta %v,X", _rwsave);
	__asm__("inx");
	__asm__("bne %g", saveloop);

	// Save addresses 0x0001-3 in bank 0
	__asm__("ldx #0");
	__asm__("stx $C073");
	__asm__("lda $01");
	__asm__("sta %v", _rwsave0_1);
	__asm__("lda $02");
	__asm__("sta %v", _rwsave0_2);
	__asm__("lda $03");
	__asm__("sta %v", _rwsave0_3);

	__asm__("sta $C008"); // Don't store in ALTZP
}

static void ramworks_restore() {
	__asm__("sta $C009"); // Store in ALTZP
	
	// Restore address 0x0000 in every bank
	__asm__("ldx #0");
	restoreloop:
	__asm__("stx $C073");
	__asm__("lda %v,X", _rwsave);
	__asm__("sta $00,X");
	__asm__("inx");
	__asm__("bne %g", restoreloop);

	// Restore addresses 0x0001-3 in bank 0
	__asm__("ldx #0");
	__asm__("stx $C073");
	__asm__("lda %v", _rwsave0_1);
	__asm__("sta $01");
	__asm__("lda %v", _rwsave0_2);
	__asm__("sta $02");
	__asm__("lda %v", _rwsave0_3);
	__asm__("sta $03");

	__asm__("sta $C008"); // Don't store in ALTZP
}

static char _cmd;
static char _arg;
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

/* auxram_detect() returns true if a RAMWorks memory is detected */
static char auxram_detect() {
	// Switch to RW bank 0 for ZP
	__asm__("lda #$00"); // Get 0x00
	__asm__("sta $C009"); // Store in ALTZP
	__asm__("sta $C073"); // Set RW bank 0

	// Store 00 FF 55 AA in RW bank 0 ZP
	__asm__("lda #$00");
	__asm__("sta $00");
	__asm__("lda #$FF");
	__asm__("sta $01");
	__asm__("lda #$55");
	__asm__("sta $02");
	__asm__("lda #$AA");
	__asm__("sta $03");

	// Check for 00 FF 55 AA
	__asm__("lda $00");
	__asm__("cmp #$00");
	__asm__("bne %g", noramworks);
	__asm__("lda $01");
	__asm__("cmp #$FF");
	__asm__("bne %g", noramworks);
	__asm__("lda $02");
	__asm__("cmp #$55");
	__asm__("bne %g", noramworks);
	__asm__("lda $03");
	__asm__("cmp #$AA");
	__asm__("bne %g", noramworks);

	// Found aux ram card
	__asm__("sta $C008"); // Don't store in ALTZP
	return true;

	// Not found
	noramworks:
	__asm__("sta $C008"); // Don't store in ALTZP
	return false;
}

/* ram2e_detect() returns true if a RAM2E II has been detected */
static uint8_t _detect;
static char ram2e_detect() {
	#ifdef SKIP_RAM2E_DETECT
	return true;
	#endif
	__asm__("sta $C009"); // Store in ALTZP
	
	// Store 0x00 at beginning of bank 0x00
	__asm__("lda #$00");
	__asm__("sta $C073");
	__asm__("sta $00");

	// Send SetRWBankFF command
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
	__asm__("lda #$FF");
	__asm__("sta $C073");
	__asm__("lda #$00");
	__asm__("sta $C073");
	// Now bank should be 0xFF if we are running on a RAM2E II
	// Other RAMWorks cards will instead set the bank to 0x00

	// Store 0xFF in this bank
	__asm__("lda #$FF");
	__asm__("sta $00");

	// Go back to bank 0
	__asm__("lda #$00");
	__asm__("sta $C073");
	__asm__("sta $C073");

	// Save result and return
	__asm__("lda $00"); // Get beginning of bank 0
	__asm__("sta $C008"); // Store in STDZP
	__asm__("sta %v", _detect); // Save in _detect
	return _detect == 0x00;
}

/* ramworks_getsize() returns the number of banks of RAM2E aux memory */
static uint8_t _rwsize;
static uint8_t _rwnot16mb;
static uint16_t ramworks_getsize() {
	_rwnot16mb = 1; // Set "not 16 mb" flag

	// Store bank number at address 0 in each bnak
	__asm__("sta $C009"); // ALTZP
	__asm__("ldy #$FF"); // Start at bank 0xFF
	BankSetLoop:
	__asm__("sty $C073"); // Set bank
	__asm__("sty $00"); // Store bank number at 0
	__asm__("dey"); // Prev. bank
	__asm__("cpy #$FF"); // Have we wrapped around?
	__asm__("bne %g", BankSetLoop); // If not, repeat

	// Count banks with matching bank number
	__asm__("ldy #$00"); // Y is bank
	__asm__("ldx #$00"); // X is count
	CountLoop:
	__asm__("sty $C073"); // Set bank
	__asm__("cpy $00"); // Is bank num stored at address 0?
	__asm__("bne %g", AfterInc); // If not, skip increment
	__asm__("inx"); // If so, increment bank count
	__asm__("bne %g", AfterInc); // Skip next if x!=0
	__asm__("stx %v", _rwnot16mb); // Othwerwise rolled over so clear rwnot16mb
	AfterInc:
	__asm__("iny"); // Move to next bank
	__asm__("bne %g", CountLoop); // Repeat if not on bank 0

	// Done. Switch back to regular zeropage and get result.
	__asm__("sta $C008"); // STDZP
	__asm__("stx %v", _rwsize); // _rwsize = X (bank count)

	if (_rwnot16mb) { return _rwsize; }
	else { return 256; }
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

static void menu(uint16_t bankcount)
{
	gwcputsxy(5, 1, "-- RAM2E Capacity Settings --");
	
	if (bankcount < 2) { gotoxy(5, 3); }
	else { gotoxy(4, 3); }
	gwcputs("Current RAM2E capacity: ");
	printf("%d", bankcount * 64);
	gwcputs(" kB");

	gwcputsxy(1, 5, "Select desired memory capacity:");

	gwcputsxy(4, 7, "1. 64 kilobytes");
	gwcputsxy(4, 9, "2. 512 kilobytes");
	gwcputsxy(4, 11, "3. 1 megabyte");
	gwcputsxy(4, 13, "4. 4 megabytes");
	gwcputsxy(4, 15, "5. 8 megabytes");

	gwcputsxy(1, 18, "Capacity will be saved until power-off.");

	gwcputsxy(1, 20, "To remember capacity setting in");
	gwcputsxy(1, 21, "nonvolatile memory, press Apple+number.");

	gwcputsxy(1, 23, "Press [Q] to quit without saving.");
}

int ram2e_main(void)
{
	char mask;
	char nvm;
	int reset_count;
	uint16_t bankcount;

	// Check for RAM2E
	ramworks_save(); // Save what will be clobbered
	if(!auxram_detect() || !ram2e_detect()) {
		ramworks_restore();
		// If no RAM2E, show an error message and quit
		gwcputsxy(0, 8, " No RAM2E II detected.");
		gwcputsxy(0, 10, " Press any key to quit.");
		cgetc(); // Wait for key
		clrscr(); // Clear screen before quitting
		return EXIT_SUCCESS;
	}

	// Get size and print menu 
	bankcount = ramworks_getsize();
	menu(bankcount); // Print menu
	ramworks_restore(); // Restore RAMWorks contents

	// Get user choice from menu
	mask = 0;
	nvm = 0;
	reset_count = 0;
	while (true) {
		// Set capacity mask or quit according to keypress.
		switch (toupper(cgetc() & 0x7F)) {
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
				reset_count++;
				if (reset_count >= 25) {
					// Show message about resetting.
					clrscr(); // Clear screen
					gwcputsxy(1, 8, "Resetting RAM2E settings.");
					gwcputsxy(1, 9, "Do not turn off your Apple.");

					ufm_erase(); // Erase RAM2E settings memory
					set_mask_temp(0x7F); // Set mask to default (0x7F)

					// Wait for >= 500ms on even the fastest systems.
					spin(32, 8);
					
					// Show success message and quit
					clrscr(); // Clear screen
					gwcputsxy(1, 8, "RAM2E settings reset successfully.");
					goto end;
				}
				continue;
			} default: reset_count = 0; continue;
		}

		// Check if pressed with apple key. If so, save to nonvolatile memory.
		if (read_applekey()) { nvm = true; }
		break;
	}

	// Set capacity in volatile memory.
	set_mask_temp(mask);

	// Clear screen in preparation to show saving or success message.
	clrscr();

	if (nvm) { // Save in NVM if requested.
		// Show message about saving.
		gwcputsxy(1, 8, "Saving RAM2E capacity setting.");
		gwcputsxy(1, 9, "Do not turn off your Apple.");
		// Save capacity in nonvolatile memory.
		set_nvm(mask);
		// Wait for >= 500ms on even the fastest systems.
		spin(32, 8);
		// Print success message
		clrscr(); // Clear screen
		gwcputsxy(1, 8, "RAM2E capacity saved successfully.");
	} else { // Print success message if not saving in NVM.
		gwcputsxy(1, 8, "RAM2E capacity set successfully.");
	}

	end:
	if (nvm) { // Show end message for nonvolatile save
		gwcputsxy(1, 10, "You may now turn off your Apple.");
		gwcputsxy(1, 12, "You may also reset your Apple for");
		gwcputsxy(1, 13, "the setting change to take effect.");
	} else { // Show end message for volatile save
		gwcputsxy(1, 10, "Please reset your Apple for");
		gwcputsxy(1, 11, "the setting change to take effect.");
	}
	// Don't quit. Instead leave prompt asking user to reset.
	while(1) { cgetc(); }
}

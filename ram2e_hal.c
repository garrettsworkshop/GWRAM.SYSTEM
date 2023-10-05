#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include "util.h"
#include "ram2e_hal.h"

static char _cmd_cmd;
static char _cmd_arg;
/* ram2e_cmd(...) issues a coded command+argument sequence to the RAM2E */
static void ram2e_cmd(char cmd, char arg) {
	// Load operation and data bytes into X and Y registers
	// in preparation for command sequence
	_cmd_cmd = cmd;
	_cmd_arg = arg;
	__asm__("ldx %v", _cmd_cmd); // X = command
	__asm__("ldy %v", _cmd_arg); // Y = argument

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
char auxram_detect() {
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
char ram2e_detect(char command) {
	#ifdef SKIP_RAM2E_DETECT
	return true;
	#endif
	// Move command into X register
	_detect = command;
	__asm__("lda %v", _detect); // Save in _detect
	__asm__("tax"); 
	
	// Store in ALTZP
	__asm__("sta $C009");
	
	// Store 0x00 at beginning of bank 0x00
	__asm__("lda #$00");
	__asm__("sta $C073");
	__asm__("sta $00");

	// Send SetRWBankFF-class command
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
	__asm__("stx $C073");
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
uint16_t ramworks_getsize() {
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

/* ram2e_set_mask(...) sends the "Set RAMWorks Capacity Mask" to the RAM2E */
void ram2e_set_mask(char mask) { ram2e_cmd(0xE0, mask); }

/* ram2e_set_led(...) */
void ram2e_set_led(char enled) { ram2e_cmd(0xE2, enled); }


static void ram2e_flashled1() {
	__asm__("sta $C009"); // ALTZP
	
	// Save address 0x0000 in every bank
	__asm__("lda $0");
	__asm__("lda $0");
	__asm__("lda $0");
	__asm__("lda $0");
	__asm__("lda $0");
	__asm__("lda $0");
	__asm__("lda $0");
	__asm__("lda $0");

	__asm__("sta $C008"); // No ALTZP
}
void ram2e_flashled(char frames) {
	char i;
	for (i = 0; i < frames; i++) {
		unsigned int l;
		for (l = 0; *VBL <  0 && l < 2500; l++) { ram2e_flashled1(); }
		for (l = 0; *VBL >= 0 && l < 2500; l++) { ram2e_flashled1(); }
	}
}

#include "ram2e_hal_max.c"
#include "ram2e_hal_spi.c"
#include "ram2e_hal_lcmxo2.c"

static char _type;
void ram2e_hal_set_type(char type) { _type = type; }

void ram2e_erase() {
	switch (_type) {
		case 0xFF: ram2e_max_erase(); break; // Altera MAX II / V
		case 0xFE: ram2e_spi_erase(); break; // Lattice MachXO / iCE40 / AGM AG256
		case 0xFD: ram2e_lcmxo2_erase(); break; // Lattice MachXO2
	}
}
void ram2e_save_start(char mask, char enled) {
	switch (_type) {
		case 0xFF: ram2e_max_save(mask, enled); break; // Altera MAX II / V
		case 0xFE: ram2e_spi_erase(); break; // Lattice MachXO / iCE40 / AGM AG256
		case 0xFD: ram2e_lcmxo2_erase(); break; // Lattice MachXO2
	}
}
void ram2e_save_end(char mask, char enled) {
	switch (_type) {
		case 0xFF: break; // Altera MAX II / V
		case 0xFE: ram2e_spi_save(mask, enled); break; // Lattice MachXO / iCE40 / AGM AG256
		case 0xFD: ram2e_lcmxo2_save(mask, enled); break; // Lattice MachXO2
	}
}


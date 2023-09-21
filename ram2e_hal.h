#ifndef RAM2GS_HAL_H
#define RAM2GS_HAL_H
#include <ctype.h>
#include <stdint.h>

char auxram_detect();
char ram2e_detect(char type);
uint16_t ramworks_getsize();
void ram2e_set_mask(char mask);
void ram2e_set_led(char enled);
void ram2e_flashled(char frames);

void ram2e_hal_set_type(char type);
void ram2e_erase();
void ram2e_save_start(char mask, char enled);
void ram2e_save_end(char mask, char enled);

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

#endif
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "util.h"
#include "gwconio.h"
#include "ramtestpat.c"

#define TEST_SIZE (8*1024*1024)
#define BANK_SIZE (65536)
#define NUM_BANKS (TEST_SIZE/BANK_SIZE)

static char getpat(uint32_t i) {
	return ramtestpat[i % RAMTESTPAT_SIZE];
}

char test_run() {
	uint32_t i;
	uint8_t ah;

	// Put read/write stubs in low RAM


	for (ah = 0, i = 0; ah < NUM_BANKS; ah++) {
		uint16_t al = 0;

		// Copy 0x0000-01FF
		*((char*)_test_wr1_dm1 + 1) = getpat(i);
		for (; al < 0x200; al++, i++) {
			_test_wr1_dm1:
			__asm__("lda #$00");
			__asm__("sta $C009"); // SETALTZP
			_test_wr1_am1:
			__asm__("lda $0000");
			__asm__("sta $C008"); // SETSTDZP
		}

		// Copy 0x0200-BFFF
		for (; al < 0xC000; al++, i++) {

		}

		// Copy 0xC000-CFFF to LC2 D000-DFFF
		for (; al < 0xD000; al++, i++) {

		}

		// Copy 0xD000-FFFF to LC1 D000-FFFF
		for (; al != 0x0000; al++, i++) {

		}
	}

	for (uint32_t a = 0; a < TEST_SIZE) {
		char d = rd(a);
		if (d != getpat(a)) { return -1; }
	}

	return 0;
}

static void rd_zplc() {
	_rd_zplc:
	__asm__("sta $C009"); // SETALTZP
	_rd_zplc_am1:
	__asm__("lda $0000");
	__asm__("sta $C008"); // SETSTDZP
	__asm__("rts");
}

static void rd_main() {
	_rd_main:
	__asm__("sta $C003"); // WRCARDRAM
	_rd_main_am1:
	__asm__("lda $0000");
	__asm__("sta $C002"); // WRMAINRAM
	__asm__("rts");
}

static void wr_zplc() {
	_wr_zplc:
	_wr_zplc_dm1:
	__asm__("lda #$00");
	__asm__("sta $C009"); // SETALTZP
	_wr_zplc_am1:
	__asm__("lda $0000");
	__asm__("sta $C008"); // SETSTDZP
	__asm__("rts");
}

static void wr_main() {
	_wr_main:
	_wr_zplc_dm1:
	__asm__("lda #$00");
	__asm__("sta $C005"); // WRCARDRAM
	_wr_main_am1:
	__asm__("lda $0000");
	__asm__("sta $C004"); // WRMAINRAM
	__asm__("rts");
}

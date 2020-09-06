#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "util.h"
#include "gwconio.h"
#include "ramtestpat.c"

#define TEST_SIZE 8*1024*1024

char test_run() {
	// Put copy stub in low RAM 
	for (uint32_t a = 0; a < TEST_SIZE) {
		wr(a, getpat(a));
	}

	for (uint32_t a = 0; a < TEST_SIZE) {
		char d = rd(a);
		if (d != getpat(a)) { return -1; }
	}

	return 0;
}

static inline char getpat(uint32_t a) {
	return ramtestpat[a % RAMTESTPAT_SIZE];
}

static char rd(uint32_t a) {
	uint16_t al = a & 0xFFFF;
	if (al < 0x0200) { return rd_zplc(a); }
	else if (al < 0xC000) { return rd_mid(a); }
	else if (al < 0xD000) { return rd_lc2(a); }
	else { return rd_zplc(a); }
}

static char rd_zplc(uint32_t a) {
	
}

static char rd_mid(uint32_t a) {
	
}

static char rd_lc2(uint32_t a) {
	
}

static char wr(uint32_t a, char d) {
	uint16_t al = a & 0xFFFF;
	if (al < 0x0200) { wr_zplc(a, d); }
	else if (al < 0xC000) { wr_mid(a, d); }
	else if (al < 0xD000) { wr_lc2(a, d); }
	else { wr_zplc(a, d); }
}

static char rd_zplc(uint32_t a) {
	
}

static char rd_mid(uint32_t a) {
	
}

static char rd_lc2(uint32_t a) {
	
}


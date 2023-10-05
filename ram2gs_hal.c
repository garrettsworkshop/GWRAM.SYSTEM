#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include "util.h"
#include "ram2gs_hal.h"

uint8_t __fastcall__ ram2gs_cmd(char cmd);

void ram2gs_set(char en8meg, char enled) {
	char cmd = 0x10;
	if (en8meg) { cmd |= 0x01; }
	if (enled) { cmd |= 0x02; }
	ram2gs_cmd(cmd);
}

void ram2gs_flashled(char frames) {
	char i;
	for (i = 0; i < frames; i++) {
		unsigned int l;
		for (l = 0; *VBL <  0 && l < 2500; l++) { ram2gs_flashled1(); }
		for (l = 0; *VBL >= 0 && l < 2500; l++) { ram2gs_flashled1(); }
	}
}

#include "ram2gs_hal_max.c"
#include "ram2gs_hal_spi.c"
#include "ram2gs_hal_lcmxo2.c"

static char _type;
void ram2gs_hal_set_type(char type) { _type = type; }

void ram2gs_erase() {
	switch (_type) {
		case 0x00: ram2gs_max_erase(); break; // Altera MAX II / V
		case 0x04: ram2gs_spi_erase(); break; // Lattice MachXO / iCE40 / AGM AG256
		case 0x08: ram2gs_lcmxo2_erase(); break; // Lattice MachXO2
	}
}
void ram2gs_save_start(char en8meg, char enled) {
	switch (_type) {
		case 0x00: ram2gs_max_save(en8meg, enled); break; // Altera MAX II / V
		case 0x04: ram2gs_spi_erase(); break; // Lattice MachXO / iCE40 / AGM AG256
		case 0x08: ram2gs_lcmxo2_erase(); break; // Lattice MachXO2
	}
}
void ram2gs_save_end(char en8meg, char enled) {
	switch (_type) {
		case 0x00: break; // Altera MAX II / V
		case 0x04: ram2gs_spi_save(en8meg, enled); break; // Lattice MachXO / iCE40 / AGM AG256
		case 0x08: ram2gs_lcmxo2_save(en8meg, enled); break; // Lattice MachXO2
	}
}

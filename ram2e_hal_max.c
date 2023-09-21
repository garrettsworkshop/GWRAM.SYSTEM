/* ram2e_max_bitbang(...) sends the "Set UFM Bitbang Outputs" to the RAM2E */
static void ram2e_max_bitbang(char bitbang) { ram2e_cmd(0xEA, bitbang); }

/* ram2e_max_program(...) sends the "UFM Program Once" command to the RAM2E */
static void ram2e_max_program() { ram2e_cmd(0xEF, 0x00); }

/* ram2e_max_erase(...) sends the "UFM Erase Once" command to the RAM2E */
static void ram2e_max_erase() { ram2e_cmd(0xEE, 0x00); }

/* ram2e_max_save(...) */
static void ram2e_max_save(char mask, char enled) {
	char i;
	char led;
	if (mask == 0xFF) { mask = 0x80; } // Encode 0xFF mask properly

	// Shift mask into UFMD 
	for (i = 0; i < 8; i++) {
		ram2e_max_bitbang(0x80 | ((mask << (i-1)) & 0x40));
	}

	// Shift LED setting into UFMD
	if 	(( enled &&  (mask >> 7)) || 
	     (!enled && !(mask >> 7))) { led = 0x80; }
	else { led = 0xC0; }
	ram2e_max_bitbang(led);

	// Shift low six bits of mask into UFMD again
	for (i = 1; i < 8; i++) {
		ram2e_max_bitbang(0x80 | ((mask << (i-1)) & 0x40));
	}

	// Program UFM
	ram2e_max_program();
}

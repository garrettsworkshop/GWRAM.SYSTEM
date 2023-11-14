/* ram2e_max_bitbang(...) sends the "Set UFM Bitbang Outputs" to the RAM2E */
static void ram2e_max_bitbang(char bitbang) { ram2e_cmd(0xEA, bitbang); }

/* ram2e_max_program(...) sends the "UFM Program Once" command to the RAM2E */
static void ram2e_max_program() { ram2e_cmd(0xEF, 0x00); }

/* ram2e_max_erase(...) sends the "UFM Erase Once" command to the RAM2E */
static void ram2e_max_erase() { ram2e_cmd(0xEE, 0x00); }

/* ram2e_max_save(...) */
static void ram2e_max_save(char mask, char enled) {
	char wmask;

	// Encode 0xFF mask properly
	if (mask == 0xFF) { wmask = 0x80; }
	else { wmask = mask; }

	// Shift mask into UFMD
	ram2e_max_bitbang(0x80 | ((wmask >> 1) & 0x40));
	ram2e_max_bitbang(0x80 | ((wmask >> 0) & 0x40));
	ram2e_max_bitbang(0x80 | ((wmask << 1) & 0x40));
	ram2e_max_bitbang(0x80 | ((wmask << 2) & 0x40));
	ram2e_max_bitbang(0x80 | ((wmask << 3) & 0x40));
	ram2e_max_bitbang(0x80 | ((wmask << 4) & 0x40));
	ram2e_max_bitbang(0x80 | ((wmask << 5) & 0x40));
	ram2e_max_bitbang(0x80 | ((wmask << 6) & 0x40));

	// Shift mask into UFMD
	if 	(( enled &&  (wmask >> 7)) || 
	     (!enled && !(wmask >> 7))) { 
			 ram2e_max_bitbang(0x80);
	} else { ram2e_max_bitbang(0xC0); }
	ram2e_max_bitbang(0xC0);
	ram2e_max_bitbang(0xC0);
	ram2e_max_bitbang(0xC0);
	ram2e_max_bitbang(0xC0);
	ram2e_max_bitbang(0xC0);
	ram2e_max_bitbang(0xC0);
	ram2e_max_bitbang(0xC0);

	// Program UFM
	ram2e_max_program();
}

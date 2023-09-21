/* _spi_erase(...) */
static void ram2e_lcmxo2_rw(char we, char addr, char data) {
	ram2e_cmd(0xEC, data);
	ram2e_cmd(0xEC, addr);
	ram2e_cmd(0xED, we ? 1 : 0);
}
static void ram2e_lcmxo2_open() { ram2e_lcmxo2_rw(1, 0x70, 0x80); }
static void ram2e_lcmxo2_close() { ram2e_lcmxo2_rw(1, 0x70, 0x00); }
static void ram2e_lcmxo2_cmd_op3(char cmd, char op1, char op2, char op3) {
	ram2e_lcmxo2_open();
	ram2e_lcmxo2_rw(1, 0x71, cmd); // Command
	ram2e_lcmxo2_rw(1, 0x71, op1); // Operand 1/3
	ram2e_lcmxo2_rw(1, 0x71, op2); // Operand 2/3
	ram2e_lcmxo2_rw(1, 0x71, op3); // Operand 3/3
}

static void ram2e_lcmxo2_encfg() {
	ram2e_lcmxo2_cmd_op3(0x74, 0x08, 0x00, 0x00);
	ram2e_lcmxo2_close();
}
static void ram2e_lcmxo2_discfg() {
	ram2e_lcmxo2_cmd_op3(0x26, 0x08, 0x00, 0x00);
	ram2e_lcmxo2_close();
}
static void ram2e_lcmxo2_bypass() {
	ram2e_lcmxo2_open();
	ram2e_lcmxo2_rw(1, 0x71, 0xFF); // Command
	ram2e_lcmxo2_close();
}
static void ram2e_lcmxo2_pollstat() {
	ram2e_lcmxo2_cmd_op3(0x3C, 0x00, 0x00, 0x00);
	ram2e_lcmxo2_rw(0, 0x73, 0x00); // Data 1/4
	ram2e_lcmxo2_rw(0, 0x73, 0x00); // Data 2/4
	ram2e_lcmxo2_rw(0, 0x73, 0x00); // Data 3/4
	ram2e_lcmxo2_rw(0, 0x73, 0x00); // Data 4/4
	ram2e_lcmxo2_close();
}
static void ram2e_lcmxo2_erase() {
	// Enable configuration interface
	ram2e_lcmxo2_encfg();
	// Poll status register
	ram2e_lcmxo2_pollstat();

	// Erase UFM
	ram2e_lcmxo2_cmd_op3(0xCB, 0x00, 0x00, 0x00);
	ram2e_lcmxo2_close();
}
static void ram2e_lcmxo2_save(char mask, char enled) {
	char i;
	// Poll status register
	ram2e_lcmxo2_pollstat();
	// Disable configuration interface
	ram2e_lcmxo2_discfg();
	// Bypass
	ram2e_lcmxo2_bypass();
	// Enable configuration interface
	ram2e_lcmxo2_encfg();
	// Poll status register
	ram2e_lcmxo2_pollstat();
	
	// Set UFM address
	ram2e_lcmxo2_cmd_op3(0xB4, 0x00, 0x00, 0x00);
	ram2e_lcmxo2_rw(1, 0x71, 0x40); // Data 1/4
	ram2e_lcmxo2_rw(1, 0x71, 0x00); // Data 2/4
	ram2e_lcmxo2_rw(1, 0x71, 0x00); // Data 3/4
	ram2e_lcmxo2_rw(1, 0x71, 190);  // Data 4/4
	ram2e_lcmxo2_close();
	
	// Write UFM page
	ram2e_lcmxo2_cmd_op3(0xC9, 0x00, 0x00, 0x01);
	// Data 0
	mask = (mask & 0x80) | (~mask & 0x7F);
	ram2e_lcmxo2_rw(1, 0x71, mask);
	// Data 1
	if (enled) ram2e_lcmxo2_rw(1, 0x71, 1);
	else ram2e_lcmxo2_rw(1, 0x71, 0);
	// Data 2-15
	for (i = 2; i < 16; i++) {
		ram2e_lcmxo2_rw(1, 0x71, 0x00);
	}
	ram2e_lcmxo2_close();

	// Poll status register
	ram2e_lcmxo2_pollstat();
	// Disable configuration interface
	ram2e_lcmxo2_discfg();
	// Bypass
	ram2e_lcmxo2_bypass();
}

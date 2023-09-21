static void ram2gs_lcmxo2_rw(char we, char addr, char data) {
	char i;
	for (i = 0; i < 8; i++) { ram2gs_cmd(0x38 + ((addr >> (7-i)) & 1)); }
	for (i = 0; i < 8; i++) { ram2gs_cmd(0x38 + ((data >> (7-i)) & 1)); }
	if (we) { ram2gs_cmd(0x39); }
	else { ram2gs_cmd(0x38); }
	ram2gs_cmd(0x3A);
}
static void ram2gs_lcmxo2_open() { ram2gs_lcmxo2_rw(1, 0x70, 0x80); }
static void ram2gs_lcmxo2_close() { ram2gs_lcmxo2_rw(1, 0x70, 0x00); }
static void ram2gs_lcmxo2_cmd_op3(char cmd, char op1, char op2, char op3) {
	ram2gs_lcmxo2_open();
	ram2gs_lcmxo2_rw(1, 0x71, cmd); // Command
	ram2gs_lcmxo2_rw(1, 0x71, op1); // Operand 1/3
	ram2gs_lcmxo2_rw(1, 0x71, op2); // Operand 2/3
	ram2gs_lcmxo2_rw(1, 0x71, op3); // Operand 3/3
}

static void ram2gs_lcmxo2_encfg() {
	ram2gs_lcmxo2_cmd_op3(0x74, 0x08, 0x00, 0x00);
	ram2gs_lcmxo2_close();
}
static void ram2gs_lcmxo2_discfg() {
	ram2gs_lcmxo2_cmd_op3(0x26, 0x08, 0x00, 0x00);
	ram2gs_lcmxo2_close();
}
static void ram2gs_lcmxo2_bypass() {
	ram2gs_lcmxo2_open();
	ram2gs_lcmxo2_rw(1, 0x71, 0xFF); // Command
	ram2gs_lcmxo2_close();
}
static void ram2gs_lcmxo2_pollstat() {
	ram2gs_lcmxo2_cmd_op3(0x3C, 0x00, 0x00, 0x00);
	ram2gs_lcmxo2_rw(0, 0x73, 0x00); // Data 1/4
	ram2gs_lcmxo2_rw(0, 0x73, 0x00); // Data 2/4
	ram2gs_lcmxo2_rw(0, 0x73, 0x00); // Data 3/4
	ram2gs_lcmxo2_rw(0, 0x73, 0x00); // Data 4/4
	ram2gs_lcmxo2_close();
}
static void ram2gs_lcmxo2_erase() {
	// Enable configuration interface
	ram2gs_lcmxo2_encfg();
	// Poll status register
	ram2gs_lcmxo2_pollstat();

	// Erase UFM
	ram2gs_lcmxo2_cmd_op3(0xCB, 0x00, 0x00, 0x00);
	ram2gs_lcmxo2_close();
}
static void ram2gs_lcmxo2_save(char en8meg, char enled) {
	char i;
	// Poll status register
	ram2gs_lcmxo2_pollstat();
	// Disable configuration interface
	ram2gs_lcmxo2_discfg();
	// Bypass
	ram2gs_lcmxo2_bypass();
	// Enable configuration interface
	ram2gs_lcmxo2_encfg();
	// Poll status register
	ram2gs_lcmxo2_pollstat();
	
	// Set UFM address
	ram2gs_lcmxo2_cmd_op3(0xB4, 0x00, 0x00, 0x00);
	ram2gs_lcmxo2_rw(1, 0x71, 0x40); // Data 1/4
	ram2gs_lcmxo2_rw(1, 0x71, 0x00); // Data 2/4
	ram2gs_lcmxo2_rw(1, 0x71, 0x00); // Data 3/4
	ram2gs_lcmxo2_rw(1, 0x71, 190);  // Data 4/4
	ram2gs_lcmxo2_close();

	// Write UFM page
	ram2gs_lcmxo2_cmd_op3(0xC9, 0x00, 0x00, 0x01);
	// Data 0
	if      (!en8meg && !enled) { ram2gs_lcmxo2_rw(1, 0x71, 0x01); }
	else if (!en8meg &&  enled) { ram2gs_lcmxo2_rw(1, 0x71, 0x03); }
	else if ( en8meg && !enled) { ram2gs_lcmxo2_rw(1, 0x71, 0x00); }
	else if ( en8meg &&  enled) { ram2gs_lcmxo2_rw(1, 0x71, 0x02); }
	// Data 1-15
	for (i = 1; i < 16; i++) { ram2gs_lcmxo2_rw(1, 0x71, 0x00); }
	ram2gs_lcmxo2_close();

	// Poll status register
	ram2gs_lcmxo2_pollstat();
	// Disable configuration interface
	ram2gs_lcmxo2_discfg();
	// Bypass
	ram2gs_lcmxo2_bypass();
}

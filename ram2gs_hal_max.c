static void ram2gs_max_erase() { ram2gs_cmd(0x28); }
static void ram2gs_max_shift(char bit) {
	char data = 0x20;
	if (bit) data |= 0x01;
	ram2gs_cmd(data);
	data |= 0x02;
	ram2gs_cmd(data);
}
static void ram2gs_max_save(char en8meg, char enled) {
	char i;
	ram2gs_max_shift(0); // Clock in 0 to enable this setting entry
	ram2gs_max_shift(en8meg); // Clock in 8 mb enable bit
	ram2gs_max_shift(!enled); // Clock in LED enable bit
	for (i = 0; i < 13; i++) { ram2gs_max_shift(1); } // Clock in 13 dummy 1s
	ram2gs_cmd(0x24); // Program
}

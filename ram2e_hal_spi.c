static void ram2e_spi_select() { ram2e_cmd(0xEB, 0x04); }
static void ram2e_spi_deselect() { ram2e_cmd(0xEB, 0x00); }
static void ram2e_spi_tx8(char data) {
	char i;
	for (i = 0; i < 8; i++) {
		char d = (data >> (7-i)) & 1;
		d <<= 1;
		d |= 0x04;
		ram2e_cmd(0xEB, d);
		d |= 0x01;
		ram2e_cmd(0xEB, d);
	}
}
static void ram2e_spi_wren() { 
	ram2e_spi_deselect();
	ram2e_spi_select();
	ram2e_spi_tx8(0x06); // 0x06 is write enable
	ram2e_spi_deselect();
}
static void ram2e_spi_erase() {
	ram2e_spi_wren(); 
	ram2e_spi_select();
	ram2e_spi_tx8(0x20); // 0x20 is sector erase (4 kB)
	ram2e_spi_tx8(0x00); // address[23:16]
	ram2e_spi_tx8(0x10); // address[15:8]
	ram2e_spi_tx8(0x00); // address[7:0]
	ram2e_spi_deselect();
}
static void ram2e_spi_save(char en8meg, char enled) {
	ram2e_spi_wren();
	ram2e_spi_select();
	ram2e_spi_tx8(0x02); // 0x02 is page (byte) program
	ram2e_spi_tx8(0x00); // address[23:16]
	ram2e_spi_tx8(0x10); // address[15:8]
	ram2e_spi_tx8(0x00); // address[7:0]
	// data[7:0]
	if      (!en8meg && !enled) { ram2e_spi_tx8(0x7F); }
	else if (!en8meg &&  enled) { ram2e_spi_tx8(0x3F); }
	else if ( en8meg && !enled) { ram2e_spi_tx8(0xFF); }
	else if ( en8meg &&  enled) { ram2e_spi_tx8(0xBF); }
	ram2e_spi_deselect();
}
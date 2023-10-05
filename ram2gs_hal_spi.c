static void ram2gs_spi_select() { ram2gs_cmd(0x34); }
static void ram2gs_spi_deselect() { ram2gs_cmd(0x30); }
static void ram2gs_spi_tx8(char data) {
	char i;
	for (i = 0; i < 8; i++) {
		ram2gs_cmd(0x34 + ((data >> (7-i)) & 1));
		ram2gs_cmd(0x36 + ((data >> (7-i)) & 1));
	}
}
static void ram2gs_spi_wren() { 
	ram2gs_spi_deselect();
	ram2gs_spi_select();
	ram2gs_spi_tx8(0x06); // 0x06 is write enable
	ram2gs_spi_deselect();
}
static void ram2gs_spi_erase() {
	ram2gs_spi_wren(); 
	ram2gs_spi_select();
	ram2gs_spi_tx8(0x20); // 0x20 is sector erase (4 kB)
	ram2gs_spi_tx8(0x00); // address[23:16]
	ram2gs_spi_tx8(0x10); // address[15:8]
	ram2gs_spi_tx8(0x00); // address[7:0]
	ram2gs_spi_deselect();
}
static void ram2gs_spi_save(char en8meg, char enled) {
	ram2gs_spi_wren();
	ram2gs_spi_select();
	ram2gs_spi_tx8(0x02); // 0x02 is page (byte) program
	ram2gs_spi_tx8(0x00); // address[23:16]
	ram2gs_spi_tx8(0x10); // address[15:8]
	ram2gs_spi_tx8(0x00); // address[7:0]
	// data[7:0]
	if      (!en8meg && !enled) { ram2gs_spi_tx8(0x7F); }
	else if (!en8meg &&  enled) { ram2gs_spi_tx8(0x3F); }
	else if ( en8meg && !enled) { ram2gs_spi_tx8(0xFF); }
	else if ( en8meg &&  enled) { ram2gs_spi_tx8(0xBF); }
	ram2gs_spi_deselect();
}

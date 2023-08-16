#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "util.h"
#include "gwconio.h"
#include "ram2gs_asm.h"

static void ram2gs_set(char en8meg, char enled) {
	char cmd = 0x10;
	if (en8meg) { cmd |= 0x01; }
	if (enled) { cmd |= 0x02; }
	ram2gs_cmd(cmd);
}

static void ram2gs_max_erase() { ram2gs_cmd(0x28); }
static void ram2gs_max_set_nvm(char en8meg, char enled) {
	char i;
	// Clock in 0 to enable this setting entry
	ram2gs_cmd(0x20);
	ram2gs_cmd(0x22);

	if (en8meg) {
		// Clock in 1 to enable 8mb
		ram2gs_cmd(0x21);
		ram2gs_cmd(0x23);
	} else {
		// Clock in 0 to disable 8mb
		ram2gs_cmd(0x20);
		ram2gs_cmd(0x22);
	}

	if (enled) {
		// Clock in 0 to enable LED
		ram2gs_cmd(0x20);
		ram2gs_cmd(0x22);
	} else {
		// Clock in 1 to disable LED
		ram2gs_cmd(0x21);
		ram2gs_cmd(0x23);
	}

	// Clock in 13 dummy "1"s
	for (i = 0; i < 13; i++) {
		ram2gs_cmd(0x21);
		ram2gs_cmd(0x23);
	}

	// Program
	ram2gs_cmd(0x24);
}

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
static void ram2gs_spi_set_nvm(char en8meg, char enled) {
	ram2gs_spi_erase(); // First erase
	spin(33, 8); // Wait for >= 500ms on even the fastest systems.
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

static void ram2gs_erase(char typecode) {
	switch (typecode) {
		case 0x00: ram2gs_max_erase(); break; // Altera MAX II / V
		case 0x04: ram2gs_spi_erase(); break; // Lattice MachXO / iCE40 / AGM AG256
		//case 0x08: ram2gs_erase_lcmxo2(); break; // Lattice MachXO2
	}
}
static void ram2gs_set_nvm(char typecode, char en8meg, char enled) {
	switch (typecode) {
		case 0x00: ram2gs_max_set_nvm(en8meg, enled); break; // Altera MAX II / V
		case 0x04: ram2gs_spi_set_nvm(en8meg, enled); break; // Lattice MachXO / iCE40 / AGM AG256
		//case 0x08: ram2gs_set_nvm_lcmxo2(en8meg, enled); break; // Lattice MachXO2
	}
}

static void menu_led(char enled) {
	if (enled) {
		gwcputsxy(1, 15, "LED enabled. Press [L] to disable LED.");
	} else {
		gwcputsxy(1, 15, "LED disabled. Press [L] to enable LED.");
	}
}

static void menu()
{
	uint8_t bankcount;
	clrscr(); // Clear screen
	
	gwcputsxy(5, 1, "-- RAM2GS Capacity Settings --");
	gwcputsxy(4, 3, "Current RAM2GS capacity: ...");

	gwcputsxy(1, 6, "Select desired memory capacity:");

	gwcputsxy(4, 8, "1. 4 megabytes");
	gwcputsxy(4, 10, "2. 8 megabytes");

	gwcputsxy(1, 18, "Capacity will be saved until power-off.");

	gwcputsxy(1, 20, "To remember capacity and LED setting in");
	gwcputsxy(1, 21, "nonvolatile memory, press Apple+number.");

	gwcputsxy(1, 23, "Press [Q] to quit without saving.");

	bankcount = ram2gs_getsize();
	gotoxy(29, 3);
	printf("%d", bankcount * 64);
	gwcputs(" kB");
}

static void loading_screen()
{
	clrscr(); // Clear screen
	gwcputsxy(8, 1, "Loading RAM2GS settings...");
}

int ram2gs_main(void)
{
	char hasled = true;
	char typecode = false;
	char enled = false;
	char en8meg = true;
	char nvm = false;
	int reset_count = false;

	loading_screen();

	if (ram2gs_detect(0x00)) {
		typecode = 0x00;
		hasled = !ram2gs_detect(0x04);
	} else if (ram2gs_detect(0x04)) {
		typecode = 0x04;
		hasled = true;
	} else {
		#ifndef SKIP_RAM2GS_DETECT
		// If no RAM2GS, show an error message and quit
		gwcputsxy(0, 8, " No RAM2GS II detected.");
		gwcputsxy(0, 10, " Press any key to quit.");
		cgetc(); // Wait for key
		clrscr(); // Clear screen before quitting
		return EXIT_SUCCESS;
		#endif
	}

	if (hasled) { enled = !ram2gs_detect(typecode | 0x02); }
	menu();
	if (hasled) { menu_led(enled); }

	// Get user choice from menu
	while (true) {
		// Set capacity or quit according to keypress.
		switch (toupper(cgetc() & 0x7F)) {
			case 'Q' : {
				clrscr();
				return EXIT_SUCCESS;
			}
			case '1': en8meg = false; ram2gs_set(en8meg, enled); break;
			case '2': en8meg = true;  ram2gs_set(en8meg, enled); break;
			case 'L': { 
				enled = !enled;
				ram2gs_set(en8meg, enled);
				if (hasled) { menu_led(enled); };
				continue;
			} case 'R': {
				reset_count++;
				if (reset_count >= 25) {
					// Show message about resetting.
					clrscr(); // Clear screen
					gwcputsxy(1, 8, "Resetting RAM2GS settings.");
					gwcputsxy(1, 9, "Do not turn off your Apple.");

					ram2gs_erase(typecode); // Erase RAM2GS settings memory
					ram2gs_set(1, 0); // Enable 8 megabytes and disable LED

					// Wait for >= 500ms on even the fastest systems.
					spin(32, 8);
					
					// Show success message and quit
					clrscr(); // Clear screen
					gwcputsxy(1, 8, "RAM2GS settings reset successfully.");
					nvm = true;
					goto end;
				}
				continue;
			} default: reset_count = 0; continue;
		}

		// Check if pressed with apple key. If so, save to nonvolatile memory.
		if (read_applekey()) { nvm = true; }
		break;
	}

	// Clear screen in preparation to show saving or success message.
	clrscr();

	if (nvm) { // Save in NVM if requested.
		// Show message about saving.
		gwcputsxy(1, 8, "Saving RAM2GS capacity setting.");
		gwcputsxy(1, 9, "Do not turn off your Apple.");
		// Save capacity in nonvolatile memory.
		ram2gs_set_nvm(typecode, en8meg, enled);
		// Wait for >= 500ms on even the fastest systems.
		spin(33, 8);
		// Print success message
		clrscr(); // Clear screen
		gwcputsxy(1, 8, "RAM2GS capacity saved successfully.");
	} else { // Print success message if not saving in NVM.
		gwcputsxy(1, 8, "RAM2GS capacity set successfully.");
	}

	end:
	if (nvm) { // Show end message for nonvolatile save
		gwcputsxy(1, 10, "You may now turn off your Apple.");
		gwcputsxy(1, 12, "You may also reset your Apple for");
		gwcputsxy(1, 13, "the setting change to take effect.");
	} else { // Show end message for volatile save
		gwcputsxy(1, 10, "Please reset your Apple for");
		gwcputsxy(1, 11, "the setting change to take effect.");
	}
	// Don't quit. Instead leave prompt asking user to reset.
	while(1) { cgetc(); }
}

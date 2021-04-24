#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "util.h"
#include "gwconio.h"
#include "ram2gs_asm.h"

// Commands common to Altera and AGM
static void ram2gs_set4mb() { ram2gs_cmd(0x10); }
static void ram2gs_set8mb() { ram2gs_cmd(0x11); }

// Commands for just altera
static void ram2gs_altera_erase() { ram2gs_cmd(0x28); }
static void ram2gs_altera_program() { ram2gs_cmd(0x24); }
static void ram2gs_altera_setnvm(char en8meg) {
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

	// Clock in 14 dummy "1"s
	for (i = 0; i < 14; i++) {
		ram2gs_cmd(0x21);
		ram2gs_cmd(0x23);
	}

	ram2gs_altera_program();
}

// Commands for just AGM
static void ram2gs_agm_select() { ram2gs_cmd(0x34); }
static void ram2gs_agm_deselect() { ram2gs_cmd(0x30); }
static void ram2gs_agm_tx8(char data) {
	char i;
	for (i = 0; i < 8; i++) {
		ram2gs_cmd(0x34 + ((data >> (7-i)) & 1));
		ram2gs_cmd(0x36 + ((data >> (7-i)) & 1));
	}
}
static void ram2gs_agm_write_en() { 
	ram2gs_agm_deselect();
	ram2gs_agm_select();
	ram2gs_agm_tx8(0x06); // 0x06 is write enable
	ram2gs_agm_deselect();
}
static void ram2gs_agm_erase() { 
	ram2gs_agm_write_en(); 
	ram2gs_agm_select();
	ram2gs_agm_tx8(0x20); // 0x20 is sector erase (4 kB)
	ram2gs_agm_tx8(0x00); // address[23:16]
	ram2gs_agm_tx8(0x10); // address[15:8]
	ram2gs_agm_tx8(0x00); // address[7:0]
	ram2gs_agm_deselect();
}
static void ram2gs_agm_write_nvm(char en8meg) {
	ram2gs_agm_write_en();
	ram2gs_agm_select();
	ram2gs_agm_tx8(0x02); // 0x02 is page (byte) program
	ram2gs_agm_tx8(0x00); // address[23:16]
	ram2gs_agm_tx8(0x10); // address[15:8]
	ram2gs_agm_tx8(0x00); // address[7:0]
	if (en8meg) { ram2gs_agm_tx8(0xFF); } // data[7:0]
	else { ram2gs_agm_tx8(0x00); } // data[7:0]
	ram2gs_agm_deselect();
}

static void menu(void)
{
	uint8_t bankcount = ram2gs_getsize();
	gwcputsxy(5, 1, "-- RAM2GS Capacity Settings --");

	gotoxy(4, 3);
	gwcputs("Current RAM2GS capacity: ");
	printf("%d", bankcount * 64);
	gwcputs(" kB");

	gwcputsxy(1, 6, "Select desired memory capacity:");

	gwcputsxy(4, 8, "1. 4 megabytes");
	gwcputsxy(4, 10, "2. 8 megabytes");

	gwcputsxy(1, 18, "Capacity will be saved until power-off.");

	gwcputsxy(1, 20, "To remember capacity setting in");
	gwcputsxy(1, 21, "nonvolatile memory, press Apple+number.");

	gwcputsxy(1, 23, "Press [Q] to quit without saving.");
}

int ram2gs_main(void)
{
	char en8meg;
	char nvm;
	int reset_count;

	// Check for RAM2GS
	#ifndef SKIP_RAM2GS_DETECT
	if(!ram2gs_detect()) {
		// If no RAM2GS, show an error message and quit
		gwcputsxy(0, 8, " No RAM2GS II detected.");
		gwcputsxy(0, 10, " Press any key to quit.");
		cgetc(); // Wait for key
		clrscr(); // Clear screen before quitting
		return EXIT_SUCCESS;
	}
	#endif

	menu(); // Print menu

	// Get user choice from menu
	en8meg = 0;
	nvm = 0;
	reset_count = 0;
	while (true) {
		// Set capacity or quit according to keypress.
		switch (toupper(cgetc() & 0x7F)) {
			case 'Q' : {
				clrscr();
				return EXIT_SUCCESS;
			}
			case '1': en8meg = 0; ram2gs_set4mb(); break;
			case '2': en8meg = 1; ram2gs_set8mb(); break;
			case 'R': {
				reset_count++;
				if (reset_count >= 25) {
					// Show message about resetting.
					clrscr(); // Clear screen
					gwcputsxy(1, 8, "Resetting RAM2GS settings.");
					gwcputsxy(1, 9, "Do not turn off your Apple.");

					// Erase RAM2GS settings memory
					ram2gs_altera_erase(); // Erase for Altera CPLD
					ram2gs_agm_erase(); // Erase for AGM CPLD
					spin(32, 8); // Wait for >= 500ms on even the fastest systems.
					ram2gs_set8mb(); // Enable 8 megabytes now (default)
					
					// Show success message and quit
					clrscr(); // Clear screen
					gwcputsxy(1, 8, "RAM2GS settings reset successfully.");
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
		ram2gs_altera_setnvm(en8meg); // Save for Altera CPLD
		ram2gs_agm_erase(); // Erase for AGM CPLD
		spin(33, 8); // Wait for >= 500ms on even the fastest systems.
		ram2gs_agm_write_nvm(en8meg); // Write for AGM CPLD
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

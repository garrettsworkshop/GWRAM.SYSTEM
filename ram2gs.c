#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "util.h"
#include "gwconio.h"
#include "ram2gs_asm.h"

static void ram2gs_erase() { ram2gs_cmd(0x28); }
static void ram2gs_program() { ram2gs_cmd(0x24); }
static void ram2gs_set4mb() { ram2gs_cmd(0x10); }
static void ram2gs_set8mb() { ram2gs_cmd(0x11); }
static void ram2gs_setnvm(char en8meg) {
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

	ram2gs_program();
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

					ram2gs_erase(); // Erase RAM2GS settings memory
					ram2gs_set8mb(); // Enable 8 megabytes now

					// Wait for >= 500ms on even the fastest systems.
					spin(32, 8);
					
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
		ram2gs_setnvm(en8meg);
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

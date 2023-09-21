#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "util.h"
#include "gwconio.h"
#include "ram2gs_hal.h"

static void menu()
{
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
}

static void menu_size(uint16_t bankcount) {
	gotoxy(29, 3);
	printf("%d", bankcount * 64);
	gwcputs(" kB");
}

static void menu_led(char enled) {
	if (enled) { gwcputsxy(1, 15, "LED enabled. Press [L] to disable LED."); }
	else { gwcputsxy(1, 15, "LED disabled. Press [L] to enable LED."); }
}

static void loading_screen()
{
	clrscr(); // Clear screen
	gwcputsxy(8, 1, "Loading RAM2GS settings...");
}

int ram2gs_main(void)
{
	char type;
	uint16_t bankcount;
	char en8meg = true;
	char hasled = true;
	char enled = false;

	char nvm = false;
	int reset_count = 0;

	loading_screen();

	if (ram2gs_detect(0x00)) { // Altera MAX II / V
		type = 0x00;
		hasled = !ram2gs_detect(0x04);
	} else if (ram2gs_detect(0x04)) { // Lattice MachXO / iCE40 / AGM AG256
		type = 0x04;
		hasled = true;
	} else if (ram2gs_detect(0x08)) { // Lattice MachXO2
		type = 0x08;
		hasled = true;
	} else {
		#ifndef SKIP_RAM2GS_DETECT
		// If no RAM2GS, show an error message and quit
		gwcputsxy(0, 8,  " No RAM2GS II detected.");
		gwcputsxy(0, 10, " Press any key to quit.");
		cgetc(); // Wait for key
		clrscr(); // Clear screen before quitting
		return EXIT_SUCCESS;
		#else
		hasled = true;
		#endif
	}

	// Set chip type
	ram2gs_hal_set_type(type);
	
	// Print menu
	menu();

	// Detect and print current capacity 
	bankcount = ram2gs_getsize();
	menu_size(bankcount);

	// Detect and print LED menu
	#ifndef SKIP_RAM2GS_DETECT
	if (hasled) {
		enled = !ram2gs_detect(type | 0x02);
		menu_led(enled);
	}
	#endif

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
				if (hasled) {
					menu_led(enled);
					if (enled) {
						wait(1);
						ram2gs_flashled(10);
						wait(10);
						ram2gs_flashled(10);
					}
				};
				continue;
			} case 'R': {
				reset_count++;
				if (reset_count >= 25) {
					// Show message about resetting.
					clrscr(); // Clear screen
					gwcputsxy(1, 8, "Resetting RAM2GS settings.");
					gwcputsxy(1, 9, "Do not turn off your Apple.");

					ram2gs_erase(); // Erase RAM2GS settings memory
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
		ram2gs_save_start(en8meg, enled);
		// Wait for >= 500ms on even the fastest systems.
		spin(33, 8);
		// Finish saving
		ram2gs_save_end(en8meg, enled);
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

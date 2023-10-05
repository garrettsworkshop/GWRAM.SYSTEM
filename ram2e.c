#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "util.h"
#include "gwconio.h"
#include "ram2e_hal.h"
#include "ram2e_save.h"

static void menu()
{
	clrscr(); // Clear screen
	
	gwcputsxy(5, 1, "-- RAM2E Capacity Settings --");

	gwcputsxy(1, 6, "Select desired memory capacity:");

	gwcputsxy(4, 7, "1. 64 kilobytes");
	gwcputsxy(4, 8, "2. 512 kilobytes");
	gwcputsxy(4, 9, "3. 1 megabyte");
	gwcputsxy(4, 10, "4. 4 megabytes");
	gwcputsxy(4, 11, "5. 8 megabytes");

	gwcputsxy(1, 17, "Capacity will be saved until power-off.");

	gwcputsxy(1, 19, "To remember capacity setting in");
	gwcputsxy(1, 20, "nonvolatile memory, press Apple+number.");

	gwcputsxy(1, 22, "Press [Q] to quit without saving.");
}

static void menu_size(uint16_t bankcount, char has16m) {
	if (bankcount < 2) { gotoxy(5, 3); }
	else { gotoxy(4, 3); }
	gwcputs("Current RAM2E capacity: ");
	printf("%d", bankcount * 64);
	gwcputs(" kB");

	if(has16m) { gwcputsxy(4, 12, "6. 16 megabytes"); }
}

static void menu_led(char enled) {
	if (enled) {
		gwcputsxy(1, 14, "LED enabled. Press [L] to disable LED.");
	} else {
		gwcputsxy(1, 14, "LED disabled. Press [L] to enable LED.");
	}
}

int ram2e_main(void)
{
	char type;
	uint16_t bankcount;
	char mask = 0;
	char has16m = false;
	char hasled = false;
	char enled = false;

	char nvm = false;
	int reset_count = 0;

	ramworks_save(); // Save what will be clobbered
	if (auxram_detect()) {
		if (ram2e_detect(0xFF)) { // MAX
			type = 0xFF;
		/*} else if (ram2e_detect(0xFE)) { // SPI
			type = 0xFE;*/
		} else if (ram2e_detect(0xFD)) { // MachXO2
			type = 0xFD;
		} else { type = 0; }
	}

	if (type == 0) {
		#ifndef SKIP_RAM2E_DETECT
		ramworks_restore();
		// If no RAM2E, show an error message and quit
		gwcputsxy(0, 8,  " No RAM2E II detected.");
		gwcputsxy(0, 10, " Press any key to quit.");
		cgetc(); // Wait for key
		clrscr(); // Clear screen before quitting
		return EXIT_SUCCESS;
		#endif
	}

	// Set chip type
	ram2e_hal_set_type(type);

	// Print menu
	menu();

	// Detect and print current capacity plus 16 MB option
	#ifndef SKIP_RAM2E_DETECT
	bankcount = ramworks_getsize();
	// If set for 8 MB, check for 16 MB capability
	if (bankcount >= 128) {
		ram2e_set_mask(0xFF);
		has16m = ramworks_getsize() == 256;
		ram2e_set_mask(0x7F);
	}
	#else
	bankcount = 128;
	#endif
	menu_size(bankcount, has16m); // Print size

	// Detect and print LED menu
	#ifndef SKIP_RAM2E_DETECT
	hasled = ram2e_detect(0xF0);
	if (hasled) {
		enled = ram2e_detect(0xE3);
		menu_led(enled);
	}
	#else
	hasled = true;
	#endif

	ramworks_restore(); // Restore RAMWorks contents

	// Get user choice from menu
	while (true) {
		// Set capacity mask or quit according to keypress.
		switch (toupper(cgetc() & 0x7F)) {
			case 'Q' : {
				clrscr();
				return EXIT_SUCCESS;
			}
			case '1': mask = 0x00; break;
			case '2': mask = 0x07; break;
			case '3': mask = 0x0F; break;
			case '4': mask = 0x3F; break;
			case '5': mask = 0x7F; break;
			case '6': {
				if (has16m) { mask = 0xFF; break; }
				else { continue; }
			#ifdef SKIP_RAM2E_DETECT
			} case '7': {
				menu_size(bankcount, 1);
				continue;
			#endif
			} case 'L': { 
				if (hasled) {
					enled = !enled;
					ram2e_set_led(enled);
					menu_led(enled);
					if (enled) {
						wait(1);
						ram2e_flashled(10);
						wait(10);
						ram2e_flashled(10);
					}
				}
				continue;
			} case 'R': {
				reset_count++;
				if (reset_count >= 25) {
					// Show message about resetting.
					clrscr(); // Clear screen
					gwcputsxy(1, 8, "Resetting RAM2E settings.");
					gwcputsxy(1, 9, "Do not turn off your Apple.");

					ram2e_erase(); // Erase RAM2E settings memory
					ram2e_set_mask(0x7F); // Set mask to default (0x7F)

					// Wait for >= 500ms on even the fastest systems.
					spin(32, 8);
					
					// Show success message and quit
					clrscr(); // Clear screen
					gwcputsxy(1, 8, "RAM2E settings reset successfully.");
					goto end;
				}
				continue;
			} default: reset_count = 0; continue;
		}

		// Check if pressed with apple key. If so, save to nonvolatile memory.
		if (read_applekey()) { nvm = true; }
		break;
	}

	// Set capacity in volatile memory.
	ram2e_set_mask(mask);

	// Clear screen in preparation to show saving or success message.
	clrscr();

	if (nvm) { // Save in NVM if requested.
		// Show message about saving.
		gwcputsxy(1, 8, "Saving RAM2E capacity setting.");
		gwcputsxy(1, 9, "Do not turn off your Apple.");
		// Save capacity in nonvolatile memory.
		ram2e_save_start(mask, enled);
		// Wait for >= 500ms on even the fastest systems.
		spin(32, 8);
		// Finish saving
		ram2e_save_end(mask, enled);
		// Print success message
		clrscr(); // Clear screen
		gwcputsxy(1, 8, "RAM2E capacity saved successfully.");
	} else { // Print success message if not saving in NVM.
		gwcputsxy(1, 8, "RAM2E capacity set successfully.");
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

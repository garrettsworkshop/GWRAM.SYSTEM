#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "util.h"
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
	gotoxy(5, 1);
	cputs("-- RAM2GS Capacity Settings --");
	gotoxy(4, 3);
	printf("Current RAM2GS capacity: %d kB", bankcount * 64);

	gotoxy(1, 6);
	cputs("Select desired memory capacity:");

	gotoxy(4, 8);
	cputs("1. 4 megabytes");
	gotoxy(4, 10);
	cputs("2. 8 megabytes");

	gotoxy(1, 18);
	cputs("Capacity will be saved until power-off.");

	gotoxy(1, 20);
	cputs("To remember capacity setting in");
	gotoxy(1, 21);
	cputs("nonvolatile memory, press Apple+number.");

	gotoxy(1, 23);
	cputs("Press [Q] to quit without saving.");
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
		gotoxy(0, 8);
		cputs(" No RAM2GS II detected.");
		gotoxy(0, 10);
		cputs(" Press any key to quit.");
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
			case '!': nvm = true;
			case '1': en8meg = 0; ram2gs_set4mb(); break;
			case '@': nvm = true;
			case '2': en8meg = 1; ram2gs_set8mb(); break;
			case 'R': {
				reset_count++;
				if (reset_count >= 100) {
					// Show message about saving.
					clrscr(); // Clear screen
					gotoxy(1, 8);
					cputs("Resetting RAM2GS settings.");
					gotoxy(1, 9);
					cputs("Do not turn off your Apple.");

					ram2gs_erase(); // Erase RAM2GS settings memory
					ram2gs_set8mb(); // Enable 8 megabytes now

					// Wait for >= 500ms on even the fastest systems.
					spin(32, 8);
					
					// Show success message and quit
					clrscr(); // Clear screen
					gotoxy(1, 8);
					cputs("RAM2GS settings reset successfully.");
					goto end;
				}
			} default: continue;
		}

		// Check if pressed with apple key. If so, save to nonvolatile memory.
		if (read_applekey()) { nvm = true; }
		break;
	}

	// Clear screen in preparation to show saving or success message.
	clrscr();

	if (nvm) { // Save in NVM if requested.
		// Show message about saving.
		gotoxy(1, 8);
		cputs("Saving RAM2GS capacity setting.");
		gotoxy(1, 9);
		cputs("Do not turn off your Apple.");
		// Save capacity in nonvolatile memory.
		ram2gs_setnvm(en8meg);
		// Wait for >= 500ms on even the fastest systems.
		spin(33, 8);
		// Print success message
		clrscr(); // Clear screen
		gotoxy(1, 8);
		cputs("RAM2GS capacity saved successfully.");
	} else { // Print success message if not saving in NVM.
		gotoxy(1, 8);
		cputs("RAM2GS capacity set successfully.");
	}

	end:
	if (nvm) { // Show end message for nonvolatile save
		gotoxy(1, 10);
		cputs("You may now turn off your Apple.");
		gotoxy(1, 12);
		cputs("You may also reset your Apple for");
		gotoxy(1, 13);
		cputs("the setting change to take effect.");
	} else { // Show end message for volatile save
		gotoxy(1, 10);
		cputs("Please reset your Apple for");
		gotoxy(1, 11);
		cputs("the setting change to take effect.");
	}
	// Don't quit. Instead leave prompt asking user to reset.
	while(1) { cgetc(); }
}

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#include "ram2e.h"
#include "ram2gs.h"
#include "gwconio.h"

char gwconiomask;

int main(void)
{
	gwconiomask = 0xFF;

	// First clear screen
	clrscr();

	// Check machine type
	switch ((get_ostype() & 0xF0)) {
		case APPLE_IIE:
			ram2e_main();
			// Set RAMWorks bank to 0
			__asm__("lda #0");
			__asm__("sta $C073");
			return EXIT_SUCCESS;
		case APPLE_IIGS:
			ram2gs_main();
			return EXIT_SUCCESS;
		default:
			gwconiomask = 0xDF;
			// If not on IIe or IIgs, show an error message and quit
			gwcputsxy(0, 8, " THIS PROGRAM REQUIRES APPLE IIE OR IIGS");
			gwcputsxy(0, 10, " PRESS ANY KEY TO QUIT.");
			cgetc(); // Wait for key
			clrscr(); // Clear screen before quitting
			return EXIT_SUCCESS;
	}
}

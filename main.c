#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#include "ram2e.h"
#include "ram2gs.h"

int main(void)
{
	// First clear screen
	clrscr();

	// Check machine type
	switch ((get_ostype() & 0xF0)) {
		case APPLE_IIE:
			ram2e_main();
			break;
		case APPLE_IIGS:
			ram2gs_main();
			break;
		default:
			// If not on IIe or IIgs, show an error message and quit
			gotoxy(0, 8);
			cputs(" THIS PROGRAM REQUIRES APPLE IIE OR IIGS");
			gotoxy(0, 10);
			cputs(" PRESS ANY KEY TO QUIT.");
			cgetc(); // Wait for key
			clrscr(); // Clear screen before quitting
			return EXIT_SUCCESS;
			break;
	} 
}

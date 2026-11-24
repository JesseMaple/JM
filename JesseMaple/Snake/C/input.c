#include "input.h"
#include <conio.h>

Keys process_input(Keys last_input) {
	if(_kbhit()) {
		int key = _getch();
		if (key == 224) { // Arrow keys
			key = _getch();
			switch (key) {
				case 72: // Up arrow
					return UP;
				case 80: // Down arrow
					return DOWN;
				case 75: // Left arrow
					return LEFT;
				case 77: // Right arrow
					return RIGHT;
			}
		}
		switch(key) {
			case 'w':
			case 'W':
				return UP;
			case 's':
			case 'S':
				return DOWN;
			case 'a':
			case 'A':
				return LEFT;
			case 'd':
			case 'D':
				return RIGHT;
			case 27:
				return ESC;
			case 13:
				return ENTER;
			default:
				return last_input; // Ignore other keys
		}
	}
	return last_input;
}
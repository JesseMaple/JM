#pragma once

#include "config.h"

typedef enum {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	ESC,
	ENTER,
	NONE,
}Keys;

Keys process_input(Keys last_input);
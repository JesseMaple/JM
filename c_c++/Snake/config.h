#pragma once

typedef struct {
	char* const history;
}FilePaths;

typedef enum {
	SUCCESSFUL_OPERATION = 0,

	FAILED_TO_OPEN_FILE = -1,
	FAILED_TO_ALLOCATE_MEMORY = -2,
	FAILED_TO_BUILD_DIR = -3,
	FOUND_ERROR_PARSING_FILE = -4,

	FAILED_TO_INIT_GAME = -50,
}Errors;

typedef enum {
	GAME_HISTORY_LINE_SIZE = 100,

	MAP_WIDTH = 40,
	MAP_HEIGHT = 20,

	INIT_SNAKE_LENGTH = 3,
}Sizes;
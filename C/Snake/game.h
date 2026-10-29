#pragma once
#include "config.h"
#include "input.h"
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

typedef struct SnakeNode SnakeNode;

typedef struct {
	SnakeNode* head;
	size_t len;
}Snake;

typedef struct {
	int x, y;
}NodePosition;

typedef enum {
	S_AVAILABLE,
	S_FAILED_TO_ALLOCATE_MEMORY,
}SnakeStatus;

typedef struct {
	bool is_pause;
	bool is_end;
	bool is_init_success;
}GameStatus;

typedef struct {
	GameStatus status;
	Snake snake;
	Keys current_dir;
	int food_x, food_y;
	int score;
	long long max_score;
	time_t start_time;
}GameData;

extern GameData game_data;

void game_init(void);

void game_run(void);

void game_destroy(void);

int get_x(SnakeNode* node);

int get_y(SnakeNode* node);

void move_behind(SnakeNode** node);
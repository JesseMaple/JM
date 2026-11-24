#pragma once

#include "config.h"

typedef struct {
	long long score;
	long long game_time;
	long long timestamp;
} GameHistoryEntry;

long long get_max_score(void);

Errors save_game_history(long long score, long long game_time, long long timestamp);

Errors load_game_history(void);
// ui.h
#pragma once
#include "game.h"

void hide_cursor(void);

void draw_map(void);

void render_score(int score, long long max_score);

void render_food(int x, int y);

void render_snake_instance(const Snake* snake); // 以后多玩家就调多次

void clear_cell(int x, int y);                  // 擦除局部

void render_pause(void);

void ui_game_over(long long score, long long max_score);
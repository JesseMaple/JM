#include "game.h"
#include "config.h"
#include "ui.h"
#include "file_io.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

typedef struct SnakeNode {
	SnakeNode* prev, * next;
	NodePosition position;
}SnakeNode;

GameData game_data = { 0 };

static void add_head(Snake* snake, int x, int y) {
	SnakeNode* new_node = (SnakeNode*)malloc(sizeof(SnakeNode));
	if (!new_node)return;

	new_node->position.x = x;
	new_node->position.y = y;

	if (!snake->head) {
		new_node->prev = new_node;
		new_node->next = new_node;
		snake->head = new_node;
	}
	else {
		SnakeNode* tail = snake->head->prev;
		new_node->next = snake->head;
		new_node->prev = tail;
		tail->next = new_node;
		snake->head->prev = new_node;
		snake->head = new_node;
	}
	snake->len++;
}

static void remove_tail(Snake* snake) {
	if (snake->head == NULL) return;

	if (snake->head->next == snake->head) {
		// 只有一个节点
		free(snake->head);
		snake->head = NULL;
	}
	else {
		SnakeNode* tail = snake->head->prev;
		// 把尾巴的前一个节点和 head 连起来
		tail->prev->next = snake->head;
		snake->head->prev = tail->prev;
		free(tail);
	}
	snake->len--;
}

static void destroy_snake(Snake* snake) {
	while (snake->head != NULL) {
		remove_tail(snake);
	}
}

static SnakeStatus init_snake(Snake* snake) {
	snake->head = NULL;
	snake->len = 0;

	// 倒着插，保证头部是 (10, 10)
	for (int i = INIT_SNAKE_LENGTH - 1; i >= 0; i--) {
		add_head(snake, 10 - i, 10);
	}
	if (snake->len != INIT_SNAKE_LENGTH)return S_FAILED_TO_ALLOCATE_MEMORY;
	return S_AVAILABLE;
}

static void generate_food(void) {
	int valid = 0;
	while (!valid) {
		game_data.food_x = rand() % (MAP_WIDTH - 2) + 1;
		game_data.food_y = rand() % (MAP_HEIGHT - 2) + 1;
		valid = 1;

		// 防止食物生成在蛇身上
		SnakeNode* curr = game_data.snake.head;
		if (curr != NULL) {
			do {
				if (curr->position.x == game_data.food_x && curr->position.y == game_data.food_y) {
					valid = 0;
					break;
				}
				curr = curr->next;
			} while (curr != game_data.snake.head);
		}
	}
}

void game_init(void) {
	game_data.status.is_init_success = false;
	game_data.status.is_pause = false;
	game_data.status.is_end = false;
	game_data.current_dir = RIGHT;
	game_data.score = 0LL;

	if (load_game_history() != SUCCESSFUL_OPERATION)return;
	game_data.max_score = get_max_score();
	if (game_data.max_score < 0)game_data.max_score = 0;

	if (init_snake(&game_data.snake) != S_AVAILABLE)return;

	srand((unsigned int)time(NULL));
	generate_food();

	game_data.start_time = time(NULL);
	game_data.status.is_init_success = true;

	draw_map();
	hide_cursor();
	render_score(game_data.score, game_data.max_score);
	render_food(game_data.food_x, game_data.food_y);
	render_snake_instance(&game_data.snake);
}

void game_run(void) {
	if (!game_data.status.is_init_success) return;

	while (!game_data.status.is_end) {
		Keys key = process_input(game_data.current_dir);

		if (key == ESC) {
			game_data.status.is_pause = !game_data.status.is_pause;
		}
		else if (key == ENTER && game_data.status.is_pause) {
			game_data.status.is_end = true;
			break;
		}

		if (!game_data.status.is_pause) {
			// 防倒车
			if ((key == UP && game_data.current_dir != DOWN) ||
				(key == DOWN && game_data.current_dir != UP) ||
				(key == LEFT && game_data.current_dir != RIGHT) ||
				(key == RIGHT && game_data.current_dir != LEFT)) {
				game_data.current_dir = key;
			}

			int next_x = game_data.snake.head->position.x;
			int next_y = game_data.snake.head->position.y;
			switch (game_data.current_dir) {
			case UP: next_y--; break;
			case DOWN: next_y++; break;
			case LEFT: next_x--; break;
			case RIGHT: next_x++; break;
			default: break;
			}

			// 撞墙检测
			if (next_x <= 0 || next_x >= MAP_WIDTH || next_y <= 0 || next_y >= MAP_HEIGHT) {
				game_data.status.is_end = true;
				break;
			}

			// 咬自己检测
			SnakeNode* curr = game_data.snake.head;
			while (curr != game_data.snake.head->prev) {
				if (curr->position.x == next_x && curr->position.y == next_y) {
					game_data.status.is_end = true;
					break;
				}
				curr = curr->next;
			}

			if (game_data.status.is_end) break;

			if (next_x == game_data.food_x && next_y == game_data.food_y) {
				game_data.score += 10;
				if (game_data.score > game_data.max_score) {
					game_data.max_score = game_data.score;
				}
				add_head(&game_data.snake, next_x, next_y);
				render_snake_instance(&game_data.snake);
				generate_food();
				render_food(game_data.food_x, game_data.food_y);
				render_score(game_data.score, game_data.max_score);
			}
			else {
				SnakeNode* tail = game_data.snake.head->prev;
				
				int tail_x = tail->position.x;
				int tail_y = tail->position.y;

				add_head(&game_data.snake, next_x, next_y);

				remove_tail(&game_data.snake);

				clear_cell(tail_x, tail_y);
				render_snake_instance(&game_data.snake);
			}
		}
		else {
			render_pause();
		}
		Sleep(150);
	}
}

void game_destroy(void) {
	destroy_snake(&game_data.snake);

	// 如果初始化成功，且正常结束
	if (game_data.status.is_init_success && game_data.status.is_end) {
		long long game_time = (long long)(time(NULL) - game_data.start_time);
		long long timestamp = (long long)time(NULL);
		save_game_history(game_data.score, game_time, timestamp);
		ui_game_over(game_data.score, game_data.max_score);
	}
}

int get_x(SnakeNode* node) {
	return node->position.x;
}

int get_y(SnakeNode* node) {
	return node->position.y;
}

void move_behind(SnakeNode** node) {
	if (node && *node) {
		*node = (*node)->next;
	}
}
// ui.c
#include "ui.h"
#include "config.h"
#include <windows.h>
#include <stdio.h>
#include <conio.h>

static void gotoxy(int x, int y) {
    COORD pos = { x, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void hide_cursor(void) {
    CONSOLE_CURSOR_INFO cursor_info = { 1, 0 };
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

void draw_map(void) {
    for (int i = 0; i <= MAP_WIDTH; i++) {
        gotoxy(i, 0);           printf("-");
        gotoxy(i, MAP_HEIGHT);  printf("-");
    }
    for (int i = 0; i <= MAP_HEIGHT; i++) {
        gotoxy(0, i);           printf("|");
        gotoxy(MAP_WIDTH, i);   printf("|");
    }
}

void render_score(int score, long long max_score) {
    gotoxy(MAP_WIDTH + 2, 5);
    printf("Score: %d      ", score);
    gotoxy(MAP_WIDTH + 2, 9);
    printf("Max Score: %lld      ", max_score);
}

void render_food(int x, int y) {
    gotoxy(x, y);
    printf("$");
}

void render_snake_instance(const Snake* snake) {
    if (snake->head == NULL) return;

    SnakeNode* curr = snake->head;
    do {
        gotoxy(get_x(curr), get_y(curr));
        if (curr == snake->head) {
            printf("@");
        }
        else {
            printf("o");
        }
        move_behind(&curr);
    } while (curr != snake->head);
}

void clear_cell(int x, int y) {
    gotoxy(x, y);
    printf(" ");
}

void render_pause(void) {
    gotoxy(MAP_WIDTH / 2 - 5, MAP_HEIGHT / 2);
    printf("== PAUSED ==");
    gotoxy(MAP_WIDTH / 2 - 10, MAP_HEIGHT / 2 + 2);
    printf("Press ESC to resume, ENTER to quit");
}

void ui_game_over(long long score, long long max_score) {
    // 游戏结束了，直接清空整个控制台，准备展示结算画面
    system("cls");

    // 计算控制台中心位置（假设控制台宽度约为 80，高度约为 25）
    int center_x = 25;
    int center_y = 10;

    // 居中打印标题（加点装饰）
    gotoxy(center_x - 5, center_y);
    printf("GAME OVER!");

    gotoxy(center_x - 10, center_y + 2);
    printf("====================");

    gotoxy(center_x - 8, center_y + 4);
    printf("Final Score: %lld", score);

    gotoxy(center_x - 8, center_y + 5);
    printf("Highest Score: %lld", max_score);

    gotoxy(center_x - 10, center_y + 7);
    printf("====================");

    gotoxy(center_x - 10, center_y + 10);
    printf("Press any key to exit...");
    _getch();
}
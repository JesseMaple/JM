//
// Created by JokerChau on 2026/11/25.
//

#include "UI.h"
#include <stdio.h>
#include "JC_MutableString.h"
#include "JC_IO.h"
#include "space_mode.h"

/* ---------- 各命令的实现，先留空 ---------- */

static void do_add(void) { }

static void do_add_some(void) { }

static void do_remove(void) { }

static void do_remove_some(void) { }

static void do_update(void) { }

static void do_update_some(void) { }

static void do_retrieve(void) { }

static void do_list(void) { }

/* ---------- 命令表：加一条命令，只加一行 ---------- */

typedef struct {
    const char* name;
    void (*fn)(void);
} command_t;

static const command_t commands[] = {
        { "add",         do_add         },
        { "add some",    do_add_some    },
        { "remove",      do_remove      },
        { "remove some", do_remove_some },
        { "update",      do_update      },
        { "update some", do_update_some },
        { "retrieve",    do_retrieve    },
        { "list",        do_list        },
};

static const size_t command_count = sizeof commands / sizeof commands[0];

/* 菜单直接由表生成，不再手抄一遍命令名 */
static void print_menu(void) {
    for (size_t i = 0; i < command_count; ++i) puts(commands[i].name);
    puts("quit");            /* quit 是控制流，不进命令表 */
    printf("\n\n");
    fflush(stdout);
}

/* 查表，找不到返回 NULL；比较沿用库里的忽略大小写版本 */
static const command_t* find_command(jc_MutableString input) {
    for (size_t i = 0; i < command_count; ++i)
        if (jcms_equalsIgnoreCaseWithCh(input, commands[i].name, JC_SPACE_ASCII))
            return &commands[i];

    return NULL;
}

int app(void) {
    int invalid_tag = 0;

    while (1) {
        print_menu();

        jc_MutableString input = jcio_readLineMutableString(stdin);
        bool stop = false;

        if (jcms_getStatus(input) != JC_MS__AVAILABLE) {
            ++invalid_tag;                 /* 分配失败、读错误或 EOF */
            stop = true;
        }
        else if (jcms_trim(input, JC_SPACE_ASCII) != JC_MS__SUCCESSFUL_OP) {
            ++invalid_tag;                 /* 去掉首尾空白失败 */
            stop = true;
        }
        else if (jcms_equalsIgnoreCaseWithCh(input, "quit", JC_SPACE_ASCII)) stop = true; /* 正常退出，不计入 invalid_tag */
        else {
            const command_t* cmd = find_command(input);
            if (cmd) cmd->fn();
            else printf("Invalid command\n");
        }

        /* 唯一释放点：正常命令、非法命令、quit、出错、EOF 都从这里过 */
        jcms_delete(input);
        if (stop) break;
    }

    printf("按下任意键退出程序");
    fflush(stdout);
    (void) getchar();
    return invalid_tag;
}

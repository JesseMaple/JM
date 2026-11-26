//
// Created by JokerChau on 2026/11/25.
//

#include "UI.h"
#include <stdio.h>
#include <stdlib.h>
#include "JC_MutableString.h"
#include "JC_IO.h"
#include "space_mode.h"
#include "JC_ArrayList.h"
#include "repo.h"

static void do_add_prompt(void) {
    printf(
        "add a contact (\"return\" at any time cancels):\n"
        "  <name>\n"
        "  <number1> <number2> ...      one per line, at least one\n"
        "  \"y\"                        numbers finished\n"
        "  <address1> <address2> ...\n"
        "  \"none\"                     no address\n"
        "  \"y\"                        addresses finished, save\n"
        "> "
        );
    fflush(stdout);
}

/* 认为像电话号码就返回 true：+86 135 1234 5678 / 13512345678 / (010) 1234-5678 都能过 */
static bool looks_like_phone(const JC_MutableString* s) {
    const size_t len = jcms_getLength(s);
    size_t digits = 0;

    for (size_t i = 0; i < len; ++i) {
        char c = '\0';
        if (jcms_charAt(s, i, &c) != JC_MS__SUCCESSFUL_OP) return false;

        if (i == 0 && c == '+') continue;                  /* 加号只认第 1 位 */
        if (c == ' ' || c == '-' || c == '(' || c == ')') continue;
        if (c < '0' || c > '9') return false;
        ++digits;
    }
    return digits >= 3 && digits <= 15;                    /* 110 这类短号合法；E.164 上限 15 位 */
}

/* 释放 ArrayList<jc_MutableString> 里的每个字符串，再释放数组本身。
   list 为 NULL 时长度为 0，jcal_delete(NULL) 也安全，所以不用单独判断。 */
static void release_mutable_list(jc_ArrayList list) {
    for (size_t i = 0; i < jcal_getLength(list); ++i) {
        jc_MutableString item = nullptr;
        if (jcal_getAt(list, i, &item) != JC_AL__SUCCESSFUL_OP) continue;
        jcms_delete(item);
    }
    jcal_delete(list);
}

/* ---------- 各命令的实现 ---------- */

static void do_add(void) {
    while (true) {
        do_add_prompt();

        JC_MutableString* name = jcio_readLineMutableString(stdin);
        bool cancelled = false;

        /* NULL 实例的状态就是 JC_MS__NULL，短路之后不会再拿它调别的接口 */
        if (jcms_getStatus(name) != JC_MS__AVAILABLE ||
            jcms_trim(name, JC_SPACE_ASCII) != JC_MS__SUCCESSFUL_OP ||
            jcms_equalsIgnoreCaseWithCh(name, "return", JC_SPACE_ASCII)) {
            cancelled = true;
        }

        if (!cancelled && jcms_getLength(name) == 0) {
            printf("name cannot be empty\n");
            jcms_delete(name);
            continue;                       /* 回到最外层，重新提示 */
        }

        jc_ArrayList numbers = jcal_newOf(jc_MutableString);
        jc_ArrayList addresses = jcal_newOf(jc_MutableString);
        if (jcal_getStatus(numbers) != JC_AL__AVAILABLE ||
            jcal_getStatus(addresses) != JC_AL__AVAILABLE) {
            cancelled = true;
        }

        int stage = 0;                      /* 0 = 收号码，1 = 收地址，2 = 收完 */
        while (!cancelled && stage < 2) {
            jc_MutableString line = jcio_readLineMutableString(stdin);

            if (jcms_getStatus(line) != JC_MS__AVAILABLE ||
                jcms_trim(line, JC_SPACE_ASCII) != JC_MS__SUCCESSFUL_OP) {
                jcms_delete(line);
                cancelled = true;
                break;
            }

            if (jcms_equalsIgnoreCaseWithCh(line, "return", JC_SPACE_ASCII)) {
                jcms_delete(line);          /* 随时取消：清掉本条数据直接回菜单 */
                cancelled = true;
                break;
            }
            if (jcms_equalsIgnoreCaseWithCh(line, "y", JC_SPACE_ASCII)) {
                if (stage == 0 && jcal_getLength(numbers) == 0) {
                    printf("at least one number is required\n");
                    jcms_delete(line);
                    continue;               /* 停在号码段，不换段 */
                }
                ++stage;                    /* 第一个 y 换段，第二个 y 提交 */
                jcms_delete(line);
                continue;
            }
            if (stage == 1 &&
                jcms_equalsIgnoreCaseWithCh(line, "none", JC_SPACE_ASCII)) {
                jcms_delete(line);          /* 地址段：没有地址 */
                continue;
            }
            if (jcms_getLength(line) == 0) {
                jcms_delete(line);
                continue;
            }

            if (stage == 0 && !looks_like_phone(line)) {
                char* raw = jcms_toCharArray(line);
                printf("not a valid phone number: %s\n", raw ? raw : "");
                free(raw);
                jcms_delete(line);
                continue;
            }

            jc_ArrayList target = (stage == 0) ? numbers : addresses;
            if (jcal_pushBack(target, &line) != JC_AL__SUCCESSFUL_OP) {
                jcms_delete(line);
                cancelled = true;
            }
        }

        /* 两个 y 都按了，数据才齐，这时才交给 repo 写盘 */
        if (!cancelled) {
            if (add_contact(name, numbers, addresses)) printf("added\n");
            else printf("failed to add\n");
        }

        /* add_contact 内部必须自己复制需要保留的数据，这里随后统一释放 */
        release_mutable_list(numbers);
        release_mutable_list(addresses);
        jcms_delete(name);

        if (cancelled) break;
    }
}

static void do_add_some() { }

static void do_remove() { }

static void do_remove_some() { }

static void do_update() { }

static void do_update_some() { }

static void do_retrieve() { }

static void do_list() { }

/* ---------- 命令表：加一条命令，只加一行 ---------- */

typedef struct {
    const char* name;
    void (*fn)();
} command_t;

static const command_t main_commands[] = {
        { "add",         do_add         },
        { "add some",    do_add_some    },
        { "remove",      do_remove      },
        { "remove some", do_remove_some },
        { "update",      do_update      },
        { "update some", do_update_some },
        { "retrieve",    do_retrieve    },
        { "list",        do_list        },
};

static constexpr size_t main_command_count = sizeof main_commands / sizeof main_commands[0];

/* 菜单直接由表生成，不再手抄一遍命令名 */
static void print_menu(const size_t cmd_count,const command_t* cmds) {
    printf("\n\n");
    for (size_t i = 0; i < cmd_count; ++i) puts(cmds[i].name);
    puts("quit");            /* quit 是控制流，不进命令表 */
    printf("\n\n");
    fflush(stdout);
}

/* 查表，找不到返回 NULL；比较沿用库里的忽略大小写版本 */
static const command_t* find_main_command(const JC_MutableString* input, const size_t cmd_count, const command_t* cmd) {
    for (size_t i = 0; i < cmd_count; ++i)
        if (jcms_equalsIgnoreCaseWithCh(input, cmd[i].name, JC_SPACE_ASCII))
            return &cmd[i];

    return nullptr;
}

int app() {
    int invalid_tag = 0;

    while (true) {
        print_menu(main_command_count, main_commands);

        JC_MutableString* input = jcio_readLineMutableString(stdin);
        bool stop = false;

        /* NULL 实例的状态是 JC_MS__NULL，会被这一条直接拦下 */
        if (jcms_getStatus(input) != JC_MS__AVAILABLE ||
            jcms_trim(input, JC_SPACE_ASCII) != JC_MS__SUCCESSFUL_OP) {
            ++invalid_tag;                 /* 分配失败、读错误或 EOF */
            stop = true;
        }
        else if (jcms_equalsIgnoreCaseWithCh(input, "quit", JC_SPACE_ASCII)) stop = true; /* 正常退出，不计入 invalid_tag */
        else {
            const command_t* cmd = find_main_command(input, main_command_count, main_commands);
            if (cmd) cmd->fn();
            else printf("Invalid command\n");
        }

        /* 唯一释放点：正常命令、非法命令、quit、出错、EOF 都从这里过 */
        jcms_delete(input);
        if (stop) break;
    }

    printf("Press Enter to exit...");
    fflush(stdout);
    (void) getchar();
    return invalid_tag;
}

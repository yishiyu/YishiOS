// 贪吃蛇的头文件
#ifndef GAME_SNAKE
#define GAME_SNAKE

#include "struct.h"
#include "syscall.h"

// 字符界面行列最大值
#define MAX_ROW (25)
#define MAX_COLUMN (80)

// 入口函数
void _start();

// 子函数
void snake_init();
void snake_handler();

#endif
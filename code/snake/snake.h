// 贪吃蛇的头文件
#ifndef GAME_SNAKE
#define GAME_SNAKE

#include "macro.h"
#include "string.h"
#include "struct.h"
#include "syscall.h"

// 字符界面行列最大值
#define MAX_ROW (25)
#define MAX_COLUMN (80)
#define SNAKE_MAX_LENGTH (MAX_ROW * MAX_COLUMN)
#define SNAKE_VALID (24 * 80)

// 贪吃蛇显示设置
#define SNAKE_COLOR MAKE_COLOR(BLACK, WHITE)
#define SNAKE_HEAD ('@')
#define SNAKE_BODY ('*')
#define SNAKE_BALL ('O')
#define SNAKE_BLANK (' ')

// 贪吃蛇方向
#define SNAKE_UP 0
#define SNAKE_DOWN 1
#define SNAKE_LEFT 2
#define SNAKE_RIGHT 3

// 贪吃蛇移动速度 (移动间隔时间,单位为ms)
#define SNAKE_SPEED 300

// 功能按键
// 功能按钮
#define KEYBOARD_FUNC_UP 0
#define KEYBOARD_FUNC_DOWN 1
#define KEYBOARD_FUNC_LEFT 2
#define KEYBOARD_FUNC_RIGHT 3
#define KEYBOARD_FUNC_SHIFT 4
#define KEYBOARD_FUNC_PAUSE ('p')

// 键盘数据类型定义
#define KEYBOARD_TYPE_EMPTY 0  // 空数据
#define KEYBOARD_TYPE_ASCII 1  // 可打印的ASCII码
#define KEYBOARD_TYPE_FUNC 2   // 功能按钮

// 内存管理宏
#define MEM_EXECUTE 0x01
#define MEM_EXEIT 0x02


// 入口函数
void _start();

// 子函数
void snake_init();
void snake_handler();
void snake_refresh();
void snake_move();
void snake_control();
void snake_exit();

// 调试函数
void disp_str(char* data);
void disp_int(int count);

#endif

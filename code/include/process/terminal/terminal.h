// 终端任务体
#ifndef YISHIOS_TERMINAL_H
#define YISHIOS_TERMINAL_H

#include "global.h"
#include "syscall.h"
#include "display.h"
#include "terminallib.h"
#include "keymap.h"

// 不同的函数即不同的终端
// 不同的终端共享相同的子函数,但是不共享数据

// 第一个终端,同时也是默认的终端
void tty_1();

// 第二个终端
void tty_2();

#endif

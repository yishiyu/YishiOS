// 终端任务体
#ifndef YISHIOS_TERMINAL_H
#define YISHIOS_TERMINAL_H

#include "filesystem.h"
#include "global.h"
#include "terminallib.h"

// 不同的函数即不同的终端
// 不同的终端共享相同的子函数,但是不共享数据

// 第一个终端,同时也是默认的终端
void tty_0();

// 第二个终端
void tty_1();

#endif

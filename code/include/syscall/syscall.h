// 系统调用函数的声明
#ifndef YISHIOS_SYSCALL_H
#define YISHIOS_SYSCALL_H

#include "global.h"
#include "keymap.h"
#include "struct.h"
#include "terminallib.h"

// 前两部分函数存在的意义是使得系统本身的任务级进程也可以调用系统调用
// 相当于是系统本身的运行库

//===============用户可以调用的系统调用==================
// 读取键盘的系统调用
// 返回结果为结构体
KEYMAP_RESULT sys_read_keyboard();
void sys_terminal_write(int terminal_index, char* data);

//============系统调用中引发中断的函数部分===============
u32 asm_read_keyboard();
u32 asm_terminal_write(int terminal_index, char* data);

//=================最终工作的函数======================
u32 kernel_read_keyboard();
u32 kernel_terminal_write(int terminal_index, char* data);

#endif
// 系统调用函数的声明
#ifndef YISHIOS_SYSCALL_H
#define YISHIOS_SYSCALL_H

#include "global.h"
#include "struct.h"
#include "keymap.h"
#include "type.h"

//===============用户可以调用的系统调用==================

// 读取键盘的系统调用
// 返回结果为结构体
KEYMAP_RESULT sys_read_keyboard();

//============系统调用中引发中断的函数部分===============
u32 asm_read_keyboard();

//=================最终工作的函数======================
u32 kernel_read_keyboard();

#endif
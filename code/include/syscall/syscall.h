// 系统调用函数的声明
#ifndef YISHIOS_SYSCALL_H
#define YISHIOS_SYSCALL_H

#include "global.h"
#include "irqhandler.h"
#include "keymap.h"
#include "memory.h"
#include "struct.h"
#include "terminallib.h"

// 前两部分函数存在的意义是使得系统本身的任务级进程也可以调用系统调用
// 相当于是系统本身的运行库

//===============用户可以调用的系统调用==================
// 读取键盘的系统调用
// 返回结果为结构体
KEYMAP_RESULT sys_read_keyboard();
void sys_terminal_write(int console_index, char* data, int pid);
int sys_sendrec(int function, int src_dest, MESSAGE* m, int pid);
u32 sys_get_ticks();

//============系统调用中引发中断的函数部分===============
u32 asm_syscall(int sys_vector, u32 para0,u32 para1,u32 para2, u32 para3);

//=================最终工作的函数======================
u32 kernel_read_keyboard();
u32 kernel_sendrec(int function, int src_dest, MESSAGE* m, int pid);
u32 kernel_get_ticks();

//=====================工具函数=======================
int ldt_seg_linear(int pid, int seg_index);
void* va2la(int pid, void* va);

#endif
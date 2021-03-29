// 系统调用函数的声明
#ifndef YISHIOS_SYSCALL_H
#define YISHIOS_SYSCALL_H

#include "global.h"
#include "irqhandler.h"
#include "keymap.h"
#include "mem.h"
#include "struct.h"
#include "terminallib.h"

//===============用户可以调用的系统调用==================
// 屏幕显示用的系统调用
void sys_terminal_write(int console_index, char* data, int pid);
void sys_terminal_clear(int console_index, int pid);
void sys_terminal_draw(int console_index, char* data, int pid);

int sys_sendrec(int function, int src_dest, MESSAGE* m, int pid);
u32 sys_get_ticks();
int sys_get_diskinfo(char* buffer, int count, int pid);
u32 sys_get_pid();

//============系统调用中引发中断的函数部分===============
u32 asm_syscall(int sys_vector, u32 para0, u32 para1, u32 para2, u32 para3);

//=================最终工作的函数======================
u32 kernel_sendrec(int function, int src_dest, MESSAGE* m, int pid);
u32 kernel_get_ticks();
u32 kernel_get_pid();

//=====================工具函数=======================
int ldt_seg_linear(int pid, int seg_index);
void* va2la(int pid, void* va);
void inform_int(int pid, u32 int_type);

#endif
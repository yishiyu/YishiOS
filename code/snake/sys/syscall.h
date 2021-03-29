// 系统调用函数的声明
#ifndef YISHIOS_SYSCALL_H
#define YISHIOS_SYSCALL_H

#include "type.h"
#include "struct.h"
#include "macro.h"

//===============用户可以调用的系统调用==================
// 屏幕显示用的系统调用
void sys_terminal_write(MESSAGE *message,int console_index, char* data, int pid);
void sys_terminal_clear(MESSAGE *message,int console_index, int pid);
void sys_terminal_draw(MESSAGE *message,int console_index, char* data, int pid);

int sys_sendrec(int function, int src_dest, MESSAGE* m, int pid);
u32 sys_get_ticks();
int sys_get_diskinfo(char* buffer, int count, int pid);
u32 sys_get_pid();
int sys_set_timer(int pid, u32 time);

//============系统调用中引发中断的函数部分===============
u32 asm_syscall(int sys_vector, u32 para0, u32 para1, u32 para2, u32 para3);


#endif
// 这个文件用来初始化系统基本进程, 系统进程调度
// 当前的系统基本进程为下面列出来的a b c三个
// 后期系统基本进程各自安放在各自的文件中(如tty进程 gui进程等)
// 系统调度算法也在这里
#ifndef YISHIOS_PROC_H
#define YISHIOS_PROC_H

#include "func.h"
#include "initirq.h"

// 初始化进程函数
int start_proc();

// 辅助函数
void init_pcb(TASK* task, PROCESS* proc, u32 pid, char* stack,
              u16 selector_ldt);
int get_pcb(PROCESS** proc);

#endif
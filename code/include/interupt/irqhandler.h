// 主要放各种中断处理函数
#ifndef YISHIOS_IRQHANDLER_H
#define YISHIOS_IRQHANDLER_H

#include "display.h"
#include "global.h"
#include "terminal.h"
#include "syscall.h"
#include "disk.h"

// 辅助宏定义
#define is_ready_empty (p_proc_ready_head == &p_proc_ready_tail)
#define is_ready_one_left (p_proc_ready_head->next_pcb == &p_proc_ready_tail)
#define is_wait_empty (p_proc_wait_head == &p_proc_wait_tail)
#define is_wait_one_left (p_proc_wait_head->next_pcb == &p_proc_wait_tail)
#define is_pause_empty (p_proc_pause_head == &p_proc_pause_tail)
#define is_pause_one_left (p_proc_pause_head->next_pcb == &p_proc_pause_tail)
#define is_empty_process (p_proc_ready_head == &PCB_empty_task)

// 时钟中断处理
void clock_handler(int irq);
void schedule();

// 键盘中断处理
void keyboard_handler(int irq);

// 磁盘中断处理
void disk_handler(int irq);

#endif
// 文件中没有函数和变量定义
// 存在的意义是使得汇编器在汇编的时候能找到include文件夹中头文件中的定义

#include "global.h"

//当前屏幕指针位置
int disp_pos;

// kernel中的GDT指针,修改后会指向下面的全局描述符表
//全局描述符表
// 0~15:Limit  16~47:Base 共48位
u8 gdt_ptr[6];
DESCRIPTOR gdt[GDT_SIZE];

// kernel中的IDT指针,修改后会指向下面的中断描述符表
//中断描述符表
// 0~15:Limit  16~47:Base 共48位
u8 idt_ptr[6];
GATE idt[IDT_SIZE];

// TSS结构体
TSS tss;

//预留给系统初始进程的栈
char task_stack[BASE_TASKS_STACK_SIZE];

// 用于控制中断重入,相当于一个信号量
// 如果该变量不为-1则当前中断为重复进入的中断,应当立即退出
// 感觉这个设计好奇怪呀...于渊老哥不知道咋想的...
// 通过设置中断屏蔽字来控制中断重入不香吗?
// 在下一版中把这个会移除
// 哦哦哦懂了懂了,这个还是有用的
// 作用在于控制中断的栈切换
// 如果从用户进程进入中断进程则需要切换堆栈
// 如果从中断进程进入中断进程则不需要切换堆栈
u32 k_reenter;

// 用于系统计时用的变量
u32 ticks;

//  空白PCB存储结构
int PCB_USED = PID_STACK_BASE;
int PCB_stack_status[MAX_PROCESS_NUM] = {0};
PROCESS PCB_stack[MAX_PROCESS_NUM];

// 进程调度指针
PROCESS* p_proc_ready_head;
PROCESS p_proc_ready_tail;
PROCESS* p_proc_wait_head;
PROCESS p_proc_wait_tail;
PROCESS* p_proc_pause_head;
PROCESS p_proc_pause_tail;

// 当前终端号
int t_present_terminal;

//中断处理函数指针数组
irq_handler irq_table[IRQ_NUM];

//系统调用处理函数指针数组
system_call sys_call_table[SYS_CALL_NUM] = {kernel_sendrec, kernel_get_ticks};

// 任务级进程
TASK task_table[TASK_NUM] = {
    {output_server, STACK_OUTPUT_SYSTEM, "output_server",
     PRIORITY_OUTPUT_SERVER, PID_OUTPUT_SERVER},
    {input_server, STACK_INPUT_SYSTEM, "input_server", PRIORITY_INTPUT_SERVER,
     PID_INPUT_SERVER},
    {tty_0, STACK_TERMINAL, "terminal_0", PRIORITY_TERMINAL, PID_TTY0},
    {tty_1, STACK_TERMINAL, "terminal_1", PRIORITY_TERMINAL, PID_TTY1}};

TASK empty_task = {empty_function, STACK_EMPTY_TASK, "empty_task",
                   PRIORITY_EMPTY_TASK, EMPTY_TASK_PID};
PROCESS PCB_empty_task;

KEYMAP_BUFFER key_buffer;
// KEYMAP_RESULT_BUFFER key_result_buffer;

TERMINAL terminal_table[TERMINAL_NUM];
CONSOLE console_table[TERMINAL_NUM];
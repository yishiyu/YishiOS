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

//系统初始进程PCB
PROCESS proc_table[BASE_TASKS_NUM];

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
int ticks;

//就绪队列头指针
//其实当前只有一个任务队列,这个指针指向当前下一个执行的进程
PROCESS* p_proc_ready;

//中断处理函数指针数组
irq_handler irq_table[IRQ_NUM];


//系统预定义进程初始状态
TASK task_table[BASE_TASKS_NUM] = {{TestA, STACK_SIZE_TESTA, "TestA"},
                                   {TestB, STACK_SIZE_TESTB, "TestB"},
                                   {TestC, STACK_SIZE_TESTC, "TestC"}};

KEYMAP_BUFFER key_buffer;
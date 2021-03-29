// 定义各种结构体
#ifndef YISHIOS_STRUCT_H
#define YISHIOS_STRUCT_H

#include "macro.h"
#include "type.h"

//存储段描述符/系统段描述符
typedef struct s_descriptor {
    u16 limit_low;        // Limit
    u16 base_low;         // Base
    u8 base_mid;          // Base
    u8 attr1;             // P(1) DPL(2) DT(1) TYPE(4)
    u8 limit_high_attr2;  // G(1) D(1) 0(1) AVL(1) LimitHigh(4)
    u8 base_high;         // Base
} DESCRIPTOR;

//门描述符(/调用门中断门/陷阱门/任务门)
typedef struct s_gate {
    u16 offset_low;  // Offset Low
    u16 selector;    // Selector
    u8 dcount;  //调用门中切换堆栈时要复制的双字参数的个数
    u8 attr;    // P(1) DPL(2) DT(1) TYPE(4)
    u16 offset_high;  // Offset High
} GATE;

// TSS结构体
typedef struct s_tss {
    u32 backlink;
    u32 esp0;  //内核段栈指针
    u32 ss0;   //内核段栈基址
    u32 esp1;
    u32 ss1;
    u32 esp2;
    u32 ss2;
    u32 cr3;
    u32 eip;
    u32 flags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u32 es;
    u32 cs;
    u32 ss;
    u32 ds;
    u32 fs;
    u32 gs;
    u32 ldt;
    u16 trap;
    u16 iobase;  // I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图
} TSS;

// 进程控制块PCB中存放的关于程序运行状态的信息
typedef struct s_stackframe {
    // 低地址======================
    //  ----------------------------------------------------
    // 高地址======================
    // 在进程切换的过程中被 save() 保存的进程状态
    u32 gs;
    u32 fs;
    u32 es;
    u32 ds;
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 kernel_esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;
    //============================
    // 在发生中断的时候esp会指向这里
    // 如果想在时钟中断里使用函数,就必须在这里预留空间来压下返回地址
    // 具体到语句就是时钟中断里的call save语句,会使用这里预留下的4字节空间
    u32 retaddr;
    //============================
    // 在中断发生的时候会被CPU自动保存的寄存器
    u32 eip;
    u32 cs;
    u32 eflags;
    u32 esp;
    u32 ss;
    //============================
} STACK_FRAME;

// 进程控制块PCB组成单元
// 存放了程序运行状态信息,ldt选择子,进程调度信息,进程id,进程名字
typedef struct s_proc {
    STACK_FRAME regs;

    //虽然下面接着就是局部描述符表,但是这个ldt_sel依然有存在的意义
    //其指出了ldt的界限,同时简化了加载ldt的步骤
    u16 ldt_sel;
    DESCRIPTOR ldts[LDT_SIZE];

    int ticks;
    int priority;

    u32 pid;
    char p_name[16];
} PROCESS;

// 这个结构体用来定义系统初始进程
// 这个设计最早来自minix
typedef struct s_task {
    task_f initial_eip;
    int stacksize;
    char name[16];
} TASK;

// 键盘缓冲区结构体
typedef struct s_keymap_buffer {
    u8 key_buf[KEY_BUF_SIZE];  //缓冲区
    u8 key_head;               // 指向缓冲区中下一个空闲位置
    u8 key_tail;               // 最早输入的键值
    int key_count;             //缓冲区中已有的键值数量
} KEYMAP_BUFFER;

typedef struct s_keyrmap_result {
    key_type type;   //键值的类型
    key_value data;  //键值的数据
} KEYMAP_RESULT;

#endif
// 定义各种宏
#ifndef YISHIOS_MACRO_H
#define YISHIOS_MACRO_H

// 设定一个调试标志
// #define __YISHIOS_DEBUG__

// 把这个声明放在需要调试的文件的头文件中即可
// #ifndef __YISHIOS_DEBUG__
// #define pause()
// #define disp_int(str)
// #define disp_str(str)
// #else
// #ifndef __DEBUG_PROC__
// #define pause()
// #define disp_int(str)
// #define disp_str(str)
// #endif
// #endif

//用于导出变量等
#define EXTERN extern

// GDT 和 IDT 中描述符的个数
#define GDT_SIZE 128
#define IDT_SIZE 256

// 每个进程中允许的局部描述符表大小
#define LDT_SIZE 2
// 两个类型的局部描述符下标
#define INDEX_LDT_C 0
#define INDEX_LDT_RW 1

//系统初始任务数量
//键盘处理进程,tty任务
//键盘结果缓冲区大小
// tty任务缓冲区大小
#define BASE_TASKS_NUM 2
#define TERMINAL_NUM 2
#define TASK_NUM (BASE_TASKS_NUM + TERMINAL_NUM)
#define KEY_RESULT_NUM 128
#define TTY_BUFFER_NUM 256

//不同任务的优先级(即占有周期数)
#define PRIORITY_OUTPUT_SERVER 5
#define PRIORITY_KEYBOARD_SERVER 5
#define PRIORITY_TERMINAL 5
#define PRIORITY_EMPTY_TASK 2

//系统初始任务分配的堆栈大小: 各32kb
#define STACK_OUTPUT_SYSTEM 0x8000
#define STACK_KEYBOARD_SERVER 0x8000
#define STACK_TERMINAL 0x8000
#define STACK_EMPTY_TASK 0x100
#define BASE_TASKS_STACK_SIZE                      \
    (STACK_OUTPUT_SYSTEM + STACK_KEYBOARD_SERVER + \
     TERMINAL_NUM * STACK_TERMINAL + STACK_EMPTY_TASK)

//定义内核代码,数据,显存选择子
#define SELECTOR_KERNEL_CS SELECTOR_FLAT_C
#define SELECTOR_KERNEL_DS SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_GS SELECTOR_VIDEO

//内核代码选择子
//其中+3意为把最后三位设置为011,即把RPL设置为3
//显存段设为3的目的是为了让所有优先级的程序都能访问显存
#define SELECTOR_DUMMY 0
#define SELECTOR_FLAT_C 0x08
#define SELECTOR_FLAT_RW 0x10
#define SELECTOR_VIDEO (0x18 + 3)
// TSS段选择子
#define SELECTOR_TSS 0x20
//第一个LDT选择子
#define SELECTOR_LDT_FIRST 0x28

//内核代码选择子偏移,用于在c文件中确定选择子对应的描述符数组下标
#define INDEX_DUMMY 0
#define INDEX_FLAT_C 1
#define INDEX_FLAT_RW 2
#define INDEX_VIDEO 3
#define INDEX_TSS 4
#define INDEX_LDT_FIRST 5

// 8259A 中断控制器的端口定义
#define INT_Master_CTL 0x20
#define INT_Master_CTL_MASK 0x21
#define INT_Slave_CTL 0xA0
#define INT_Slave_CTL_MASK 0xA1

// 8259A 中断的起始向量
#define INT_VECTOR_IRQ0 0x20
#define INT_VECTOR_IRQ8 0x28

//  CPU保留中断和异常
#define INT_VECTOR_DIVIDE 0x0
#define INT_VECTOR_DEBUG 0x1
#define INT_VECTOR_NMI 0x2
#define INT_VECTOR_BREAKPOINT 0x3
#define INT_VECTOR_OVERFLOW 0x4
#define INT_VECTOR_BOUNDS 0x5
#define INT_VECTOR_INVAL_OP 0x6
#define INT_VECTOR_COPROC_NOT 0x7
#define INT_VECTOR_DOUBLE_FAULT 0x8
#define INT_VECTOR_COPROC_SEG 0x9
#define INT_VECTOR_INVAL_TSS 0xA
#define INT_VECTOR_SEG_NOT 0xB
#define INT_VECTOR_STACK_FAULT 0xC
#define INT_VECTOR_PROTECTION 0xD
#define INT_VECTOR_PAGE_FAULT 0xE
#define INT_VECTOR_COPROC_ERR 0x10

// 系统段描述符类型值说明
#define DESEC_ATTR_LDT 0x82      /* 局部描述符表段类型值			*/
#define DESEC_ATTR_TaskGate 0x85 /* 任务门类型值   */
#define DESEC_ATTR_386TSS 0x89   /* 可用 386 任务状态段类型值		*/
#define DESEC_ATTR_386CGate 0x8C /* 386 调用门类型值			*/
#define DESEC_ATTR_386IGate 0x8E /* 386 中断门类型值			*/
#define DESEC_ATTR_386TGate 0x8F /* 386 陷阱门类型值			*/
// 存储段描述符类型值说明
#define DESEC_ATTR_DATA_R 0x90  /* 存在的只读数据段类型值		*/
#define DESEC_ATTR_DATA_RW 0x92 /* 存在的可读写数据段属性值		*/
#define DESEC_ATTR_DATA_RWA 0x93 /* 存在的已访问可读写数据段类型值	*/
#define DESEC_ATTR_CODE_E 0x98  /* 存在的只执行代码段属性值		*/
#define DESEC_ATTR_CODE_ER 0x9A /* 存在的可执行可读代码段属性值		*/
#define DESEC_ATTR_CODE_ECO 0x9C /* 存在的只执行一致代码段属性值 */
#define DESEC_ATTR_CODE_ECRO 0x9E /* 存在的可执行可读一致代码段属性值	*/
//描述符类型值说明
#define DESEC_ATTR_32 0x4000       /* 32 位段				*/
#define DESEC_ATTR_LIMIT_4K 0x8000 /* 段界限粒度为 4K 字节 */
#define DESEC_ATTR_DPL0 0x00       /* DPL = 0				*/
#define DESEC_ATTR_DPL1 0x20       /* DPL = 1				*/
#define DESEC_ATTR_DPL2 0x40       /* DPL = 2				*/
#define DESEC_ATTR_DPL3 0x60       /* DPL = 3				*/

//选择子属性
// 1. 请求优先级属性
#define SELEC_ATTR_RPL_MASK 0xFFFC
#define SELEC_ATTR_RPL0 0
#define SELEC_ATTR_RPL1 1
#define SELEC_ATTR_RPL2 2
#define SELEC_ATTR_RPL3 3
// 2. 选择子类型属性(TI位)
#define SELEC_TI_MASK 0xFFFB
#define SELEC_TI_GLOBAL 0
#define SELEC_TI_LOCAL 4

// 权限/优先级
// 优先级只分三个,内核,任务,用户
// 已经够用了,其实linux只有内核和用户两个优先级,照样够用
#define PRIVILEGE_KRNL 0
#define PRIVILEGE_TASK 1
#define PRIVILEGE_USER 3
#define RPL_KRNL SELEC_ATTR_RPL0
#define RPL_TASK SELEC_ATTR_RPL1
#define RPL_USER SELEC_ATTR_RPL3

//把线性地址转换成物理地址
#define vir2phys(seg_base, vir) (u32)(((u32)seg_base) + (u32)(vir))

// 8253/8254 PIT (Programmable Interval Timer)
// 相关介绍在ORANGE书的227页 (6.5.2)
// 8253 芯片有三个计时器,分别用于时钟中断,SRAM刷新,连接PC喇叭
// 时钟中断计时器端口号为0x40
// 8253模式控制寄存器端口号为0x43
// 0x34设置PIT模式为rate generate模式,同时指定接下来先写低字节后写高字节
// 长整数1193182L 为计时器的PC输入频率
#define TIMER0 0x40
#define TIMER_MODE 0x43
#define RATE_GENERATOR 0x34
#define TIMER_FREQ 1193182L
#define HZ 100

// 8259A 芯片的中断号和对应的中断
#define IRQ_NUM 16
#define IRQ_CLOCK 0
#define IRQ_KEYBOARD 1

// 系统调用的个数及其对应的中断号
#define SYS_CALL_NUM 3
#define SYS_CALL_VECTOR 0x90

// 系统调用表
#define SYS_READ_KEYBOARD 0
#define SYS_SENDREC 1
#define SYS_GET_TICKS 2

// 键盘缓冲区大小
#define KEY_BUF_SIZE 128

// 终端显示部分
#define VIDEO_MEM_SIZE 0x8000   /* 32K: B8000H -> BFFFFH */
#define VIDEO_MEM_BASE 0xB8000  /* base of color video memory */
#define CRT_CTRL_ADDR_REG 0x3D4 /* CRT Controller Registers - Addr Register */
#define CRT_CTRL_DATA_REG 0x3D5 /* CRT Controller Registers - Data Register */
#define START_ADDR_H 0xC        /* reg index of video mem start addr (MSB) */
#define START_ADDR_L 0xD        /* reg index of video mem start addr (LSB) */
#define CURSOR_H 0xE            /* reg index of cursor position (MSB) */
#define CURSOR_L 0xF            /* reg index of cursor position (LSB) */

// 终端字符颜色
#define DEFAULT_CHAR_COLOR (MAKE_COLOR(BLACK, WHITE))
#define BLANK_CHAR_COLOR (MAKE_COLOR(BLACK, BLACK))
#define GRAY_CHAR (MAKE_COLOR(BLACK, BLACK) | BRIGHT)
#define RED_CHAR (MAKE_COLOR(BLUE, RED) | BRIGHT)
// 字符颜色
#define BLACK 0x0   /* 0000 */
#define WHITE 0x7   /* 0111 */
#define RED 0x4     /* 0100 */
#define GREEN 0x2   /* 0010 */
#define BLUE 0x1    /* 0001 */
#define FLASH 0x80  /* 1000 0000 */
#define BRIGHT 0x08 /* 0000 1000 */
#define MAKE_COLOR(x, y) ((x << 4) | y)
// 终端参数 设置
#define TERMINAL_WIDTH 80

// 系统中存在的进程的最大个数
#define MAX_PROCESS_NUM 10

// 进程间通信相关定义
// 进程间通信功能代号
#define SEND 1
#define RECEIVE 2
#define BOTH 3

// 进程状态标志
#define RUNNING 0x00
#define SENDING 0x02
#define RECEIVING 0x04

// 死锁状态结果
#define NO_DEADLOCK 0
#define SEND_DEADLOCK 1
#define RECV_DEADLOCK 2

// 发送/期待信息来源类型
// #define INVALID_DRIVER	-20
#define INTERRUPT -10
#define OUTPUT_SYSTEM 0
// #define TASK_TTY	0
// #define TASK_SYS	1
// #define TASK_WINCH	2
// #define TASK_FS	3
// #define TASK_MM	4
#define ANY (MAX_PROCESS_NUM + 10)
// 注意这两个,一个是不接受信息状态,一个是空进程
#define NO_TASK (MAX_PROCESS_NUM + 20)
#define EMPTY_TASK_PID (MAX_PROCESS_NUM + 30)

// 系统预定义的进程的PID(永远绑定,便于实现其他系统调用)
#define PID_OUTPUT_SERVER 0
#define PID_KEYBOARD_SERVER 1
#define PID_TTY0 2
#define PID_TTY1 3
// 剩下可以分配的PCB的起点
#define PID_STACK_BASE 4


// 消息类型
#define HARD_INT 0          // 硬件中断
#define SERVER_OUTPUT 1     // 输出信息


// OUTPUT子系统宏定义
// OUTPUT子系统消息类型
#define OUTPUT_MESSTYPE_DISP 0
#define OUTPUT_MESSTYPE_FUNC 1

// OUTPUT子系统特殊功能定义
// 部分功能与键盘功能按键对应(上下左右)
#define OUTPUT_DISP_FUNC_UP 0
#define OUTPUT_DISP_FUNC_DOWN 1
#define OUTPUT_DISP_FUNC_LEFT 2
#define OUTPUT_DISP_FUNC_RIGHT 3
#define OUTPUT_DISP_FUNC_DRAW 4         // 调整至指定控制台
#define OUTPUT_DISP_FUNC_CLEAR 5        // 清空屏幕
#define OUTPUT_DISP_FUNC_RESET 6        // 清空整个控制台




#endif
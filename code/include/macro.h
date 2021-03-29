// 定义各种宏
#ifndef YISHIOS_MACRO_H
#define YISHIOS_MACRO_H

//用于导出变量等
#define EXTERN extern

// GDT 和 IDT 中描述符的个数
#define GDT_SIZE 128
#define IDT_SIZE 256

// 每个进程中允许的局部描述符表大小
#define LDT_SIZE 2

//系统初始任务数量
#define BASE_TASKS_NUM 3

//系统初始任务分配的堆栈大小: 各32kb
#define STACK_SIZE_TESTA 0x8000
#define STACK_SIZE_TESTB 0x8000
#define STACK_SIZE_TESTC 0x8000
#define BASE_TASKS_STACK_SIZE \
    (STACK_SIZE_TESTA + STACK_SIZE_TESTB + STACK_SIZE_TESTC)

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

#endif
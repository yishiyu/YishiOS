// 定义各种宏
#ifndef YISHIOS_MACRO_H
#define YISHIOS_MACRO_H

// 最大进程数
#define MAX_PROCESS_NUM 10

// 终端缓冲区大小
#define TTY_BUFFER_NUM 256

// 系统调用的个数及其对应的中断号
#define SYS_CALL_NUM 3
#define SYS_CALL_VECTOR 0x90

// 系统调用表
// 系统调用中三个是宏内核的方式实现,其他的用IPC实现
// 需要与global.c声明的系统调用表顺序保持一致
#define SYS_SENDREC 0
#define SYS_GET_TICKS 1
#define SYS_GET_PID 2
#define SYS_SET_TIMER 3

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
// x底y字
#define MAKE_COLOR(x, y) ((x << 4) | y)

// 进程间通信相关定义
// 进程间通信功能代号
#define SEND 1
#define RECEIVE 2
#define BOTH 3

// 进程状态标志
#define RUNNING 0x00
#define SENDING 0x02
#define RECEIVING 0x04

// 发送/期待信息来源类型
// #define INVALID_DRIVER	-20
#define INTERRUPT PID_INTERRUPT
#define OUTPUT_SYSTEM PID_OUTPUT_SERVER
#define INPUT_SYSTEM PID_INPUT_SERVER
#define DISK_SYSTEM PID_DISK_SERVER
#define FILE_SYSTEM PID_FS_SERVER
#define MEM_SYSTEM PID_MEM_SERVER
// #define TASK_TTY	0
// #define TASK_SYS	1
// #define TASK_WINCH	2
// #define TASK_FS	3
// #define TASK_MM	4
#define ANY (MAX_PROCESS_NUM + 10)
// 注意这两个,一个是不接受信息状态,一个是空进程
#define NO_TASK (MAX_PROCESS_NUM + 20)

// 系统预定义的进程的PID(永远绑定,便于实现其他系统调用)
#define PID_INTERRUPT -10
#define PID_OUTPUT_SERVER 0
#define PID_INPUT_SERVER 1
#define PID_DISK_SERVER 2
#define PID_FS_SERVER 3
#define PID_MEM_SERVER 4
#define PID_TTY0 5
#define PID_TTY1 6
// 剩下可以分配的PCB的起点
#define PID_STACK_BASE 7
#define PID_EMTPY_TASK (MAX_PROCESS_NUM + 30)

// 消息类型
#define SERVER_OUTPUT PID_OUTPUT_SERVER  // 输出信息
#define SERVER_INPUT PID_INPUT_SERVER    // 输入信息
#define SERVER_DISK PID_DISK_SERVER      // 磁盘信息
#define SERVER_FS PID_FS_SERVER          // 文件系统的消息
#define SERVER_MEM PID_MEM_SERVER        // 内存管理信息

// 硬件中断类型
#define HARD_INT_KEYBOARD 0x01  // 键盘中断
#define HARD_INT_DISK 0x02      // 硬盘中断

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
#define OUTPUT_DISP_FUNC_DRAW 4   // 根据一块内存刷新界面
#define OUTPUT_DISP_FUNC_CLEAR 5  // 下滚至清空屏幕
#define OUTPUT_DISP_FUNC_RESET 6  // 清空整个控制台

// 磁盘操作宏定义
#define DISK_OPEN 0
#define DISK_CLOSE 1
#define DISK_READ 2
#define DISK_WRITE 3
#define DISK_INFO 4

#endif
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

    // 双向链表的两个指针
    struct s_proc* pre_pcb;
    struct s_proc* next_pcb;

    // 进程间通信需要用到的数据
    // 进程状态标志
    // 0 --> 正常执行
    // 2 --> 发送信息阻塞中
    // 4 --> 接收信息阻塞中
    u32 flags;

    // 本进程准备发送的信息或等待接收的信息
    // 尝试发送信息的对象
    // 尝试接收信息的源
    struct mess* message;
    int recv_from;
    int send_to;

    // 0 --> ready to handle an interupt
    // 1 --> handling an interupt
    // 操作系统通过把这一位从0设为1来通知中断的发生
    int has_int_msg;

    // 希望发信息给该进程的进程的链表
    struct s_proc* sending_to_this;
    // 用于产生链表结构的指针
    struct s_proc* next_sending;

} PROCESS;

// 这个结构体用来定义系统初始进程
// 这个设计最早来自minix
typedef struct s_task {
    task_f initial_eip;
    int stacksize;
    char name[16];
    int priority;
    u32 pid;
} TASK;

// 键盘缓冲区结构体
typedef struct s_keymap {
    u8 key_buf[KEY_BUF_SIZE];   //缓冲区
    u8 key_flag[KEY_BUF_SIZE];  //每一个码的标志位
    u8 key_head;                // 指向缓冲区中下一个空闲位置
    u8 key_tail;                // 最早输入的键值
    int key_count;              //缓冲区中已有的键值数量
} KEYMAP_BUFFER;

typedef struct s_keyrmap_result {
    key_type type;   //键值的类型
    key_value data;  //键值的数据
} KEYMAP_RESULT;

// 终端中的数据,相当于类中的成员变量
//控制台结构体
typedef struct console {
    u32 current_start_addr;  // 当前控制台显存起始地址
    u32 original_addr;       // 当前控制台对应的显存位置
    u32 mem_limit;           //当前控制台显存大小
    u32 cursor;              // 光标

} CONSOLE;

// 文件描述符
typedef struct file_descriptor {
    int fd_pos;
    struct inode* fd_inode;
} FILE_DESCRIPTOR;

// 终端结构体
typedef struct terminal {
    //待处理的命令缓冲区
    char in_buf[TTY_BUFFER_NUM];
    int in_head;
    int in_tail;
    int in_count;
    int terminal_ID;
    int pid;
    //控制台
    CONSOLE* console;
    // 文件系统相关
    char* directory_buffer;     // 当前目录缓冲区
    int directory_buffer_size;  //缓冲区大小
    FILE_DESCRIPTOR* directory_fd;   //目录文件描述符
} TERMINAL;

// ext2文件系统结构
struct inode {
    u16 i_mode;
    u16 i_uid;
    u32 i_size;
    u32 i_atime;
    u32 i_ctime;
    u32 i_mtime;
    u32 i_dtime;
    u16 i_gid;
    u16 i_links_count;
    u32 i_blocks;
    u32 i_flags;
    u32 osd1;
    u32 i_block[15];
    u32 i_generation;
    u32 i_file_acl;
    u32 i_dir_acl;
    u32 i_faddr;
    u32 osd2[3];
};

// 以下内容改自Minix
// output子系统信息结构体, 信息类型为OUTPUT_SYSTEM (0)
struct OUTPUT_MESSAGE {
    char function;     // 执行的功能 --> 显示字符==0 特殊功能==1
    CONSOLE* console;  // 要输出的控制台指针
    u32 pid;  // 发送进程的pid,用于确定要显示字符的内存地址
    char* data;      // 要显示字符的指针
    char disp_func;  // 执行的具体功能
};
// input子系统信息
struct INPUT_MESSAGE {
    int input_source;  // input输入源
    KEYMAP_RESULT keyboard_result;
};
// 硬盘操作信息
struct DISK_MESSAGE {
    u8 function;      // 执行的操作类型
    u32 pid;          // 信息来源进程
    char* buffer;     // 缓冲区指针
    u32 sector_head;  // 操作的起始位置
    int bytes_count;  //读取的字节数
    u8 result;        // 磁盘操作的结果
};
struct FS_MESSAGE {
    u8 function;                 // 执行的操作类型
    u32 pid;                     // 信息来源 进程
    struct file_descriptor* fd;  // 文件描述符指针
    char* buffer;                // 数据缓冲区
    u32 count;                   // 读取的大小
    u8 result;
};

typedef struct mess {
    int source;
    int type;
    union {
        struct OUTPUT_MESSAGE output_message;
        struct INPUT_MESSAGE input_message;
        struct DISK_MESSAGE disk_message;
        struct FS_MESSAGE fs_message;
    } u;
} MESSAGE;

#endif
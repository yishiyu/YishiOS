// 定义各种结构体
#ifndef YISHIOS_STRUCT_H
#define YISHIOS_STRUCT_H

#include "macro.h"
#include "type.h"

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
    char* directory_buffer;         // 当前目录缓冲区
    int directory_buffer_size;      //缓冲区大小
    FILE_DESCRIPTOR* directory_fd;  //目录文件描述符
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
    char function;      // 执行的功能 --> 显示字符==0 特殊功能==1
    int console_index;  // 要输出的控制台指针
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
    u8 result;                   //返回值结果
    char* file_name;             // 文件名字
};
struct MEM_MESSAGE {
    u8 function;        // 执行的操作类型
    u32 pid;            // 信息来源进程
    struct inode file;  // 目标文件
    // 创建子进程: -1: 创建失败  0~MAX_PRO_NUM : 子进程pid
    int result;  //返回值结果
};

typedef struct mess {
    int source;
    int type;
    union {
        struct OUTPUT_MESSAGE output_message;
        struct INPUT_MESSAGE input_message;
        struct DISK_MESSAGE disk_message;
        struct FS_MESSAGE fs_message;
        struct MEM_MESSAGE mem_message;
    } u;
} MESSAGE;

// 显存字符单元
typedef struct video_unit {
    char data;
    u8 color;
} VIDEO_UNIT;

#endif
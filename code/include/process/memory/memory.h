// 内存管理系统
// 原本的memory.h中是内存操作函数,改名为mem.h

#ifndef YISHIOS_MEMORY_H
#define YISHIOS_MEMORY_H

#include "global.h"
#include "struct.h"

// 入口函数
void mem_server();

// 功能函数
// 函数功能: 加载并运行可执行文件
// 输入参数: 可执行文件(elf格式)的inode, 父进程pid
// 返回值: 子进程pid
int execute(MESSAGE* message);
// 函数功能: 终止调用该功能的进程,并向其父进程发送退出信息
// 输入参数: 要退出的进程的pid(必须是当前进程)
void exit(MESSAGE* message);

// 子函数
void mem_init();
// 1. pcb的申请与释放
int get_pcb(PROCESS** proc);
void free_pcb(int pid);
// 2. 内存的申请与释放
int get_mem(u32* mem_ptr);
void free_mem();
// 3. elf文件的读取与重新放置
int read_elf(char* buffer, struct inode* elf_inode);
u32 replace_elf(char* buffer, u32 segment_base);
// 4. 进程的处理函数
void set_pcb(PROCESS* proc, char* name, u32 pid, u32 segment_base);
void shedule_pcb(PROCESS* proc);

// 关于内存管理的宏定义
// 内存管理提供的功能
#define MEM_EXECUTE 0x01
#define MEM_EXEIT 0x02

// MM_mem_bitmap的大小
#define MM_BITMAP_SIZE 64

// 内存块大小 --> 1MB
#define MM_BLOCK_SIZE 0x100000

// 堆栈在一块儿内存中的位置
// 堆栈段基址 - 代码段基址 距离代码段15 * 64kb处
#define MM_STACK_OFFSET 0x10000

#endif

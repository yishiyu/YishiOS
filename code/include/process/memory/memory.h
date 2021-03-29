// 内存管理系统
// 原本的memory.h中是内存操作函数,改名为mem.h

#ifndef YISHIOS_MEMORY_H
#define YISHIOS_MEMORY_H

#include "global.h"
#include "struct.h"
#include "type.h"

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

// 关于内存管理的宏定义
// 内存管理提供的功能
#define MEM_EXECUTE 0x01
#define MEM_EXEIT 0x02


#endif

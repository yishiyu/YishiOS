// 终端结构体(其实就是类)
#ifndef YISHIOS_TERMINALLIB_H
#define YISHIOS_TERMINALLIB_H

#include "macro.h"
#include "type.h"

// 仅在终端中使用的函数,不可被其他文件调用

//控制台结构体
typedef struct console {
	u32 current_start_addr;	// 当前控制台显存起始地址
	u32 original_addr;				// 当前控制台对应的显存位置
	u32 mem_limit;				//当前控制台显存大小
	u32 cursor;						// 光标

} CONSOLE;

// 终端结构体
typedef struct terminal {
    //待处理的命令缓冲区
    char in_buf[TTY_BUFFER_NUM];
    int in_head;
    int in_tail;
    int in_count;

	//控制台
	CONSOLE* console; 

} TERMINAL;

#endif

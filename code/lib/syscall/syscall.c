// 直接调用的系统调用
#include "syscall.h"
#include "display.h"

//#define __DEBUG_SYSCALL__

#ifndef __YISHIOS_DEBUG__
#define pause()
#define disp_int(str)
#define disp_str(str)
#else
#ifndef __DEBUG_SYSCALL__
#define pause()
#define disp_int(str)
#define disp_str(str)
#else
extern void pause();
#endif
#endif

//===============用户可以调用的系统调用==================
void sys_terminal_write(int console_index, char* data, int pid) {
    MESSAGE message;
    message.source = pid;
    message.type = SERVER_OUTPUT;
    message.u.output_message.console = &console_table[console_index];
    message.u.output_message.data = data;
    message.u.output_message.function = OUTPUT_MESSTYPE_DISP;
    message.u.output_message.pid = pid;
    asm_syscall(SYS_SENDREC, SEND, OUTPUT_SYSTEM, (u32)&message, (u32)pid);
}

int sys_sendrec(int function, int src_dest, MESSAGE* m, int pid) {
    return (int)asm_syscall(SYS_SENDREC, (u32)function, (u32)src_dest, (u32)m,
                            (u32)pid);
}

u32 sys_get_ticks() { return asm_syscall(SYS_GET_TICKS, 0, 0, 0, 0); }

//=================最终工作的函数======================

// 相关函数太多了,单独写在一个文件中 ---- ipc.c (inter process communication)
// u32 kernel_sendrec(int function, int src_dest, MESSAGE* m, int pid);

u32 kernel_get_ticks() { return ticks; }
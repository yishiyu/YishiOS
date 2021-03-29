// 直接调用的系统调用
#include "syscall.h"
#include "display.h"

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
    if (function == BOTH) {
        asm_syscall(SYS_SENDREC, (u32)function, (u32)src_dest, (u32)m,
                    (u32)pid);
        asm_syscall(SYS_SENDREC, (u32)function, (u32)src_dest, (u32)m,
                    (u32)pid);
    } else {
        return (int)asm_syscall(SYS_SENDREC, (u32)function, (u32)src_dest,
                                (u32)m, (u32)pid);
    }
}

u32 sys_get_ticks() { return asm_syscall(SYS_GET_TICKS, 0, 0, 0, 0); }

int sys_get_diskinfo(char* buffer, int count, int pid) {
    MESSAGE message;
    message.source = pid;
    message.type = DISK_SYSTEM;
    message.u.disk_message.function = DISK_INFO;
    message.u.disk_message.pid = pid;
    message.u.disk_message.buffer = buffer;
    message.u.disk_message.bytes_count = count;
    sys_sendrec(SEND, DISK_SYSTEM, &message, pid);
    sys_sendrec(RECEIVE, DISK_SYSTEM, &message, pid);
    return message.u.disk_message.result;
}

u32 sys_get_pid() { return asm_syscall(SYS_GET_PID, 0, 0, 0, 0); }

//=================最终工作的函数======================
// IPC相关函数单独放在一个文件中

u32 kernel_get_ticks() { return ticks; }
u32 kernel_get_pid() { return p_proc_ready_head->pid; }
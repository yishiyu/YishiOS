// 直接调用的系统调用
#include "syscall.h"
#include "display.h"

//===============用户可以调用的系统调用==================
// 显示一个字符串
void sys_terminal_write(int console_index, char* data, int pid) {
    MESSAGE message;
    message.source = pid;
    message.type = SERVER_OUTPUT;
    message.u.output_message.console_index = console_index;
    message.u.output_message.data = data;
    message.u.output_message.function = OUTPUT_MESSTYPE_DISP;
    message.u.output_message.pid = pid;
    asm_syscall(SYS_SENDREC, SEND, OUTPUT_SYSTEM, (u32)&message, (u32)pid);
}
// 屏幕下滚至清空
void sys_terminal_clear(int console_index, int pid) {
    MESSAGE message;
    message.source = pid;
    message.type = SERVER_OUTPUT;
    message.u.output_message.console_index = console_index;
    message.u.output_message.function = OUTPUT_MESSTYPE_FUNC;
    message.u.output_message.pid = pid;
    message.u.output_message.disp_func = OUTPUT_DISP_FUNC_CLEAR;
    asm_syscall(SYS_SENDREC, SEND, OUTPUT_SYSTEM, (u32)&message, (u32)pid);
}
// 根据一块内存刷新界面
void sys_terminal_draw(int console_index, char* data, int pid){
    MESSAGE message;
    message.source = pid;
    message.type = SERVER_OUTPUT;
    message.u.output_message.console_index = console_index;
    message.u.output_message.data = data;
    message.u.output_message.function = OUTPUT_MESSTYPE_FUNC;
    message.u.output_message.pid = pid;
    message.u.output_message.disp_func = OUTPUT_DISP_FUNC_DRAW;
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
    return 0;
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

int sys_set_timer(int pid, u32 time){
    return asm_syscall(SYS_SET_TIMER, pid, time, 0, 0);
}

//=================最终工作的函数======================
// IPC相关函数单独放在一个文件中

u32 kernel_get_ticks() { return ticks; }
u32 kernel_get_pid() { return p_proc_ready_head->pid; }
u32 kernel_set_timer(int pid, u32 time){
    // 检查时间是否为正
    if(time ==0) return 0;

    // 寻找是否还有空余的定时器
    for(int i=0;i<TASK_NUM;i++){
        if(timers[i].pid == NO_TASK){
            timers[i].pid = pid;
            timers[i].time = time;
            return 1;
        }
    }
    return 0;
}
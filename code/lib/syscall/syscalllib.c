// 工具函数库
#include "syscall.h"

// 计算给定进程某个段的线性地址
int ldt_seg_linear(int pid, int seg_index) {
    DESCRIPTOR* des = &(PCB_stack[pid].ldts[seg_index]);
    return des->base_high << 24 | des->base_mid << 16 | des->base_low;
}

// 把虚拟地址转化成线性地址
// 函数默认传进来的参数是正确的
// virtual address to linear address
void* va2la(int pid, void* va) {
    u32 seg_base = ldt_seg_linear(pid, INDEX_LDT_RW);
    u32 la = seg_base + (u32)va;
    return (void*)la;
}
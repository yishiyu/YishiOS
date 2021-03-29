#include "proc.h"

// 本文件调试开关
// #define __DEBUG_PROC__

// 逻辑关系: 任何一个没定义,就消除函数定义
#ifndef __YISHIOS_DEBUG__
#define pause()
#define disp_int(str)
#define disp_str(str)
#else
#ifndef __DEBUG_PROC__
#define pause()
#define disp_int(str)
#define disp_str(str)
#endif
#endif

void restart();

// 初始化所有系统任务,终端任务,同时在就绪队列中设置一个指针指向第一个终端
int start_proc() {
    disp_clear_screen();

    // ======================初始化系统进程==================
    // 指向系统为初始进程留下的栈空间
    // 加上栈总长是为了使得该指针指向栈顶(最高地址)
    char* p_task_stack = task_stack + BASE_TASKS_STACK_SIZE;
    u16 selector_ldt = SELECTOR_LDT_FIRST;

    // 用于暂存上一个节点的指针
    // 临时指针
    PROCESS* pre_proc;

    // 有一个非常重要的地方,使用进程队列的时候一定要规范地赋值指针,否则会给后面埋不少坑

    //-----------------------------------初始化就绪队列--------------------------------
    TASK* p_task = task_table;
    PROCESS* temp_proc;

    for (int i = 0; i < TASK_NUM; i++) {
        // 不需要从空pcb栈中获取的空pcb了,直接使用系统预留的pid及对应的pcb初始化
        u32 PID = p_task[i].pid;
        temp_proc = &PCB_stack[PID];
        PCB_stack_status[PID] = 1;

        // 初始化PCB
        init_pcb(&p_task[i], temp_proc, PID, p_task_stack, selector_ldt);

        // 初始化就绪队列
        if (i == 0) {
            p_proc_ready_head = temp_proc;
            pre_proc = temp_proc;
        } else {
            pre_proc->next_pcb = temp_proc;
            temp_proc->pre_pcb = pre_proc;
            pre_proc = temp_proc;
        }

        // 从内核栈中为初始进程分配堆
        p_task_stack -= p_task[i].stacksize;
        // 指向GDT中下一个空描述符
        selector_ldt += 1 << 3;
        // 修改PID
        PID++;
    }
    pre_proc->next_pcb = &p_proc_ready_tail;
    p_proc_ready_tail.pre_pcb = pre_proc;

    //-----------------------------------初始化挂起队列和阻塞队列------------------------------
    p_proc_pause_head = &p_proc_pause_tail;
    p_proc_wait_head = &p_proc_wait_tail;

    //------------------------------------------初始化空进程--------------------------------------
    init_pcb(&empty_task, &PCB_empty_task, PID_EMTPY_TASK, p_task_stack,
             selector_ldt);
    PCB_empty_task.next_pcb = &p_proc_ready_tail;

    //===============系统初始化完成,进入系统进程=================
    k_reenter = 0;
    ticks = 0;
    t_present_terminal = 0;

    // 之所以在启动进程之前进行中断的初始化
    // 是为了防止在设置其他内容的时候发生中断
    init_IRQ();

    // 操作系统初始化完毕,进入初始进程中
    restart();
}

// 根据一个预定义的任务初始化一个任务级进程的pcb
// 同时初始化全局描述符表中的相关描述符
void init_pcb(TASK* task, PROCESS* proc, u32 pid, char* stack,
              u16 selector_ldt) {
    //初始化名字, id
    memcpy(&proc->p_name, &task->name, 16);
    proc->pid = pid;

    //初始化ldt选择子和局部描述符表
    proc->ldt_sel = selector_ldt;
    memcpy(&proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
    proc->ldts[0].attr1 = DESEC_ATTR_CODE_E | PRIVILEGE_TASK << 5;
    memcpy(&proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
    proc->ldts[1].attr1 = DESEC_ATTR_DATA_RW | PRIVILEGE_TASK << 5;

    //填充GDT中的LDT描述符
    //描述符地址, 段起始地址, 段界限, 段属性
    init_descriptor(&gdt[selector_ldt >> 3],
                    vir2phys(seg2phys(SELECTOR_KERNEL_DS), proc->ldts),
                    LDT_SIZE * sizeof(DESCRIPTOR) - 1, DESEC_ATTR_LDT);

    // 初始化PCB中记录的进程的运行状态
    // 选择子低三位为属性,第四位开始为偏移为,单位为选择子长度
    // 使用的是内核的代码段和数据段, 但是请求优先级为任务级
    proc->regs.cs = ((8 * 0) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                    SELEC_TI_LOCAL | RPL_TASK;
    proc->regs.ds = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                    SELEC_TI_LOCAL | RPL_TASK;
    proc->regs.es = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                    SELEC_TI_LOCAL | RPL_TASK;
    proc->regs.fs = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                    SELEC_TI_LOCAL | RPL_TASK;
    proc->regs.ss = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                    SELEC_TI_LOCAL | RPL_TASK;
    proc->regs.gs = (SELECTOR_KERNEL_GS & SELEC_ATTR_RPL_MASK) | RPL_TASK;

    // 设置进程当前运行的PC寄存器和栈寄存器
    // 之所以没有堆,是因为操作系统还没有内存管理功能...
    proc->regs.eip = (u32)task->initial_eip;
    proc->regs.esp = (u32)stack;
    // IF=1, IOPL=1
    // 允许中断,IO优先级为2,也就是说只允许内核级和任务级进程进行I/O操作
    proc->regs.eflags = 0x1202;
    proc->priority = proc->ticks = task->priority;

    // 初始化IPC相关的标志位
    proc->flags = RUNNING;
    proc->has_int_msg = 0;
    proc->message = 0;
    proc->recv_from = NO_TASK;
    proc->recv_from = NO_TASK;
    proc->sending_to_this = 0;
    proc->next_sending = 0;
}



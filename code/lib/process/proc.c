
#include "proc.h"

void restart();

// 初始化所有系统任务,终端任务,同时在就绪队列中设置一个指针指向第一个终端
int start_proc() {
    disp_clear_screen();
    // ======================初始化系统进程==================
    // 指向系统为初始进程留下的栈空间
    // 加上栈总长是为了使得该指针指向栈顶(最高地址)
    char* p_task_stack = task_stack + BASE_TASKS_STACK_SIZE;
    u16 selector_ldt = SELECTOR_LDT_FIRST;

    // 进程编号
    u32 PID = 0;

    // ------------------------------------初始化普通进程-------------------------------------
    //系统初始进程相关结构体
    TASK* p_task = task_table;
    PROCESS* p_proc = proc_table;

    for (int i = 0; i < BASE_TASKS_NUM; i++) {
        init_pcb(&p_task[i], &p_proc[i], PID, p_task_stack, selector_ldt);

        // 从内核栈中为初始进程分配堆
        p_task_stack -= p_task[i].stacksize;
        // 指向GDT中下一个空描述符
        selector_ldt += 1 << 3;
        // 修改PID
        PID++;
    }

    // -------------------------------------初始化终端进程-----------------------------------
    TASK* t_task = tty_task_table;
    PROCESS* t_proc = terminal_table;

    for (int i = 0; i < TERMINAL_NUM; i++) {
        init_pcb(&t_task[i], &t_proc[i], PID, p_task_stack, selector_ldt);

        // 从内核栈中为初始进程分配堆
        p_task_stack -= p_task[i].stacksize;
        // 指向GDT中下一个空描述符
        selector_ldt += 1 << 3;
        // 修改PID
        PID++;
    }

    //===============系统初始化完成,进入系统进程=================
    k_reenter = 0;
    ticks = 0;
    p_proc_ready_head = proc_table;
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
}

// 从预定义的PCB中取得一个空节点,如果没有多余节点,直接触发系统错误
PROCESS get_pcb() {
    // 有多余的pcb块
    if (PCB_stack_top < MAX_PROCESS_NUM) {
        return PCB_stack[PCB_stack_top++];
    } else {
        // 触发错误
        while (1)
            ;
    }
}
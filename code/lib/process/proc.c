
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

    // ------------------------------------初始化进程-------------------------------------
    //系统初始进程相关结构体
    TASK* p_task = task_table;
    PROCESS* p_proc = proc_table;

    //根据每个系统初始进程状态初始化系统PCB块
    for (int i = 0; i < BASE_TASKS_NUM; i++) {
        //初始化名字, id
        memcpy(&p_proc->p_name, &p_task->name, 16);
        p_proc->pid = i;

        //初始化ldt选择子和局部描述符表
        p_proc->ldt_sel = selector_ldt;
        memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
               sizeof(DESCRIPTOR));
        p_proc->ldts[0].attr1 = DESEC_ATTR_CODE_E | PRIVILEGE_TASK << 5;
        memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
               sizeof(DESCRIPTOR));
        p_proc->ldts[1].attr1 = DESEC_ATTR_DATA_RW | PRIVILEGE_TASK << 5;

        // 把系统任务的优先级提升为内核级
        // p_proc->ldt_sel = selector_ldt;
        // memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
        //        sizeof(DESCRIPTOR));
        // p_proc->ldts[0].attr1 = DESEC_ATTR_CODE_E | PRIVILEGE_KRNL << 5;
        // memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
        //        sizeof(DESCRIPTOR));
        // p_proc->ldts[1].attr1 = DESEC_ATTR_DATA_RW | PRIVILEGE_KRNL << 5;

        //填充GDT中的LDT描述符
        //描述符地址, 段起始地址, 段界限, 段属性
        init_descriptor(&gdt[selector_ldt >> 3],
                        vir2phys(seg2phys(SELECTOR_KERNEL_DS), p_proc->ldts),
                        LDT_SIZE * sizeof(DESCRIPTOR) - 1, DESEC_ATTR_LDT);

        // 初始化PCB中记录的进程的运行状态
        // 选择子低三位为属性,第四位开始为偏移为,单位为选择子长度
        // 使用的是内核的代码段和数据段, 但是请求优先级为任务级
        p_proc->regs.cs = ((8 * 0) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                          SELEC_TI_LOCAL | RPL_TASK;
        p_proc->regs.ds = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                          SELEC_TI_LOCAL | RPL_TASK;
        p_proc->regs.es = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                          SELEC_TI_LOCAL | RPL_TASK;
        p_proc->regs.fs = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                          SELEC_TI_LOCAL | RPL_TASK;
        p_proc->regs.ss = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                          SELEC_TI_LOCAL | RPL_TASK;
        p_proc->regs.gs = (SELECTOR_KERNEL_GS & SELEC_ATTR_RPL_MASK) | RPL_TASK;

        //为了使得键盘服务器可以切换终端,把其提升为内核级
        // p_proc->regs.cs = ((8 * 0) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
        //                   SELEC_TI_LOCAL | RPL_KRNL;
        // p_proc->regs.ds = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
        //                   SELEC_TI_LOCAL | RPL_KRNL;
        // p_proc->regs.es = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
        //                   SELEC_TI_LOCAL | RPL_KRNL;
        // p_proc->regs.fs = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
        //                   SELEC_TI_LOCAL | RPL_KRNL;
        // p_proc->regs.ss = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
        //                   SELEC_TI_LOCAL | RPL_KRNL;
        // p_proc->regs.gs = (SELECTOR_KERNEL_GS & SELEC_ATTR_RPL_MASK) |
        // RPL_TASK;

        // 设置进程当前运行的PC寄存器和栈寄存器
        // 之所以没有堆,是因为操作系统还没有内存管理功能...
        p_proc->regs.eip = (u32)p_task->initial_eip;
        p_proc->regs.esp = (u32)p_task_stack;
        // IF=1, IOPL=1
        // 允许中断,IO优先级为2,也就是说只允许内核级和任务级进程进行I/O操作
        p_proc->regs.eflags = 0x1202;

        // 从内核栈中为初始进程分配堆
        p_task_stack -= p_task->stacksize;
        p_proc++;
        p_task++;
        // 指向GDT中下一个空描述符
        selector_ldt += 1 << 3;
    }
    // 为进程设置优先级
    proc_table[0].ticks = proc_table[0].priority = PRIORITY_KEYBOARD_SERVER;

    // -------------------------------------初始化终端-----------------------------------
    TASK* t_task = tty_task_table;
    PROCESS* t_proc = terminal_table;

    for (int i = 0; i < TERMINAL_NUM; i++) {
        //初始化名字, id
        memcpy(&t_proc->p_name, &t_task->name, 16);
        t_proc->pid = BASE_TASKS_NUM + i;

        //初始化ldt选择子和局部描述符表
        t_proc->ldt_sel = selector_ldt;
        memcpy(&t_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
               sizeof(DESCRIPTOR));
        t_proc->ldts[0].attr1 = DESEC_ATTR_CODE_E | PRIVILEGE_TASK << 5;
        memcpy(&t_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
               sizeof(DESCRIPTOR));
        t_proc->ldts[1].attr1 = DESEC_ATTR_DATA_RW | PRIVILEGE_TASK << 5;

        //填充GDT中的LDT描述符
        //描述符地址, 段起始地址, 段界限, 段属性
        init_descriptor(&gdt[selector_ldt >> 3],
                        vir2phys(seg2phys(SELECTOR_KERNEL_DS), t_proc->ldts),
                        LDT_SIZE * sizeof(DESCRIPTOR) - 1, DESEC_ATTR_LDT);

        // 初始化PCB中记录的进程的运行状态
        // 选择子低三位为属性,第四位开始为偏移为,单位为选择子长度
        // 使用的是内核的代码段和数据段, 但是请求优先级为任务级
        t_proc->regs.cs = ((8 * 0) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                          SELEC_TI_LOCAL | RPL_TASK;
        t_proc->regs.ds = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                          SELEC_TI_LOCAL | RPL_TASK;
        t_proc->regs.es = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                          SELEC_TI_LOCAL | RPL_TASK;
        t_proc->regs.fs = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                          SELEC_TI_LOCAL | RPL_TASK;
        t_proc->regs.ss = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                          SELEC_TI_LOCAL | RPL_TASK;
        t_proc->regs.gs = (SELECTOR_KERNEL_GS & SELEC_ATTR_RPL_MASK) | RPL_TASK;

        // 设置进程当前运行的PC寄存器和栈寄存器
        // 之所以没有堆,是因为操作系统还没有内存管理功能...
        // 接着之前进程的栈继续往下
        t_proc->regs.eip = (u32)t_task->initial_eip;
        t_proc->regs.esp = (u32)p_task_stack;
        // IF=1, IOPL=1
        // 允许中断,IO优先级为2,也就是说只允许内核级和任务级进程进行I/O操作
        t_proc->regs.eflags = 0x1202;

        // 为终端设置优先级
        t_proc->ticks = t_proc->priority = PRIORITY_TERMINAL;

        // 从内核栈中为初始进程分配堆
        p_task_stack -= t_task->stacksize;
        t_proc++;
        t_task++;
        // 指向GDT中下一个空描述符
        selector_ldt += 1 << 3;
    }

    //===============系统初始化完成,进入系统进程=================
    k_reenter = 0;
    ticks = 0;
    p_proc_ready = proc_table;
    t_present_tty = terminal_table;

    // 之所以在启动进程之前进行中断的初始化
    // 是为了防止在设置其他内容的时候发生中断
    init_IRQ();

    // 操作系统初始化完毕,进入初始进程中
    restart();
}

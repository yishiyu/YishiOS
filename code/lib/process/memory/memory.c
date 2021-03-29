// 文件管理系统
#include "memory.h"
#include "global.h"
#include "proc.h"
#include "struct.h"

// 内存系统缓冲区放在10mb~15mb(共6MB)
static char* MM_buffer = (char*)0xa00000;
// 内存分配bitmap,每个bit代表1mb的内存
// 前16个bit都被初始化成1,前16mb的空间被系统占用
static u8 MM_mem_bitmap[MM_BITMAP_SIZE];

// 内存管理入口
void mem_server() {
    MESSAGE message;

    mem_init();

    memset(&message, 0, sizeof(message));

    while (1) {
        sys_sendrec(RECEIVE, ANY, &message, PID_MEM_SERVER);
        // 对收到的信息进行判断,排除中断信息(否则会出现试图向中断发送信息的情况)
        if ((message.source < 0) || (message.source >= MAX_PROCESS_NUM))
            continue;

        int src = message.source;
        int result = 0;

        switch (message.u.fs_message.function) {
            case MEM_EXECUTE:
                // 运行一个程序
                result = execute(&message);
                break;

            // case MEM_EXEIT:
            //     // 运行一个程序
            //     break;
            default:
                break;
        }
        // message.source = FILE_SYSTEM;
        // message.type = SERVER_FS;
        // message.u.disk_message.result = result;
        // sys_sendrec(SEND, src, &message, PID_FS_SERVER);
        // memset(&message, 0, sizeof(message));
    }
}

// 初始化函数
void mem_init() {
    // 1. 初始化内存 bitmap
    memset((void*)&MM_mem_bitmap, 0, MM_BITMAP_SIZE);
    MM_mem_bitmap[0] = 0xff;
    MM_mem_bitmap[1] = 0xff;
}

// 内存管理功能函数
// -1 : 无空白PCB
// -2 : 无空闲内存
int execute(MESSAGE* message) {
    int result = 0;
    // 1. 申请一个空PCB并初始化
    PROCESS* temp_proc;
    u32 pid = get_pcb(&temp_proc);
    // 如果无空白PCB,则返回
    if (pid < 0) return -1;

    // 2. 申请一块空内存
    u32 mem_ptr = 0;
    result = get_mem(&mem_ptr);
    if (result == 1) return -2;

    // 3. 连接空白PCB与空内存
    set_pcb(temp_proc, "user_proc", pid, mem_ptr);

    // 4. 读取文件到缓冲区
    read_elf(MM_buffer,
             va2la(message->u.mem_message.pid, message->u.mem_message.file));

    // 5. 根据ELF重新放置文件,设置eip寄存器
    u32 entry = replace_elf(MM_buffer,mem_ptr);
    temp_proc->regs.eip = entry;

    // 6. 把PCB加入挂起队列
    shedule_pcb(temp_proc);

    // 7. 返回子进程pid
    return pid;
}
void exit(MESSAGE* message) {}

#pragma region PCB的申请与释放
// 从预定义的PCB中取得一个空节点
// 如果没有多余节点,直接触发系统错误
int get_pcb(PROCESS** proc) {
    // 有多余的pcb块
    if (PCB_USED < MAX_PROCESS_NUM) {
        // 寻找可用的pid(即空白 PCB)
        for (int pid = 0; pid < MAX_PROCESS_NUM; pid++) {
            if (PCB_stack_status[pid] == 0) {
                PCB_stack_status[pid] = 1;
                PCB_USED++;
                *proc = &PCB_stack[pid];
                return pid;
            }
        }
    }
    // 触发错误
    return -1;
}

// 释放一个PCB
void free_pcb(int pid) {
    // 因为释放PCB需要操作三个进程队列
    // 而时钟中断,硬盘中断,键盘中断都能唤醒一个进程
    // 所以在释放一个pcb的时候不能受中断的影响
    disable_int();
    // 把原本的PCB从三个队列中释放出来
    // 1. 先检查PCB是不是各个队列的首个节点
    // 1.1 就绪队列头,这个不可能,因为当前运行的一定是MM进程哈哈哈哈
    //       但是为了对称好看一点,把这个也加上
    if (p_proc_ready_head == &PCB_stack[pid]) {
        p_proc_ready_head = PCB_stack[pid].next_pcb;
    } else if (p_proc_wait_head == &PCB_stack[pid]) {
        p_proc_wait_head = PCB_stack[pid].next_pcb;
    } else if (p_proc_pause_head == &PCB_stack[pid]) {
        p_proc_wait_head = PCB_stack[pid].next_pcb;
    }
    // 2. 如果不是首节点,则直接把前一个节点和后一个节点连接起来就行了
    else {
        PCB_stack[pid].next_pcb->pre_pcb = PCB_stack[pid].pre_pcb;
        PCB_stack[pid].pre_pcb->next_pcb = PCB_stack[pid].next_pcb;
    }

    // 在PCB栈中释放PCB
    PCB_stack_status[pid] = 0;
    PCB_USED--;
    enable_int();
}
#pragma endregion

#pragma region n内存的申请与释放

// 申请一块儿空内存
int get_mem(u32* mem_ptr) {
    // 1. 寻找空白内存
    for (int i = 0; i < MM_BITMAP_SIZE; i++) {
        if (MM_mem_bitmap[i] != 0xff) {
            // 2. 查看是那块内存空闲
            for (int j = 0; j < 8; j++) {
                if (~((1 << j) & MM_mem_bitmap[i])) {
                    MM_mem_bitmap[i] |= (1 << j);
                    // 3. 计算内存地址
                    *mem_ptr = (u32)(MM_BLOCK_SIZE * (i * 8 + j));
                    // 4. 返回
                    return 1;
                }
            }
        }
    }
    return 0;
}
void free_mem() {}

#pragma endregion

#pragma region 文件的读取与重新放置

// 读取ELF文件到缓冲区
int read_elf(char* buffer, struct inode* elf_inode) {
    // 请求文件系统的服务
    MESSAGE message;
    FILE_DESCRIPTOR file_fd;
    file_fd.fd_pos = 0;
    file_fd.fd_inode = elf_inode;
    message.source = PID_MEM_SERVER;
    message.type = PID_MEM_SERVER;
    message.u.fs_message.pid = PID_MEM_SERVER;
    message.u.fs_message.buffer = MM_buffer;
    message.u.fs_message.count = MM_BLOCK_SIZE;
    message.u.fs_message.fd = &file_fd;
    message.u.fs_message.function = FS_READ;
    sys_sendrec(SEND, SERVER_FS, &message, PID_MEM_SERVER);
    sys_sendrec(RECEIVE, SERVER_FS, &message, PID_MEM_SERVER);
}

// 重新放置ELF文件
// 返回值是程序入口地址
u32 replace_elf(char* buffer, u32 segment_base) {
    Elf32_header* elf_header = (Elf32_header*)(buffer);
    int i;
    // 依次放置每一个代码段
    for (i = 0; i < elf_header->e_phnum; i++) {
        // 计算出段表头的位置
        Elf32_Phdr* prog_header = (Elf32_Phdr*)(buffer + elf_header->e_phoff +
                                                (i * elf_header->e_phentsize));
        // 如果该段属于可加载段,则放置到内存指定位置
        if (prog_header->p_type == PT_LOAD) {
            phys_copy(
                (void*)(segment_base + prog_header->p_vaddr),
                (void*)va2la(PID_MEM_SERVER, buffer + prog_header->p_offset),
                prog_header->p_filesz);
        }
    }
    return elf_header->e_entry;
}

#pragma endregion

#pragma region 进程的处理函数

// 初始化一个用户级进程
// 输入参数:
// PCB指针
// 进程名指针, 最长16个字符
// 进程PID (同时用来确定选择子位置)
// 代码段和数据段基址(默认分配空间为1MB, 堆栈段基址为1mb空间最后128kb)
void set_pcb(PROCESS* proc, char* name, u32 pid, u32 segment_base) {
    // 1. 初始化名字, id
    memcpy(&proc->p_name, name, 16);
    proc->pid = pid;
    // 2. 计算选择子位置和堆栈位置
    u16 selector_ldt = SELECTOR_LDT_FIRST + ((u16)pid << 3);
    u32 segment_stack = segment_base + MM_STACK_OFFSET;

    // 3. 初始化ldt选择子和局部描述符表
    proc->ldt_sel = selector_ldt;
    init_descriptor(&proc->ldts[0], segment_base, MM_BLOCK_SIZE >> 3,
                    DESEC_ATTR_CODE_E | DESEC_ATTR_32 | DESEC_ATTR_LIMIT_4K |
                        PRIVILEGE_USER);
    init_descriptor(&proc->ldts[1], segment_base, MM_BLOCK_SIZE >> 3,
                    DESEC_ATTR_DATA_RW | DESEC_ATTR_32 | DESEC_ATTR_LIMIT_4K |
                        PRIVILEGE_USER);

    // 4. 填充GDT中的LDT描述符
    //描述符地址, 段起始地址, 段界限, 段属性
    // 这里使用SELECTOR_KERNEL_DS的原因是,PCB依然属于内核范围,基址当然是内核代码段基址了
    init_descriptor(&gdt[selector_ldt >> 3],
                    vir2phys(seg2phys(SELECTOR_KERNEL_DS), proc->ldts),
                    LDT_SIZE * sizeof(DESCRIPTOR) - 1, DESEC_ATTR_LDT);

    // 5. 初始化PCB中记录的进程的运行状态
    // 选择子低三位为属性,第四位开始为偏移为,单位为选择子长度
    // 使用的是内核的代码段和数据段, 但是请求优先级为任务级
    proc->regs.cs = ((8 * 0) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                    SELEC_TI_LOCAL | RPL_USER;
    proc->regs.ds = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                    SELEC_TI_LOCAL | RPL_USER;
    proc->regs.es = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                    SELEC_TI_LOCAL | RPL_USER;
    proc->regs.fs = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                    SELEC_TI_LOCAL | RPL_USER;
    proc->regs.ss = ((8 * 1) & SELEC_ATTR_RPL_MASK & SELEC_TI_MASK) |
                    SELEC_TI_LOCAL | RPL_USER;
    proc->regs.gs = (SELECTOR_KERNEL_GS & SELEC_ATTR_RPL_MASK) | RPL_USER;

    // 6. 设置进程当前运行的PC寄存器和栈寄存器
    // 之所以没有堆,是因为操作系统还没有内存管理功能...
    proc->regs.eip = (u32)0;
    proc->regs.esp = segment_stack;
    // IF=1, IOPL=1
    // 允许中断,IO优先级为2,也就是说只允许内核级和任务级进程进行I/O操作
    proc->regs.eflags = 0x1202;
    proc->priority = proc->ticks = PRIORITY_USER;

    // 初始化IPC相关的标志位
    proc->flags = RUNNING;
    proc->has_int_msg = 0;
    proc->message = 0;
    proc->recv_from = NO_TASK;
    proc->recv_from = NO_TASK;
    proc->sending_to_this = 0;
    proc->next_sending = 0;
}

// 把PCB加入挂起队列
void shedule_pcb(PROCESS* proc) {
    // 为防止中断影响,屏蔽中断
    disable_int();
    // 1. 挂起队列为空
    if (p_proc_pause_head == &p_proc_pause_tail) {
        p_proc_pause_head = proc;
        proc->next_pcb = &p_proc_pause_tail;
        p_proc_pause_tail.pre_pcb = proc;
    } else {
        proc->pre_pcb = p_proc_pause_tail.pre_pcb;
        proc->next_pcb = &p_proc_pause_tail;
        p_proc_pause_tail.pre_pcb->next_pcb = proc;
        p_proc_pause_tail.pre_pcb = proc;
    }
    enable_int();
}

#pragma endregion
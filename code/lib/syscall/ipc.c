// 只实现一个系统调用--进程间通信机制
// 很多内容是从Orange中借鉴的 (Orange又是从Minix中借鉴的哈哈哈哈)
#include "syscall.h"

// #define __DEBUG_IPC__

#ifndef __YISHIOS_DEBUG__
#define pause()
#define disp_int(str)
#define disp_str(str)
#else
#ifndef __DEBUG_IPC__
#define pause()
#define disp_int(str)
#define disp_str(str)
#else
extern void pause();
#endif
#endif

// 子函数
void* va2la(int pid, void* va);
int ldt_seg_linear(int pid, int seg_index);
void reset_msg(MESSAGE* p) { memset(p, 0, sizeof(MESSAGE)); }
int deadlock(int src, int dest);
int block(int pid);
int unblock(int pid);
int msg_send(int current, int dest, MESSAGE* m);
int msg_receive(int current, int src, MESSAGE* m);

// 进程间通信系统调用 入口
// function --> 功能号
// src_dest --> 目标源的pid
// m             --> 消息体
// pid           --> 调用者的pid
u32 kernel_sendrec(int function, int src_dest, MESSAGE* m, int pid) {
    disp_str("point ipc.c kernel_sendrec a,pid == ");
    disp_int(pid);
    disp_str(" src_dest == ");
    disp_int(src_dest);
    disp_str("\n");
    pause();
    // 根据传进来参数的相对地址得到在内存中的绝对地址
    MESSAGE* mla = (MESSAGE*)va2la(pid, m);
    mla->source = pid;

    // 暂存返回值
    int ret = 0;

    disp_str("point ipc.c kernel_sendrec 0\n");
    pause();

    // function除了SEND,RECEIVE之外还有BOTH
    // 这个在用户空间通过调用两次系统调用实现
    if (function == SEND) {
        disp_str("point ipc.c kernel_sendrec 1\n");
        pause();

        ret = msg_send(pid, src_dest, m);
        if (ret != 0) return ret;
    } else if (function == RECEIVE) {
        disp_str("point ipc.c kernel_sendrec 2\n");
        pause();

        ret = msg_receive(pid, src_dest, m);
        if (ret != 0) return ret;
    } else {
        disp_str("point 0 ipc.c kernel_sendrec 3\n");
        pause();
    }

    return 0;
}

// 进程间通信系统调用  发送信息函数
// 成功发送之后返回 0
int msg_send(int current, int dest, MESSAGE* m) {
    disp_str("point ipc.c msg_send a,current == ");
    disp_int(current);
    disp_str(" dest == ");
    disp_int(dest);
    disp_str("\n");
    pause();

    // 获取发送方和接收方的PCB
    PROCESS* sender = &PCB_stack[current];
    PROCESS* destiny = &PCB_stack[dest];

    disp_str("point ipc.c msg_send 0\n");
    pause();

    // 检查死锁, 只需要对发送死锁进行检测即可
    if (deadlock(current, dest) == SEND_DEADLOCK) {
        disp_str("point ipc.c msg_send 1\n");
        pause();
    }

    // 1. 目标等待接收此进程的信息
    if ((destiny->flags & RECEIVING) &&
        (destiny->recv_from == current || destiny->recv_from == ANY)) {
        // 1.1 把消息送给目标
        // 相当于在目标进程内部产生返回值
        phys_copy(va2la(dest, destiny->message), va2la(current, m),
                  sizeof(MESSAGE));

        // 1.2 把目标从阻塞状态唤醒
        destiny->message = 0;
        destiny->flags &= ~RECEIVING; /* dest has received the msg */
        destiny->recv_from = NO_TASK;
        unblock(dest);

        // 2. 目标并非在等待此进程的消息
    } else {
        // 2.1 设置发送者的状态(阻塞状态)
        sender->flags |= SENDING;
        sender->send_to = dest;
        sender->message = m;

        // 2.2 把发送者挂在接受者的消息链表上
        PROCESS* p;
        if (destiny->sending_to_this) {
            p = destiny->sending_to_this;
            while (p->sending_to_this) p = (p->sending_to_this);
            p->sending_to_this = sender;
        } else {
            destiny->sending_to_this = sender;
        }
        sender->next_sending = 0;

        // 2.3 阻塞当前进程
        block(current);
    }

    return 0;
}

// 进程间通信系统调用 接收信息函数
// 成功后返回 0
int msg_receive(int current, int src, MESSAGE* m) {
    disp_str("point ipc.c msg_receive 0\n");
    pause();
    PROCESS* p_who_wanna_recv = &PCB_stack[current];

    PROCESS* p_from = 0;
    PROCESS* prev = 0;
    int copyok = 0;

    // 1. 有一个中断需要该进程处理,且该进程也准备好了
    if ((p_who_wanna_recv->has_int_msg) &&
        ((src == ANY) || (src == INTERRUPT))) {
        MESSAGE msg;
        reset_msg(&msg);
        msg.source = INTERRUPT;
        msg.type = HARD_INT;

        // 1.1 把该消息附在该进程上
        phys_copy(va2la(current, m), &msg, sizeof(MESSAGE));

        // 1.2 该中断消息已成功送达
        p_who_wanna_recv->has_int_msg = 0;

        return 0;
    }

    // 2. 没有中断发生,期待任意一个消息
    if (src == ANY) {
        // 2.1 存在待接收消息
        if (p_who_wanna_recv->sending_to_this) {
            p_from = p_who_wanna_recv->sending_to_this;
            copyok = 1;
        }
    }
    // 3. 没有中断发生,期待特定消息
    else if (src >= 0 && src < MAX_PROCESS_NUM) {
        // 3.2 获取目标源PCB
        p_from = &PCB_stack[src];

        // 3.3 存在期待的消息
        if ((p_from->flags & SENDING) && (p_from->send_to == current)) {
            copyok = 1;

            // 获取目标消息链表头
            PROCESS* p = p_who_wanna_recv->sending_to_this;

            // 搜索到目标消息
            while (p) {
                if (p->pid == src) /* if p is the one */
                    break;

                prev = p;
                p = p->next_sending;
            }
        }
    }

    // A 成功收到消息
    if (copyok) {
        // A.1 处理待接收消息列表
        if (p_from == p_who_wanna_recv->sending_to_this) {
            // 刚好是第一个
            p_who_wanna_recv->sending_to_this = p_from->next_sending;
            p_from->next_sending = 0;
        } else {
            // 不是第一个
            prev->next_sending = p_from->next_sending;
            p_from->next_sending = 0;
        }

        // A.2 把消息结构体复制过去
        phys_copy(va2la(current, m), va2la(p_from->pid, p_from->message),
                  sizeof(MESSAGE));

        // A.3 把目标源唤醒
        p_from->message = 0;
        p_from->send_to = NO_TASK;
        p_from->flags &= ~SENDING;

        unblock(p_from->pid);
    }

    // B 没有收到消息
    else {
        // B.1 设置自己的状态
        p_who_wanna_recv->flags |= RECEIVING;
        p_who_wanna_recv->message = m;
        p_who_wanna_recv->recv_from = src;

        // B.3 阻塞自己
        block(current);
    }

    return 0;
}

// 把虚拟地址转化成线性地址
// 函数默认传进来的参数是正确的
// virtual address to linear address
void* va2la(int pid, void* va) {
    u32 seg_base = ldt_seg_linear(pid, INDEX_LDT_RW);
    u32 la = seg_base + (u32)va;

    return (void*)la;
}

// 计算给定进程某个段的线性地址
int ldt_seg_linear(int pid, int seg_index) {
    DESCRIPTOR* des = &(PCB_stack[pid].ldts[seg_index]);
    return des->base_high << 24 | des->base_mid << 16 | des->base_low;
}

// 判断死锁的发生
// 非死锁的时候返回 0
// 发送请求死锁时返回 1
// 接收请求死锁时返回  2
int deadlock(int src, int dest) {
    PROCESS* p = &PCB_stack[dest];

    while (1) {
        // 该节点非空闲
        if (p->flags & SENDING) {
            // 发送目标为此任务的源目标,形成环路
            if (p->send_to == src) {
                disp_str("point ipc.c deadlock 1,sending deadlock\n");
                pause();
                return SEND_DEADLOCK;
            }
            p = &PCB_stack[p->send_to];
        } else {
            break;
        }
    }

    while (1) {
        // 该节点非空闲
        if (p->flags & RECEIVING) {
            // 发送目标为此任务的源目标,形成环路
            if (p->recv_from == src) {
                disp_str("point ipc.c deadlock 1,receiving deadlock\n");
                pause();
                return RECV_DEADLOCK;
            }
            p = &PCB_stack[p->recv_from];
        } else {
            break;
        }
    }

    return NO_DEADLOCK;
}

// 阻塞一个进程
// 基本方法是把一个进程(通常是当前进程)从就绪队列中提取出来,放到阻塞队列中
// pid --> 要锁定的进程号
int block(int pid) {
    disp_str("point ipc.c block 0,pid == ");
    disp_int(pid);
    disp_str("\n");
    pause();

    // 1. 把目标进程换到阻塞队列
    // 1.1 位于就绪队列第一个
    if (p_proc_ready_head->pid == pid) {
        disp_str("point ipc.c block 1 \n");
        pause();

        p_proc_ready_head = p_proc_ready_head->next_pcb;

        // 1.1.1 阻塞队列为空
        if (is_wait_empty) {
            disp_str("point ipc.c block 2\n");
            pause();
            p_proc_wait_head = &PCB_stack[pid];
            PCB_stack[pid].next_pcb = &p_proc_wait_tail;
            p_proc_wait_tail.pre_pcb = &PCB_stack[pid];
        }
        // 1.1.2 阻塞队列不为空
        else {
            disp_str("point ipc.c block 3\n");
            pause();
            (p_proc_wait_tail.pre_pcb)->next_pcb = &PCB_stack[pid];
            PCB_stack[pid].next_pcb = &p_proc_wait_tail;
            p_proc_wait_tail.pre_pcb = &PCB_stack[pid];
        }

    }
    // 1.2 不位于第一个,这个是不应该出现的情况
    else {
        disp_str("point ipc.c block 4\n");
        pause();
        while (1)
            ;
    }

    // 2. 调用调度函数
    disp_str("point ipc.c block 5\n");
    pause();
    schedule();
    disp_str("point ipc.c block 6\n");
    pause();

    return 0;
}

// 解锁一个进程
int unblock(int pid) {
    disp_str("point ipc.c unblock 0\n");
    pause();

    // 1. 把目标进程换到挂起队列
    // 1.1 位于阻塞队列第一个
    if (p_proc_wait_head->pid == pid) {
        disp_str("point ipc.c unblock 1\n");
        pause();
        p_proc_wait_head = p_proc_wait_head->next_pcb;

        // 1.1.1 阻塞队列为空
        if (is_pause_empty) {
            disp_str("point ipc.c unblock 2\n");
            pause();

            p_proc_pause_head = &PCB_stack[pid];
            PCB_stack[pid].next_pcb = &p_proc_pause_tail;
            p_proc_pause_tail.pre_pcb = &PCB_stack[pid];
        }
        // 1.1.2 阻塞队列不为空
        else {
            disp_str("point ipc.c unblock 3\n");
            pause();

            (p_proc_pause_tail.pre_pcb)->next_pcb = &PCB_stack[pid];
            PCB_stack[pid].next_pcb = &p_proc_pause_tail;
            p_proc_pause_tail.pre_pcb = &PCB_stack[pid];
        }

    }
    // 1.2 不位于第一个,说明阻塞队列不止一个PCB
    else {
        disp_str("point ipc.c unblock 4\n");
        pause();

        (PCB_stack[pid].pre_pcb)->next_pcb = PCB_stack[pid].next_pcb;
        (PCB_stack[pid].next_pcb)->pre_pcb = PCB_stack[pid].pre_pcb;

        // 1.1.1 阻塞队列为空
        if (is_pause_empty) {
            disp_str("point ipc.c unblock 5\n");
            pause();
            p_proc_pause_head = &PCB_stack[pid];
            PCB_stack[pid].next_pcb = &p_proc_pause_tail;
            p_proc_pause_tail.pre_pcb = &PCB_stack[pid];
        }
        // 1.1.2 阻塞队列不为空
        else {
            disp_str("point ipc.c unblock 6\n");
            pause();
            (p_proc_pause_tail.pre_pcb)->next_pcb = &PCB_stack[pid];
            PCB_stack[pid].next_pcb = &p_proc_pause_tail;
            p_proc_pause_tail.pre_pcb = &PCB_stack[pid];
        }
    }

    disp_str("point ipc.c unblock 7\n");
    pause();
    // 2. 调用调度函数
    schedule();
    disp_str("point ipc.c unblock 8\n");
    pause();
    return 0;
}

// 只实现一个系统调用--进程间通信机制
// 很多内容是从Orange中借鉴的 (Orange又是从Minix中借鉴的哈哈哈哈)
#include "syscall.h"

// 子函数
extern int ldt_seg_linear(int pid, int seg_index);
extern void* va2la(int pid, void* va);
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
    // 根据传进来参数的相对地址得到在内存中的绝对地址
    MESSAGE* mla = (MESSAGE*)va2la(pid, m);
    mla->source = pid;

    // 暂存返回值
    int ret = 0;
    // function除了SEND,RECEIVE之外还有BOTH
    // 这个在用户空间通过调用两次系统调用实现
    if (function == SEND) {
        ret = msg_send(pid, src_dest, mla);
        if (ret != 0) return ret;
    } else if (function == RECEIVE) {
        ret = msg_receive(pid, src_dest, mla);
        if (ret != 0) return ret;
    } else {
        while (1)
            ;
    }

    return 0;
}

// 进程间通信系统调用  发送信息函数
// 成功发送之后返回 0
int msg_send(int current, int dest, MESSAGE* m) {
    // 获取发送方和接收方的PCB
    PROCESS* sender = &PCB_stack[current];
    PROCESS* destiny = &PCB_stack[dest];

    // 检查死锁, 只需要对发送死锁进行检测即可
    if (deadlock(current, dest) == SEND_DEADLOCK) {
        disp_str("point ipc.c msg_send 1\n");
        while (1)
            ;
    }

    // 1. 目标等待接收此进程的信息
    if ((destiny->flags & RECEIVING) &&
        (destiny->recv_from == current || destiny->recv_from == ANY)) {
        // 1.1 把消息送给目标
        // 相当于在目标进程内部产生返回值
        phys_copy(destiny->message, m, sizeof(MESSAGE));

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
        // 可能有多个中断发生,has_int_msg 变量的不同bit表示不同的中断是否发生
        msg.type = p_who_wanna_recv->has_int_msg;
        p_who_wanna_recv->has_int_msg = 0;

        // 1.1 把该消息附在该进程上
        phys_copy(m, &msg, sizeof(MESSAGE));

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
        phys_copy(m, p_from->message, sizeof(MESSAGE));

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
    // 1. 把目标进程换到阻塞队列
    // 1.1 位于就绪队列第一个
    if (p_proc_ready_head->pid == pid) {
        p_proc_ready_head = p_proc_ready_head->next_pcb;

        // 1.1.1 阻塞队列为空
        if (is_wait_empty) {
            p_proc_wait_head = &PCB_stack[pid];
            PCB_stack[pid].next_pcb = &p_proc_wait_tail;
            p_proc_wait_tail.pre_pcb = &PCB_stack[pid];
        }
        // 1.1.2 阻塞队列不为空
        else {
            PCB_stack[pid].pre_pcb = p_proc_wait_tail.pre_pcb;
            PCB_stack[pid].next_pcb = &p_proc_wait_tail;
            (p_proc_wait_tail.pre_pcb)->next_pcb = &PCB_stack[pid];
            p_proc_wait_tail.pre_pcb = &PCB_stack[pid];
        }

    }
    // 1.2 不位于第一个,这个是不应该出现的情况
    else {
        while (1)
            ;
    }

    // 阻塞操作有一个很严重的后果就是就绪指针指向空队列尾
    // 所以首先要判断一下当前就绪队列是否为空
    // 如果为空一定要执行进程调度
    if (is_ready_empty) {
        schedule();
    }
    return 0;
}

// 解锁一个进程
int unblock(int pid) {
    // 1. 把目标进程换到挂起队列
    // 1.1 位于阻塞队列第一个
    if (p_proc_wait_head->pid == pid) {
        p_proc_wait_head = p_proc_wait_head->next_pcb;

        // 1.1.1 阻塞队列为空
        if (is_pause_empty) {
            p_proc_pause_head = &PCB_stack[pid];
            p_proc_pause_tail.pre_pcb = &PCB_stack[pid];
            PCB_stack[pid].next_pcb = &p_proc_pause_tail;
        }
        // 1.1.2 阻塞队列不为空
        else {
            PCB_stack[pid].pre_pcb = p_proc_pause_tail.pre_pcb;
            PCB_stack[pid].next_pcb = &p_proc_pause_tail;
            (p_proc_pause_tail.pre_pcb)->next_pcb = &PCB_stack[pid];
            p_proc_pause_tail.pre_pcb = &PCB_stack[pid];
        }

    }
    // 1.2 不位于第一个,说明阻塞队列不止一个PCB
    else {
        (PCB_stack[pid].pre_pcb)->next_pcb = PCB_stack[pid].next_pcb;
        (PCB_stack[pid].next_pcb)->pre_pcb = PCB_stack[pid].pre_pcb;

        // 1.1.1 挂起队列为空
        if (is_pause_empty) {
            p_proc_pause_head = &PCB_stack[pid];
            p_proc_pause_tail.pre_pcb = &PCB_stack[pid];
            PCB_stack[pid].next_pcb = &p_proc_pause_tail;
        }
        // 1.1.2 挂起队列不为空
        else {
            PCB_stack[pid].pre_pcb = p_proc_pause_tail.pre_pcb;
            PCB_stack[pid].next_pcb = &p_proc_pause_tail;
            (p_proc_pause_tail.pre_pcb)->next_pcb = &PCB_stack[pid];
            p_proc_pause_tail.pre_pcb = &PCB_stack[pid];
        }
    }

    // 2. 调用调度函数
    // 唤醒一个进程并不需要执行进程调度,继续执行当前进程即可
    // schedule();

    return 0;
}

// 一个中断历程唤醒一个进程
void inform_int(int pid, u32 int_type) {
    PROCESS* p = &PCB_stack[pid];

    // 1. 如果进程阻塞
    if ((p->flags & RECEIVING) &&
        ((p->recv_from == INTERRUPT) || (p->recv_from == ANY))) {
        p->message->source = INTERRUPT;
        p->message->type = int_type;
        p->message = 0;
        p->has_int_msg = 0;
        p->flags &= ~RECEIVING;
        p->recv_from = NO_TASK;
        unblock(pid);
    }
    // 2. 如果进程没有阻塞(进程没有准备好接收信息)
    // 把中断信息存放在has_int_msg变量中
    else {
        p->has_int_msg |= int_type;
    }
}
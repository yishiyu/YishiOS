// 防止各种中断处理
#include "irqhandler.h"

// 之所以加了个参数,是因为才处理中断的时候把中断号压栈了
// 具体情况看include/base/kernel.inc 中关于中断处理函数的宏定义
void clock_handler(int irq) {
    for (int pid = 0; pid < TASK_NUM; pid++) {
        // 判断该进程是否设置了定时器
        if (timers[pid].pid != NO_TASK) {
            timers[pid].time--;
            // 判断该进程设置的时间是否到了
            if (timers[pid].time <= 1) {
                // 向对应的进程发送唤醒信息,然后取消该定时器
                inform_int(timers[pid].pid, HARD_INT_CLOCK);
                timers[pid].pid = NO_TASK;
            }
        }
    }

    ticks++;
    p_proc_ready_head->ticks--;

    //如果从中断进程进入其中,则此时
    if (k_reenter != 0) {
        return;
    }

    if (p_proc_ready_head->ticks > 0) {
        return;
    }

    // 进行一次系统调度
    schedule();
}

// 进程调度函数
void schedule() {
    PROCESS* temp;

    // 首先检查当前进程是不是空进程
    // 如果是的话,把就绪队列指针指向就绪队列尾空指针
    // 否则进行进程调度
    if (is_empty_process) {
        p_proc_ready_head = &p_proc_ready_tail;
    }

    // 如果就绪队列和挂起队列同时为空,则使用空进程
    if (is_ready_empty && is_pause_empty) {
        p_proc_ready_head = &PCB_empty_task;
        return;
    }

    // 普通调度,直接调取下一个待执行进程
    if (is_ready_empty) {
        // 就绪队列为空,这种情况可能出现在进程间通信的时候阻塞当前进程之后的调度
        // 处理方法为直接重新给挂起进程分配时间片,然后把其作为就绪队列
        // 重新为其分配时间片
        temp = p_proc_pause_head;
        do {
            p_proc_pause_head->ticks = p_proc_pause_head->priority;
            p_proc_pause_head = p_proc_pause_head->next_pcb;
        } while (!is_pause_empty);

        // 交换两个链表
        p_proc_ready_head = temp;
        (p_proc_pause_tail.pre_pcb)->next_pcb = &p_proc_ready_tail;
        p_proc_ready_tail.pre_pcb = p_proc_pause_tail.pre_pcb;

    } else if (is_ready_one_left) {
        // 就绪队列执行完毕,需要把挂起队列转换为新的就绪队列
        // 按道理来说,挂起队列中应该至少有一个进程,否则会出现错误
        // 而由于有系统本身的进程存在,所以这一点基本不用考虑

        // 先把就绪队列中剩余的那个进程转移到挂起进程再统一操作
        temp = p_proc_ready_head;
        p_proc_ready_head = p_proc_ready_head->next_pcb;
        if (is_pause_empty) {
            p_proc_pause_head = temp;
            p_proc_pause_tail.pre_pcb = temp;
            temp->next_pcb = &p_proc_pause_tail;
        } else {
            temp->pre_pcb = p_proc_pause_tail.pre_pcb;
            temp->next_pcb = &p_proc_pause_tail;
            (p_proc_pause_tail.pre_pcb)->next_pcb = temp;
            p_proc_pause_tail.pre_pcb = temp;
        }

        // 重新为其分配时间片
        temp = p_proc_pause_head;
        do {
            p_proc_pause_head->ticks = p_proc_pause_head->priority;
            p_proc_pause_head = p_proc_pause_head->next_pcb;
        } while (!is_pause_empty);

        // 交换两个链表
        p_proc_ready_head = temp;
        (p_proc_pause_tail.pre_pcb)->next_pcb = &p_proc_ready_tail;
        p_proc_ready_tail.pre_pcb = p_proc_pause_tail.pre_pcb;
    } else {
        // 就绪链表中还有剩余进程
        // 1. 取下一个可运行进程
        temp = p_proc_ready_head;
        p_proc_ready_head = p_proc_ready_head->next_pcb;
        // 2. 把当前进程移入挂起队列
        if (is_pause_empty) {
            p_proc_pause_head = temp;
            p_proc_pause_tail.pre_pcb = temp;
            temp->next_pcb = &p_proc_pause_tail;
        } else {
            temp->pre_pcb = p_proc_pause_tail.pre_pcb;
            temp->next_pcb = &p_proc_pause_tail;
            (p_proc_pause_tail.pre_pcb)->next_pcb = temp;
            p_proc_pause_tail.pre_pcb = temp;
        }
    }
}

// 键盘中断处理
void keyboard_handler(int irq) {
    u8 scan_code = in_byte(0x60);

    //向缓冲区中添加scan code
    //不进行其他处理,尽量加快处理速度
    // 1. 检查缓冲区是否满
    if (key_buffer.key_count < KEY_BUF_SIZE) {
        // disp_int((int)scan_code);
        // 2. 填充缓冲区
        // 3. 头指针后移
        // 4. 修改计数器
        key_buffer.key_buf[key_buffer.key_head] = scan_code;

        // 如果当前写入的字符是E0,则把该字符的标志位置为0,否则置为1
        // 如果上一个写入的字符是E0,则写入当前字符后把两个标志位置都置为1
        if (scan_code == 0xE0) {
            key_buffer.key_flag[key_buffer.key_head] = 0;
        } else {
            key_buffer.key_flag[key_buffer.key_head] = 1;
            if (key_buffer.key_count > 0) {
                key_buffer.key_flag[(key_buffer.key_head - 1) % KEY_BUF_SIZE] =
                    1;
            }
        }

        key_buffer.key_head++;
        key_buffer.key_head %= KEY_BUF_SIZE;
        key_buffer.key_count++;

        // 通知input子系统
        inform_int(PID_INPUT_SERVER, HARD_INT_KEYBOARD);
    }
}

// 磁盘中断处理
void disk_handler(int irq) {
    // 获取磁盘状态
    disk_status = in_byte(REG_STATUS);

    // 通知磁盘服务器
    inform_int(PID_DISK_SERVER, HARD_INT_DISK);
}
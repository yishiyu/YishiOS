// 防止各种中断处理
#include "irqhandler.h"

// 辅助宏定义
#define is_ready_empty (p_proc_ready_head == &p_proc_ready_tail)
#define is_ready_one_left (p_proc_ready_head->next_pcb == &p_proc_ready_tail)
#define is_wait_empty (p_proc_wait_head == &p_proc_wait_tail)
#define is_wait_one_left (p_proc_wait_head->next_pcb == &p_proc_wait_tail)
#define is_pause_empty (p_proc_pause_head == &p_proc_pause_tail)
#define is_pause_one_left (p_proc_pause_head->next_pcb == &p_proc_pause_tail)

// 之所以加了个参数,是因为才处理中断的时候把中断号压栈了
// 具体情况看include/base/kernel.inc 中关于中断处理函数的宏定义
void clock_handler(int irq) {
    ticks++;
    p_proc_ready_head->ticks--;

    //如果从中断进程进入其中,则此时
    if (k_reenter != 0) {
        return;
    }

    if (p_proc_ready_head->ticks > 0) {
        return;
    }

    disp_str("point irqhandler.c clock_handler 1 \n");
    pause();
    PROCESS* temp;

    // 普通调度,直接调取下一个待执行进程
    if (is_ready_empty) {
        // 按道理来说绝对不会出现这种情况
        while (1)
            ;
    } else if (!is_ready_one_left) {
        disp_str("point irqhandler.c clock_handler 2 \n");
        pause();

        // 就绪链表中还有剩余进程
        temp = p_proc_ready_head;
        p_proc_ready_head = p_proc_ready_head->next_pcb;
        if (is_pause_empty) {
            p_proc_pause_head = temp;
            p_proc_pause_tail.pre_pcb = temp;
            temp->next_pcb = &p_proc_pause_tail;
        } else {
            (p_proc_pause_tail.pre_pcb)->next_pcb = temp;
            p_proc_pause_tail.pre_pcb = temp;
            temp->next_pcb = &p_proc_pause_tail;
        }
        disp_str("point irqhandler.c clock_handler 3 \n");
        pause();

    } else {
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
            (p_proc_pause_tail.pre_pcb)->next_pcb = temp;
            p_proc_pause_tail.pre_pcb = temp;
            temp->next_pcb = &p_proc_pause_tail;
        }
        disp_str("point irqhandler.c clock_handler 4 \n");
        pause();

        // 重新为其分配时间片
        temp = p_proc_pause_head;
        do {
            p_proc_pause_head->ticks = p_proc_pause_head->priority;
            p_proc_pause_head = p_proc_pause_head->next_pcb;
        } while (!is_pause_empty);

        disp_str("point irqhandler.c clock_handler 5 \n");
        pause();

        // 交换两个链表
        p_proc_ready_head = temp;
        (p_proc_pause_tail.pre_pcb)->next_pcb = &p_proc_ready_tail;
        p_proc_ready_tail.pre_pcb = p_proc_pause_tail.pre_pcb;

        disp_str("point irqhandler.c clock_handler 6 \n");
        pause();
    }
}

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
    }
}

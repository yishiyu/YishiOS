// 防止各种中断处理
#include "irqhandler.h"

// 之所以加了个参数,是因为才处理中断的时候把中断号压栈了
// 具体情况看include/base/kernel.inc 中关于中断处理函数的宏定义
void clock_handler(int irq) {
    ticks++;
    p_proc_ready->ticks--;
    // disp_str("#");

    //如果从中断进程进入其中,则此时
    if (k_reenter != 0) {
        return;
    }

    if (p_proc_ready->ticks > 0) {
        return;
    }

    PROCESS* p;
    int greatest_ticks = 0;

    while (!greatest_ticks) {
        for (p = proc_table; p < proc_table + BASE_TASKS_NUM; p++) {
            if (p->ticks > greatest_ticks) {
                greatest_ticks = p->ticks;
                p_proc_ready = p;
            }
        }

        if (!greatest_ticks) {
            for (p = proc_table; p < proc_table + BASE_TASKS_NUM; p++) {
                p->ticks = p->priority;
            }
        }
    }
}

void keyboard_handler(int irq) {
    u8 scan_code = in_byte(0x60);

    //向缓冲区中添加scan code
    //不进行其他处理,尽量加快处理速度
    // 1. 检查缓冲区是否满
    if (key_buffer.key_count >= KEY_BUF_SIZE) {
        return;
    } else {
        //disp_int((int)scan_code);
        // 2. 填充缓冲区
        // 3. 头指针后移
        // 4. 修改计数器
        key_buffer.key_buf[key_buffer.key_head] = scan_code;
        key_buffer.key_head++;
        key_buffer.key_head %= KEY_BUF_SIZE;
        key_buffer.key_count++;
    }
}

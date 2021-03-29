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
    int terminal_id = 0;
    int greatest_ticks = 0;

    while (!greatest_ticks) {
        // 处理任务队列
        for (p = proc_table; p < proc_table + BASE_TASKS_NUM; p++) {
            if (p->ticks > greatest_ticks) {
                greatest_ticks = p->ticks;
                p_proc_ready = p;
            }
        }

        // 处理终端队列
        // 当前只处理当前终端,这样做的话是无法做到后台运行终端的,后面再改
        for (terminal_id=0;terminal_id<TERMINAL_NUM;terminal_id++) {
            if (terminal_table[terminal_id].ticks > greatest_ticks) {
                greatest_ticks = terminal_table[terminal_id].ticks;
                p_proc_ready = &terminal_table[terminal_id];
            }
        }

        // 重置时钟片分配
        if (!greatest_ticks) {
            // 处理任务队列
            for (p = proc_table; p < proc_table + BASE_TASKS_NUM; p++) {
                p->ticks = p->priority;
            }
            //处理终端队列
            for(terminal_id=0;terminal_id<TERMINAL_NUM;terminal_id++){
                terminal_table[terminal_id].ticks = terminal_table[terminal_id].priority;
            }
        }
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

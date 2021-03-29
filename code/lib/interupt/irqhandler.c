// 防止各种中断处理
#include "irqhandler.h"

void clock_handler(int irq) {
    ticks++;
    p_proc_ready->ticks--;
    disp_str("#");

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

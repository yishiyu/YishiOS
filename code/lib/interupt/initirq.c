// 初始化自己写的中断函数

#include "initirq.h"

void init_IRQ() {
    // 打开主片上的中断
    //=======================打开时钟中断========================
    // 初始化 8253 PIT
    // 8253 PIT 芯片用于控制时钟中断,这里设置了时钟中断发生的频率
    // 宏定义和PIT芯片的关系在macro.h文件中
    out_byte(TIMER_MODE, RATE_GENERATOR);
    out_byte(TIMER0, (u8)(TIMER_FREQ / HZ));
    out_byte(TIMER0, (u8)((TIMER_FREQ / HZ) >> 8));
    put_irq_handler(IRQ_CLOCK, clock_handler);
    enable_irq(IRQ_CLOCK);

    //=======================打开键盘中断========================
    //设置键盘中断并打开开关(先设置一下键盘缓冲区的第一个标志位)
    key_buffer.key_flag[0] = 0;
    key_buffer.key_count = 0;
    key_buffer.key_head = 0;
    key_buffer.key_tail = 0;
    put_irq_handler(IRQ_KEYBOARD, keyboard_handler);
    enable_irq(IRQ_KEYBOARD);

    // 打开从片上的中断
    enable_irq(IRQ_SLAVE);

    //=======================打开磁盘中断========================
    put_irq_handler(IRQ_DISK, disk_handler);
    enable_irq(IRQ_DISK);
}
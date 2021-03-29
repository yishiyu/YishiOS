// 主要放各种中断处理函数
#ifndef	YISHIOS_IRQHANDLER_H
#define	YISHIOS_IRQHANDLER_H

#include "global.h"              
#include "display.h"
#include "keymap.h"

// 时钟中断处理
void clock_handler(int irq);

// 键盘中断处理
void keyboard_handler(int irq);

#endif
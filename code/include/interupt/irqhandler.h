// 主要放各种中断处理函数
#ifndef YISHIOS_IRQHANDLER_H
#define YISHIOS_IRQHANDLER_H

//#define __DEBUG_IRQHANDLER__

#ifndef __YISHIOS_DEBUG__
#ifndef __DEBUG_IRQHANDLER__
#define pause()
#define disp_int(str)
#define disp_str(str)
#endif
#endif

#include "display.h"
#include "global.h"
#include "terminal.h"

// 时钟中断处理
void clock_handler(int irq);

// 键盘中断处理
void keyboard_handler(int irq);

#endif
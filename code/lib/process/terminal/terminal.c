#include "terminal.h"

// int sys_sendrec(int function, int src_dest, MESSAGE* m, int pid);

#define __DEBUG_TERMINAL__

#ifndef __YISHIOS_DEBUG__
#define pause()
#define disp_int(str)
#define disp_str(str)
#else
#ifndef __DEBUG_TERMINAL__
#define pause()
#define disp_int(str)
#define disp_str(str)
#else
extern void pause();
#endif
#endif

// 第一个终端,同时也是默认的终端
void tty_0() {
    // 根据task_table可知,pid为1
    // TERMINAL* terminal = &terminal_console_table[0];
    // terminal_init(terminal);
    // terminal_draw_screen(terminal);
    // terminal_main(terminal);

    u32 temp = sys_get_ticks();
    u32 pre = temp;
    disp_str("point terminal.c tty_0 0, ticks == ");
    disp_int(temp);
    disp_str(" pre = ");
    disp_int(pre);
    disp_str("\n");
    pause();
    while (1) {
        temp = sys_get_ticks();
        if (temp > (pre + 100)) {
            disp_str("point terminal.c tty_0 1, ticks == ");
            disp_int(temp);
            disp_str("\n");
            pre = temp;
        }
    }
}

// 第二个终端
void tty_1() {
    // 根据task_table可知,pid为2
    // TERMINAL* terminal = &terminal_console_table[1];
    // terminal_init(terminal);
    // terminal_main(terminal);
    MESSAGE temp;
    while (1) {
    }
}

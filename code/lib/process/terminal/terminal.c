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
    MESSAGE temp;
    temp.source = 1;
    temp.type = 0;
    while (1) {
        disp_str("point terminal.c tty_0 0, msg sended, type == ");
        disp_int(temp.type);
        disp_str("\n");
        pause();
        sys_sendrec(SEND, 2, &temp, 1);
        temp.type++;
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
        sys_sendrec(RECEIVE, 1, &temp, 2);
        disp_str("point terminal.c tty_1 0, msg received, source == ");
        disp_int(temp.source);
        disp_str("type == ");
        disp_int(temp.type);
        disp_str("\n");
        pause();
    }
}

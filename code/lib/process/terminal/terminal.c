#include "terminal.h"

// int sys_sendrec(int function, int src_dest, MESSAGE* m, int pid);

// #define __DEBUG_TERMINAL__

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
    TERMINAL* terminal = &terminal_table[0];
    terminal_init(terminal);
    terminal_draw_screen(terminal);
    terminal_main(terminal);
    // char data[] = "hello world";
    // MESSAGE message;
    // message.source = PID_TTY0;
    // message.type = SERVER_OUTPUT;
    // message.u.output_message.console = &console_table[0];
    // message.u.output_message.data = &data;
    // message.u.output_message.function = OUTPUT_MESSTYPE_DISP;
    // message.u.output_message.pid = PID_TTY0;
    // sys_sendrec(SEND, OUTPUT_SYSTEM, &message, PID_TTY0);
    // sys_terminal_write(0,&data,PID_TTY0);
    // while (1)
    //     ;
}

// 第二个终端
void tty_1() {
    TERMINAL* terminal = &terminal_table[1];
    terminal_init(terminal);
    terminal_main(terminal);
}

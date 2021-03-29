#include "terminal.h"

// 第一个终端,同时也是默认的终端
void tty_0() {
    MESSAGE message;
    char buffer[1024];
    memset(&buffer, 0, 1024);

    while (1) {
        message.source = PID_TTY0;
        message.type = DISK_SYSTEM;
        message.u.disk_message.function = DISK_INFO;
        message.u.disk_message.pid = PID_TTY0;
        message.u.disk_message.buffer = &buffer;
        message.u.disk_message.bytes_count = 1024;
        sys_sendrec(SEND, DISK_SYSTEM, &message, PID_TTY0);
        sys_sendrec(RECEIVE, DISK_SYSTEM, &message, PID_TTY0);
        int i=0;
        int j=1;
    }
    // TERMINAL* terminal = &terminal_table[0];
    // terminal_init(terminal);
    // terminal_draw_screen(terminal);
    // terminal_main(terminal, PID_TTY0);
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
}

// 第二个终端
void tty_1() {
    // TERMINAL* terminal = &terminal_table[1];
    // terminal_init(terminal);
    // // terminal_draw_screen(terminal);
    // terminal_main(terminal, PID_TTY1);
    while (1)
        ;
}

#include "terminal.h"

// 第一个终端,同时也是默认的终端
void tty_1() {
    TERMINAL terminal;
    terminal_init(&terminal);
    terminal_draw_screen(&terminal);
    //terminal_disp_str(&terminal,"terminal 1\n");
    terminal_main(&terminal);
}

// 第二个终端
void tty_2() {
    TERMINAL terminal;
    terminal_init(&terminal);
    terminal_main(&terminal);
}

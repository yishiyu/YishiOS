#include "terminal.h"

// 第一个终端,同时也是默认的终端
void tty_0() {
    TERMINAL* terminal = &terminal_console_table[0];
    terminal_init(terminal);
    terminal_draw_screen(terminal);
    terminal_main(terminal);
}

// 第二个终端
void tty_1() {
    // TERMINAL* terminal = &terminal_console_table[1];
    // terminal_init(terminal);
    // terminal_main(terminal);
    while (1) {
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 1000; j++) {
            }
        }
        sys_terminal_write(0," lalala ");
    }
}

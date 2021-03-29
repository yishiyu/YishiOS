#include "snake.h"

void _start() {
    MESSAGE message[4];
    int pid = sys_get_pid();
    sys_terminal_clear(&message[0], 0, pid);
    sys_terminal_write(&message[1], 0, "hello world !!! \n", pid);
    sys_terminal_write(&message[1], 0, "  YISHI OS !!! \n", pid);
    while (1)
        ;
}
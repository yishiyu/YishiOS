// 贪吃蛇的代码
#include "snake.h"
#include "syscall.h"

// 函数起始标志
void _start() {
    int pid = sys_get_pid();
    sys_terminal_clear(0, pid);
    sys_terminal_write(0, "hello world!!!", pid);
    while (1)
        ;
    return;
}

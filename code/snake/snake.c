// 贪吃蛇的代码
#include "snake.h"

// 消息结构体
static MESSAGE message;
// 图形显示结构体
static VIDEO_UNIT video_mem[MAX_ROW][MAX_COLUMN];
// 进程号
static int pid_snake = 0;
static int pid_parent = 0;

// 函数起始标志
void _start() {
    // 初始化
    snake_init();

    // 主循环
    while (1) {
        // 1. 获取消息
        sys_sendrec(RECEIVE, ANY, &message, pid_snake);
        // 2. 交给消息处理函数
        
    }

    return;
}

// 初始化
void snake_init() {
    // 初始化进程号
    pid_snake = sys_get_pid();

    // 初始化显示内容
    for (int i = 0; i < MAX_ROW; i++) {
        for (int j = 0; j < MAX_COLUMN; j++) {
            video_mem[i][j].data = '#';
            video_mem[i][j].color = MAKE_COLOR(BLACK, WHITE);
        }
    }
    sys_terminal_clear(&message, 0, pid_snake);
}

// 消息处理函数
void snake_handler() {
    switch (message.source) {
        case INTERRUPT:
            // 1. 时钟中断消息


            break;
        case PID_TTY0:
            // 2. 终端消息


            break;

        default:
            break;
    }
}
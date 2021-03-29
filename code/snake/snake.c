// 贪吃蛇的代码
#include "snake.h"

// 消息结构体
static MESSAGE messages[4];
static MESSAGE message;
// 图形显示结构体
static VIDEO_UNIT video_mem[MAX_ROW * MAX_COLUMN];
// 进程号
static int pid_snake = 0;
static int pid_parent = PID_TTY0;

// 游戏数据结构
// 贪吃蛇的位置 (循环队列)
static u32 snake_head = 0;  // 位置队列头指针
static u32 snake_tail = 0;  // 位置队列尾指针
static u32 snake_now = 0;   // 当前贪吃蛇蛇头位置队列下标
static int snake_pos[SNAKE_MAX_LENGTH] = {0};
static u32 snake_ball = 80 * 5 + 60;

// 控制贪吃蛇的移动方向
static u8 snake_dire = 3;
static const int direction[4] = {-MAX_COLUMN, MAX_COLUMN, -1, 1};

// 函数起始标志
void _start() {
    pid_snake = sys_get_pid();
    sys_terminal_clear(&messages[0], 0, pid_snake);
    sys_terminal_write(&messages[1], 0, "hello world !!! \n", pid_snake);
    sys_terminal_write(&messages[2], 0, "  YISHI OS !!! \n", pid_snake);

    MESSAGE temp[2];
    // 初始化
    // snake_init();
    sys_set_timer(pid_snake, SNAKE_SPEED);
    sys_sendrec(RECEIVE, ANY, &message, pid_snake);

    sys_terminal_write(&messages[3], 0, "  SNAKE !!! \n", pid_snake);
    // 主循环
    while (1) {
        // 2. 交给消息处理函数
        // snake_handler();
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
            video_mem[i * MAX_ROW + j].data = ' ';
            video_mem[i * MAX_ROW + j].color = MAKE_COLOR(BLACK, WHITE);
        }
    }
    sys_terminal_clear(&message, 0, pid_snake);

    // 初始化贪吃蛇为一个点
    snake_tail = 0;
    snake_now = 0;
    snake_head = 1;
    snake_pos[0] = 0;
    video_mem[0].data = SNAKE_HEAD;
    video_mem[0].color = SNAKE_COLOR;

    // 设置一个定时器
    sys_set_timer(pid_snake, SNAKE_SPEED);

    // 刷新缓存
    snake_refresh();
}

// 消息处理函数
void snake_handler() {
    switch (message.source) {
        case INTERRUPT:
            // 1. 时钟中断消息
            snake_move();
            break;

        case PID_TTY0:
            // 2. 终端消息
            snake_control();
            break;

        default:
            break;
    }
}

// 刷新显存
void snake_refresh() {
    sys_terminal_draw(&message, 0, (char*)video_mem, pid_snake);
}

// 时钟信号处理
void snake_move() {
    // // 1. 修改蛇头标志
    // video_mem[snake_pos[snake_now]].color = SNAKE_COLOR;
    // video_mem[snake_pos[snake_now]].data = SNAKE_BODY;

    // // 2. 根据运动方向修改贪吃蛇位置
    // snake_pos[snake_head] =
    //     (snake_pos[snake_now] + direction[snake_dire]) % SNAKE_MAX_LENGTH;
    // snake_now = snake_head;
    // snake_head++;
    // snake_head %= SNAKE_MAX_LENGTH;
    // video_mem[snake_pos[snake_now]].color = SNAKE_COLOR;
    // video_mem[snake_pos[snake_now]].data = SNAKE_HEAD;

    // // 3. 修改显存并清除最后一节蛇尾
    // video_mem[snake_pos[snake_tail]].color = SNAKE_COLOR;
    // video_mem[snake_pos[snake_tail]].data = SNAKE_BLANK;
    // snake_tail++;
    // snake_tail %= SNAKE_MAX_LENGTH;

    // video_mem[snake_head].data = '+';
    // video_mem[snake_head].color = SNAKE_COLOR;
    // snake_head++;
    // snake_head %= SNAKE_MAX_LENGTH;

    // 4. 刷新缓存
    snake_refresh();

    // 5. 重新设置一个定时器
    sys_set_timer(pid_snake, SNAKE_SPEED);
}
// 键盘信号处理
void snake_control() {
    switch (message.u.input_message.keyboard_result.data) {
        case KEYBOARD_FUNC_UP:
            snake_dire = SNAKE_UP;
            break;
        case KEYBOARD_FUNC_DOWN:
            snake_dire = SNAKE_DOWN;
            break;
        case KEYBOARD_FUNC_LEFT:
            snake_dire = SNAKE_LEFT;
            break;
        case KEYBOARD_FUNC_RIGHT:
            snake_dire = SNAKE_RIGHT;
            break;

        default:
            break;
    }
}
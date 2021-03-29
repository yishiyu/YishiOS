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
static int snake_ball = 80 * 5 + 60;

// 控制贪吃蛇的移动方向
static int snake_dire = 1;
static const int direction[4] = {-MAX_COLUMN, MAX_COLUMN, -1, 1};

static char buffer[1024] = {0};

void _start() {
    // 1. 初始化数据和界面
    snake_init();

    // 2. 进入主循环
    while (1) {
        sys_sendrec(RECEIVE, ANY, &message, pid_snake);
        snake_handler();
    }
}

// 初始化
void snake_init() {
    // 初始化进程号
    pid_snake = sys_get_pid();

    // 初始化显示内容
    for (int i = 0; i < MAX_ROW; i++) {
        for (int j = 0; j < MAX_COLUMN; j++) {
            video_mem[i * MAX_COLUMN + j].data = ' ';
            video_mem[i * MAX_COLUMN + j].color = SNAKE_COLOR;
            snake_pos[i * MAX_COLUMN + j] = 0;
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

    // 初始化球球
    snake_ball = sys_get_ticks() % SNAKE_VALID;
    video_mem[snake_ball].data = SNAKE_BALL;

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

        default:
            // 2. 键盘信息
            if (message.type == INPUT_SYSTEM) {
                snake_control();
            }
            break;
    }
}

// 刷新显存
void snake_refresh() {
    sys_terminal_draw(&message, 0, (char*)video_mem, pid_snake);
}

// 时钟信号处理
void snake_move() {
    // 1. 修改蛇头标志
    video_mem[snake_pos[snake_now]].data = SNAKE_BODY;

    // 2. 根据运动方向修改贪吃蛇位置
    snake_pos[snake_head] =
        (snake_pos[snake_now] + direction[snake_dire]) % SNAKE_MAX_LENGTH;
    video_mem[snake_pos[snake_head]].data = SNAKE_HEAD;
    snake_now = snake_head;
    snake_head++;
    snake_head %= SNAKE_MAX_LENGTH;

    // 3. 修改显存并清除最后一节蛇尾
    // 查看是否吃到了球球,如果吃到了球球就不消去尾巴
    if (snake_pos[snake_now] != snake_ball) {
        video_mem[snake_pos[snake_tail]].data = SNAKE_BLANK;
        snake_tail++;
        snake_tail %= SNAKE_MAX_LENGTH;
    } else {
        snake_ball = sys_get_ticks() % SNAKE_VALID;
        video_mem[snake_ball].data = SNAKE_BALL;
    }

    // 4. 刷新缓存
    snake_refresh();

    // 5. 重新设置一个定时器
    sys_set_timer(pid_snake, SNAKE_SPEED);
}
// 键盘信号处理
void snake_control() {
    if (message.u.input_message.keyboard_result.type == KEYBOARD_TYPE_ASCII) {
        // ASCII 字符
        switch (message.u.input_message.keyboard_result.data) {
            case KEYBOARD_FUNC_PAUSE:
                // 暂停键
                while (1) {
                    sys_sendrec(RECEIVE, ANY, &message, pid_snake);
                    if ((message.type == INPUT_SYSTEM) &&
                        (message.u.input_message.keyboard_result.type ==
                         KEYBOARD_TYPE_ASCII) &&
                        (message.u.input_message.keyboard_result.data ==
                         KEYBOARD_FUNC_PAUSE)) {
                        sys_set_timer(pid_snake, SNAKE_SPEED);
                        break;
                    }
                }
                break;
            default:
                break;
        }
    } else if (message.u.input_message.keyboard_result.type ==
               KEYBOARD_TYPE_FUNC) {
        //  功能按键
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
            case KEYBOARD_FUNC_SHIFT:
                snake_exit();
                break;
            default:
                break;
        }
    }
}

// 退出函数
void snake_exit() {
    for (int i = 0; i < MAX_ROW; i++) {
        for (int j = 0; j < MAX_COLUMN; j++) {
            video_mem[i * MAX_COLUMN + j].data = ' ';
            video_mem[i * MAX_COLUMN + j].color = SNAKE_COLOR;
        }
    }
    snake_refresh();
    message.source = pid_snake;
    message.type = 0;
    message.u.mem_message.function = MEM_EXEIT;
    message.u.mem_message.pid = pid_snake;
    sys_sendrec(SEND, PID_TTY0, &message, pid_snake);
    sys_sendrec(SEND, MEM_SYSTEM, &message, pid_snake);
    sys_sendrec(RECEIVE, SERVER_MEM, &message, pid_snake);
    // 等待退出
    while (1)
        ;
}

// 显示字符
void disp_str(char* data) {
    MESSAGE temp;
    sys_terminal_write(&temp, 0, data, pid_snake);
}

// 显示数字
void disp_int(int count) {
    MESSAGE temp;
    itoa(count, buffer);
    sys_terminal_write(&temp, 0, buffer, pid_snake);
}

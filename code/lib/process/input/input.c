// input 子系统
#include "input.h"

// 子函数
void input_server_init();

// 输入系统入口
void input_server() {
    // 初始化input系统
    input_server_init();
    // 进入主循环
    MESSAGE input_message;
    while (1) {
        sys_sendrec(RECEIVE, ANY, &input_message, PID_INPUT_SERVER);
        input_handler(&input_message);
    }
}

// 处理函数,由于可能需要同时处理多个中断
void input_handler(MESSAGE* message) {
    // 核实信息类型
    if (message->source != INTERRUPT) return;

    // 依次检测各个中断是否发生并处理
    // 键盘中断
    if (message->type & HARD_INT_KEYBOARD) {
        input_keyboard(message);
    }
    // 硬盘中断绝对不能放在这里,否则如果一个终端卡住了input系统,硬盘中断也会被卡住的
}

void input_server_init() {
    // 初始化键盘
    key_buffer.key_count = 0;
    key_buffer.key_head = 0;
    key_buffer.key_tail = 0;
    for (int i = 0; i < KEY_BUF_SIZE; i++) {
        key_buffer.key_flag[i] = 0;
    }
}

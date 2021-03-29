#include "output.h"

// 输出系统入口
void output_server() {
    MESSAGE output_message;
    while (1) {
        sys_sendrec(RECEIVE, ANY, &output_message, 0);
        output_handler(&output_message);
    }
}

// 处理收到的信息
void output_handler(MESSAGE* message) {
    // 核实信息类型
    if (message->type != SERVER_OUTPUT) return;
    char* data;
    switch (message->u.output_message.function) {
        // 输出信息
        case OUTPUT_MESSTYPE_DISP:
            data = (char*)va2la(message->u.output_message.pid,
                                message->u.output_message.data);
            output_disp_str(message->u.output_message.console, data);
            break;

        // 调整控制台
        case OUTPUT_MESSTYPE_FUNC:
            output_disp_func(message->u.output_message.console,
                             message->u.output_message.disp_func);
            break;

        default:
            break;
    }
}

// 打印字符串
void output_disp_str(CONSOLE* console, char* data) {
    // 根据光标得到要显示的位置
    u8* video_mem_position = (u8*)(VIDEO_MEM_BASE + console->cursor * 2);

    // 限制单条信息的长度为100
    for (int limit = 0; (*data != 0) && (limit < 100); limit++, data++) {
        // 根据不同字符进行不同操作
        switch (*data) {
            case '\n':
            case 0x0D:
                // 回车符
                // 换行
                console->cursor += TERMINAL_WIDTH;
                console->cursor -= ((console->cursor - console->original_addr) %
                                    TERMINAL_WIDTH);
                break;

            case '\b':
                // 退格符
                video_mem_position--;
                *video_mem_position = BLANK_CHAR_COLOR;
                console->cursor--;
                break;

            default:
                *video_mem_position++ = *data;
                *video_mem_position++ = DEFAULT_CHAR_COLOR;
                console->cursor++;
                break;
        }
    }

    output_set_cursor(console);
}

// 显示调整函数(如上下移,左右移光标等)
void output_disp_func(CONSOLE* console, char function) {
    switch (function) {
        case OUTPUT_DISP_FUNC_UP:
            // 向上滚动屏幕
            if (console->current_start_addr > console->original_addr) {
                console->current_start_addr -= 80;
                output_draw_screen(console);
            }
            break;

        case OUTPUT_DISP_FUNC_DOWN:
            // 向下滚动屏幕
            if (console->current_start_addr <
                (console->original_addr + console->mem_limit)) {
                console->current_start_addr += 80;
                output_draw_screen(console);
            }
            break;

        case OUTPUT_DISP_FUNC_LEFT:
            console->cursor--;
            output_draw_screen(console);
            break;

        case OUTPUT_DISP_FUNC_RIGHT:
            console->cursor++;
            output_draw_screen(console);
            break;

        default:
            output_draw_screen(console);
            break;
    }
}

// 设置光标
void output_set_cursor(CONSOLE* console) {
    disable_int();
    // 设置光标位置
    out_byte(CRT_CTRL_ADDR_REG, CURSOR_H);
    out_byte(CRT_CTRL_DATA_REG, (console->cursor >> 8) & 0xFF);
    out_byte(CRT_CTRL_ADDR_REG, CURSOR_L);
    out_byte(CRT_CTRL_DATA_REG, (console->cursor) & 0xFF);
    enable_int();
}

// 根据终端结构体调整输出
void output_draw_screen(CONSOLE* console) {
    disable_int();
    //设置起始位置
    out_byte(CRT_CTRL_ADDR_REG, START_ADDR_H);
    out_byte(CRT_CTRL_DATA_REG, (console->current_start_addr >> 8) & 0xFF);
    out_byte(CRT_CTRL_ADDR_REG, START_ADDR_L);
    out_byte(CRT_CTRL_DATA_REG, (console->current_start_addr) & 0xFF);
    //设置光标位置
    out_byte(CRT_CTRL_ADDR_REG, CURSOR_H);
    out_byte(CRT_CTRL_DATA_REG, (console->cursor >> 8) & 0xFF);
    out_byte(CRT_CTRL_ADDR_REG, CURSOR_L);
    out_byte(CRT_CTRL_DATA_REG, (console->cursor) & 0xFF);
    enable_int();
}
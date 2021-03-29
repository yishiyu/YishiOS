// 在终端中会用到的函数
#include "terminallib.h"

static int tty_count = 0;

// 初始化终端的所有内容
void terminal_init(TERMINAL* terminal) {
    terminal->in_head = 0;
    terminal->in_tail = 0;
    terminal->in_count = 0;

    terminal_init_screen(terminal);
}

// 初始化 终端的显示内容
void terminal_init_screen(TERMINAL* terminal) {
    int mem_in_word = VIDEO_MEM_SIZE >> 1;
    int mem_size = mem_in_word / TERMINAL_NUM;
    terminal->console.original_addr = (u32)tty_count * mem_size;
    terminal->console.mem_limit = mem_size;
    terminal->console.current_start_addr = terminal->console.original_addr;
    terminal->console.cursor = terminal->console.original_addr;
    terminal->terminal_ID = tty_count;
    tty_count++;
}

// 终端的主函数
void terminal_main(TERMINAL* terminal) {
    while (1) {
        if (t_present_terminal == terminal->terminal_ID) {
            terminal_handler(terminal, sys_read_keyboard());
        }
    }
}

// 处理接收到的命令
void terminal_handler(TERMINAL* terminal, KEYMAP_RESULT result) {
    // 处理普通字符
    if (result.type == KEYBOARD_TYPE_ASCII) {
        disp_char(result.data);
    }

    // 处理特殊命令
    if (result.type == KEYBOARD_TYPE_FUNC) {
        switch (result.data) {
            case KEYBOARD_FUNC_ALT:
                // 切换终端后重新刷新屏幕
                terminal_draw_screen(terminal);
                break;
            default:
                break;
        }
    }
}

// 切换到终端之后重新绘制屏幕
void terminal_draw_screen(TERMINAL* terminal) {
    disable_int();
    //设置起始位置
    out_byte(CRT_CTRL_ADDR_REG, START_ADDR_H);
    out_byte(CRT_CTRL_DATA_REG, (terminal->console.original_addr >> 8) & 0xFF);
    out_byte(CRT_CTRL_ADDR_REG, START_ADDR_L);
    out_byte(CRT_CTRL_DATA_REG, (terminal->console.original_addr) & 0xFF);
    //设置光标位置
    out_byte(CRT_CTRL_ADDR_REG, CURSOR_H);
    out_byte(CRT_CTRL_DATA_REG, (terminal->console.cursor >> 8) & 0xFF);
    out_byte(CRT_CTRL_ADDR_REG, CURSOR_L);
    out_byte(CRT_CTRL_DATA_REG, (terminal->console.cursor) & 0xFF);
    enable_int();
}

// 设置光标
void terminal_set_cursor(TERMINAL* terminal) {
    disable_int();
    // 设置光标位置
    out_byte(CRT_CTRL_ADDR_REG, CURSOR_H);
    out_byte(CRT_CTRL_DATA_REG, (terminal->console.cursor >> 8) & 0xFF);
    out_byte(CRT_CTRL_ADDR_REG, CURSOR_L);
    out_byte(CRT_CTRL_DATA_REG, (terminal->console.cursor) & 0xFF);
    enable_int();
}
// 在终端中会用到的函数
#include "terminallib.h"

static int tty_count = 0;

#pragma region 终端初始化和主循环

// 初始化终端的所有内容
void terminal_init(TERMINAL* terminal) {
    // 1. 命令缓冲区初始化
    terminal->in_head = 0;
    terminal->in_tail = 0;
    terminal->in_count = 0;
    // 2. 显存初始化
    terminal_init_screen(terminal);
    // 文件系统初始化需要绑定内存给终端
    // 而函数中的变量是所有调用该函数的进程所共享的
    // 故这一步必须在终端的入口函数处初始化
    // (因为终端的入口函数内的变量只由固定终端使用)
}

// 初始化 终端的显示内容
void terminal_init_screen(TERMINAL* terminal) {
    terminal->terminal_ID = tty_count;
    terminal->console = &console_table[tty_count];
    int mem_in_word = VIDEO_MEM_SIZE >> 1;
    int mem_size = mem_in_word / TERMINAL_NUM;
    terminal->console->original_addr = (u32)tty_count * mem_size;
    terminal->console->mem_limit = mem_size;
    terminal->console->current_start_addr = terminal->console->original_addr;
    terminal->console->cursor = terminal->console->original_addr;
    tty_count++;
}

// 终端的主函数
void terminal_main(TERMINAL* terminal, int terminal_pid) {
    terminal_disp_str(terminal, "Terminal ");
    terminal_disp_int(terminal, terminal->terminal_ID);
    terminal_disp_str(terminal, "\n> ");
    MESSAGE message;
    while (1) {
        sys_sendrec(RECEIVE, PID_INPUT_SERVER, &message, terminal_pid);
        terminal_handler(terminal, message.u.input_message.keyboard_result);
    }
}
#pragma endregion

// 处理接收到的字符
void terminal_handler(TERMINAL* terminal, KEYMAP_RESULT result) {
    // 处理普通字符
    if (result.type == KEYBOARD_TYPE_ASCII) {
        // 处理特殊字符
        switch (result.data) {
            case '\n':
            case 0x0d:
                // 回显字符
                terminal_disp_char(terminal, result.data);
                // 执行命令
                terminal_command_handler(terminal);
                // 显示一个提示符
                terminal_disp_str(terminal, "\n> ");
                break;
            case '\b':
                // 回显字符
                // 为什么退格符的回显字符要放在处理之前呢,因为退格符的回显中有特殊的判定条件
                terminal_disp_char(terminal, result.data);

                // 缓冲区中非空,且当前光标处于输入字符的最右边
                if ((terminal->in_head != terminal->in_tail) &&
                    (((terminal->in_head - terminal->in_tail) %
                      TTY_BUFFER_NUM) == terminal->in_count)) {
                    terminal->in_head--;
                    terminal->in_head %= TTY_BUFFER_NUM;
                    terminal->in_count--;
                }
                break;

            default:
                // 可显示字符
                // 当且仅当输入字符使总命令长度改变的时候才改变count变量
                if (((terminal->in_head - terminal->in_tail) %
                     TTY_BUFFER_NUM) == terminal->in_count) {
                    terminal->in_count++;
                }
                // 添加字符到中断结构体命令缓冲区
                terminal->in_buf[terminal->in_head] = result.data;
                terminal->in_head++;
                terminal->in_head %= TTY_BUFFER_NUM;

                // 回显字符
                terminal_disp_char(terminal, result.data);
                break;
        }
    }

    // 处理特殊命令
    if (result.type == KEYBOARD_TYPE_FUNC) {
        switch (result.data) {
            case KEYBOARD_FUNC_ALT:
                // 切换终端后重新刷新屏幕
                terminal_draw_screen(terminal);
                break;

            case KEYBOARD_FUNC_UP:
                // 向上滚动屏幕
                if (terminal->console->current_start_addr >
                    terminal->console->original_addr) {
                    terminal->console->current_start_addr -= 80;
                    terminal_draw_screen(terminal);
                }
                break;

            case KEYBOARD_FUNC_DOWN:
                // 向下滚动屏幕
                if (terminal->console->current_start_addr <
                    (terminal->console->original_addr +
                     terminal->console->mem_limit)) {
                    terminal->console->current_start_addr += 80;
                    terminal_draw_screen(terminal);
                }
                break;

            case KEYBOARD_FUNC_LEFT:
                // 左移光标
                // 命令 还有空余空间,可以左移
                if (terminal->in_head != terminal->in_tail) {
                    terminal->in_head--;
                    terminal->in_head %= TTY_BUFFER_NUM;
                    terminal->console->cursor--;
                    terminal_set_cursor(terminal);
                }
                break;

            case KEYBOARD_FUNC_RIGHT:
                // 右移光标
                // 命令 还有空余空间,可以右移
                if (((terminal->in_head - terminal->in_tail) % TTY_BUFFER_NUM) <
                    terminal->in_count) {
                    terminal->in_head++;
                    terminal->in_head %= TTY_BUFFER_NUM;
                    terminal->console->cursor++;
                    terminal_set_cursor(terminal);
                }
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
    out_byte(CRT_CTRL_DATA_REG,
             (terminal->console->current_start_addr >> 8) & 0xFF);
    out_byte(CRT_CTRL_ADDR_REG, START_ADDR_L);
    out_byte(CRT_CTRL_DATA_REG, (terminal->console->current_start_addr) & 0xFF);
    //设置光标位置
    out_byte(CRT_CTRL_ADDR_REG, CURSOR_H);
    out_byte(CRT_CTRL_DATA_REG, (terminal->console->cursor >> 8) & 0xFF);
    out_byte(CRT_CTRL_ADDR_REG, CURSOR_L);
    out_byte(CRT_CTRL_DATA_REG, (terminal->console->cursor) & 0xFF);
    enable_int();
}

// 设置光标
void terminal_set_cursor(TERMINAL* terminal) {
    disable_int();
    // 设置光标位置
    out_byte(CRT_CTRL_ADDR_REG, CURSOR_H);
    out_byte(CRT_CTRL_DATA_REG, (terminal->console->cursor >> 8) & 0xFF);
    out_byte(CRT_CTRL_ADDR_REG, CURSOR_L);
    out_byte(CRT_CTRL_DATA_REG, (terminal->console->cursor) & 0xFF);
    enable_int();
}

// 处理命令
void terminal_command_handler(TERMINAL* terminal) {
    // 1. 判断命令
    // 暂时假设没有参数,不用先处理命令格式

    // 2. 执行命令
    char* temp = terminal->in_buf + terminal->in_tail;
    if (strcmp(terminal->in_buf + terminal->in_tail, "root") == 0) {
        terminal_root(terminal);
    } else if (strcmp(terminal->in_buf + terminal->in_tail, "ls") == 0) {
        terminal_ls(terminal);
    }

    // 3. 暂时简单地把缓冲区清空
    terminal->in_tail = terminal->in_head;
    terminal->in_count = 0;
}

#pragma region 终端显示函数
// 屏幕显示函数
void terminal_disp_char(TERMINAL* terminal, char data) {
    // 把字符填充到显存中并修改光标
    // 1. 根据光标得到要显示的位置
    u8* video_mem_position =
        (u8*)(VIDEO_MEM_BASE + terminal->console->cursor * 2);

    // 2. 填充字符和颜色
    // disp_int(data);
    switch (data) {
        case '\n':
        case 0x0D:
            // 回车符
            // 换行
            terminal->console->cursor += TERMINAL_WIDTH;
            terminal->console->cursor -= ((terminal->console->cursor -
                                           terminal->console->original_addr) %
                                          TERMINAL_WIDTH);
            break;

        case '\b':
            // 退格符
            if ((terminal->in_head != terminal->in_tail) &&
                (((terminal->in_head - terminal->in_tail) % TTY_BUFFER_NUM) ==
                 terminal->in_count)) {
                // 缓冲区非空,即此条命令有未被消除的部分
                video_mem_position--;
                *video_mem_position = BLANK_CHAR_COLOR;
                terminal->console->cursor--;
            }
            break;

        default:
            *video_mem_position++ = data;
            *video_mem_position = DEFAULT_CHAR_COLOR;
            terminal->console->cursor++;
            break;
    }
    // 3. 修改光标位置
    terminal_set_cursor(terminal);
}

void terminal_disp_str(TERMINAL* terminal, char* data) {
    // 输入的参数为字符串起始位置,字符串终结符为0
    while (*data != 0) {
        terminal_disp_char(terminal, *data);
        data++;
    }
}

void terminal_disp_int(TERMINAL* terminal, int data) {
    char message[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char* temp = &message[19];

    // 把数字转成字符串
    if (data == 0) {
        temp--;
        *temp = '0';
    }
    while (data > 0) {
        temp--;
        *temp = (data % 10 + '0');
        data /= 10;
    }

    // 输出字符串
    terminal_disp_str(terminal, temp);
}
#pragma endregion

#pragma region 命令处理函数

// 打开根目录并显示文件
void terminal_root(TERMINAL* terminal) {
    // 1. 参数准备
    char* directory_buffer = terminal->directory_buffer;
    DIR_ENTRY* directory_entry;
    // 2. 请求文件系统的服务
    MESSAGE message;
    message.source = terminal->pid;
    message.type = terminal->pid;
    message.u.fs_message.pid = terminal->pid;
    message.u.fs_message.buffer = terminal->directory_buffer;
    message.u.fs_message.count = DIRET_BUF_SIZE;
    message.u.fs_message.fd = terminal->directory_fd;
    message.u.fs_message.function = FS_ROOT;
    sys_sendrec(SEND, SERVER_FS, &message, terminal->pid);
    sys_sendrec(RECEIVE, SERVER_FS, &message, terminal->pid);
    // 3. 解析根目录
    int file_size = terminal->directory_fd->fd_inode->i_size;
    int buffer_size = terminal->directory_buffer_size;
    int directory_limit = (file_size > buffer_size) ? buffer_size : file_size;
    int entry_index = 0;
    for (int i = 0; (i < 10) && (entry_index < directory_limit); i++) {
        directory_entry =
            (struct directory_entry*)&directory_buffer[entry_index];
        entry_index += directory_entry->rec_len;
        // 文件为普通文件或目录文件
        if ((directory_entry->file_type == 1) ||
            (directory_entry->file_type == 2)) {
            terminal_disp_str(terminal, &(directory_entry->name));
            terminal_disp_char(terminal, '\n');
        }
    }
}

// 显示当前文件夹中的文件
void terminal_ls(TERMINAL* terminal) {
    // 1. 参数准备
    char* directory_buffer = terminal->directory_buffer;
    DIR_ENTRY* directory_entry;

    // 2. 解析目录
    int file_size = terminal->directory_fd->fd_inode->i_size;
    int buffer_size = terminal->directory_buffer_size;
    int directory_limit = (file_size > buffer_size) ? buffer_size : file_size;
    int entry_index = 0;
    for (int i = 0; (i < 10) && (entry_index < directory_limit); i++) {
        directory_entry =
            (struct directory_entry*)&directory_buffer[entry_index];
        entry_index += directory_entry->rec_len;
        // 文件为普通文件或目录文件
        if ((directory_entry->file_type == 1) ||
            (directory_entry->file_type == 2)) {
            terminal_disp_str(terminal, &(directory_entry->name));
            terminal_disp_char(terminal, '\n');
        }
    }
}


#pragma endregion
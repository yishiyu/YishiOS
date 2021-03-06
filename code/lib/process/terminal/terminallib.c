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
    // 3. 初始化父子进程pid
    terminal->child_pid = NO_TASK;
}

// 初始化 终端的显示内容
void terminal_init_screen(TERMINAL* terminal) {
    terminal->terminal_ID = tty_count;
    terminal->console = &console_table[tty_count];
    terminal->console->original_addr =
        (u32)tty_count * ((VIDEO_MEM_SIZE >> 1) / TERMINAL_NUM);
    terminal->console->mem_limit = ((VIDEO_MEM_SIZE >> 1) / TERMINAL_NUM);
    terminal->console->current_start_addr = terminal->console->original_addr;
    terminal->console->cursor = terminal->console->original_addr;
    tty_count++;
}

// 主函数
void terminal_main(TERMINAL* terminal, int terminal_pid, MESSAGE* message) {
    terminal_disp_str(terminal, "Terminal ");
    terminal_disp_int(terminal, terminal->terminal_ID);
    terminal_disp_str(terminal, "\n> ");
    while (1) {
        sys_sendrec(RECEIVE, ANY, message, terminal_pid);

        switch (message->source) {
            case PID_INPUT_SERVER:
                // 如果终端创建了子进程的话,就把消息继续向子进程传递
                if (terminal->child_pid != NO_TASK) {
                    message->source = terminal->pid;
                    message->type = INPUT_SYSTEM;
                    message->u.input_message.input_source = terminal->pid;
                    sys_sendrec(SEND, terminal->child_pid, message,
                                terminal->pid);
                } else {
                    terminal_handler(terminal,
                                     message->u.input_message.keyboard_result);
                }
                break;

            default:
                if ((terminal->child_pid != NO_TASK) &&
                    (message->source == terminal->child_pid)) {
                    // 如果是子进程发送来的退出信息
                    if (message->type == 0) {
                        terminal->child_pid = NO_TASK;
                        terminal_disp_str(terminal, "\n> ");
                    }
                } else {
                    terminal_handler(terminal,
                                     message->u.input_message.keyboard_result);
                }
                break;
        }
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
                // 1. 加入一个终止符0,使命令称为一个完整的字符串
                terminal->in_buf[(terminal->in_tail + terminal->in_count) %
                                 TTY_BUFFER_NUM] = 0;
                // 2. 执行命令
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
                // 缓冲区没满才进行操作(留一些多余的空间)
                if (terminal->in_count < (TTY_BUFFER_NUM - 8)) {
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
                }
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
    // 1.1 根据count变量修改tail以获取命令终点
    terminal->in_head = terminal->in_tail + terminal->in_count;
    // 1.2 把命令中的空格替换为0,把命令分割成多个字符串
    str_replace(terminal->in_buf + terminal->in_tail, ' ', 0);

    // 2. 执行命令
    char* temp = terminal->in_buf + terminal->in_tail;
    if (strcmp(terminal->in_buf + terminal->in_tail, "root") == 0) {
        terminal_root(terminal);
    } else if (strcmp(terminal->in_buf + terminal->in_tail, "ls") == 0) {
        terminal_ls(terminal);
    } else if (strcmp(terminal->in_buf + terminal->in_tail, "cd") == 0) {
        // cd 命令有一个参数, 修改in_tail以读取参数
        terminal->in_tail += strlen(terminal->in_buf + terminal->in_tail);
        terminal->in_tail++;
        terminal_cd(terminal, terminal->in_buf + terminal->in_tail);
    } else if (strcmp(terminal->in_buf + terminal->in_tail, "cls") == 0) {
        // 屏幕下滚至清空屏幕
        // 1. 光标移至下一行行首
        terminal->console->cursor += TERMINAL_WIDTH;
        terminal->console->cursor -=
            (terminal->console->cursor % TERMINAL_WIDTH);
        // 2. 终端显示首地址转换
        terminal->console->current_start_addr = terminal->console->cursor;
        terminal_draw_screen(terminal);
    } else if (strcmp(terminal->in_buf + terminal->in_tail, "open") == 0) {
        terminal->in_tail += strlen(terminal->in_buf + terminal->in_tail);
        terminal->in_tail++;
        terminal_open(terminal, terminal->in_buf + terminal->in_tail);
    } else if (strcmp(terminal->in_buf + terminal->in_tail, "run") == 0) {
        terminal_run(terminal);
    }

    // 3. 把缓冲区清空
    terminal->in_tail = terminal->in_head = 0;
    terminal->in_count = 0;
}

#pragma region 终端显示函数
// 屏幕显示函数
void terminal_disp_char(TERMINAL* terminal, char data) {
    // 把字符填充到显存中并修改光标
    // 1. 根据光标得到要显示的位置
    u8* video_mem_position[TERMINAL_NUM];
    video_mem_position[terminal->terminal_ID] =
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
                video_mem_position[terminal->terminal_ID]--;
                *(video_mem_position[terminal->terminal_ID]) = BLANK_CHAR_COLOR;
                terminal->console->cursor--;
            }
            break;

        default:
            *(video_mem_position[terminal->terminal_ID])++ = data;
            *(video_mem_position[terminal->terminal_ID]) = DEFAULT_CHAR_COLOR;
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
    char message[TERMINAL_NUM][20];
    memset((void*)message[terminal->terminal_ID], 0, 20);
    char* temp[TERMINAL_NUM];
    temp[terminal->terminal_ID] = &message[terminal->terminal_ID][19];

    // 把数字转成字符串
    if (data == 0) {
        temp[terminal->terminal_ID]--;
        *(temp[terminal->terminal_ID]) = '0';
    }
    while (data > 0) {
        temp[terminal->terminal_ID]--;
        *(temp[terminal->terminal_ID]) = (data % 10 + '0');
        data /= 10;
    }

    // 输出字符串
    terminal_disp_str(terminal, temp[terminal->terminal_ID]);
}
#pragma endregion

#pragma region 命令处理函数

// 打开根目录并显示文件
void terminal_root(TERMINAL* terminal) {
    // 1. 参数准备
    char* directory_buffer = terminal->directory_buffer;
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

// 切换文件夹
int terminal_cd(TERMINAL* terminal, char* file_name) {
    // 1. 参数准备
    char* directory_buffer = terminal->directory_buffer;
    // 2. 请求文件系统的服务
    MESSAGE message;
    message.source = terminal->pid;
    message.type = terminal->pid;
    message.u.fs_message.pid = terminal->pid;
    message.u.fs_message.buffer = terminal->directory_buffer;
    message.u.fs_message.count = DIRET_BUF_SIZE;
    message.u.fs_message.fd = terminal->directory_fd;
    message.u.fs_message.function = FS_CD;
    message.u.fs_message.file_name = file_name;
    sys_sendrec(SEND, SERVER_FS, &message, terminal->pid);
    sys_sendrec(RECEIVE, SERVER_FS, &message, terminal->pid);
}

// 读取一个文件的inode
int terminal_open(TERMINAL* terminal, char* file_name) {
    // 1. 参数准备
    char* directory_buffer = terminal->directory_buffer;
    // 2. 请求文件系统的服务
    MESSAGE message;
    message.source = terminal->pid;
    message.type = terminal->pid;
    message.u.fs_message.pid = terminal->pid;
    message.u.fs_message.buffer = terminal->directory_buffer;
    // 这个不仅是读取内容的数量,还是文件夹缓冲区的大小,所以不能为0
    message.u.fs_message.count = DIRET_BUF_SIZE;
    message.u.fs_message.fd = terminal->file_fd;
    message.u.fs_message.function = FS_OPENFILE;
    message.u.fs_message.file_name = file_name;
    sys_sendrec(SEND, SERVER_FS, &message, terminal->pid);
    sys_sendrec(RECEIVE, SERVER_FS, &message, terminal->pid);
}

// 运行当前打开的文件
int terminal_run(TERMINAL* terminal) {
    MESSAGE message;
    message.source = terminal->pid;
    message.type = terminal->pid;
    message.u.mem_message.pid = terminal->pid;
    message.u.mem_message.function = MEM_EXECUTE;
    message.u.mem_message.result == 0;
    message.u.mem_message.file = terminal->file_fd->fd_inode;
    sys_sendrec(SEND, SERVER_MEM, &message, terminal->pid);
    MESSAGE message_rec;
    sys_sendrec(RECEIVE, SERVER_MEM, &message_rec, terminal->pid);
    // 创建子进程成功
    if (message_rec.u.mem_message.result >= 0) {
        terminal->child_pid = message_rec.u.mem_message.result;
    }
}
#pragma endregion
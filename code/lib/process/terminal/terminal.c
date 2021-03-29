#include "terminal.h"

// 第一个终端,同时也是默认的终端
void tty_0() {
    TERMINAL* terminal = &terminal_table[0];
    // 文件系统初始化
    char directory_buffer[DIRET_BUF_SIZE];
    struct inode directory_inode;
    struct inode file_inode;

    FILE_DESCRIPTOR directory_fd;
    directory_fd.fd_pos = 0;
    directory_fd.fd_inode = &directory_inode;

    FILE_DESCRIPTOR file_fd;
    file_fd.fd_pos = 0;
    file_fd.fd_inode = &file_inode;

    terminal->directory_buffer = directory_buffer;
    terminal->directory_buffer_size = DIRET_BUF_SIZE;
    terminal->directory_fd = &directory_fd;
    terminal->file_fd = &file_fd;
    terminal->pid = PID_TTY0;

    int i = 0;
    int pid = sys_get_pid();

    MESSAGE message;

    // 显示欢迎界面
    Wellocome_to_YishiOS();

    // 其他部分初始化
    terminal_init(terminal);
    terminal_draw_screen(terminal);
    terminal_main(terminal, PID_TTY0, &message);
}

// 第二个终端
void tty_1() {
    TERMINAL* terminal = &terminal_table[1];
    // 文件系统初始化
    char directory_buffer[DIRET_BUF_SIZE];
    struct inode directory_inode;
    struct inode file_inode;

    FILE_DESCRIPTOR directory_fd;
    directory_fd.fd_pos = 0;
    directory_fd.fd_inode = &directory_inode;

    FILE_DESCRIPTOR file_fd;
    file_fd.fd_pos = 0;
    file_fd.fd_inode = &file_inode;

    terminal->directory_buffer = directory_buffer;
    terminal->directory_buffer_size = DIRET_BUF_SIZE;
    terminal->directory_fd = &directory_fd;
    terminal->file_fd = &file_fd;
    terminal->pid = PID_TTY1;

    int i = 0;
    int pid = sys_get_pid();

    MESSAGE message;
    while (1) {
        // sys_sendrec(RECEIVE, ANY, &message, PID_TTY1);
    }

    terminal_init(terminal);
    terminal_main(terminal, PID_TTY1, &message);
}

// 欢迎界面显示字符
VIDEO_UNIT YishiOS_hello_world[80 * 25];
// 字模
u8 Y_word[35] = {1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0,
                 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0};
u8 I_word[35] = {0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
                 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0};
u8 S_word[35] = {0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1,
                 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0};
u8 H_word[35] = {1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1,
                 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1};
u8 O_word[35] = {0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0,
                 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0};
char tip_word[27] = "Press any key to continue !";

void Wellocome_to_YishiOS() {
    MESSAGE message;
    // 1. 初始化
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 80; j++) {
            YishiOS_hello_world[i * 80 + j].color = MAKE_COLOR(BLACK, WHITE);
            YishiOS_hello_world[i * 80 + j].data = ' ';
        }
    }
    int row = 4;
    int col = 18;
    // 2. 填充字模
    init_word(&(YishiOS_hello_world[row * 80 + col]), Y_word);
    col += 6;
    init_word(&(YishiOS_hello_world[row * 80 + col]), I_word);
    col += 6;
    init_word(&(YishiOS_hello_world[row * 80 + col]), S_word);
    col += 6;
    init_word(&(YishiOS_hello_world[row * 80 + col]), H_word);
    col += 6;
    init_word(&(YishiOS_hello_world[row * 80 + col]), I_word);
    col += 6;
    init_word(&(YishiOS_hello_world[row * 80 + col]), O_word);
    col += 6;
    init_word(&(YishiOS_hello_world[row * 80 + col]), S_word);

    row = 13;
    col = 26;
    for (int i = 0; i < 28; i++) {
        YishiOS_hello_world[row * 80 + col].data = tip_word[i];
        col++;
    }

    sys_terminal_draw(&message, 0, (char*)YishiOS_hello_world, PID_TTY0);
    sys_sendrec(RECEIVE, ANY, &message, PID_TTY0);

    // 3. 清空屏幕
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 80; j++) {
            YishiOS_hello_world[i * 80 + j].data = ' ';
        }
    }
    sys_terminal_draw(&message, 0, (char*)YishiOS_hello_world, PID_TTY0);
    sys_set_timer(PID_TTY0, 100);
    sys_sendrec(RECEIVE, ANY, &message, PID_TTY0);
}

// 填充字符
void init_word(VIDEO_UNIT* buffer, u8* word) {
    int index = 0;
    // 填充一行
    for (int row = 0; row < 7; row++) {
        // 填充一行中的每个块
        for (int column = 0; column < 5; column++) {
            buffer[index].data = (word[row * 5 + column] == 1) ? '#' : ' ';
            index++;
        }
        // 下标移至下一行起始位置
        index += (80 - 5);
    }
}
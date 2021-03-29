#include "terminal.h"

// 第一个终端,同时也是默认的终端
void tty_0() {
    TERMINAL* terminal = &terminal_table[0];
    // 文件系统初始化
    char directory_buffer[DIRET_BUF_SIZE];
    struct inode root_inode;
    FILE_DESCRIPTOR directory_fd;
    directory_fd.fd_pos = 0;
    directory_fd.fd_inode = &root_inode;
    terminal->directory_buffer = directory_buffer;
    terminal->directory_buffer_size = DIRET_BUF_SIZE;
    terminal->directory_fd = &directory_fd;
    terminal->pid = PID_TTY0;

    int i = 0;
    int pid = sys_get_pid();

    // 其他部分初始化
    terminal_init(terminal);
    terminal_draw_screen(terminal);
    terminal_main(terminal, PID_TTY0);

    // while (1) {
    //     MESSAGE message;
    //     sys_set_timer(PID_TTY0, 1000);
    //     sys_sendrec(RECEIVE, ANY, &message, PID_TTY0);
    //     disp_int(sys_get_ticks());
    //     disp_str("\n");
    // }
}

// 第二个终端
void tty_1() {
    TERMINAL* terminal = &terminal_table[1];
    // 文件系统初始化
    char directory_buffer[DIRET_BUF_SIZE];
    struct inode root_inode;
    FILE_DESCRIPTOR directory_fd;
    directory_fd.fd_pos = 0;
    directory_fd.fd_inode = &root_inode;
    terminal->directory_buffer = directory_buffer;
    terminal->directory_buffer_size = DIRET_BUF_SIZE;
    terminal->directory_fd = &directory_fd;
    terminal->pid = PID_TTY1;

    int i = 0;
    int pid = sys_get_pid();

    terminal_init(terminal);
    terminal_main(terminal, PID_TTY1);

}

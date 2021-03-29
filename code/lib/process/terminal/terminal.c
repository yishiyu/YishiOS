#include "terminal.h"

// 第一个终端,同时也是默认的终端
void tty_0() {
    // TERMINAL* terminal = &terminal_table[0];
    // terminal_init(terminal);
    // terminal_draw_screen(terminal);
    // terminal_main(terminal, PID_TTY0);

    // 1. 参数准备
    char root_buf[4096];
    DIR_ENTRY* root_dir_entry;
    struct inode root_inode;
    FILE_DESCRIPTOR root_fd;
    root_fd.fd_pos = 0;
    root_fd.fd_inode = &root_inode;
    // 2. 请求文件系统的服务
    MESSAGE message;
    message.source = PID_TTY0;
    message.type = PID_TTY0;
    message.u.fs_message.pid = PID_TTY0;
    message.u.fs_message.buffer = root_buf;
    message.u.fs_message.count = 4096;
    message.u.fs_message.fd = &root_fd;
    message.u.fs_message.function = FS_ROOT;
    sys_sendrec(SEND, SERVER_FS, &message, PID_TTY0);
    sys_sendrec(RECEIVE, SERVER_FS, &message, PID_TTY0);
    // 3. 解析根目录
    int entry_index = 0;
    for (int i = 0; i < 10; i++) {
        root_dir_entry = (struct directory_entry*)&root_buf[entry_index];
        entry_index += root_dir_entry->rec_len;
        // 文件为普通文件或目录文件
        if ((root_dir_entry->file_type == 1) ||
            (root_dir_entry->file_type == 2)) {
            disp_str(&(root_dir_entry->name));
            disp_str("\n");
        }
    }
    while (1)
        ;
}

// 第二个终端
void tty_1() {
    // TERMINAL* terminal = &terminal_table[1];
    // terminal_init(terminal);
    // // terminal_draw_screen(terminal);
    // terminal_main(terminal, PID_TTY1);
    while (1)
        ;
}

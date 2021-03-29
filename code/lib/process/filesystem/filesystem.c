#include "filesystem.h"

static MESSAGE message;

// 入口函数
void FS_server() {
    FS_init();
    while (1) {
    }
}

// 子函数
void FS_handler(MESSAGE* message) {}
void FS_init(MESSAGE* message) {}
void FS_get_root(MESSAGE* message){}
void FS_change_dir(MESSAGE* message){}
void FS_init_get_superblock(){}

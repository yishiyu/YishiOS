// 文件管理系统
#include "memory.h"
#include "struct.h"

// 内存管理入口
void mem_server() {
    MESSAGE message;

    mem_init();

    memset(&message, 0, sizeof(message));

    while (1) {
        sys_sendrec(RECEIVE, ANY, &message, PID_MEM_SERVER);
        // 对收到的信息进行判断,排除中断信息(否则会出现试图向中断发送信息的情况)
        if ((message.source < 0) || (message.source >= MAX_PROCESS_NUM))
            continue;

        int src = message.source;
        int result = 0;

        switch (message.u.fs_message.function) {
            // case MEM_EXECUTE:
            //     // 运行一个程序
            //     break;

            // case MEM_EXEIT:
            //     // 运行一个程序
            //     break;
            default:
                break;
        }
        // message.source = FILE_SYSTEM;
        // message.type = SERVER_FS;
        // message.u.disk_message.result = result;
        // sys_sendrec(SEND, src, &message, PID_FS_SERVER);
        // memset(&message, 0, sizeof(message));
    }
}

// 内存管理功能函数
int execute(MESSAGE* message) { return 0; }
void exit(MESSAGE* message) {}

// 子函数
void mem_init() {}
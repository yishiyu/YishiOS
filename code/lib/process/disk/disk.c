#include "disk.h"

u8 disk_status;
u8 disk_buffer[SECTOR_SIZE * 2];

void disk_server() {
    MESSAGE message;

    disk_init();

    while (1) {
        sys_sendrec(RECEIVE, ANY, &message, PID_DISK_SERVER);

        int src = message.source;
        int result = 0;

        switch (message.u.disk_message.function) {
            case DISK_OPEN:
                disk_open(&message);
                break;

            case DISK_CLOSE:
                disk_close(&message);
                break;

            case DISK_READ:
                disk_read(&message);
                break;

            case DISK_WRITE:
                disk_write(&message);
                break;

            case DISK_INFO:
                disk_info(&message);
                break;

            default:
                break;
        }
        message.source = DISK_SYSTEM;
        message.type = SERVER_DISK;
        message.u.disk_message.result = result;
        sys_sendrec(SEND, src, &message, PID_DISK_SERVER);
    }
}

// 初始化磁盘服务器变量
void disk_init() {}

// 磁盘操作函数
// 开关磁盘函数(其实没啥用,先不写)
void disk_open(MESSAGE* message) {}
void disk_close(MESSAGE* message) {}
// 读写磁盘函数
void disk_read(MESSAGE* message) {}
void disk_write(MESSAGE* message) {}
// 获取磁盘信息函数
void disk_info(MESSAGE* message) {
    DISK_CMD cmd;
    // #define MAKE_DEVICE_REG(lba, drv, lba_highest)
    // master 开启lba模式 高四位为0000
    cmd.device = MAKE_DEVICE_REG(0, 0, 0);
    cmd.command = ATA_INFO;

    // 读取键盘消息
    // disk_status = in_byte(REG_STATUS);
    disk_cmd_out(&cmd);
    // disk_status = in_byte(REG_STATUS);
    // orange书上有这一条 interrupt_wait() p328
    // 但是好像获取硬盘信息的操作不会耗费很长时间,硬盘也不会触发中断
    // interrupt_wait();

    port_read(REG_DATA, disk_buffer, SECTOR_SIZE);

    // 把磁盘信息转化成纯ascii字符串(存放在disk_buffer后512字节中)
    // 转化的过程中,字符串的位置是固定的
    // char* result_buf = &disk_buffer[512];

    // 把信息送给调用者
    void* la = (void*)va2la(message->u.disk_message.pid,
                            message->u.disk_message.buffer);
    u32 bytes_count = (SECTOR_SIZE > message->u.disk_message.bytes_count)
                          ? message->u.disk_message.bytes_count
                          : SECTOR_SIZE;
    phys_copy(la, (void*)va2la(PID_DISK_SERVER, disk_buffer), bytes_count);
}

// 磁盘操作函数
// 0 --> 操作失败
// 1 --> 操作成功
int disk_cmd_out(DISK_CMD* command) {
    // 所有操作都必须等待磁盘空闲之后才能进行
    if (!waitfor(STATUS_BSY, 0, HD_TIMEOUT)) {
        return 0;
    }

    // 打开磁盘的中断使能
    out_byte(REG_DEV_CTRL, 0);
    // 写入命令参数
    // 由于在这里的限制,只能使用4GB的磁盘空间
    out_byte(REG_FEATURES, command->features);
    out_byte(REG_SECTOR_COUNT, command->count);
    out_byte(REG_LBA_LOW, command->lba_low);
    out_byte(REG_LBA_MID, command->lba_mid);
    out_byte(REG_LBA_HIGH, command->lba_high);
    out_byte(REG_DEVICE, command->device);
    // 写入命令
    out_byte(REG_CMD, command->command);
    return 1;
}

// 磁盘超时/等待函数
// mask --> 检测屏蔽值(mask & status == 检测目标)
// val --> 检测目标目的值
// timeout --> 超时时间,单位毫秒
// 0 --> 操作失败
// 1 --> 操作成功
int waitfor(int mask, int val, int timeout) {
    int t = sys_get_ticks();
    u8 temp = 0;
    while (((sys_get_ticks() - t) * 1000 / HZ) < timeout) {
        temp = in_byte(REG_STATUS);
        if ((temp & mask) == val) return 1;
    }

    return 0;
}

// 等待磁盘中断
void interrupt_wait() {
    MESSAGE msg;
    sys_sendrec(RECEIVE, INTERRUPT, &msg, PID_DISK_SERVER);
}
#include "disk.h"

#define DISK_BUFFER_SIZE SECTOR_SIZE
u8 disk_status;
u8 disk_buffer[DISK_BUFFER_SIZE];

void disk_server() {
    MESSAGE message;

    disk_init();
    memset(&disk_buffer, 0, 1024);
    while (1) {
        sys_sendrec(RECEIVE, ANY, &message, PID_DISK_SERVER);
        // 对收到的信息进行判断,排除中断信息(否则会出现试图向中断发送信息的情况)
        if ((message.source < 0) || (message.source >= MAX_PROCESS_NUM))
            continue;

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
        memset(&disk_buffer, 0, 1024);
    }
}

// 初始化磁盘服务器变量
void disk_init() {}

// 磁盘操作函数
// 开关磁盘函数(其实没啥用,先不写)
void disk_open(MESSAGE* message) {}
void disk_close(MESSAGE* message) {}
// 读写磁盘函数
void disk_read(MESSAGE* message) {
    // 读取的起始扇区  读取的字节数   缓冲区的起始指针
    u32 sector_head = message->u.disk_message.sector_head;
    u32 bytes_count = message->u.disk_message.bytes_count;

    // 硬盘命令
    u32 sector_left = (bytes_count + SECTOR_SIZE - 1) / SECTOR_SIZE;
    u32 sector_done = 0;
    // 准备向调用者传送数据
    int bytes_left = bytes_count;
    void* la = (void*)va2la(message->u.disk_message.pid,
                            message->u.disk_message.buffer);

    while (sector_left > 0) {
        DISK_CMD command;
        command.features = 0;
        command.count = (sector_left > 128) ? 128 : sector_left;
        command.lba_low = sector_head & 0xff;
        command.lba_mid = (sector_head >> 8) & 0xff;
        command.lba_high = (sector_head >> 16) & 0xff;
        command.device = MAKE_DEVICE_REG(1, 0, (sector_head >> 24) & 0xf);
        command.command = ATA_READ;
        disk_cmd_out(&command);
        sector_left -= command.count;
        sector_done += command.count;
        sector_head += command.count;

        while (command.count > 0) {
            int bytes =
                (bytes_left < DISK_BUFFER_SIZE) ? bytes_left : DISK_BUFFER_SIZE;
            interrupt_wait();
            // 从磁盘读取
            port_read(REG_DATA, disk_buffer, DISK_BUFFER_SIZE);
            phys_copy(la, (void*)va2la(PID_DISK_SERVER, disk_buffer), bytes);
            la += bytes;
            command.count -= 1;
            bytes_left -= bytes;
        }
    }
}
void disk_write(MESSAGE* message) {
    // 写入的起始扇区  写入的字节数   缓冲区的起始指针
    u32 sector_head = message->u.disk_message.sector_head;
    u32 bytes_count = message->u.disk_message.bytes_count;

    // 硬盘命令
    u32 sector_left = (bytes_count + SECTOR_SIZE - 1) / SECTOR_SIZE;
    u32 sector_done = 0;
    // 准备向调用者传送数据
    int bytes_left = bytes_count;
    void* la = (void*)va2la(message->u.disk_message.pid,
                            message->u.disk_message.buffer);

    while (sector_left > 0) {
        DISK_CMD command;
        command.features = 0;
        command.count = (sector_left > 128) ? 128 : sector_left;
        command.lba_low = sector_head & 0xff;
        command.lba_mid = (sector_head >> 8) & 0xff;
        command.lba_high = (sector_head >> 16) & 0xff;
        command.device = MAKE_DEVICE_REG(1, 0, (sector_head >> 24) & 0xf);
        command.command = ATA_WRITE;
        disk_cmd_out(&command);
        sector_left -= command.count;
        sector_done += command.count;
        sector_head += command.count;

        while (command.count > 0) {
            int bytes = (bytes_left < SECTOR_SIZE) ? bytes_left : SECTOR_SIZE;
            // 等待硬盘响应
            if (!waitfor(STATUS_BSY, 0, HD_TIMEOUT)) {
                return;
            }
            // 向硬盘写入数据
            port_write(REG_DATA, la, bytes);
            interrupt_wait();
            la += bytes;
            command.count -= 1;
            bytes_left -= bytes;
        }
    }
}
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
    // 就说感觉不太对,之前不触发这个中断的原因是从片设置有问题,硬盘中断被屏蔽了
    interrupt_wait();

    port_read(REG_DATA, disk_buffer, SECTOR_SIZE);

    // 把磁盘信息转化成纯ascii字符串(存放在disk_buffer后512字节中)
    // 转化的过程中,字符串的位置是固定的,下面这一段是借鉴orange的
    // 由于磁盘数据以小端方式存放,而使用char数组获取数据有点麻烦
    // 把char数组指针转化为u16指针来操作
    u16* buf_ptr = (u16*)disk_buffer;
    char* result_buf = &disk_buffer[512];
    int i, k;
    char s[64];
    struct iden_info_ascii {
        int idx;
        int len;
        char* desc;
    } iinfo[] = {{10, 20, "HD SN: "},     /* Serial number in ASCII */
                 {27, 40, "HD Model: "}}; /* Model number in ASCII */

    // 转化相关信息
    for (k = 0; k < sizeof(iinfo) / sizeof(iinfo[0]); k++) {
        char* p = (char*)&buf_ptr[iinfo[k].idx];

        // 用这种奇怪的方式复制字符串是因为磁盘传过来的数据是小端存放
        for (i = 0; i < iinfo[k].len / 2; i++) {
            s[i * 2 + 1] = *p++;
            s[i * 2] = *p++;
        }
        s[i * 2] = 0;

        result_buf = strcpy(result_buf, iinfo[k].desc);
        result_buf = strcpy(result_buf, s);
        *(result_buf++) = '\n';
    }
    *result_buf = 0;

    // 获取其他信息
    result_buf = strcpy(result_buf, "LBA supported: ");
    if (buf_ptr[49] & 0x0200)
        result_buf = strcpy(result_buf, "yes \n");
    else
        result_buf = strcpy(result_buf, "no \n");

    result_buf = strcpy(result_buf, "LBA48 supported: ");
    if (buf_ptr[83] & 0x0400)
        result_buf = strcpy(result_buf, "yes \n");
    else
        result_buf = strcpy(result_buf, "no \n");

    int sectors = (((int)buf_ptr[61] << 16) + buf_ptr[60]) / 2048;
    result_buf = strcpy(result_buf, "Disk size (MB) : ");
    result_buf = itoa(sectors, result_buf);
    *result_buf = 0;

    // 把信息送给调用者
    void* la = (void*)va2la(message->u.disk_message.pid,
                            message->u.disk_message.buffer);
    u32 bytes_count = (SECTOR_SIZE > message->u.disk_message.bytes_count)
                          ? message->u.disk_message.bytes_count
                          : SECTOR_SIZE;
    phys_copy(la, (void*)va2la(PID_DISK_SERVER, &disk_buffer[512]),
              bytes_count);
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
    msg.source = PID_DISK_SERVER;
    msg.type = PID_DISK_SERVER;
    sys_sendrec(RECEIVE, INTERRUPT, &msg, PID_DISK_SERVER);
}
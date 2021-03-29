#include "filesystem.h"

// 文件系统变量
static struct super_block FS_superblock;
// 组描述符缓冲区(预留1kb),组描述符数量
static int group_descriptor_count = 0;
static struct group_descriptor FS_groupdescriptor[32];
// 两个bitmap
static u8 FS_block_bitmap[32][1024];
static u8 FS_inode_bitmap[32][1024];
// inode table存放在6mb~10mb处
static const struct inode* FS_inode_table = (struct inode*)0x600000;
// 文件系统缓冲区放在10mb~20mb
static const char* FS_buffer = (char*)0xa00000;
// 根目录inode
static struct inode FS_root_inode;

// 入口函数
void FS_server() {
    MESSAGE message;
    FS_init();

    memset(&message, 0, sizeof(message));

    while (1) {
        sys_sendrec(RECEIVE, ANY, &message, PID_FS_SERVER);
        // 对收到的信息进行判断,排除中断信息(否则会出现试图向中断发送信息的情况)
        if ((message.source < 0) || (message.source >= MAX_PROCESS_NUM))
            continue;

        int src = message.source;
        int result = 0;

        switch (message.u.fs_message.function) {
            case FS_ROOT:
                // 打开根目录
                result = FS_get_root(&message);
                break;

            default:
                break;
        }
        message.source = FILE_SYSTEM;
        message.type = SERVER_FS;
        message.u.disk_message.result = result;
        sys_sendrec(SEND, src, &message, PID_FS_SERVER);
        memset(&message, 0, sizeof(message));
    }
}

// 打开根目录
// 返回值含义
// result=1 --> 成功读取
// result=2 --> 根目录大小小于要读取的字节数,只读取根目录大小内容
int FS_get_root(MESSAGE* message) {
    // 1. 复制文件描述符
    *(message->u.fs_message.fd->fd_inode) = FS_root_inode;
    message->u.fs_message.fd->fd_pos = 0;

    // 2. 复制一部分根目录
    // 2.1 参数准备
    int bytes_left = message->u.fs_message.count;
    int result = (bytes_left > FS_root_inode.i_size) ? 2 : 1;
    bytes_left =
        (bytes_left > FS_root_inode.i_size) ? FS_root_inode.i_size : bytes_left;
    char* buffer = (char*)va2la(message->u.fs_message.pid,
                                (void*)message->u.fs_message.buffer);
    DIR_ENTRY* temp = (DIR_ENTRY*)&buffer;

    // 2.1 读取前12个block(直接索引)
    for (int block_index = 0; (bytes_left > 0) && block_index < 12;
         block_index++) {
        // 每次最多读取一个block
        int bytes_to_read = (bytes_left > block_size) ? block_size : bytes_left;
        bytes_left -= bytes_to_read;

        // 磁盘操作
        FS_read_disk(B2S(FS_root_inode.i_block[block_index]), buffer,
                     bytes_to_read);
        buffer += bytes_to_read;
    }

    // 2.2 读取第13个block(一级间接索引)
    // 2.3 读取第14个block(二级间接索引)
    // 2.4 读取第15个block(三级间接索引)
    return result;
}

#pragma region 子函数

int FS_get_inode(u32 inode_index, struct inode* inode_buf) {
    // 序号越界
    if (inode_index >= FS_superblock.s_inodes_count) return 0;

    phys_copy((void*)inode_buf, (void*)&FS_inode_table[inode_index - 1],
              inode_size);
}

// 读磁盘
int FS_read_disk(u32 sector_head, char* buffer, int count) {
    MESSAGE read_message;
    read_message.source = PID_FS_SERVER;
    read_message.type = SERVER_DISK;
    read_message.u.disk_message.pid = PID_FS_SERVER;
    read_message.u.disk_message.function = DISK_READ;
    read_message.u.disk_message.bytes_count = count;
    read_message.u.disk_message.buffer = buffer;
    read_message.u.disk_message.sector_head = sector_head;
    sys_sendrec(SEND, SERVER_DISK, &read_message, PID_FS_SERVER);
    sys_sendrec(RECEIVE, SERVER_DISK, &read_message, PID_FS_SERVER);
    return read_message.u.disk_message.result;
}
// 写磁盘
int FS_write_disk(u32 sector_head, char* buffer, int count) {
    MESSAGE read_message;
    read_message.source = PID_FS_SERVER;
    read_message.type = SERVER_DISK;
    read_message.u.disk_message.pid = PID_FS_SERVER;
    read_message.u.disk_message.function = DISK_WRITE;
    read_message.u.disk_message.bytes_count = count;
    read_message.u.disk_message.buffer = buffer;
    read_message.u.disk_message.sector_head = sector_head;
    sys_sendrec(SEND, SERVER_DISK, &read_message, PID_FS_SERVER);
    sys_sendrec(RECEIVE, SERVER_DISK, &read_message, PID_FS_SERVER);
    return read_message.u.disk_message.result;
}

void FS_init() {
    u32 block_index = 1;
    // 1. 读取超级块
    FS_read_disk(B2S(block_index), (char*)&FS_superblock, block_size);

    // 2. 读取组描述符
    group_descriptor_count =
        (FS_superblock.s_blocks_count / FS_superblock.s_blocks_per_group);
    block_index = 2;
    FS_read_disk(B2S(block_index), (char*)&FS_groupdescriptor, block_size);

    // static u8 FS_block_bitmap[32][1024];
    // static u8 FS_inode_bitmap[32][1024];
    // static const struct inode* FS_inode_table = (struct inode*)0x600000;

    // 3. 读取FS_block_bitmap, FS_inode_bitmap 和 FS_inode_table
    // 有多少个group就需要读取几次
    int inode_table_size_per_group =
        FS_superblock.s_inodes_per_group * inode_size;
    int inode_table_des_index = 0;
    for (int i = 0; i < 2 /*group_descriptor_count*/; i++) {
        // 1.1 初始化FS_block_bitmap
        block_index = FS_groupdescriptor[i].bg_block_bitmap;
        FS_read_disk(B2S(block_index), (char*)&FS_block_bitmap[i], block_size);

        // 1.2 初始化FS_inode_bitmap
        block_index = FS_groupdescriptor[i].bg_inode_bitmap;
        FS_read_disk(B2S(block_index), (char*)&FS_inode_bitmap[i], block_size);

        // 1.3 初始化FS_inode_table
        block_index = FS_groupdescriptor[i].bg_inode_table;
        FS_read_disk(B2S(block_index),
                     (char*)&FS_inode_table[inode_table_des_index],
                     inode_table_size_per_group);
        inode_table_des_index += FS_superblock.s_inodes_per_group;
    }

    // 4. 初始化根目录inode -- FS_root_inode
    FS_get_inode(root_inode_index, &FS_root_inode);
}

#pragma endregion
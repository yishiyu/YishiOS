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
// 根目录inode
static struct inode FS_root_inode;
// 大小为1kb的一级索引缓冲区
static u32 FS_first_index_block[256];

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
                result = FS_read_file(&message);
                break;
            case FS_READ:
                // 读取文件
                result = FS_read_file(&message);
                break;
            case FS_CD:
                // 切换文件夹
                // 1. 搜索新文件夹
                result = FS_search_file(&message, FILE_TYPE_DIR);
                // 2. 成功找到文件夹,读取新文件夹
                if (result == FILE_TYPE_DIR) {
                    result = FS_read_file(&message);
                }
            case FS_OPENFILE:
                // 打开文件(读取大小用来指示是否读取文件)
                // 1. 搜索新文件
                result = FS_search_file(&message, FILE_TYPE_REG);
                // 2. 成功找到文件,读取新文件
                if (result == FILE_TYPE_REG) {
                    result = FS_read_file(&message);
                }
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
// 仅仅返回文件描述符,读取文件被分离到读取文件函数中
int FS_get_root(MESSAGE* message) {
    // 复制文件描述符
    *(message->u.fs_message.fd->fd_inode) = FS_root_inode;
    message->u.fs_message.fd->fd_pos = 0;

    return 1;
}

// 读取文件
// 输入参数:  文件描述符指针, 读取数据量, 数据缓冲区, 进程pid
// 函数功能: 读取指定文件到缓冲区中
// 返回值含义: 1 --> 成功读取   0 --> 文件剩余部分不足,只读取剩余部分
// 现在只支持读取前12个block,也就是12kb
int FS_read_file(MESSAGE* message) {
    // 1. 参数准备
    // 1.1 计算文件剩余大小
    int bytes_left = (message->u.fs_message.fd->fd_inode->i_size -
                      message->u.fs_message.fd->fd_pos);
    int block_index = 0;
    // 1.2 文件剩余部分是否足够大 | 1 --> 足够大 |  0 --> 不够大  |
    int result = (bytes_left > message->u.fs_message.count);
    // 1.3 取(剩余部分大小) (要读取大小)二者较小者
    bytes_left = (result) ? message->u.fs_message.count : bytes_left;
    // 1.4 计算缓冲区位置
    char* buffer = (char*)va2la(message->u.fs_message.pid,
                                (void*)message->u.fs_message.buffer);

    // 2.1 读取前12个block(直接索引)
    for (block_index = 0; (bytes_left > 0) && block_index < 12; block_index++) {
        // 每次最多读取一个block
        int bytes_to_read = (bytes_left > BLOCK_SIZE) ? BLOCK_SIZE : bytes_left;
        bytes_left -= bytes_to_read;

        // 磁盘操作
        FS_read_disk(
            B2S(message->u.fs_message.fd->fd_inode->i_block[block_index]),
            buffer, bytes_to_read);
        buffer += bytes_to_read;
    }

    // 2.2 读取第13个block(一级间接索引)
    if ((bytes_left > 0) && block_index == 12) {
        // 读取一级索引块
        FS_read_disk(
            B2S(message->u.fs_message.fd->fd_inode->i_block[block_index]),
            &FS_first_index_block, BLOCK_SIZE);

        // 根据一级索引块读取文件
        // 一级索引块中最多放1kb/4b = 256 个block索引号
        for (block_index = 0; (bytes_left > 0) && block_index < 256;
             block_index++) {
            // 每次最多读取一个block
            int bytes_to_read =
                (bytes_left > BLOCK_SIZE) ? BLOCK_SIZE : bytes_left;
            bytes_left -= bytes_to_read;

            // 磁盘操作
            FS_read_disk(B2S(FS_first_index_block[block_index]), buffer,
                         bytes_to_read);
            buffer += bytes_to_read;
        }
    }

    // 2.3 读取第14个block(二级间接索引)
    // 2.4 读取第15个block(三级间接索引)
    return result;
}

// 搜索文件(文件夹本身也是一种文件夹)
// 输入参数:  文件夹缓冲区指针, 缓冲区大小, 进程pid, 文件名字,  文件类型
// 文件类型含义:  | 0 --> 文件不存在 | 1 --> 普通文件  | 2 --> 文件夹 |
// 函数功能:  在当前文件夹中搜索指定文件(或文件夹), 返回文件描述符
// 本函数本身只负责寻找文件描述符(如果目标文件存在的话)
// 读取文件的任务交给FS_read_file()
int FS_search_file(MESSAGE* message, u8 file_type) {
    // 1. 参数准备
    // 1.1 指针地址转化
    char* directory_buffer = message->u.fs_message.buffer;
    directory_buffer =
        (char*)va2la(message->u.fs_message.pid, (void*)directory_buffer);
    char* file_name = message->u.fs_message.file_name;
    file_name = (char*)va2la(message->u.fs_message.pid, (void*)file_name);
    // 1.2 结构体指针
    DIR_ENTRY* directory_entry;
    int result = 0;

    // 2. 解析目录
    int buffer_size = message->u.fs_message.count;
    int entry_index = 0;
    for (int i = 0; (i < 20) && (entry_index < buffer_size); i++) {
        directory_entry =
            (struct directory_entry*)&directory_buffer[entry_index];
        entry_index += directory_entry->rec_len;
        // 文件为目标文件
        if ((directory_entry->file_type == file_type) &&
            (strcmp(file_name, &directory_entry->name) == 0)) {
            // 修改返回值, 修改文件操作位置, 复制文件inode结构体
            result = file_type;
            message->u.fs_message.fd->fd_pos = 0;
            FS_get_inode(directory_entry->inode,
                         message->u.fs_message.fd->fd_inode);
            break;
        }
    }
    return result;
}

#pragma region 子函数

int FS_get_inode(u32 inode_index, struct inode* inode_buf) {
    // 序号越界
    if (inode_index >= FS_superblock.s_inodes_count) return 0;

    phys_copy((void*)inode_buf, (void*)&FS_inode_table[inode_index - 1],
              INODE_SIZE);
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
    FS_read_disk(B2S(block_index), (char*)&FS_superblock, BLOCK_SIZE);

    // 2. 读取组描述符
    group_descriptor_count =
        (FS_superblock.s_blocks_count / FS_superblock.s_blocks_per_group);
    block_index = 2;
    FS_read_disk(B2S(block_index), (char*)&FS_groupdescriptor, BLOCK_SIZE);

    // static u8 FS_block_bitmap[32][1024];
    // static u8 FS_inode_bitmap[32][1024];
    // static const struct inode* FS_inode_table = (struct inode*)0x600000;

    // 3. 读取FS_block_bitmap, FS_inode_bitmap 和 FS_inode_table
    // 有多少个group就需要读取几次
    int inode_table_size_per_group =
        FS_superblock.s_inodes_per_group * INODE_SIZE;
    int inode_table_des_index = 0;
    for (int i = 0; i < group_descriptor_count; i++) {
        // 1.1 初始化FS_block_bitmap
        block_index = FS_groupdescriptor[i].bg_block_bitmap;
        FS_read_disk(B2S(block_index), (char*)&FS_block_bitmap[i], BLOCK_SIZE);

        // 1.2 初始化FS_inode_bitmap
        block_index = FS_groupdescriptor[i].bg_inode_bitmap;
        FS_read_disk(B2S(block_index), (char*)&FS_inode_bitmap[i], BLOCK_SIZE);

        // 1.3 初始化FS_inode_table
        block_index = FS_groupdescriptor[i].bg_inode_table;
        FS_read_disk(B2S(block_index),
                     (char*)&FS_inode_table[inode_table_des_index],
                     inode_table_size_per_group);
        inode_table_des_index += FS_superblock.s_inodes_per_group;
    }

    // 4. 初始化根目录inode -- FS_root_inode
    FS_get_inode(ROOT_INODE_INDEX, &FS_root_inode);
}

#pragma endregion
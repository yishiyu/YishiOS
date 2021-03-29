// input子系统

#ifndef YISHIOS_FILE_H
#define YISHIOS_FILE_H

#include "global.h"
#include "struct.h"
#include "type.h"

// 关于文件系统的数据定义
// 参考自<<深入理解Linux内核>> 第18章
// 超级块结构体
struct super_block {
    u32 s_inodes_count;
    u32 s_blocks_count;
    u32 s_r_blocks_count;
    u32 s_free_blocks_count;
    u32 s_free_inodes_count;
    u32 s_first_data_block;
    u32 s_log_block_size;
    u32 s_log_frag_size;
    u32 s_blocks_per_group;
    u32 s_frags_per_group;
    u32 s_inodes_per_group;
    u32 s_mtime;
    u32 s_wtime;
    u16 s_mnt_count;
    u16 s_max_mnt_count;
    u16 s_magic;
    u16 s_state;
    u16 s_errors;
    u16 s_minor_rev_level;
    u32 s_lastcheck;
    u32 s_checkinterval;
    u32 s_creator_os;
    u32 s_rev_level;
    u16 s_def_resuid;
    u16 s_def_resgid;
    u32 s_first_ino;
    u16 s_inode_size;
    u16 s_block_group_nr;
    u32 s_feature_compat;
    u32 s_feature_incompat;
    u32 s_feature_ro_compat;
    u8 s_uuid[16];
    char s_volume_name[16];
    char s_last_mounted[64];
    u32 s_algorithm_usage_bitmap;
    u8 s_prealloc_blocks;
    u8 s_prealloc_dir_blocks;
    u16 s_padding;
    u32 s_reserved[204];
};
// 组描述符
struct group_descriptor {
    u32 bg_block_bitmap;
    u32 bg_inode_bitmap;
    u32 bg_inode_table;
    u16 bg_free_blocks_count;
    u16 bg_free_inodes_count;
    u16 bg_used_dirs_count;
    u16 bg_pad;
    u32 bg_reserved[3];
};
typedef struct directory_entry {
    u32 inode;
    u16 rec_len;
    u8 name_len;
    u8 file_type;
    char name;  // 长度==name_len
} DIR_ENTRY;

// 入口函数
void FS_server();

// 功能函数
int FS_get_root(MESSAGE* message);
int FS_read_file(MESSAGE* message);
int FS_search_file(MESSAGE* message, u8 file_type);

// 子函数
int FS_get_inode(u32 inode_index, struct inode* inode_buf);
int FS_read_disk(u32 sector_head, char* buffer, int count);
int FS_write_disk(u32 sector_head, char* buffer, int count);
void FS_init();

// 关于文件系统的宏定义
// block号转sector号
#define B2S(block) (block * 2)
#define BLOCK_SIZE (0x400)
#define INODE_SIZE (0x80)
#define ROOT_INODE_INDEX 2
// 文件系统提供的功能
#define FS_ROOT 0x01      // 打开根目录
#define FS_READ 0x02      // 读取文件
#define FS_CD 0x03        // 切换文件夹
#define FS_OPENFILE 0x04  // 打开文件

// 文件类型
#define FILE_TYPE_REG 0x01  // regular 普通文件
#define FILE_TYPE_DIR 0x02  // directory 文件夹文件

#endif

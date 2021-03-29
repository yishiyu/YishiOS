#ifndef YISHIOS_DISK_H
#define YISHIOS_DISK_H

#include "global.h"
#include "macro.h"
#include "struct.h"
#include "string.h"

// 磁盘命令结构体
typedef struct disk_cmd {
    u8 features;
    u8 count;
    u8 lba_low;
    u8 lba_mid;
    u8 lba_high;
    u8 device;
    u8 command;
} DISK_CMD;

extern u8 disk_status;

// 磁盘服务器
void disk_server();
void disk_init();
// 磁盘操作函数
void disk_open(MESSAGE* message);
void disk_close(MESSAGE* message);
void disk_read(MESSAGE* message);
void disk_write(MESSAGE* message);
void disk_info(MESSAGE* message);
// 磁盘命令输出函数
int disk_cmd_out(DISK_CMD* command);

// 其他功能函数
// 磁盘超时/等待函数
int waitfor(int mask, int val, int timeout);
// 等待磁盘中断函数
void interrupt_wait();
// 端口读写函数
void port_read(u16 port, void* buf, int n);
void port_write(u16 port, void* buf, int n);

// 磁盘服务器目的是封装磁盘读写操作
// 所以尽量不暴露内部细节
// 把只有磁盘服务器用到的定义写在这个头文件内
// 磁盘操作宏定义
#define DISK_OPEN 0
#define DISK_CLOSE 1
#define DISK_READ 2
#define DISK_WRITE 3
#define DISK_INFO 4

// 磁盘超时等待时间
#define HD_TIMEOUT 10000  // 单位毫秒,即10s

// OUTPUT --> 输出命令
// INPUT --> 读取数据

// 主ATA通道磁盘控制寄存器,详见Orange P326
// bit位 --> 功能           OUTPUT
// 1 --> 中断使能,0开启,1关闭
// 2 --> reset位,0-1-0,重启磁盘
// 7 --> 高位LBA设置完毕
#define REG_DEV_CTRL 0x3F6

// 数据寄存器            INPUT/OUTPUT
#define REG_DATA 0x1F0

// 属性/错误码寄存器        INPUT/OUTPUT
// 输出的时候作为寄存器
// 出现磁盘错误的时候作为错误码寄存器
#define REG_FEATURES 0x1F1
#define REG_ERROR REG_FEATURES

// 读写扇区数                   INPUT/OUTPUT
#define REG_SECTOR_COUNT 0x1F2

// LBA 地址寄存器            INPUT/OUTPUT
// 每个寄存器存8个bit,每个寄存器可以使用两次
// LBA最大为48bit,最大支持 128 PB 的磁盘容量
// 但我们只需要使用28位就够了(128GB),还有4个bit存放在DEVICE寄存器中
// 有两种寻址防止,一种是LBA式,一种是磁头-柱面-扇区式
#define REG_LBA_LOW 0x1F3   //   LBA 0~7位 / 扇区号
#define REG_LBA_MID 0x1F4   //   LBA 8~15位 / 柱面号低8位
#define REG_LBA_HIGH 0x1F5  //  LBA 16~23位 / 柱面号高8位

// 设备寄存器              INPUT/OUTPUT
// bit位 --> 功能           INPUT/OUTPUT
// 0~3  --> LBA 的24~27位 / 磁头号
//      4  --> 设备选择位, 0=master,1=slave
//      6  --> 寻址方式选择,0=CHS式,1=LBA式
#define REG_DEVICE 0x1F6

// 磁盘状态寄存器       INPUT
// bit位 --> 功能           INPUT
// 0 --> 错误位,磁盘出现错误
// 3 --> 数据传送准备就绪
// 6 --> 磁盘准备就绪
// 7 --> BUSY位,此位为1时,磁盘忙,其他位无效
#define REG_STATUS 0x1F7
#define STATUS_BSY 0x80   // 磁盘空闲
#define STATUS_DRDY 0x40  // 磁盘就绪
#define STATUS_DFSE 0x20
#define STATUS_DSC 0x10
#define STATUS_DRQ 0x08  // 数据传输准备就绪
#define STATUS_CORR 0x04
#define STATUS_IDX 0x02
#define STATUS_ERR 0x01  // 磁盘错误

// 磁盘命令寄存器,和状态寄存器是同一个  OUTPUT
#define REG_CMD REG_STATUS

// 把信息转化为设备寄存器格式
#define MAKE_DEVICE_REG(lba, drv, lba_highest) \
    (((lba) << 6) | ((drv) << 4) | (lba_highest & 0xF) | 0xA0)

// 部分命令
#define ATA_INFO 0xEC   // 获取磁盘信息
#define ATA_READ 0x20   // 读取磁盘
#define ATA_WRITE 0x30  // 写入磁盘

// 扇区大小
#define SECTOR_SIZE 512
#define SECTOR_SIZE_SHIFT 9

#endif
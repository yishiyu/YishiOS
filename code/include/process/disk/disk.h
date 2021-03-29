#ifndef YISHIOS_DISK_H
#define YISHIOS_DISK_H

#include "global.h"

// 磁盘服务器
void disk_server();

// 磁盘服务器目的是封装磁盘读写操作
// 所以尽量不暴露内部细节  
// 把只有磁盘服务器用到的定义写在这个头文件内




#endif
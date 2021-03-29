// 定义端口操作函数
#ifndef	YISHIOS_PORT_H
#define	YISHIOS_PORT_H

#include "global.h"

void	out_byte(u16 port, u8 value);
u8	in_byte(u16 port);

#endif
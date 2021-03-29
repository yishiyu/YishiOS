// 用于定义基本数据类型的别名
#ifndef YISHIOS_TYPE_H
#define YISHIOS_TYPE_H

//三种基本数据类型
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

//定义 int_handler 类型为无返回值无参数的函数指针
//同理task_f, 其作用是提供指向系统最初进程的指针(服务于s_task结构体)
//同理irq_handler
typedef void (*int_handler)();
typedef void (*task_f)();
typedef	void	(*irq_handler)	(int irq);

#endif
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
typedef void*	system_call;

//读取键盘相关
// key_type表明读取键盘得到的数据的类型
// key_type = 0  ==> ascii码
// key_type = 1  ==> 特殊信息
typedef char key_type;
typedef char key_value;


#endif
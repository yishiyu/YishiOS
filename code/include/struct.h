// 定义各种结构体
#ifndef	YISHIOS_STRUCT_H
#define	YISHIOS_STRUCT_H

#include "type.h"

//存储段描述符/系统段描述符
typedef struct s_descriptor {
    u16 limit_low;         // Limit
    u16 base_low;         // Base
    u8 base_mid;          // Base
    u8 attr1;                   // P(1) DPL(2) DT(1) TYPE(4)
    u8 limit_high_attr2;  // G(1) D(1) 0(1) AVL(1) LimitHigh(4)
    u8 base_high;         // Base
} DESCRIPTOR;


//门描述符(/调用门中断门/陷阱门/任务门)
typedef struct s_gate
{
	u16	offset_low;	    // Offset Low
	u16	selector;	       //Selector
	u8	dcount;		        //调用门中切换堆栈时要复制的双字参数的个数
	u8	attr;		            //P(1) DPL(2) DT(1) TYPE(4)
	u16	offset_high;  //Offset High
}GATE;

#endif
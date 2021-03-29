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

//TSS结构体
typedef struct s_tss {
	u32	backlink;
	u32	esp0;		//内核段栈指针
	u32	ss0;		 //内核段栈基址
	u32	esp1;
	u32	ss1;
	u32	esp2;
	u32	ss2;
	u32	cr3;
	u32	eip;
	u32	flags;
	u32	eax;
	u32	ecx;
	u32	edx;
	u32	ebx;
	u32	esp;
	u32	ebp;
	u32	esi;
	u32	edi;
	u32	es;
	u32	cs;
	u32	ss;
	u32	ds;
	u32	fs;
	u32	gs;
	u32	ldt;
	u16	trap;
	u16	iobase;	// I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图 
}TSS;


#endif
// 全局变量定义在这里
#ifndef	YISHIOS_GLOBAL_H
#define	YISHIOS_GLOBAL_H

#include "macro.h"
#include "struct.h"
#include "type.h"


//当前屏幕指针位置
int disp_pos;

//kernel中的GDT指针,修改后会指向下面的全局描述符表
//全局描述符表
// 0~15:Limit  16~47:Base 共48位
u8		gdt_ptr[6];
DESCRIPTOR	gdt[GDT_SIZE];

//kernel中的IDT指针,修改后会指向下面的中断描述符表
//中断描述符表
//0~15:Limit  16~47:Base 共48位
u8      idt_ptr[6];
GATE    idt[IDT_SIZE];

#endif
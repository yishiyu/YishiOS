// kernel.c 中的二级函数
//创建这个文件的目的是为了减小kernel.c的体积
#ifndef YISHIOS_FUNC_H
#define YISHIOS_FUNC_H

#include "display.h"
#include "global.h"
#include "mem.h"

// 中断处理函数
// 具体实现在kernel.asm中
void divide_error();
void single_step_exception();
void nmi();
void breakpoint_exception();
void overflow();
void bounds_check();
void inval_opcode();
void copr_not_available();
void double_fault();
void copr_seg_overrun();
void inval_tss();
void segment_not_present();
void stack_exception();
void general_protection();
void page_fault();
void copr_error();
void hwint00();
void hwint01();
void hwint02();
void hwint03();
void hwint04();
void hwint05();
void hwint06();
void hwint07();
void hwint08();
void hwint09();
void hwint10();
void hwint11();
void hwint12();
void hwint13();
void hwint14();
void hwint15();
void sys_call();

//本文件内使用的二级子函数
void init_8259A();
void init_idt_desc(unsigned char vector, u8 desc_type, int_handler handler,
                   unsigned char privilege);
void init_idt();
void init_descriptor(DESCRIPTOR *p_desc, u32 base, u32 limit, u16 attribute);
u32 seg2phys(u16 seg);

void enable_irq();
void disable_irq();
void put_irq_handler(int irq, irq_handler handler);

#endif
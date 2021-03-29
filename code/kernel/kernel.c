// ======================================================================
// 仅在kernel.asm中使用的内核函数
// ======================================================================

#include "func.h"

//文件内函数声明
//向其他文件提供的功能函数
void init_gdt();
void init_interupt();
void exception_handler(int vec_no, int err_code, int eip, int cs, int eflags);
void init_tss();

// 初始化gdt
void init_gdt() {
    //本函数作用: 将原本位于loader区域(以后不再用了)的gdt转移到kernel区域
    // 1. 把原本gdt的内容复制到新gdt中
    // 2. 把新gdt的地址放入gdt寄存器中,这一步需要用全局变量gdt_ptr进行中转
    // 变量说明:
    // gdt => 新的全局描述符结构体数组
    // gdt_ptr => 用于临时中转gdt地址的全局变量
    // 0~15:Limit  16~47:Base 共48位

    //第一步 将 LOADER 中的 GDT 复制到新的 GDT 中
    //目的地址
    //源地址 把最后32位转换为无类型指针
    //复制的字节数 16为Limit部分
    memcpy(&gdt, (void *)(*((u32 *)(&gdt_ptr[2]))),
           *((u16 *)(&gdt_ptr[0])) + 1);

    //第二步 把新GDt的地址通过gdt_ptr传送给kernel.asm
    // dt_ptr[6] 共 6 个字节：0~15:Limit  16~47:Base
    u16 *p_gdt_limit = (u16 *)(&gdt_ptr[0]);
    u32 *p_gdt_base = (u32 *)(&gdt_ptr[2]);
    *p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
    *p_gdt_base = (u32)&gdt;
}

// 初始化中断
void init_interupt() {
    //本函数作用:初始化并打开中断
    // 1. 初始化8259A芯片
    // 2. 初始化中断描述符表
    // 3. 设置idt寄存器,需要通过idt_ptr变量把idt的地址传送给kernel.asm

    //第一步 初始化8259A芯片
    init_8259A();

    //第二步 初始化中断描述符表
    init_idt();

    //第三步 通过idt_ptr传送idt的地址
    // idt_ptr[6] 共 6 个字节：0~15:Limit  16~47:Base
    u16 *p_idt_limit = (u16 *)(&idt_ptr[0]);
    u32 *p_idt_base = (u32 *)(&idt_ptr[2]);
    *p_idt_limit = IDT_SIZE * sizeof(GATE) - 1;
    *p_idt_base = (u32)&idt;
}

// CPU保留中断处理函数
//改自Orange's OS by 于渊
void exception_handler(int vec_no, int err_code, int eip, int cs, int eflags) {
    int i;
    int text_color = 0x74; /* 灰底红字 */

    char *err_msg[] = {"#DE Divide Error",
                       "#DB RESERVED",
                       "—  NMI Interrupt",
                       "#BP Breakpoint",
                       "#OF Overflow",
                       "#BR BOUND Range Exceeded",
                       "#UD Invalid Opcode (Undefined Opcode)",
                       "#NM Device Not Available (No Math Coprocessor)",
                       "#DF Double Fault",
                       "    Coprocessor Segment Overrun (reserved)",
                       "#TS Invalid TSS",
                       "#NP Segment Not Present",
                       "#SS Stack-Segment Fault",
                       "#GP General Protection",
                       "#PF Page Fault",
                       "—  (Intel reserved. Do not use.)",
                       "#MF x87 FPU Floating-Point Error (Math Fault)",
                       "#AC Alignment Check",
                       "#MC Machine Check",
                       "#XF SIMD Floating-Point Exception"};

    /* 通过打印空格的方式清空屏幕的前五行，并把 disp_pos 清零 */
    disp_pos = 0;
    for (i = 0; i < 80 * 5; i++) {
        disp_str(" ");
    }
    disp_pos = 0;

    disp_color_str("Exception! --> ", text_color);
    disp_color_str(err_msg[vec_no], text_color);
    disp_color_str("\n\n", text_color);
    disp_color_str("EFLAGS:", text_color);
    disp_int(eflags);
    disp_color_str("CS:", text_color);
    disp_int(cs);
    disp_color_str("EIP:", text_color);
    disp_int(eip);

    if (err_code != 0xFFFFFFFF) {
        disp_color_str("Error code:", text_color);
        disp_int(err_code);
    }
}

//初始化TSS段
void init_tss() {
    //填充GDT中的TSS段描述符
    memset(&tss, 0, sizeof(tss));
    tss.ss0 = SELECTOR_KERNEL_DS;
    init_descriptor(&gdt[INDEX_TSS],
                    vir2phys(seg2phys(SELECTOR_KERNEL_DS), &tss),
                    sizeof(tss) - 1, DA_386TSS);
    tss.iobase = sizeof(tss); /* 没有I/O许可位图 */
}

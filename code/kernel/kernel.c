// ======================================================================
// 仅在kernel.asm中使用的内核函数
// ======================================================================

#include "global.h"
#include "memory.h"
#include "display.h"
#include "port.h"

//文件内函数声明
//向其他文件提供的功能函数
void init_gdt();
void init_interupt();
void exception_handler(int vec_no, int err_code, int eip, int cs, int eflags);

//本文件内使用的二级子函数
void init_8259A();
void init_idt_desc(unsigned char vector, u8 desc_type, int_handler handler,
                   unsigned char privilege);
void init_idt();

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

//初始化中断描述符
// vector => 中断号
// desc_type => 描述符类型
// handler => 中断处理函数
// privilege => 描述符的优先级
void init_idt_desc(unsigned char vector, u8 desc_type, int_handler handler,
                   unsigned char privilege) {
    GATE *p_gate = &idt[vector];
    u32 base = (u32)handler;
    p_gate->offset_low = base & 0xFFFF;
    p_gate->selector = SELECTOR_KERNEL_CS;
    p_gate->dcount = 0;
    p_gate->attr = desc_type | (privilege << 5);
    p_gate->offset_high = (base >> 16) & 0xFFFF;
}

//初始化8259A芯片
//其中的信号发送顺序不可更改
//改自Orange's OS by 于渊
void init_8259A() {
    // Master 8259, ICW1
    out_byte(INT_Master_CTL, 0x11);

    // Slave  8259, ICW1
    out_byte(INT_Slave_CTL, 0x11);

    // Master 8259, ICW2. 设置 '主8259' 的中断入口地址为 0x20
    out_byte(INT_Master_CTL_MASK, INT_VECTOR_IRQ0);

    // Slave  8259, ICW2. 设置 '从8259' 的中断入口地址为 0x28
    out_byte(INT_Slave_CTLMASK, INT_VECTOR_IRQ8);

    // Master 8259, ICW3
    //意思是从芯片接在主芯片的IR2位置
    out_byte(INT_Master_CTL_MASK, 0x4);

    // Slave  8259, ICW3
    //意思是从芯片接在主芯片的IR2位置
    out_byte(INT_Slave_CTLMASK, 0x2);

    // Master 8259, ICW4
    // 设置芯片工作在80x86模式
    out_byte(INT_Master_CTL_MASK, 0x1);

    // Slave  8259, ICW4
    // 设置芯片工作在80x86模式
    out_byte(INT_Slave_CTLMASK, 0x1);

    // Master 8259, OCW1
    // 主芯片仅打开键盘中断
    out_byte(INT_Master_CTL_MASK, 0xFD);

    // Slave  8259, OCW1
    //从芯片关闭所有中断
    out_byte(INT_Slave_CTLMASK, 0xFF);
}

// 初始化中断描述符表
// 改自Orange's OS by 于渊
void init_idt() {
    // 全部初始化成中断门(没有陷阱门)
    init_idt_desc(INT_VECTOR_DIVIDE, DA_386IGate, divide_error, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_DEBUG, DA_386IGate, single_step_exception,
                  PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_NMI, DA_386IGate, nmi, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_BREAKPOINT, DA_386IGate, breakpoint_exception,
                  PRIVILEGE_USER);

    init_idt_desc(INT_VECTOR_OVERFLOW, DA_386IGate, overflow, PRIVILEGE_USER);

    init_idt_desc(INT_VECTOR_BOUNDS, DA_386IGate, bounds_check, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_INVAL_OP, DA_386IGate, inval_opcode,
                  PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_COPROC_NOT, DA_386IGate, copr_not_available,
                  PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_DOUBLE_FAULT, DA_386IGate, double_fault,
                  PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_COPROC_SEG, DA_386IGate, copr_seg_overrun,
                  PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_INVAL_TSS, DA_386IGate, inval_tss, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_SEG_NOT, DA_386IGate, segment_not_present,
                  PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_STACK_FAULT, DA_386IGate, stack_exception,
                  PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_PROTECTION, DA_386IGate, general_protection,
                  PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_PAGE_FAULT, DA_386IGate, page_fault,
                  PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_COPROC_ERR, DA_386IGate, copr_error,
                  PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ0 + 0, DA_386IGate, hwint00, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ0 + 1, DA_386IGate, hwint01, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ0 + 2, DA_386IGate, hwint02, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ0 + 3, DA_386IGate, hwint03, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ0 + 4, DA_386IGate, hwint04, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ0 + 5, DA_386IGate, hwint05, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ0 + 6, DA_386IGate, hwint06, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ0 + 7, DA_386IGate, hwint07, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ8 + 0, DA_386IGate, hwint08, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ8 + 1, DA_386IGate, hwint09, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ8 + 2, DA_386IGate, hwint10, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ8 + 3, DA_386IGate, hwint11, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ8 + 4, DA_386IGate, hwint12, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ8 + 5, DA_386IGate, hwint13, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ8 + 6, DA_386IGate, hwint14, PRIVILEGE_KRNL);

    init_idt_desc(INT_VECTOR_IRQ8 + 7, DA_386IGate, hwint15, PRIVILEGE_KRNL);
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

// 8259A中断默认处理函数
void spurious_irq(int irq)
{
        disp_str("spurious_irq: ");
        disp_int(irq);
        disp_str("\n");
}

// kernel.c 中的二级函数
//创建这个文件的目的是为了减小kernel.c的体积

#include "func.h"

//初始化描述符
void init_descriptor(DESCRIPTOR *p_desc, u32 base, u32 limit, u16 attribute) {
    p_desc->limit_low = limit & 0x0FFFF;      // 段界限 1		(2 字节)
    p_desc->base_low = base & 0x0FFFF;        // 段基址 1		(2 字节)
    p_desc->base_mid = (base >> 16) & 0x0FF;  // 段基址 2		(1 字节)
    p_desc->attr1 = attribute & 0xFF;         // 属性 1
    p_desc->limit_high_attr2 =
        ((limit >> 16) & 0x0F) | (attribute >> 8) & 0xF0;  // 段界限 2 + 属性 2
    p_desc->base_high = (base >> 24) & 0x0FF;  // 段基址 3		(1 字节)
}

//初始化中断描述符
void init_idt_desc(unsigned char vector, u8 desc_type, int_handler handler,
                   unsigned char privilege) {
    // vector => 中断号
    // desc_type => 描述符类型
    // handler => 中断处理函数
    // privilege => 描述符的优先级
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

// 8259A中断默认处理函数
void spurious_irq(int irq) {
    disp_str("spurious_irq: ");
    disp_int(irq);
    disp_str("\n");
}

//由段名求绝对地址
u32 seg2phys(u16 seg) {
    DESCRIPTOR *p_dest = &gdt[seg >> 3];

    return (p_dest->base_high << 24) | (p_dest->base_mid << 16) |
           (p_dest->base_low);
}
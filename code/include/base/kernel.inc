; kernel.asm 中的宏定义

; 选择子
SELECTOR_KERNEL_CS	equ	08h
SELECTOR_TSS	equ 020h

; 8259A相关
INT_MASTER_CTL  equ 020h
INT_MASTER_CTL_MASK	 equ 021h
INT_SLAVE_CTL  equ 0a0h
INT_SLAVE_CTL_MASK	 equ 0a1h
EOI		equ	020h

;PCB块的结构相关
P_STACKBASE	equ	0
GSREG		equ	P_STACKBASE
FSREG		equ	GSREG		+ 4
ESREG		equ	FSREG		+ 4
DSREG		equ	ESREG		+ 4
EDIREG		equ	DSREG		+ 4
ESIREG		equ	EDIREG		+ 4
EBPREG		equ	ESIREG		+ 4
KERNELESPREG	equ	EBPREG		+ 4
EBXREG		equ	KERNELESPREG	+ 4
EDXREG		equ	EBXREG		+ 4
ECXREG		equ	EDXREG		+ 4
EAXREG		equ	ECXREG		+ 4
RETADR		equ	EAXREG		+ 4
EIPREG		equ	RETADR		+ 4
CSREG		equ	EIPREG		+ 4
EFLAGSREG	equ	CSREG		+ 4
ESPREG		equ	EFLAGSREG	+ 4
SSREG		equ	ESPREG		+ 4
P_STACKTOP	equ	SSREG		+ 4
P_LDT_SEL	equ	P_STACKTOP
P_LDT		equ	P_LDT_SEL	+ 4

; TSS相关
TSS3_S_SP0	equ	4

; 定义默认的8259A中断处理函数
%macro  hwint_master    1
    ; 保存进程信息
    call	save

    ; 修改中断屏蔽寄存器,禁止当前中断
    in	al, INT_MASTER_CTL_MASK
    or	al, (1 << %1)
    out	INT_MASTER_CTL_MASK, al

    ; 允许8259A接收其他中断
    mov	al, EOI
    out	INT_MASTER_CTL, al
    sti

    ; 根据当前中断号调用相关的处理
    push	%1
    call	[irq_table + 4 * %1]
    pop	ecx

    ; 恢复原本 的中断屏蔽寄存器
    cli
    in	al, INT_MASTER_CTL_MASK
    and	al, ~(1 << %1)
    out	INT_MASTER_CTL_MASK, al
    sti
    ret
%endmacro
%macro  hwint_slave     1
    push    %1
    call    [irq_table + 4 * %1]
    add     esp, 4
    hlt
%endmacro

;========================================
;	内核入口文件
;========================================

; 导入全局变量
	extern	disp_pos
	extern	gdt_ptr
	extern	idt_ptr
	extern	k_reenter
	extern p_proc_ready_head
	extern tss
; 导入全局变量结束

;导入c文件函数
	extern init_gdt
	extern init_interupt
	extern	irq_table
	extern	sys_call_table
	extern exception_handler
	extern init_tss
	extern start_proc
;导入c文件函数结束

;导出中断处理函数
	global restart
	global	divide_error
	global	single_step_exception
	global	nmi
	global	breakpoint_exception
	global	overflow
	global	bounds_check
	global	inval_opcode
	global	copr_not_available
	global	double_fault
	global	copr_seg_overrun
	global	inval_tss
	global	segment_not_present
	global	stack_exception
	global	general_protection
	global	page_fault
	global	copr_error
	global  hwint00
	global  hwint01
	global  hwint02
	global  hwint03
	global  hwint04
	global  hwint05
	global  hwint06
	global  hwint07
	global  hwint08
	global  hwint09
	global  hwint10
	global  hwint11
	global  hwint12
	global  hwint13
	global  hwint14
	global  hwint15
	global	sys_call
;导出中断处理函数结束

%include "kernel.inc"

;堆栈段 ---------------------------------------------------------------------------------------------------------------------------------------------------------------
	[SECTION .bss]
	; resb 意为不初始化定义空间,单位为字节
	; 栈大小为2kb
	Kernel_Stack		resb	2 * 1024		; 栈空间
	StackTop:													; 栈顶		
;堆栈段结束---------------------------------------------------------------------------------------------------------------------------------------------------------


;代码段 ---------------------------------------------------------------------------------------------------------------------------------------------------------------
[section .text]

; 导出程序的入口
global _start

	; 注意! 在使用 C 代码的时候一定要保证 ds, es, ss 这几个段寄存器的值是一样的
	; 因为编译器有可能编译出使用它们的代码, 而编译器默认它们是一样的. 比如串拷贝操作会用到 ds 和 es.
	; 在进入kernel的时候cs, ds, es, fs, ss中的选择子基地址为 00h

_start:

	; 1. 第一步,切换堆栈
	mov	esp, StackTop	; 堆栈在 bss 段中

	; 把当前指针位置恢复成左上角
	mov	dword [disp_pos], 0

	; 2. 第二步,切换gdt
	; 因为原本的gdt位置在kernel中没有变量对应,且以后kernel的活动可能会覆盖掉原本的gdt
	sgdt	[gdt_ptr]
	call	init_gdt
	lgdt	[gdt_ptr]

	;3. 第三部,打开中断
	call	init_interupt
	lidt	[idt_ptr]

	;前面更换了gdtr和idtr，跳转语句强制使用他们
	jmp	SELECTOR_KERNEL_CS:kernel_init_done					

kernel_init_done:

	call init_tss

	xor	eax, eax
	mov	ax, SELECTOR_TSS
	ltr	ax


	call start_proc

	jmp $

; 系统调用入口,通过int 0x80调用
; eax为系统调用号
; ebx,ecx,edx为参数
; 返回值为eax
sys_call:
	call	save
	sti

	push	esi
	push 	dword [p_proc_ready_head]
	push 	edi
	push	edx
	push	ecx
	push 	ebx
	call 	[sys_call_table + eax * 4]
	add 	esp, 4*5

	pop	esi
	; ; 如果是IPC系统调用,则需要注意返回的时候是否对原进程进行了阻塞
	; ; 如果对原进程进行了阻塞,则需要注意不能修改eax,否则会对新进程造成破坏
	; 啊当上面的东西没说,于渊大佬太秀了这个
	; 这个esi埋伏了一手,如果发生了上述情况,修改的eax是原进程的,转入的是新进程,互不相关
	mov 	[esi+EAXREG - P_STACKBASE], eax
	cli
	ret



;改自Orange's OS by 于渊
; 8259A 中断处理
	ALIGN   16
	hwint00:                ; Interrupt routine for irq 0 (the clock).
			hwint_master    0

	ALIGN   16
	hwint01:                ; Interrupt routine for irq 1 (keyboard)
			hwint_master    1

	ALIGN   16
	hwint02:                ; Interrupt routine for irq 2 (cascade!)
			hwint_master    2

	ALIGN   16
	hwint03:                ; Interrupt routine for irq 3 (second serial)
			hwint_master    3

	ALIGN   16
	hwint04:                ; Interrupt routine for irq 4 (first serial)
			hwint_master    4

	ALIGN   16
	hwint05:                ; Interrupt routine for irq 5 (XT winchester)
			hwint_master    5

	ALIGN   16
	hwint06:                ; Interrupt routine for irq 6 (floppy)
			hwint_master    6

	ALIGN   16
	hwint07:                ; Interrupt routine for irq 7 (printer)
			hwint_master    7


	ALIGN   16
	hwint08:                ; Interrupt routine for irq 8 (realtime clock).
			hwint_slave     8

	ALIGN   16
	hwint09:                ; Interrupt routine for irq 9 (irq 2 redirected)
			hwint_slave     9

	ALIGN   16
	hwint10:                ; Interrupt routine for irq 10
			hwint_slave     10

	ALIGN   16
	hwint11:                ; Interrupt routine for irq 11
			hwint_slave     11

	ALIGN   16
	hwint12:                ; Interrupt routine for irq 12
			hwint_slave     12

	ALIGN   16
	hwint13:                ; Interrupt routine for irq 13 (FPU exception)
			hwint_slave     13

	ALIGN   16
	hwint14:                ; Interrupt routine for irq 14 (AT winchester)
			hwint_slave     14

	ALIGN   16
	hwint15:                ; Interrupt routine for irq 15
			hwint_slave     15
; 8259A 中断处理结束

;改自Orange's OS by 于渊
; CPU保留中断处理
	divide_error:
		push	0xFFFFFFFF	; no err code
		push	0		; vector_no	= 0
		jmp	exception
	single_step_exception:
		push	0xFFFFFFFF	; no err code
		push	1		; vector_no	= 1
		jmp	exception
	nmi:
		push	0xFFFFFFFF	; no err code
		push	2		; vector_no	= 2
		jmp	exception
	breakpoint_exception:
		push	0xFFFFFFFF	; no err code
		push	3		; vector_no	= 3
		jmp	exception
	overflow:
		push	0xFFFFFFFF	; no err code
		push	4		; vector_no	= 4
		jmp	exception
	bounds_check:
		push	0xFFFFFFFF	; no err code
		push	5		; vector_no	= 5
		jmp	exception
	inval_opcode:
		push	0xFFFFFFFF	; no err code
		push	6		; vector_no	= 6
		jmp	exception
	copr_not_available:
		push	0xFFFFFFFF	; no err code
		push	7		; vector_no	= 7
		jmp	exception
	double_fault:
		push	8		; vector_no	= 8
		jmp	exception
	copr_seg_overrun:
		push	0xFFFFFFFF	; no err code
		push	9		; vector_no	= 9
		jmp	exception
	inval_tss:
		push	10		; vector_no	= A
		jmp	exception
	segment_not_present:
		push	11		; vector_no	= B
		jmp	exception
	stack_exception:
		push	12		; vector_no	= C
		jmp	exception
	general_protection:
		push	13		; vector_no	= D
		jmp	exception
	page_fault:
		push	14		; vector_no	= E
		jmp	exception
	copr_error:
		push	0xFFFFFFFF	; no err code
		push	16		; vector_no	= 10h
		jmp	exception

	exception:
		call	exception_handler
		add	esp, 4*2	; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
		hlt
		; 这里直接停机会使得IF位仍然为0 即无法接收新中断
		; 会造成永久停机

; CPU保留中断处理结束

; 中断保存进程状态函数--------------------------------------------
save:
		; 记录进程的状态
        pushad
        push    ds
        push    es
        push    fs
        push    gs

		; edx中可能存着系统调用的参数,先保护起来
		mov	esi, edx
		
        mov     dx, ss
        mov     ds, dx
        mov     es, dx
		mov		fs, dx

		; 恢复edx
		mov edx, esi

		;  记录进程表起始地址
        mov     esi, esp

        inc     dword [k_reenter]
        cmp     dword [k_reenter], 0
        jne     end_without_switch_stack

		; 切换到内核栈
        mov     esp, StackTop
        push    restart
		; 回到中断处理中
        jmp     [esi + RETADR - P_STACKBASE]

end_without_switch_stack:
		; 之所以重入的中断不需要切换栈空间
		; 是因为本来就在栈空间
        push    restart_reenter
		; 回到中断处理中
        jmp     [esi + RETADR - P_STACKBASE]
; 中断保存进程状态函数--------------------------------------------

; 中断恢复函数---------------------------------------------------------------
; 用于从中断恢复到进程中
; 普通地从中断恢复到进程
restart:
	mov	esp, [p_proc_ready_head]
	lldt	[esp + P_LDT_SEL]
	lea	eax, [esp + P_STACKTOP]
	mov	dword [tss + TSS3_S_SP0], eax
; 从中断恢复到中断,不需要切换ldt和设置tss
restart_reenter:
	dec	dword [k_reenter]
	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad
	add	esp, 4
	iretd


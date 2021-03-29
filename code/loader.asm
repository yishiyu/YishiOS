;=======================================
;	loader 程序
;	1. 获取内存的信息
;	2. 搜索并加载 kernel.bin
;	3. 进入保护模式(由于保护模式下仅根据偏移就能寻址 4G 的空间,所以不再在内核中划分不同的段)
;	4. 启动分页机制(根据得到的内存信息, 部分初始化页表)
;	5. 重新放置 kernel (kernel 文件为 elf 格式文件,与引导扇区一样,现在麻烦点可以给以后积累很多可以用的代码)
;	6. 跳转进入 kernel
;
;	代码布局:
;		数据定义
;		实模式代码
;		保护模式代码
;		函数定义
;=======================================

jmp	loader_start

%include "pm.inc"
%include "loader.inc"

; GDT (全局描述符表) ------------------------------------------------------------------------------------------------------------------------------------------
;                                              							  段基址           	 段界限     		, 属性
LABEL_GDT:
LABEL_DESC_FLAT_C:		Descriptor             0,              0fffffh, 			DA_CR  | DA_32 | DA_LIMIT_4K			; 0 ~ 4G(靠偏移寻址)
LABEL_DESC_FLAT_RW:	 Descriptor             0,              0fffffh, 			DA_DRW | DA_32 | DA_LIMIT_4K			; 0 ~ 4G(靠偏移寻址)
LABEL_DESC_VIDEO:		 Descriptor	 0B8000h,              0ffffh, 			DA_DRW | DA_DPL3									; 显存首地址
GdtLen		equ	$ - LABEL_GDT
; GDT -----------------------------------------------------------------------------------------------------------------------------------------------------------------

GdtPtr:
		dw	GdtLen - 1														; 段界限, 根据全局描述符个数确定
		dd	Loader_Phy_Address + LABEL_GDT		  ; 段基址
;需要注意的是GdtPtr并不仅是一个段基址,而是一个数据结构
;让GdtPtr基地址8字节对齐可提高速度 (提高查询全局描述符速度),但是太麻烦了...

; GDT Selector (全局描述符表选择子) ---------------------------------------------------------------------------------------------------------------------
SelectorFlatC		equ	LABEL_DESC_FLAT_C	- LABEL_GDT								; 全局 32位 可执行可读 0级 代码段描述符 范围为0 ~ 4G
SelectorFlatRW		equ	LABEL_DESC_FLAT_RW	- LABEL_GDT						  ; 全局 32位 可读可写 	   0级 数据段描述符 范围为0 ~ 4G
SelectorVideo		equ	LABEL_DESC_VIDEO	- LABEL_GDT + SA_RPL3	    ; 全局 32位 可读可写 	 3级 显存段描述符 段长度为1Mb
; GDT Selector ------------------------------------------------------------------------------------------------------------------------------------------------------


; 字符串 -----------------------------------------------------------------------------------------------------------------------------------------------------------------
; 需要注意的是,这里定义的变量和函数只能在实模式下使用,在保护模式下寻址机制会发生改变
; kernel 文件名
Kernel_File_Name:		db	"kernel.bin"
Kernel_File_Length	equ $ - Kernel_File_Name

; 显示用的字符串
Message_Length		equ	9
Message:
		db	"Loading  "
		db	"         "
		db	"Ready.   "
		db	"No KERNEL"
; 字符串 -----------------------------------------------------------------------------------------------------------------------------------------------------------------

; 实模式变量和宏 ----------------------------------------------------------------------------------------------------------------------------------------------------
; 把堆栈定义为 0x20000 ~ 0x20400 共1kb空间
; 跳入保护模式后会重新设置栈, 所以仅在实模式下这些空间足够用了
; Orange's OS 中于渊老哥设置的堆栈为cs:00h ~ cs:100h
; 有点奇怪栈区和代码区这么不会冲突吗...
Stack_Base	equ 02000h
Stack_Top	equ	0400h
; 实模式变量和宏 ----------------------------------------------------------------------------------------------------------------------------------------------------



; 保护模式变量和宏 -------------------------------------------------------------------------------------------------------------------------------------------------
; 需要注意的是是模式和保护模式都能使用这里定义的变量, 不过访问的方式不太一样
[SECTION .data1]
ALIGN	32
Data_Section:

; 实模式下使用这些符号-------------------------

; 变量
_Mem_Check_Result_Number: dd 0						 ; 记录最终得到的检查结果描述符数量
_ARDStruct:																		; Address Range Descriptor Structure -- 描述内存检查结果的数据结构
	_ARD_Base_L:		dd	0
	_ARD_Base_H:		dd	0
	_ARD_Length_L:		dd	0
	_ARD_Length_H:		dd	0
	_ARD_Type:		dd	0
_Mem_Check_Buf: times 156 db 0							; 检查内存大小时用到的缓冲区



; 保护模式下使用这些符号-------------------------

; 变量
Mem_Check_Result_Number	equ Loader_Phy_Address  + _Mem_Check_Result_Number
ARDStruct	equ Loader_Phy_Address + _ARDStruct
	ARD_Base_L	equ Loader_Phy_Address + _ARD_Base_L
	ARD_Base_H	equ Loader_Phy_Address + _ARD_Base_H
	ARD_Length_L	equ Loader_Phy_Address + _ARD_Length_L
	ARD_Length_H	equ Loader_Phy_Address + _ARD_Length_H
	ARD_Type	equ Loader_Phy_Address + _ARD_Type
Mem_Check_Buf equ Loader_Phy_Address + _Mem_Check_Buf

; 保护模式变量和宏 -------------------------------------------------------------------------------------------------------------------------------------------------



loader_start:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov ax, Stack_Base
	mov	ss, ax
	mov	sp, Stack_Top

	mov dx, 0300h
	call DispStr_In_RealMode

	; 得到内存数
	mov	ebx, 0									 ; ebx = 后续值, 开始时需为 0
	mov	di, _Mem_Check_Buf		; es:di 指向一个地址范围描述符结构 (Address Range Descriptor Structure)
Mem_Check:
	mov	eax, 0E820h						  ; eax = 0000E820h 对应的功能为检查内存
	mov	ecx, 20									; ecx = 地址范围描述符结构的大小
	mov	edx, 0534D4150h			   ; edx = 'SMAP'
	int	15h

	jc	Mem_Check_Fail					; 获得内存失败
	add	di, 20
	inc	dword [_Mem_Check_Result_Number]		; 检查成功, 增加检查结果描述符计数器
	cmp	ebx, 0
	jne	Mem_Check
	jmp	Mem_Check_Done
Mem_Check_Fail:
	mov	dword [_Mem_Check_Result_Number], 0
	jmp $
Mem_Check_Done:

	mov ax, ds
	mov bx, _Mem_Check_Result_Number
	mov cx, _Mem_Check_Buf

	xchg bx, bx



;======================================
; DispStr_In_RealMode
; dh 表示要显示行数
; dl 表示要显示的信息的编号
; 需要注意的是boot程序已经在前两行显示了东西了
;======================================
DispStr_In_RealMode:
	mov	ax, Message_Length
	mul	dl
	add	ax, Message
	mov	bp, ax			; ┓
	mov	ax, ds			; ┣ ES:BP = 串地址
	mov	es, ax			; ┛
	mov	cx, Message_Length	; CX = 串长度
	mov	ax, 01301h		; AH = 13,  AL = 01h
	mov	bx, 0007h		; 页号为0(BH = 0) 黑底白字(BL = 07h)
	mov	dl, 0
	int	10h			; int 10h
	ret

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
;		实模式函数定义
;		保护模式代码
;		保护模式函数定义
;=======================================

jmp	loader_start

%include "pm.inc"
%include "loader.inc"

; GDT (全局描述符表) ------------------------------------------------------------------------------------------------------------------------------------------
; 注意这个空描述符是不能删的,否则跳转进保护模式的时候会发生错误
;                                              							  段基址           	 段界限     		, 属性
LABEL_GDT:			Descriptor            				 0,                    		0, 					0																	; 空描述符
LABEL_DESC_FLAT_C:		Descriptor  		  0,              0fffffh, 			DA_CR  | DA_32 | DA_LIMIT_4K				; 0 ~ 4G(靠偏移寻址)
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
Kernel_File_Name_Length	equ $ - Kernel_File_Name

; 显示用的字符串
Message_Length		equ	13
Message:
		db	"Loading      "
		db	"kernel found "
		db	"kernel loaded"
		db	"No kernel    "
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
	; 其实这里确实是包括ROM在内的总的内存的大小
	_Mem_Size:			dd	0												; 总的内存大小
	_Mem_Check_Result_Number: dd 0						 ; 记录最终得到的检查结果描述符数量
	_ARDStruct:																		; Address Range Descriptor Structure -- 描述内存检查结果的数据结构
			_ARD_Base_L:		dd	0
			_ARD_Base_H:		dd	0
			_ARD_Length_L:		dd	0
			_ARD_Length_H:		dd	0
			_ARD_Type:		dd	0
	_Mem_Check_Buf: times 156 db 0							; 检查内存大小时用到的缓冲区

	; 用来读取硬盘的数据结构
	_Disk_Address_Packet:
			db	0x10		; [ 0 ] 数据结构的长度,单位为byte
			db	0				; [ 1 ] 保留位,取0
			db	2				; [ 2 ] 读取的块数,超级块1k占用两个扇区(块)
			db	0				; [ 3 ] 保留位,取0
			dw	RootDir_Offset			  ; [ 4 ] 写入内存位置的偏移
			dw	RootDir_Base			   ; [ 6 ] 写入内存位置的段
			dd	 0			    ; [ 8 ] LBA. Low  32-bits.
			dd	0			    ; [12] LBA. High 32-bits.

	; 屏幕上当前写的位置
	_Disp_Pos:			dd	(80 * 6 + 0) * 2			; 第6行 第0个 (6 ,0)

	; 字符串
	_Char_Return:	db 0Ah, 0
	_Test_message: db "jojo!  Protect Mode !!!",0
	_Page_Init_Success:	db "jojo! Page init successfully !!!",0
	_Init_Kernel_Success:	db "jojo! kernel is ready !!!", 0
	_Mem_Info_Title:	db	"BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
	_Mem_Size_Title:	db	"Memory size: ", 0


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
	Disk_Address_Packet	equ Loader_Phy_Address + _Disk_Address_Packet

	Disp_Pos	equ Loader_Phy_Address + _Disp_Pos

	Test_message	equ Loader_Phy_Address + _Test_message
	Char_Return	equ Loader_Phy_Address + _Char_Return
	Mem_Info_Title	equ Loader_Phy_Address + _Mem_Info_Title
	RAM_Size_Title	equ Loader_Phy_Address + _Mem_Size_Title
	Mem_Size	equ	Loader_Phy_Address + _Mem_Size
	Page_Init_Success	equ Loader_Phy_Address + _Page_Init_Success
	Init_Kernel_Success equ Loader_Phy_Address + _Init_Kernel_Success


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
	int	15h			; int 15h

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

	; 在C盘根目录寻找kernel.bin 文件
	; 不知道根目录检查loader.bin的时候检查到了哪里, 所以需要从头进行
	;得到inode结构体
	;es:bx => 2号inode
	mov eax, 2
	call get_inode

	; 从根目录中寻找kernel.bin

	;读取根目录
	mov word	[_Disk_Address_Packet + 4],	RootDir_Offset
	mov	word	[_Disk_Address_Packet + 6],	RootDir_Base
	mov	eax, [es:bx + Inode_Block]
	add eax,eax
	mov	dword	[_Disk_Address_Packet + 8],	eax
	mov	dword	[_Disk_Address_Packet + 12],	0
	
	;先获取根目录占用的block数量
	;最多只搜索根目录前12个block
	mov	eax, [es:bx + Inode_Blocks]
	shr eax, 1
	mov	cx,	ax
	cmp cx, 12
	jle root_dir_read
	mov cx, 12
	
	;bx = inode中inode_block数据的地址
	;cx = 剩余未搜索block数量
	;es = InodeTable的基地址

	;读取根目录的一个block,减少block计数并修改数据结构
root_dir_read:

	cmp	cx, 0
	je	kernel_not_found
	call	read_sector
	dec cx
	add bx,4
	mov	eax,[es:bx]
	add eax,eax
	mov	dword	[_Disk_Address_Packet + 8],	eax

	;搜索当前读取到的block
root_dir_search:
	; bx = 在当前根目录中读取指针的偏移
	; ds:si = 正确的文件名字
	; gs:bx = 当前磁盘上目录项
	pusha
	mov si,Kernel_File_Name
	mov eax, RootDir_Base
	mov gs,eax
	mov bx, RootDir_Offset

root_file_match:

	; 对比名字长度
file_length_cmp:
	xor cx,cx
	mov cl, Kernel_File_Name_Length
	cmp cl, byte [gs:bx+Name_Len_Offset]
	jnz file_not_match

	mov si,Kernel_File_Name
	push bx

	; 对比名字
file_name_cmp:
	lodsb				; ds:si -> al
	;只有bx能当做基址寄存器
	cmp al,  byte[gs:bx+File_Name_Offset]
	jnz file_name_cmp_end
	dec cl
	jz file_name_cmp_end
	inc bx
	jmp file_name_cmp

file_name_cmp_end:
	;离开对比名字这一段之前必须恢复bx
	pop bx
	cmp cl, 0
	jnz file_not_match
	jmp kernel_found


file_not_match:
	add bx, word [gs:bx+Record_Length_Offset]
	cmp bx,1024
	jl root_file_match
	popa
	jmp root_dir_read


kernel_not_found:
	mov dx, 0403h
	call DispStr_In_RealMode
	jmp $

kernel_found:
	mov dx, 0401h
	call DispStr_In_RealMode

kernel_load:
	; 加载kernel.bin
	; gs:bx => 当前目录项的指针
	mov eax, dword [gs:bx + Inode_Number_Offset]
	call	get_inode
	
	mov word	[_Disk_Address_Packet + 4],	Kernel_File_Offset
	mov	word	[_Disk_Address_Packet + 6],	Kernel_File_Base
	
	call	loade_file
	
	; 到目前位置已经把kernel.bin 移入了内存
kernel_ready:

	mov dx, 0502h
	call DispStr_In_RealMode


	; 准备进入保护模式
	; 加载 GDTR
	lgdt	[GdtPtr]

	; 关中断
	cli

	; 打开地址线A20
	in	al, 92h
	or	al, 00000010b
	out	92h, al

	; 准备切换到保护模式
	mov	eax, cr0
	or	eax, 1
	mov	cr0, eax

	; 真正进入保护模式
	; 一个神奇的跳转: 16位的代码却使用了32位的操作数
	jmp	dword SelectorFlatC:(Loader_Phy_Address+Protect_Mode_Start)


;======================================
; DispStr_In_RealMode
; dh 表示要显示行数
; dl 表示要显示的信息的编号
; 需要注意的是boot程序已经在前两行显示了东西了
;======================================
DispStr_In_RealMode:
	;刚开始没加保护寄存器现场...加载内核的时候这个函数改变了bx寄存器...透..
	;透...刚开始把这个放在了32位代码段的后面,自动变成了32为代码段....
	;看来必须重新调整一下函数和代码的顺序了
	pusha 
	mov	cx, Message_Length	; CX = 串长度
	mov	ax, Message_Length
	mul	dl
	add	ax, Message
	mov	bp, ax			; ┓
	mov	ax, ds			; ┣ ES:BP = 串地址
	mov	es, ax			; ┛
	mov	ax, 01301h		; AH = 13,  AL = 01h
	mov	bx, 0007h		; 页号为0(BH = 0) 黑底白字(BL = 07h)
	mov	dl, 0
	int	10h			; int 10h
	popa
	ret



;======================================
;	get_inode
;	获取inode号对应的inode
;	eax = 目标inode号
;	结束后es:bx指向目标inode
;======================================
get_inode:
	;把获取到的inode所在的block放在InodeTable处
	;修改es和bx使其指向目标inode
	mov word	[_Disk_Address_Packet + 4],	InodeTable_Offset
	mov	word	[_Disk_Address_Packet + 6],	InodeTable_Base

	dec eax		;需要注意的是inode的编号是从1开始的
	mov bl,8	;每个block可以存放8个inode
	div	bl			; al => 商,单位为block
	mov cl, ah ; cl => 余数,为block内偏移
	xor ah, ah

	mov bx, GroupDescriptors_Base
	mov	es, bx
	mov	ebx,	dword [es:GroupDescriptors_Offset + 8]				;获取inode table首地址,单位为block
	add eax, ebx		; eax => 所在的总的block
	add eax, eax		; eax => 所在的总的扇区号
	mov	dword	[_Disk_Address_Packet + 8],	eax

	call read_sector

	mov bx, InodeTable_Base
	mov es, bx
	mov al, cl
	mov bl, Inode_Length
	mul bl
	mov bx,ax

	ret

;======================================
;	read_sector
;	调用前需要设置好Disk_Address_Packet结构体
;	使用前后寄存器会发生改变,为了节省空间不再管这个了
;======================================
read_sector:
	pusha
	xor	ebx, ebx
	mov	ah, 042h
	mov	dl, 080h			; c 盘盘符
	mov	si, _Disk_Address_Packet
	int	0x13
	popa
	ret

;======================================
;	loade_file
;	根据inode加载file,最大支持加载前12个block,即12k,对于一个loader足够大了
;	es:bx 指向文件inode
;	内存目标地址需要实现填充在访问磁盘的数据结构中
;======================================
loade_file:
	; 判断文件大小
	; 在这里不提供虚拟内存服务,最多扩展到1级索引
	; 最大的加载大小为12 + 256 = 268 kb
	; inode 40-99 bytes 描述指向数据的block号(60bytes 描述了15个block)
	; 一个block能存放1024/4 = 256 个block号
	;前12个block为直接索引
	;第13个block为一级索引
	;第14个block为二级索引
	;低15个block为三级索引

	;保存原始的bx
	push bx

	; 读取直接索引的block
	mov ecx, dword [es:bx + Inode_Blocks]
	cmp ecx, 24																;直接索引最大支持24个扇区
	jbe	direct_block
	mov	ecx,24
direct_block:
	mov byte [_Disk_Address_Packet + 2], 2		;分多次读取,每次读取一个block

read_direct_block:
	mov eax, dword [es:bx + Inode_Block]
	add bx, 4
	add eax, eax
	mov	dword	[_Disk_Address_Packet + 8],	eax
	
	;读取磁盘并使内存目的地址指向下一个block
	call read_sector
	add	word	[_Disk_Address_Packet + 4], 0x400
	sub ecx, 2
	ja	read_direct_block


	;暂存下一个block的位置
	mov ax, bx
	; 恢复原始的bx
	pop bx

	;判断有没有一级索引的block没有读
	mov ecx, dword [es:bx + Inode_Blocks]
	cmp ecx, 24
	jbe	loade_file_end
	sub ecx, 24											;获取还需要读的扇区数
	;恢复指向下一个block的位置
	mov bx, ax

	;保存当前文件的内存写入位置
	push dword [_Disk_Address_Packet + 4]
	push dword [_Disk_Address_Packet + 6]

	; 读取直接索引的block
	; 读取索引block
	mov eax, dword [es:bx + Inode_Block]
	add eax, eax
	mov	dword	[_Disk_Address_Packet + 8],	eax
	mov word	[_Disk_Address_Packet + 4],	First_Index_Block_Offset
	mov	word	[_Disk_Address_Packet + 6],	First_Index_Block_Base
	call read_sector

	; 根据索引block读取
	;调整es和bx
	mov ax, First_Index_Block_Base
	mov es, ax
	mov bx, First_Index_Block_Offset
	;恢复原本文件的内存写入位置
	pop dword [_Disk_Address_Packet + 6]
	pop dword [_Disk_Address_Packet + 4]


read_first_index_block:
	mov eax, dword [es:bx]
	add bx, 4
	add eax, eax
	mov	dword	[_Disk_Address_Packet + 8],	eax

	;读取磁盘并使内存目的地址指向下一个block
	call read_sector
	add	word	[_Disk_Address_Packet + 4], 0x400
	sub ecx, 2
	ja	read_first_index_block

loade_file_end:

	ret


; 从此跨入 保护模式 的大门 	---------------------------------------------------------
; 32 位代码段. 由实模式跳入 ---------------------------------------------------------
[SECTION .s32]
ALIGN	32
[BITS	32]

Protect_Mode_Start:
	mov	ax, SelectorVideo
	mov	gs, ax
	mov	ax, SelectorFlatRW
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	ss, ax
	mov esp, Protect_Mode_Stack_Top

	;显示进入信息
	push Test_message
	call 	DispStr
	call	DispReturn
	call	DispReturn
	add esp,	4

	;显示内存信息
	call	DispMemInfo

	;启动分页机制
	call	SetupPaging
	call	DispReturn
	call	DispReturn
	push Page_Init_Success
	call 	DispStr
	call	DispReturn
	add esp,	4

	;重新放置内核
	call	InitKernel
	push Init_Kernel_Success
	call 	DispStr
	call	DispReturn
	add esp,	4

	mov ax,SelectorVideo
	mov gs,ax
	
	;***************************************************************
	jmp	SelectorFlatC:Kernel_Enter_point	; 正式进入内核 
	;***************************************************************

; ------------------------------------------------------------------------
; 显示 AL 中的数字
; ------------------------------------------------------------------------
DispAL:
	push	ecx
	push	edx
	push	edi

	mov	edi, [Disp_Pos]

	mov	ah, 0Fh			; 0000b: 黑底    1111b: 白字
	mov	dl, al
	shr	al, 4
	mov	ecx, 2

num_trans:
	; 把al中的数字转化成ascii码
	and	al, 01111b
	cmp	al, 9
	ja	big_num
	add	al, '0'
	jmp	display

big_num:
	; 对大于9的数字特殊处理
	sub	al, 0Ah
	add	al, 'A'

display:
	;对显存进行操作
	mov	[gs:edi], ax
	add	edi, 2

	mov	al, dl
	loop	num_trans

	mov	[Disp_Pos], edi

	pop	edi
	pop	edx
	pop	ecx

	ret
; DispAL 结束-------------------------------------------------------------

; ------------------------------------------------------------------------
;	DispInt
;	显示一个整形数
;	通过堆栈传递一个Int参数
;	遵守c语言的规范: 堆栈谁调用谁清理
;	显示位置放在当前显示的数字后面,不负责换行
; ------------------------------------------------------------------------
DispInt:
	mov	eax, [esp + 4]
	shr	eax, 24
	call	DispAL

	mov	eax, [esp + 4]
	shr	eax, 16
	call	DispAL

	mov	eax, [esp + 4]
	shr	eax, 8
	call	DispAL

	mov	eax, [esp + 4]
	call	DispAL

	mov	ah, 07h			; 0000b: 黑底    0111b: 灰字
	mov	al, 'h'
	push	edi
	mov	edi, [Disp_Pos]
	mov	[gs:edi], ax
	add	edi, 4
	mov	[Disp_Pos], edi
	pop	edi

	ret
; DispInt 结束------------------------------------------------------------


; ------------------------------------------------------------------------
;	DispStr
; 	显示一个字符串
;	通过堆栈传递一个4字节字符串起始位置变量
;	字符串结尾用0来标识
; ------------------------------------------------------------------------
DispStr:
	push	ebp
	mov	ebp, esp
	push	ebx
	push	esi
	push	edi

	mov	esi, [ebp + 8]
	mov	edi, [Disp_Pos]
	mov	ah, 0Fh

display_char:
	lodsb
	test	al, al					;test指令对符号位的影响和and指令相同
	jz	disp_end				;判断字符串是否已经显示完

	cmp	al, 0Ah					; 判断当前字符是否为回车
	jnz	disp_return

	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	display_char

disp_return:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	display_char

disp_end:
	mov	[Disp_Pos], edi

	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	ret
; DispStr 结束------------------------------------------------------------

; ------------------------------------------------------------------------
; DispReturn
; 调用后切换到下一行
; ------------------------------------------------------------------------
DispReturn:
	push	Char_Return
	call	DispStr			;printf("\n");
	add	esp, 4

	ret
; DispReturn 结束---------------------------------------------------------

; ------------------------------------------------------------------------
; 内存拷贝，仿 memcpy
; void* MemCpy(void* es:pDest, void* ds:pSrc, int iSize);
; ------------------------------------------------------------------------
MemCpy:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]					 ; Destination
	mov	esi, [ebp + 12]			    	; Source
	mov	ecx, [ebp + 16]				   ; Counter
transfer:
	cmp	ecx, 0
	jz	transfer_end

	;逐字节移动内存
	mov	al, [ds:esi]
	inc	esi	
	mov	byte [es:edi], al
	inc	edi	

	dec	ecx	
	jmp	transfer

transfer_end:
	mov	eax, [ebp + 8]	; 返回值

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret
; MemCpy 结束-------------------------------------------------------------



; 显示内存信息 --------------------------------------------------------------
;	DispMemInfo 显示内存信息
; -----------------------------------------------------------------------------------
DispMemInfo:
	push	esi
	push	edi
	push	ecx

	push	Mem_Info_Title
	call	DispStr
	add	esp, 4

	mov	esi, Mem_Check_Buf
	mov	ecx, [Mem_Check_Result_Number]

display_entry:
	mov	edx, 5
	mov	edi, ARDStruct

fill_the_ARDStruct:

	;填充数据结构
	;每填充一个成员就显示出来
	push	dword [esi]
	call	DispInt
	pop	eax
	stosd
	add	esi, 4
	dec	edx
	cmp	edx, 0
	jnz	fill_the_ARDStruct

	call	DispReturn
	; 根据当前entry的类型显示不同内容
	cmp	dword [ARD_Type], 1
	jne	next_entry
	mov	eax, [ARD_Base_L]
	add	eax, [ARD_Length_L]
	cmp	eax, [Mem_Size]
	jb	next_entry
	mov	[Mem_Size], eax

next_entry:
	loop	display_entry

	push	RAM_Size_Title
	call	DispStr
	add	esp, 4

	push	dword [Mem_Size]
	call	DispInt
	add	esp, 4

	pop	ecx
	pop	edi
	pop	esi
	ret
; ---------------------------------------------------------------------------



; 启动分页机制 --------------------------------------------------------------
; 其实感觉根据内存大小分配页表有些鸡肋,毕竟不缺那一点空间
SetupPaging:
	; 根据内存大小计算应初始化多少PDE以及多少页表
	xor	edx, edx
	mov	eax, [Mem_Size]
	mov	ebx, 400000h									; 400000h = 4M = 4096 * 1024, 一个页表对应的内存大小
	div	ebx
	mov	ecx, eax											  ; 此时 ecx 为页表的个数，也即 PDE 应该的个数
	test	edx, edx
	jz	no_remainder									  ; 没有余数,直接跳转到下一步
	inc	ecx															; 如果余数不为 0 就需增加一个页表

no_remainder:
	push	ecx													 ; 暂存页表个数

	; 为简化处理, 所有线性地址对应相等的物理地址. 并且不考虑内存空洞.

	; 首先初始化页目录
	mov	ax, SelectorFlatRW
	mov	es, ax
	mov	edi, Page_Dir_Base						; 此段首地址为 Page_Dir_Base
	xor	eax, eax
	mov	eax, Page_Table_BASE | PG_P  | PG_USU | PG_RWW

init_page_dir:
	stosd
	add	eax, 4096										  ; 为了简化, 所有页表在内存中是连续的.
	loop	init_page_dir

	; 再初始化所有页表
	pop	eax														; 页表个数
	mov	ebx, 1024										 ; 每个页表 1024 个 PTE
	mul	ebx
	mov	ecx, eax											; PTE个数 = 页表个数 * 1024
	mov	edi, Page_Table_BASE	; 此段首地址为 PAGE_TBL_BASE
	xor	eax, eax
	mov	eax, PG_P  | PG_USU | PG_RWW

init_page_table:
	stosd
	add	eax, 4096										  ; 每一页指向 4K 的空间
	loop	init_page_table

	mov	eax, Page_Dir_Base
	mov	cr3, eax
	mov	eax, cr0
	or	eax, 80000000h
	mov	cr0, eax
	jmp	short init_page_end
init_page_end:
	nop

	ret
; 分页机制启动完毕 ----------------------------------------------------------


; InitKernel ---------------------------------------------------------------------------------
; 将 KERNEL.BIN 的内容经过整理对齐后放到新的位置
; --------------------------------------------------------------------------------------------
InitKernel:	
	; 遍历每一个 Program Header，根据 Program Header 中的信息来确定把什么放进内存，放到什么位置，以及放多少。
	xor	esi, esi
	mov	cx, word [Kernel_File_Phy_Addr + ELF_Header_e_phnum];	;Program Header Table中entry个数
	movzx	ecx, cx
	mov	esi, [Kernel_File_Phy_Addr + ELF_Header_e_phoff]				;Program Header Table 在文件中的偏移
	add	esi, Kernel_File_Phy_Addr																   ;Program Header Table 在内存中的偏移

deal_with_one_program_header_table_entry:


	mov	eax, [esi]
	cmp	eax, 0																										  ; 0 -> unused  1 -> loadable
	jz	init_next_section
	push	dword [esi + ELF_Program_Header_p_size]							   ; 获取section的大小
	mov	eax, [esi + ELF_Program_Header_p_offset]								; 获取section在文件中的偏移
	add	eax, Kernel_File_Phy_Addr																 ; 获取section在内存中的偏移
	push	eax																										   ; section在内存中的源地址
	push	dword [esi +ELF_Program_Header_p_vaddr]						    ; section在内存中的目的地址
	call	MemCpy
	add	esp, 12																										 ; 恢复堆栈

init_next_section:
	add	esi, 020h																								   ; 指向下一个 Program Header Table entry
	dec	ecx
	jnz	deal_with_one_program_header_table_entry


	ret





; 堆栈就在32位代码段的末尾
Protect_Mode_Stack_Space:	times	1000h	db	0
Protect_Mode_Stack_Top	equ	Loader_Phy_Address + $
;堆栈段基址其实就是cs ds es 通用的以0x00为起始地址的段
;Stack_Base	equ 0

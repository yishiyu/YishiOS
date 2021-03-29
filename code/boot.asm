
	org	07c00h									; 告诉编译器程序加载到7c00处
															; 之所以是7c00和bios有关
	jmp boot_start

%include "BOOT.inc"
%include "EXT2.inc"

	;用来读取硬盘的数据结构
	disk_address_packet:
				db	0x10		; [ 0 ] 数据结构的长度,单位为byte
				db	0				; [ 1 ] 保留位,取0
				db	2				; [ 2 ] 读取的块数,超级块1k占用两个扇区(块)
				db	0				; [ 3 ] 保留位,取0
				dw	SuperBlock_Offset			   ; [ 4 ] 写入内存位置的偏移
				dw	SuperBlock_Base				    ; [ 6 ] 写入内存位置的段
				dd	SuperBlock_LBA_L			   ; [ 8 ] LBA. Low  32-bits.
				dd	SuperBlock_LBA_H			  ; [12] LBA. High 32-bits.

	BootMessage:						;加载信息,统一为10字节长
				db	"Booting   "
				db	"Loading   "
				db	"No Loader "
	BootMessage_Length	equ 10
	Loader_Name:	db	"loader.bin"
	Loader_Name_Length: equ	$-Loader_Name

boot_start:
	;读取硬盘中的Boot Sector到内存中
	mov ax, cs
	mov ds,	ax
	mov	es,	ax
	mov	ss,	ax
	mov	sp,	Stack_Base

	; 显示"Boot"
	call	clear_screen
	mov	dh,	0
	call	disp_str
	
	; 读取超级块
	call	read_sector
	

	;读取Group Desriptors(因为inode table的位置可能根本不按常理出牌!!!!)
	mov word	[disk_address_packet + 4],	GroupDescriptors_Offset
	mov	word	[disk_address_packet + 6],	GroupDescriptors_Base
	mov	dword	[disk_address_packet + 8],	GroupDescriptors_LBA_L
	mov	dword	[disk_address_packet + 12],	GroupDescriptors_LBA_H
	call	read_sector

	;读取Inode Table(具体位置从Group Desriptors中获取)
	mov word	[disk_address_packet + 4],	InodeTable_Offset
	mov	word	[disk_address_packet + 6],	InodeTable_Base
	mov ax, GroupDescriptors_Base
	mov	ds, ax
	mov	eax,	dword [GroupDescriptors_Offset + 8]				;获取inode table首地址,单位为block
	add eax,eax
	mov bx, cs
	mov	ds,	bx																						 ;一定要记得把ds寄存器恢复!!!!!找了半天出错原因!!!(其实完全可以用es寄存器...傻了...)
	mov	dword	[disk_address_packet + 8],	eax						 ;高地址依然为0

	call	read_sector
	
	;根据Inode Table读取根目录(ext2系统2号inode保留为根目录inode)
	; inode 40-99 bytes 描述指向数据的block号(60bytes 描述了15个block)
	;前12个block为直接索引
	;第13个block为一级索引
	;第14个block为二级索引
	;低15个block为三级索引

	;读取根目录

	mov word	[disk_address_packet + 4],	RootDir_Offset
	mov	word	[disk_address_packet + 6],	RootDir_Base
	;InodeTable 的基地址
	;第二个inode中inode_block的偏移
	mov	bx,InodeTable_Base
	mov	es,bx
	mov	bx,Inode_Length + Inode_Block
	mov	eax,[es:bx]
	add eax,eax
	mov	dword	[disk_address_packet + 8],	eax
	mov	dword	[disk_address_packet + 12],	0
	;只搜索根目录前12个block
	mov	cx,	12
	
	;bx = inode中inode_block数据的地址
	;cx = 剩余未搜索block数量
	;es = InodeTable的基地址

	;读取根目录的一个block,减少block计数并修改数据结构
root_dir_read:
	cmp	cx, 0
	je	loader_not_found
	call	read_sector
	dec cx
	add bx,4
	mov	eax,[es:bx]
	add eax,eax
	mov	dword	[disk_address_packet + 8],	eax

	;搜索当前读取到的block
root_dir_search:
	; bx = 在当前根目录中读取指针的偏移
	; ds:si = 正确的文件名字
	; gs:bx = 当前磁盘上目录项
	pusha
	mov si,Loader_Name
	mov eax, RootDir_Base
	mov gs,eax
	mov bx, RootDir_Offset

root_file_match:

	; 对比名字长度
file_length_cmp:
	xor cx,cx
	mov cl, Loader_Name_Length
	cmp cl, byte [gs:bx+Name_Len_Offset]
	jnz file_not_match

	mov si,Loader_Name
	push bx
	; 对比名字
file_name_cmp:
	; 阿西吧了,这个losb有副作用会使si寄存器加1
	; 我说为啥总是第一个字符匹配上了后面就错了
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
	jmp loader_found


file_not_match:
	add bx, word [gs:bx+Record_Length_Offset]
	cmp bx,1024
	jl root_file_match
	popa
	jmp root_dir_read


loader_not_found:
	mov	dh,	2
	call disp_str
	jmp	$

loader_found:
	;这个堆栈以后就不用了,没必要恢复栈顶指针
	mov	dh,	1
	call	disp_str

	; 好了终于能加载Loader了!!!!!!!
	; 此时寄存器情况
	; gs:bx => 当前目录项的指针

	mov word	[disk_address_packet + 4],	Loader_Offset
	mov	word	[disk_address_packet + 6],	Loader_Base
	mov	eax, [gs:bx + Inode_Number_Offset]
	add eax, eax
	mov	dword	[disk_address_packet + 8],	eax

	




;======================================
;	clear_screen
;	清空80×50的屏幕为黑底白字
;======================================
clear_screen:
	pusha
	mov	ax,	0600h
	mov	bx,	07h
	mov	cx,	0
	mov	dx,	0184fh
	int 10h
	popa
	ret

;======================================
;	disp_str
;	显示加载信息
;	dh=显示信息的偏移
;======================================
disp_str:
	pusha
	mov	ax,	BootMessage_Length
	mul	dh
	add	ax,	BootMessage
	mov	bp,	ax
	mov	ax,	ds
	mov	es,	ax
	mov cx,BootMessage_Length
	mov ax,01301h
	mov	bx,	07h
	mov	dl,	0
	int	10h	
	popa
	ret

;======================================
;	read_sector
;	调用前需要设置好disk_address_packet结构体
;	使用前后寄存器会发生改变,为了节省空间不再管这个了
;======================================
read_sector:
	pusha
	xor	ebx, ebx
	mov	ah, 042h
	mov	dl, 080h			; c 盘盘符
	mov	si, disk_address_packet
	int	0x13
	popa
	ret


times 	510-($-$$)	db	0					; 填充剩下的空间，使生成的二进制代码恰好为512字节
dw 	0xaa55											; 结束标志

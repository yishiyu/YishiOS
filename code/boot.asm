
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
	BootMessage_Length	equ $-BootMessage

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
	xchg bx,bx
	call	read_sector

	;读取Inode Table(具体位置从Group Desriptors中获取)
	mov word	[disk_address_packet + 4],	InodeTable_Offset
	mov	word	[disk_address_packet + 6],	InodeTable_Base
	mov ax, GroupDescriptors_Base
	mov	ds, ax
	mov	eax,	dword [GroupDescriptors_Offset + 8]				;获取inode table首地址,单位为block
	add eax,eax
	mov bx, cs
	mov	ds,	bx
	mov	dword	[disk_address_packet + 8],	eax						 ;高地址依然为0

	xchg bx,bx
	call	read_sector
	xchg bx,bx


;======================================
;	clear_screen
;	清空80×50的屏幕为黑底白字
;======================================
clear_screen:
	mov	ax,	0600h
	mov	bx,	07h
	mov	cx,	0
	mov	dx,	0184fh
	int 10h
	ret

;======================================
;	disp_str
;	显示加载信息
;	dh=显示信息的偏移
;======================================
disp_str:
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
	ret

;======================================
;	read_sector
;	调用前需要设置好disk_address_packet结构体
;	使用前后寄存器会发生改变,为了节省空间不再管这个了
; Exit:
;     - es:bx -> data read
; registers changed:
;     - eax, ebx, dl, si, es
;======================================
read_sector:
	xor	ebx, ebx
	mov	ah, 042h
	mov	dl, 080h			; c 盘盘符
	mov	si, disk_address_packet
	int	0x13

	mov	ax, [disk_address_packet + 6]
	mov	es, ax
	mov	bx, [disk_address_packet + 4]
	ret



times 	510-($-$$)	db	0					; 填充剩下的空间，使生成的二进制代码恰好为512字节
dw 	0xaa55											; 结束标志

[SECTION .s32]
ALIGN	32
[BITS	32]

global _start	; 导出 _start


_start:	; 跳到这里来的时候，我们假设 gs 指向显存
	xchg bx,bx
	call	disp_str
	jmp	$


; 保证生成的kernel.bin文件足够大(至少20kb)
empty:  times 0x5000 db '1'

_kernel_message db "in kernel"
kernel_message	equ 0x10000 + _kernel_message


disp_str:
	mov	ah, 0Fh				; 0000: 黑底    1111: 白字
	mov bx,kernel_message
	mov	al, [bx +0]
	mov edi,((80 * 20 + 0)*2)
	mov	[gs:edi], ax
	mov	al, [bx +1]
	add edi,2
	mov	[gs:edi], ax
	mov	al, [bx +2]
	add edi,2
	mov	[gs:edi], ax
	mov	al, [bx +3]
	add edi,2
	mov	[gs:edi], ax
	mov	al, [bx +4]
	add edi,2
	mov	[gs:edi], ax
	mov	al, [bx +5]
	add edi,2
	mov	[gs:edi], ax
	mov	al, [bx +6]
	add edi,2
	mov	[gs:edi], ax
	mov	al, [bx +7]
	add edi,2
	mov	[gs:edi], ax
	mov	al, [bx +8]
	add edi,2
	mov	[gs:edi], ax

	ret


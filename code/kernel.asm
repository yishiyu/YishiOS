[section .text]	; 代码在此

global _start	; 导出 _start


_start:	; 跳到这里来的时候，我们假设 gs 指向显存
	mov	ah, 0Fh				; 0000: 黑底    1111: 白字
	mov	al, 'i'
	mov edi,((80 * 20 + 0)*2)
	mov	[gs:edi], ax	; 屏幕第 1 行, 第 39 列。
	mov al, 'n'
	add edi,2
	mov	[gs:edi], ax
	mov al, ' '
	add edi,2
	mov	[gs:edi], ax
	mov al, 'k'
	add edi,2
	mov	[gs:edi], ax
	mov al, 'e'
	add edi,2
	mov	[gs:edi], ax
	mov al, 'r'
	add edi,2
	mov	[gs:edi], ax
	mov al, 'n'
	add edi,2
	mov	[gs:edi], ax
	mov al, 'e'
	add edi,2
	mov	[gs:edi], ax
	mov al, 'l'
	add edi,2
	mov	[gs:edi], ax
	jmp	$



;========================================
;	内存操作函数
;   author: 《Orange's OS: 一个操作系统的实现》作者于渊
;   原书文件路径: lib/string.asm
;   感谢大佬!
;========================================

; 导出函数
global	memcpy
global	memset

[SECTION .text]

; ------------------------------------------------------------------------
; 内存拷贝，仿 memcpy
; void* MemCpy(void* es:pDest, void* ds:pSrc, int iSize);
; ------------------------------------------------------------------------
memcpy:
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


; ------------------------------------------------------------------------
; 内存赋值
; void memset(void* p_dst, char ch, int size);
; ------------------------------------------------------------------------
memset:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	edx, [ebp + 12]	; Char to be putted
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	byte [edi], dl		; ┓
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回
; ------------------------------------------------------------------------
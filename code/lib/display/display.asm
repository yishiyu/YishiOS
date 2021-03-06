;========================================
;	字符界面显示函数
;========================================


; 导入全局变量
extern	disp_pos


[SECTION .text]

; 导出函数
global	disp_str
global	disp_color_str
global disp_int
global disp_clear_screen
global disp_char


; ------------------------------------------------------------------------
;	Dispchar
; 	显示一个字符(参数必须为ascii码可显示字符的地址)
;	通过堆栈传递一个字节变量
; ------------------------------------------------------------------------
disp_char:
	mov eax, [esp + 4]
	mov	edi, [disp_pos]

	; 处理回车符号
	cmp	al, 0dh
	jnz deal_with_return_end

	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	mov	[disp_pos], edi
	jmp disp_char_end

deal_with_return_end:
	; 处理退格符
	cmp	al, 08h
	jnz deal_with_back_end

	sub	edi, 2
	mov	ah, 0Fh
	mov al, 020h
	mov	[gs:edi], ax
	mov	[disp_pos], edi
	jmp disp_char_end

deal_with_back_end:

	mov	ah, 0Fh
	mov	[gs:edi], ax

	; 修改指针位置
	add	edi, 2
	mov	[disp_pos], edi

disp_char_end:
	ret

; ------------------------------------------------------------------------
;	DispStr
; 	显示一个字符串
;	通过堆栈传递一个4字节字符串起始位置变量
;	字符串结尾用0来标识
; ------------------------------------------------------------------------
disp_str:
	push	ebp
	mov	ebp, esp
	push	ebx
	push	esi
	push	edi

	mov	esi, [ebp + 8]
	mov	edi, [disp_pos]
	mov	ah, 0Fh

display_char:
	lodsb
	test	al, al					;test指令对符号位的影响和and指令相同
	jz	disp_end				;判断字符串是否已经显示完

	cmp	al, 0Ah					; 判断当前字符是否为回车
	jnz	disp_return_end

	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax

	; 实现滚屏操作,如果越界则回滚到第一行
	cmp eax, 20
	jbe disp_return_go_on
	xor eax,eax

disp_return_go_on:
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	display_char

disp_return_end:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	display_char

disp_end:
	mov	[disp_pos], edi

	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	ret
; DispStr 结束------------------------------------------------------------

; ------------------------------------------------------------------------
;	disp_color_str
; 	显示一个彩色字符串
;	比disp_str多一个颜色参数
;   后面的参数先入栈
;	字符串结尾用0来标识
; ------------------------------------------------------------------------
disp_color_str:
	push	ebp
	mov	ebp, esp
	push	ebx
	push	esi
	push	edi

	mov	esi, [ebp + 8]
	mov	edi, [disp_pos]
	mov ah, [ebp + 12]

display_color_char:
	lodsb
	test	al, al					;test指令对符号位的影响和and指令相同
	jz	disp_end				;判断字符串是否已经显示完

	cmp	al, 0Ah					; 判断当前字符是否为回车
	jnz	disp_color_return

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

disp_color_return:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	display_char

disp_color_end:
	mov	[disp_pos], edi

	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	ret
; DispStr 结束------------------------------------------------------------

; ------------------------------------------------------------------------
;	disp_int
;	显示一个整形数
;	通过堆栈传递一个Int参数
;	遵守c语言的规范: 堆栈谁调用谁清理
;	显示位置放在当前显示的数字后面,不负责换行
; ------------------------------------------------------------------------
disp_int:
	mov	eax, [esp + 4]
	shr	eax, 24
	call	disp_al

	mov	eax, [esp + 4]
	shr	eax, 16
	call	disp_al

	mov	eax, [esp + 4]
	shr	eax, 8
	call	disp_al

	mov	eax, [esp + 4]
	call	disp_al

	mov	ah, 07h			; 0000b: 黑底    0111b: 灰字
	mov	al, 'h'
	push	edi
	mov	edi, [disp_pos]
	mov	[gs:edi], ax
	add	edi, 4
	mov	[disp_pos], edi
	pop	edi

	ret
; disp_int 结束------------------------------------------------------------

; ------------------------------------------------------------------------
; 显示 AL 中的数字
; ------------------------------------------------------------------------
disp_al:
	push	ecx
	push	edx
	push	edi

	mov	edi, [disp_pos]

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

	mov	[disp_pos], edi

	pop	edi
	pop	edx
	pop	ecx

	ret
; disp_al 结束-------------------------------------------------------------


; ------------------------------------------------------------------------
; 清空屏幕并把指针归位到左上角
; ------------------------------------------------------------------------
disp_clear_screen:
	push	ecx
	push	edx
	push	edi

	mov edi, 0
	mov	ah, 0Fh			; 0000b: 黑底    1111b: 白字
	mov ecx, 80*25
	mov al, ' '

clear_process:
	mov	[gs:edi], ax
	add	edi, 2

	loop clear_process

	mov edi, (80 * 0 + 0) * 2
	mov	dword [disp_pos], edi

	pop	edi
	pop	edx
	pop	ecx

	ret
; disp_clear_screen 结束-------------------------------------------------------------
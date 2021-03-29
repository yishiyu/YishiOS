
	org	07c00h			; 告诉编译器程序加载到7c00处
	mov	ax, cs			; 之所以是7c00和bios有关
	mov	ds, ax
	mov	es, ax
	
	;输出helloworld
	mov	ax, BootMessage
	mov	bp, ax			; ES:BP = 串地址
	mov	cx, 16			; CX = 串长度
	mov	ax, 01301h		; AH = 13,  AL = 01h
	mov	bx, 000ch		; 页号为0(BH = 0) 黑底红字(BL = 0Ch,高亮)
	mov	dl, 0
	int	10h			; 10h 号中断
	jmp $			; 停机


BootMessage:		db	"Hello, OS world!"	;输出信息
times 	510-($-$$)	db	0					; 填充剩下的空间，使生成的二进制代码恰好为512字节
dw 	0xaa55									; 结束标志

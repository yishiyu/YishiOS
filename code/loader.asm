
Loader_start:
    mov ax, cs
	mov ds,	ax
	mov	es,	ax
	mov	ss,	ax
	mov	sp,	Stack_Base

    ;ES:BP = 串地址
	mov	bp,	Message
	mov cx,Message_Length
	mov ax,01301h
	mov	bx,	07h
    mov dh, 3
	mov	dl,	0
	int	10h	

    xchg bx,bx

	
    ; 栈底,从这个位置向低地址生长,BIOS ROM使用的最高地址为500h,只要不覆盖到那里就不会有问题 
    Stack_Base	equ	07c00h	
    Message:    db  "jojo! loading the load.bin successfully !!!"
    Message_Length  equ $-Message
    
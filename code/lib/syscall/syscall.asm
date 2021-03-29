; 实现系统调用必须的汇编部分

; 键盘系统调用的调用号
SYS_READ_KEYBOARD   equ     0
SYS_TERMIBAL_WRITE    equ     1
SYS_SENDREC equ 2

; 系统调用的中断号
SYS_CALL_VECTOR equ 0x90

global asm_read_keyboard
global asm_terminal_write
global asm_sendrec
global enable_int
global disable_int
global pause

;=========================
; 读取键盘函数
;=========================
asm_read_keyboard:
    mov eax, SYS_READ_KEYBOARD
    int SYS_CALL_VECTOR
    ret
    
;=========================
; 终端写入函数
;=========================
asm_terminal_write:
    mov eax, SYS_TERMIBAL_WRITE
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]
    int SYS_CALL_VECTOR
    ret

;=========================
; 进程间通信函数
;   asm_sendrec(int function, int src_dest, MESSAGE* m, int pid);
;=========================
asm_sendrec:
    mov eax, SYS_SENDREC
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]
    mov edx, [esp + 12]
    mov edi, [esp + 16]
    int SYS_CALL_VECTOR
    ret

;========================
; 控制中断
;========================
enable_int:
    sti
    ret
disable_int:
    cli
    ret

;========================
; 调试断点
;========================
pause:
    xchg bx,bx
    ret

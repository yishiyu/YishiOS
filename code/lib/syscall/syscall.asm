; 实现系统调用必须的汇编部分

; 键盘系统调用的调用号
SYS_READ_KEYBOARD   equ     0

; 系统调用的中断号
SYS_CALL_VECTOR equ 0x90

global asm_read_keyboard

;=========================
; 读取键盘函数
;=========================
asm_read_keyboard:
    mov eax, SYS_READ_KEYBOARD
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

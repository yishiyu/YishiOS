; 实现系统调用必须的汇编部分

; 系统调用的中断号
SYS_CALL_VECTOR equ 0x90

global asm_syscall
global enable_int
global disable_int
global pause

;=========================
; 系统调用通用函数,用于触发中断
;  最多传递五个参数,其中第一个为系统调用号
;  asm_syscall(int sys_vector, u32 para0,u32 para1,u32 para2, u32 para3);
;=========================
asm_syscall:
    mov eax, [esp + 4]
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    mov edx, [esp + 16]
    mov edi, [esp + 20]
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

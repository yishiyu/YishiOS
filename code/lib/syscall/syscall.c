// 直接调用的系统调用
#include "syscall.h"

//===============用户可以调用的系统调用==================
KEYMAP_RESULT sys_read_keyboard() {
    u8 scancode = (u8)asm_read_keyboard();

    KEYMAP_RESULT result;
    result.type = 0;
    result.data = scancode;
    return result;
}


//=================最终工作的函数======================
//读取键盘缓冲区并从中取数据
u32 kernel_read_keyboard(){

    return 8;
}
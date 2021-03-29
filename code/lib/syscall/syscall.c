// 直接调用的系统调用
#include "syscall.h"
#include "display.h"

//===============用户可以调用的系统调用==================
// 本函数工作在用户态,是用户态调用内核态函数的中间函数
// 键盘缓冲区结构体
// typedef struct s_keymap_result_buffer {
//     KEYMAP_RESULT result_buf[KEY_RESULT_NUM];
//     u8 key_head;
//     u8 key_tail;
//     int key_count;
// } KEYMAP_RESULT_BUFFER;
KEYMAP_RESULT sys_read_keyboard() {
    // 结果结构体
    KEYMAP_RESULT result;

    // 跳入内核态
    u32 temp = asm_read_keyboard();
    result.type = (char)(temp >> 8);
    result.data = (char)temp;

    return result;
}

//=================最终工作的函数======================
// 本函数工作在内核态
u32 kernel_read_keyboard() {
    // 4个8位无符号数拼成一个32位无符号数
    u8 result[4] = {0, 0, 0, 0};

    // 试图从键盘服务进程处理后的缓冲区中取出数据
    if (key_result_buffer.key_count <= 0) {
        // 奇了怪了如果下标写4,编译器居然不报错...
        result[1] = KEYBOARD_TYPE_EMPTY;
        result[0] = 0;
    } else {
        result[1] =
            key_result_buffer.result_buf[key_result_buffer.key_tail].type;
        result[0] =
            key_result_buffer.result_buf[key_result_buffer.key_tail].data;
        key_result_buffer.key_tail =
            (key_result_buffer.key_tail + 1) % KEY_RESULT_NUM;
        key_result_buffer.key_count--;
    }

    // 这个转换可真是个花活...需要考虑到系统是小端存储
    return *((u32*)(result));
}
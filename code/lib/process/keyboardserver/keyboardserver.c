#include "keyboardserver.h"

//#define __DEBUG_KEYBOARD_SERVER__

#ifndef __YISHIOS_DEBUG__
#define pause()
#define disp_int(str)
#define disp_str(str)
#else
#ifndef __DEBUG_KEYBOARD_SERVER__
#define pause()
#define disp_int(str)
#define disp_str(str)
#else
extern void pause();
#endif
#endif

// 静态变量 ,用于记录键盘状态
static int code_with_E0;
static int shift_l;
static int shift_r;
static int alt_l;
static int alt_r;
static int ctrl_l;
static int ctrl_r;
static int caps_lock;
static int num_lock;
static int scroll_lock;
static int column;

//子函数
u8 keyboard_server_get_keyboard();
void keyboard_server_decode();
void keyboard_server_save();
void keyboard_server_handle();
// 这两个函数来自syscall.asm ,用于控制中断的开启和关闭
extern void enable_int();
extern void disable_int();

// 全局变量
KEYMAP_RESULT result;  // 结果结构体

// 处理键盘信息的进程,负责处理系统热键
// 此进程属于系统任务级,可以直接调用内核函数
// 作为减少读取键盘中断耗费资源的一种方式
void keyboard_server() {
    // 初始化结果缓冲区
    key_result_buffer.key_head = 0;
    key_result_buffer.key_tail = 0;
    key_result_buffer.key_count = 0;
    while (1) {
        // 暂且不处理任何系统快捷键,对所有结果直接保存
        keyboard_server_decode();
        keyboard_server_handle();
        keyboard_server_save();
    }
}

// 从键盘缓冲区读取信息
u8 keyboard_server_get_keyboard() {
    u8 scancode;
    if (!key_buffer.key_flag[key_buffer.key_tail]) {
        scancode = 0xff;
    } else {
        // 2. 取出键值
        // 3. 修改尾指针
        // 4. 修改计数器
        scancode = key_buffer.key_buf[key_buffer.key_tail];
        key_buffer.key_tail = (key_buffer.key_tail + 1) % KEY_BUF_SIZE;
        key_buffer.key_count--;
        key_buffer.key_flag[(key_buffer.key_tail - 1) % KEY_BUF_SIZE] = 0;
    }

    return scancode;
}

// 对从键盘得到的信息进行译码
void keyboard_server_decode() {
    u8 scancode;
    int key_row;  // 键盘码表的行列坐标
    int key_column;
    u32 key_value;
    int make;  // 判断是否为make code.  1为make code,0 为break code

    // 读取一次数据
    scancode = keyboard_server_get_keyboard();

    // 读到空数据
    if (scancode == 0xff) {
        result.type = KEYBOARD_TYPE_EMPTY;
        result.data = 0;
    } else if (scancode == 0xe1) {
        result.type = KEYBOARD_TYPE_EMPTY;
        result.data = 0;
    } else if (scancode == 0xe0) {
        // 这个主要获取方向键
        key_column = 2;

        scancode = keyboard_server_get_keyboard();
        // 判断是make code还是break code
        make = (scancode & FLAG_BREAK ? 0 : 1);
        // 确定行
        key_row = (scancode & 0x7f) * MAP_COLS;
        // 取出数据
        key_value = keymap[key_row + key_column];

        // 最终处理
        result.type = KEYBOARD_TYPE_FUNC;
        result.data = (char)key_value;
        // 忽略break code
        if (!make) {
            result.type = KEYBOARD_TYPE_EMPTY;
            result.data = 0;
        }
    } else {
        // 判断是make code还是break code
        make = (scancode & FLAG_BREAK ? 0 : 1);

        // 确定行
        key_row = (scancode & 0x7f) * MAP_COLS;

        // 确定列
        key_column = 0;
        if (shift_l || shift_r) key_column = 1;

        // 取出数据
        key_value = keymap[key_row + key_column];

        // 对数据进行处理
        switch (key_value) {
            case SHIFT_L:
                shift_l = make;
                result.type = KEYBOARD_TYPE_FUNC;
                result.data = KEYBOARD_FUNC_SHIFT;
                break;
            case SHIFT_R:
                shift_r = make;
                result.type = KEYBOARD_TYPE_FUNC;
                result.data = KEYBOARD_FUNC_SHIFT;
                break;
            case CTRL_L:
                ctrl_l = make;
                result.type = KEYBOARD_TYPE_FUNC;
                result.data = KEYBOARD_FUNC_CTRL;
                break;
            case CTRL_R:
                ctrl_r = make;
                result.type = KEYBOARD_TYPE_FUNC;
                result.data = KEYBOARD_FUNC_CTRL;
                break;
            case ALT_L:
                alt_l = make;
                result.type = KEYBOARD_TYPE_FUNC;
                result.data = KEYBOARD_FUNC_ALT;
                break;
            case ALT_R:
                alt_r = make;
                result.type = KEYBOARD_TYPE_FUNC;
                result.data = KEYBOARD_FUNC_ALT;
                break;
            // 普通的可打印字符
            default:
                result.type = KEYBOARD_TYPE_ASCII;
                result.data = (char)key_value;
                break;
        }

        // 忽略break code
        if (!make) {
            result.type = KEYBOARD_TYPE_EMPTY;
            result.data = 0;
        }
    }
}

// 处理键盘事件
void keyboard_server_handle() {
    if (result.type != KEYBOARD_TYPE_FUNC) {
        return;
    }

    //按下ALT键切换tty
    if (result.data == KEYBOARD_FUNC_ALT) {
        disable_int();
        t_present_terminal += 1;
        t_present_terminal %= TERMINAL_NUM;
        enable_int();
    }
}

// 保存译码结果
void keyboard_server_save() {
    // 如果读取类型为空数据,直接返回
    if (result.type == KEYBOARD_TYPE_EMPTY) {
        return;
    }

    // 填充到缓冲区内
    // typedef struct s_keymap_result_buffer {
    //     KEYMAP_RESULT result_buf[KEY_RESULT_NUM];
    //     u8 key_head;
    //     u8 key_tail;
    //     int key_count;
    // } KEYMAP_RESULT_BUFFER;
    if (key_result_buffer.key_count < KEY_RESULT_NUM) {
        key_result_buffer.result_buf[key_result_buffer.key_head] = result;
        key_result_buffer.key_head =
            (key_result_buffer.key_head + 1) % KEY_RESULT_NUM;
        key_result_buffer.key_count++;
    }
}
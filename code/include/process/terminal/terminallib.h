// 终端结构体(其实就是类)
#ifndef YISHIOS_TERMINALLIB_H
#define YISHIOS_TERMINALLIB_H

#include "display.h"
#include "filesystem.h"
#include "global.h"
#include "keymap.h"
#include "macro.h"
#include "struct.h"
#include "syscall.h"
#include "type.h"

// 子函数
extern void disable_int();
extern void enable_int();
extern void out_byte(u16 port, u8 value);
extern u8 in_byte(u16 port);
extern int disp_pos;

// 仅在终端中使用的函数,不可被其他文件调用
// 相当于类中的成员函数
void terminal_init(TERMINAL* terminal);
void terminal_init_screen(TERMINAL* terminal);
void terminal_main(TERMINAL* terminal, int terminal_pid);
void terminal_handler(TERMINAL* terminal, KEYMAP_RESULT result);
void terminal_draw_screen(TERMINAL* terminal);
void terminal_set_cursor(TERMINAL* terminal);
void terminal_command_handler(TERMINAL* terminal);

// 屏幕显示函数
void terminal_disp_char(TERMINAL* terminal, char data);
void terminal_disp_str(TERMINAL* terminal, char* data);
void terminal_disp_int(TERMINAL* terminal, int data);

// shell命令对应的函数
void terminal_root(TERMINAL* terminal);
void terminal_ls(TERMINAL* terminal);
int terminal_cd(TERMINAL* terminal, char* file_name);

#endif

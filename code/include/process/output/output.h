// output子系统

#ifndef YISHIOS_OUTPUT_H
#define YISHIOS_OUTPUT_H

#include "global.h"
#include "type.h"
#include "struct.h"

// 入口函数
void output_server();

// 工具函数
extern int ldt_seg_linear(int pid, int seg_index);
extern void* va2la(int pid, void* va);
extern void disable_int();
extern void enable_int();
extern void out_byte(u16 port, u8 value);
extern u8 in_byte(u16 port);

// 子函数
void output_handler(MESSAGE* message);
void output_disp_str(CONSOLE* console, char* data);
void output_disp_func(CONSOLE* console,char func);
void output_draw_screen(CONSOLE* console);
void output_set_cursor(CONSOLE* console);

#endif

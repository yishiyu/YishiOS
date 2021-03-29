// input子系统

#ifndef YISHIOS_INPUT_H
#define YISHIOS_INPUT_H

#include "global.h"
#include "struct.h"
#include "type.h"

// 入口函数
void input_server();

// 子函数
void input_handler(MESSAGE* message);
void input_keyboard(MESSAGE* message);


#endif

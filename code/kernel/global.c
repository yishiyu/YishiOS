// 文件中没有函数和变量定义
// 存在的意义是使得汇编器在汇编的时候能找到include文件夹中头文件中的定义

#include "global.h"

//系统预定义进程初始状态
TASK task_table[BASE_TASKS_NUM] = {{TestA, STACK_SIZE_TESTA, "TestA"},
                                   {TestB, STACK_SIZE_TESTB, "TestB"},
                                   {TestC, STACK_SIZE_TESTC, "TestC"}};


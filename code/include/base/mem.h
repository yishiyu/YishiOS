// ======================================================================
// 内存操作函数
// ======================================================================

#ifndef YISHIOS_MEMFUNC_H
#define YISHIOS_MEMFUNC_H

#define phys_copy memcpy
#define phys_set memset

//内存复制函数
void* memcpy(void* p_dst, void* p_src, int size);

//内存赋值函数
void memset(void* p_dst, char ch, int size);

#endif
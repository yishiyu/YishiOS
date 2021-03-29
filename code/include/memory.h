// ======================================================================
// 内存操作函数
// ======================================================================

#ifndef	YISHIOS_MEMORY_H
#define	YISHIOS_MEMORY_H

//内存复制函数
void*	memcpy(void* p_dst, void* p_src, int size);

//内存赋值函数
void memset(void* p_dst, char ch, int size);

#endif
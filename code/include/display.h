// 定义字符界面的显示函数
#ifndef	YISHIOS_DISPLAY_H
#define	YISHIOS_DISPLAY_H

//简单显示字符
//彩色显示字符
void	disp_str(char * info);
void	disp_color_str(char * info, int color);
void    disp_int(int num);
//清空屏幕,把指针归位到0位置
void    disp_clear_screen();
#endif 
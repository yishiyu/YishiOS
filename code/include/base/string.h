// 一点一点处理字符串实在是太费劲了...
// 写成一套函数来处理这个

#ifndef YISHIOS_STRING_H
#define YISHIOS_STRING_H

int strlen(const char *s);
char *strcpy_limit(char *destination, const char *source, int count);
char *strcpy(char *destination, const char *source);
char *strcat(char *target, const char *source);
int atoi(const char *s);
char *itoa(int n, char *s);
int strcmp(const char *s1,const char *s2);
void str_replace(char *str, char src, char des);

#endif
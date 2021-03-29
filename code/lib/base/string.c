#include "string.h"

// 获取字符串长度
int strlen(const char *s) {
    const char *p;
    for (p = s; *p; ++p)
        ;
    return p - s;
}

// 复制字符串
char *strcpy_limit(char *destination, const char *source, int count) {
    while ((*destination++ = *source++) && (count--))
        ;
    return (destination - 1);
}
char *strcpy(char *destination, const char *source) {
    while (*destination++ = *source++)
        ;
    return (destination - 1);
}

// 字符串拼接
char *strcat(char *target, const char *source) {
    char *original = target;
    while (*target) target++;  // Find the end of the string
    while (*target++ = *source++)
        ;
    return (original);
}

// 字符串转整数
int atoi(const char *s) {
    int i, n, sign;
    for (i = 0; s[i] == ' '; i++)  //跳过空白符;
        sign = (s[i] == '-') ? -1 : 1;
    if (s[i] == '+' || s[i] == '-')  //跳过符号
        i++;
    for (n = 0; ((s[i] >= '0') && (s[i] <= '9')); i++)
        n = 10 * n + (s[i] - '0');  //将数字字符转换成整形数字
    return sign * n;
}

// 整数转字符串
char *itoa(int n, char *s) {
    //记录符号,保证n为正数
    if (n < 0) {
        n = -n;
        *s = '-';
        s++;
    }
    // 记录首字符位置
    char *head = s;

    // 转化整数
    do {
        *s = n % 10 + '0';
        s++;
    } while ((n /= 10) > 0);
    char *result = s;
    *(s--) = 0;

    // 翻转字符串
    char temp;
    for (; s > head; s--, head++) {
        temp = *head;
        *head = *s;
        *s = temp;
    }
    return result;
}
//内核进程的函数体
#include "kerneltask.h"
#include "syscall.h"

void TestA() {
    int i = 0;
    KEYMAP_RESULT result;
    while (1) {
        disp_str("A.");
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 1000; j++);
        }
        result = sys_read_keyboard();
        disp_int((int)result.data);
    }
}

void TestB() {
    int i = 0x1000;
    while (1) {
        //disp_str("B.");
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 1000; j++)
                ;
        }
    }
}

void TestC() {
    int i = 0x2000;
    while (1) {
        //disp_str("C.");
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 1000; j++)
                ;
        }
    }
}

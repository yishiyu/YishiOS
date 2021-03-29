//内核进程的函数体
#include "kerneltask.h"

void TestA() {
    int i = 0;
    KEYMAP_RESULT result;
    while (1) {
        // disp_str("A.");
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++)
                ;
        }
        result = sys_read_keyboard();
        if (result.type == KEYBOARD_TYPE_ASCII) {
            disp_char(result.data);
        }
        if(result.type == KEYBOARD_TYPE_FUNC){
            disp_int((int)result.data);
        }
    }
}

void TestB() {
    int i = 0x1000;
    while (1) {
        // disp_str("B.");
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 1000; j++)
                ;
        }
    }
}

void TestC() {
    int i = 0x2000;
    while (1) {
        // disp_str("C.");
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 1000; j++)
                ;
        }
    }
}

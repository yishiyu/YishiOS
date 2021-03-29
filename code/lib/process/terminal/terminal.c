#include "terminal.h"

// 第一个终端,同时也是默认的终端
void tty_1() {
    KEYMAP_RESULT temp;
    while (1) {
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++)
                ;
        }
        temp = sys_read_keyboard();
        if(temp.type == KEYBOARD_TYPE_ASCII){
            disp_str("  tty1:  ");
            disp_char(temp.data);
        }
    }
}

// 第二个终端
void tty_2() {
    KEYMAP_RESULT temp;
    while (1) {
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++)
                ;
        }
        temp = sys_read_keyboard();
        if(temp.type == KEYBOARD_TYPE_ASCII){
            disp_str("  tty2:  ");
            disp_char(temp.data);
        }
    }
}

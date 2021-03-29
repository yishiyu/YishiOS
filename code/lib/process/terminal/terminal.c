#include "terminal.h"

//后期tty任务的基础
void tty_1() {
    KEYMAP_RESULT result;
    while (1) {
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++)
                ;
        }
        result = sys_read_keyboard();
        if (result.type == KEYBOARD_TYPE_ASCII) {
            disp_char(result.data);
        }
        if (result.type == KEYBOARD_TYPE_FUNC) {
            disp_int((int)result.data);
        }
    }
}
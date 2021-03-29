#include "kerneltask.h"


// #define __DEBUG_KERNELTASK__

#ifndef __YISHIOS_DEBUG__
#define pause()
#define disp_int(str)
#define disp_str(str)
#else
#ifndef __DEBUG_KERNELTASK__
#define pause()
#define disp_int(str)
#define disp_str(str)
#else
extern void pause();
#endif
#endif

void empty_function() {
    while (1){
        disp_str("point kerneltask.c empty_function 0\n");
        pause();
    }
}
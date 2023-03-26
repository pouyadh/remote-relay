#include "utils.h"

u16 utilsU16 = 0;
void delay(u32 d) {while(d--);}

u8 flagTimeout(__IO u8 * reg,u8 mask,u8 state,u16 t) {
    if (state) {
        while((u8)(*reg & mask) == 0) {
            if (--t == 0) return 1;
        }
    } else {
        while((u8)(*reg & mask) != 0) {
            if (--t == 0) return 1;
        }
    }
    return 0;
}

u8 retry(u8 (*fn)(void),u8 expect, u8 times) {
    while ((*fn)() != expect) {
        if (--times == 0) return 1;
    };
    return 0;
}
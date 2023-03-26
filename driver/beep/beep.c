#include "beep.h"
#include "timestamp.h"


void beepPlay(const u16 *melody,u16 interval) {
    u8 i = 0;
    for(i=0;i<4;i++) {
        beepHz((u32)melody[i]);
        delayMs(interval);
    }
    beepOff();
}
void beepHz(u32 hz) {
    hz = 128000/hz;
    if (hz<64) {
        hz = hz / 2 - 2;
        beepOn((u8)hz,2);
    } else if (hz<128) {
        hz = hz / 4 - 2;
        beepOn((u8)hz,4);
    } else {
        hz = hz / 8 - 2;
        beepOn((u8)hz,8);
    }
}
void beepOn(u8 div, u8 sel) {
    if (div == 0) div = 1;
    if (div > 0x1E) div = 0x1E;
    if (sel > 2) sel = 2;
    BEEP->CSR = div | (u8)(1<<5) | (u8)(sel<<6);
}
void beepOff() {
    BEEP->CSR &= (u8)(~(1<<5));
}
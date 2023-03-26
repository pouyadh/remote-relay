#ifndef __BEEP_H
#define __BEEP_H
#include "stm8s.h"
void beepPlay(const u16 *melody,u16 interval);
void beepHz(u32 hz);
void beepOn(u8 div, u8 sel);
void beepOff();
#endif
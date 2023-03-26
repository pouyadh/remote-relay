#ifndef __UTILS_H
#define __UTILS_H
#include "stm8s.h"

extern u16 utilsU16;
#define repeat(times) for(utilsU16=times;utilsU16;utilsU16--)
#define whileClear(reg,mask) while((u8)(reg & bit) == 0);
#define whileSet(reg,mask) while((u8)(reg & bit) != 0);

void delay(u32 d);
u8 flagTimeout(__IO u8 * reg,u8 mask,u8 state,u16 t);

u8 retry(u8 (*fn)(void) ,u8 expect, u8 times);


#endif
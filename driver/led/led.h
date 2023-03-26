#ifndef __LED_H
#define __LED_H

#include "stm8s.h"

#define LED_GPIO GPIOC
#define LED_BIT 3
#define LED_BITMAST (1<<LED_BIT);

#define ledToggle() __BCPL((u8 *)&GPIOC->ODR,3)
#define ledOff() __BSET((u8 *)&GPIOC->ODR,3)
#define ledOn() __BRES((u8 *)&GPIOC->ODR,3)

#define LED_CODE_UNKNOWN 1
#define LED_CODE_EEPROM 2

void ledInit();
void ledBlink(u8 times,u16 interval);
void ledError(u8 code, u8 infinite);

#endif
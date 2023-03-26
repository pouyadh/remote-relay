#include "led.h"
#include "stm8s_gpio.h"
#include "timestamp.h"

void ledInit() {
    GPIO_Init(GPIOC,GPIO_PIN_3,GPIO_MODE_OUT_PP_HIGH_FAST);
}

void ledError(u8 code , u8 infinite) {
    ledErrorBegin:
    ledBlink(10,300);
    ledOff();
    delayMs(80000);
    ledBlink(code,2000);
    if (infinite) goto ledErrorBegin;
}

void ledBlink(u8 times,u16 interval) {
    if (times == 0) {
        while (1)
        {
            ledToggle();
            delayMs(interval);
        }
    } else {
        while(times--) {
            ledToggle();
            delayMs(interval);
            ledToggle();
            delayMs(interval);
        }
    }
}
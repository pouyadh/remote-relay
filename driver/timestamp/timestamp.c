#include "timestamp.h"
#include "stm8s.h"
#include "main.h"
#include "stm8s_clk.h"
#include "stm8s_tim4.h"

volatile u32 timestamp = 0;

void timestampInit() {
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4,ENABLE);
    TIM4_DeInit();
    TIM4_TimeBaseInit(TIM4_PRESCALER_128,125);
    TIM4_UpdateRequestConfig(TIM4_UPDATESOURCE_REGULAR);
    TIM4_Cmd(DISABLE);
    TIM4_ITConfig(TIM4_IT_UPDATE,ENABLE);
}

void timestampStart() {
    TIM4_Cmd(ENABLE);
}
void timestampStop() {
    TIM4_Cmd(DISABLE);
}
void timestampClear() {
    timestamp = 0;
}

void delayMs(u32 ms) {
    ms += timestamp;
    while (timestamp < ms);
}

INTERRUPT_HANDLER(TIM4_UPD_OVF_IRQHandler, 23) {
    __BRES((u8 *)&TIM4->SR1,0);
    timestamp++;
}
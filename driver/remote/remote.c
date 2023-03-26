#include "remote.h"
#include "remote-consts.h"
#include "stm8s_tim2.h"
#include "stm8s_clk.h"
#include "led.h"
#include "stm8s_itc.h"
#include "serial.h"
#include "string.h"
#include "timestamp.h"
#include "main.h"

typedef struct {
    u8 step;
    u8 ibit;
    u8 ibyte;
    u8 phlr;
    u8 ocp;
    u8 code[3];
    u8 sequenceCount;
    u8 antiCode;
} EV1527Tracker_t,HT6P20Tracker_t;



EV1527Tracker_t ev1527;
HT6P20Tracker_t ht6p20;

u16 th=0,tl=0,thl=0;
u8 edge=0,verifyCount=0;
u32 remoteButtonRelaseTimestamp = 0;
volatile u8 remoteStatus = 0;
volatile u8 remoteCode[3] = {0,0,0};
volatile RemoteType_t remoteType = REMOTE_TYPE_EV1527;


void ev1527CycleHandler();
void ht6p20CycleHandler();
void codeHandler(u8*code,RemoteType_t type);

void remoteInit() {
    
    TIM2_DeInit();
    th=0;tl=0;thl=0;edge=0;
    memset((u8*)&ev1527,0,sizeof(ev1527));
    memset((u8*)&ht6p20,0,sizeof(ev1527));

    CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2,ENABLE);
    TIM2_TimeBaseInit((TIM2_Prescaler_TypeDef)REMOTE_TIM1_PRESCALE_REG_VAL,65535);
    TIM2_Cmd(ENABLE);
    TIM2_ICInit(
        TIM2_CHANNEL_3,
        TIM2_ICPOLARITY_FALLING,
        TIM2_ICSELECTION_DIRECTTI,
        TIM2_ICPSC_DIV1,
        0
    );
    TIM2_CCxCmd(TIM2_CHANNEL_3,DISABLE);
    TIM2_ITConfig(TIM2_IT_CC3,ENABLE);
    remoteStatus = REMOTE_STATUS_INIT;
}
void remoteEnable(void) {
    th=0;tl=0;thl=0;edge=0;
    memset((u8*)&ev1527,0,sizeof(ev1527));
    memset((u8*)&ht6p20,0,sizeof(ev1527));
    __BRES((u8 *)&TIM2->SR1,3);
    __BRES((u8 *)&TIM2->SR2,3);
    TIM2_CCxCmd(TIM2_CHANNEL_3,ENABLE);
    remoteStatus |= REMOTE_STATUS_EN;

}
void remoteDisable(void) {
    TIM2_CCxCmd(TIM2_CHANNEL_3,DISABLE);
    remoteStatus &= (u8)(~REMOTE_STATUS_EN);
}

INTERRUPT_HANDLER(TIM2_CAP_COM_IRQHandler, 14) {
    TIM2->CNTRH = 0;TIM2->CNTRL = 0;
    if (TIM2->CCER2 & (1<<1)) {
        //Falling Edge
        __BRES((u8 *)&TIM2->CCER2,1);
        th = TIM2_GetCapture3();
        edge = 0;
    } else {
        //Rising Edge
        __BSET((u8 *)&TIM2->CCER2,1);
        tl = TIM2_GetCapture3();
        edge = 1;
    }
    thl = th + tl;
    ev1527CycleHandler();
    ht6p20CycleHandler();
}

void ev1527CycleHandler() {
    if (edge == 0) return;
    if (ev1527.step == 0) {
        if (thl < REMOTE_TP_MIN || thl > REMOTE_TP_MAX) return;
        ev1527.phlr = thl / th;
        ev1527.step = 1;
        ev1527.ibit = 0;
        ev1527.ibyte = 0;
        ev1527.code[0] = 0; ev1527.code[1] = 0; ev1527.code[2] = 0;
        ev1527.sequenceCount = 0;
        ev1527.ocp = thl/32;
    } else if (ev1527.step == 1) {
        if (thl < REMOTE_TD_MIN || thl > REMOTE_TD_MAX) { ev1527.step = 0; return;}
        ev1527.code[ev1527.ibyte] <<= 1;
        if (th>tl) ev1527.code[ev1527.ibyte]++;
        if (++ev1527.ibit == 8) {
            ev1527.ibit=0;
            if (++ev1527.ibyte == 3) ev1527.step = 2;
        }
    } else {
        if (thl < REMOTE_TP_MIN || thl > REMOTE_TP_MAX) { ev1527.step = 0; return;}
        codeHandler(ev1527.code,REMOTE_TYPE_EV1527);
        ev1527.step = 1;
        ev1527.ibit = 0;
        ev1527.ibyte = 0;
        ev1527.code[0] = 0; ev1527.code[1] = 0; ev1527.code[2] = 0;
        ev1527.sequenceCount++;
    }
}

void ht6p20CycleHandler() {
    if (edge == 1) return;
    if (ht6p20.step == 0) {
        if (thl < REMOTE_TP_MIN || thl > REMOTE_TP_MAX) return;
        ht6p20.phlr = thl / th;
        ht6p20.step = 1;
        ht6p20.ibit = 0;
        ht6p20.ibyte = 0;
        ht6p20.code[0] = 0; ht6p20.code[1] = 0; ht6p20.code[2] = 0;
        ht6p20.sequenceCount = 0;
        ht6p20.ocp = thl/24;
        ht6p20.antiCode = 0;
    } else if (ht6p20.step == 1) {
        if (thl < REMOTE_TD_MIN || thl > REMOTE_TD_MAX) { ht6p20.step = 0; return;}
        ht6p20.code[ht6p20.ibyte] <<= 1;
        if (tl>th) ht6p20.code[ht6p20.ibyte]++;
        if (++ht6p20.ibit == 8) {
            ht6p20.ibit=0;
            if (++ht6p20.ibyte == 3) ht6p20.step = 2;
        }
    } else if (ht6p20.step == 2) {
        if (thl < REMOTE_TD_MIN || thl > REMOTE_TD_MAX) { ht6p20.step = 0; return;}
        ht6p20.antiCode <<= 1;
        if (tl>th) ht6p20.antiCode++;
        if (++ht6p20.ibit == 4) {
            if (ht6p20.antiCode == 5) {
                ht6p20.step = 3;
            } else {
                ht6p20.step = 0;
            }
        }
    } else {
        if (thl < REMOTE_TP_MIN || thl > REMOTE_TP_MAX) { ht6p20.step = 0; return;}
        //Code Received
        codeHandler(ht6p20.code,REMOTE_TYPE_HT6P20);
        // ReInit
        ht6p20.step = 1;
        ht6p20.ibit = 0;
        ht6p20.ibyte = 0;
        ht6p20.code[0] = 0; ht6p20.code[1] = 0; ht6p20.code[2] = 0;
        ht6p20.sequenceCount++;
    }
}

void codeHandler(u8*code,RemoteType_t type) {
    if (
        remoteType != type || 
        remoteCode[0] != code[0] ||
        remoteCode[1] != code[1] ||
        remoteCode[2] != code[2]
    ) {
        // New Code
        verifyCount = 0;
        remoteButtonRelaseTimestamp = 0;
        remoteType = type;
        remoteCode[0] = code[0];
        remoteCode[1] = code[1];
        remoteCode[2] = code[2];
    } else {
        // Same as last code
        if (remoteButtonRelaseTimestamp) {
            if (timestamp < remoteButtonRelaseTimestamp) {
                remoteButtonRelaseTimestamp = timestamp + REMOTE_RELEASE_TIMEOUT;
                return;
            } else {
                remoteButtonRelaseTimestamp = 0;
                verifyCount = 0;
            }
        } else {
            verifyCount++;
        }
    }

    if (verifyCount < REMOTE_VERIFY_COUNT) return;

    remoteButtonRelaseTimestamp = timestamp + REMOTE_RELEASE_TIMEOUT;
    verifyCount = 0;
    remoteType = type;
    remoteCode[0] = code[0];
    remoteCode[1] = code[1];
    remoteCode[2] = code[2];
    remoteStatus |= REMOTE_STATUS_CODE;
}
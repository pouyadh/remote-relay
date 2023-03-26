#include "stm8s_conf.h"
#include "serial.h"
#include "utils.h"
#include "main.h"
#include "eeprom.h"
#include "led.h"
#include "remote.h"
#include "timestamp.h"
#include "stm8s_beep.h"
#include "beep.h"
#include "remote-store.h"
#include "init.h"
#include "string.h"

const u16 beepStartupMelody[] = {500,900,4000,200};
const u16 beepLearnMelody[] = {400,600,800,1000};
const u16 beepClearMemoryMelody[] = {1000,500,300,2000};
const u16 beepExistMelody[] = {500,4000,500,1000};
const u16 beepMemoryFullMelody[] = {100,4000,1500,900};
const u16 beepCancelMelody[] = {100,500,100,100};
const u16 beepPushMelody[] = {100,100,100,100};


u8 output;
//u8 tempCode[3];
u8 result;
u8 i2;
u8 *ptr;

typedef enum {
    LEARN_MODE_CANCEL = -1,
    LEARN_MODE_ALL = 0,
    LEARN_MODE_RELAY1 = 1,
    LEARN_MODE_RELAY2 = 2,
    LEARN_MODE_RELAY3 = 3,
    LEARN_MODE_RELAY4 = 4,
} LearnMode;

LearnMode learnMode = LEARN_MODE_ALL;
u32 t = 0;

int main() {
    
    init();
    enableInterrupts();

    ledBlink(10,50);

    if (isKeyPushed()) {
        //Clear Memory
        remoteStoreErase();
        beepPlay(beepClearMemoryMelody,100);
        while(isKeyPushed());
    }

    beepPlay(beepStartupMelody,100);
    ledOn();
    remoteEnable();

    
    
    while (1){
        // if (isKeyPushed()) {
        //     //Learning Preocess
        //     remoteDisable();
        //     while(isKeyPushed());
        //     delayMs(100);
        //     remoteEnable();
        //     learnMode = LEARN_MODE_ALL;
        //     while (1)
        //     {
        //         if (isRemoteCodeReceived()) {
        //             remoteDisable();
        //             remoteCode[2] &= 0xF0;
        //             memcpy((u8*)tempCode+1,remoteCode,3);
        //             (u8*)&tempCode = learnMode;
        //             result = remoteStoreAdd(&tempCode);
        //             if (result == 0) {
        //                 //Success
        //                 beepPlay(beepLearnMelody,100);
        //                 ledBlink(4,100);
        //             } else if (result == 1) {
        //                 //Exist
        //                 beepPlay(beepExistMelody,100);
        //                 ledBlink(2,300);
        //             } else {
        //                 //Memory Full
        //                 beepPlay(beepMemoryFullMelody,100);
        //                 ledBlink(3,300);
        //             }
        //             remoteEnable();
        //             break;
        //         }
        //         if (isKeyPushed()) {
        //             t = timestamp + 3000;
        //             while (isKeyPushed()) {
        //                 if (t && timestamp > t) {
        //                     //Cancel Learning
        //                     beepPlay(beepCancelMelody,100);
        //                     learnMode = LEARN_MODE_CANCEL;
        //                     t = 0;
        //                     ledOff();
        //                 }
        //             }
        //             if (learnMode == LEARN_MODE_CANCEL) break;

        //             if (++learnMode > LEARN_MODE_RELAY4) learnMode = LEARN_MODE_ALL;
        //             beepPlay(beepPushMelody,25);
                    
        //         }
        //         ledBlink(1,100);
        //         ledBlink(learnMode,500);
        //     }
        //     ledOn();
        // }
        if(isRemoteCodeReceived()) {
            // Remote Pushed
            u8 tmpCode[3],tmpType;
            memcpy((u8*)tmpCode,(u8 *)remoteCode,3);
            tmpType = remoteType;
            serialSend((u8*)tmpCode,3);
            serialSendChar(tmpType);
            // u8 buttons = c[2] & 0x0F;
            // c[2] &= 0xF0;
            // //if (hasCode(c)) {
            // if (remoteStoreHas((u8*)c)) {
            //     // Exists in memory
            //     output ^= buttons;
            //     updateRelays();
            //     ledBlink(10,50);
            // }
            remoteClearCodeFlag();
            ledToggle();
        }
    }
}


void updateRelays() {
    if (output & 0x1) 
        GPIO_WriteHigh(RELAY1_GPIO,RELAY1_GPIO_PIN);
    else GPIO_WriteLow(RELAY1_GPIO,RELAY1_GPIO_PIN);

    if (output & 0x2) 
        GPIO_WriteHigh(RELAY2_GPIO,RELAY2_GPIO_PIN);
    else GPIO_WriteLow(RELAY2_GPIO,RELAY2_GPIO_PIN);

    if (output & 0x4) 
        GPIO_WriteHigh(RELAY3_GPIO,RELAY3_GPIO_PIN);
    else GPIO_WriteLow(RELAY3_GPIO,RELAY3_GPIO_PIN);

    if (output & 0x8) 
        GPIO_WriteHigh(RELAY4_GPIO,RELAY4_GPIO_PIN);
    else GPIO_WriteLow(RELAY4_GPIO,RELAY4_GPIO_PIN);
}

void assert_failed(uint8_t* file, uint32_t line) {}

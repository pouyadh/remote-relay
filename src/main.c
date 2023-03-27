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
const u16 beepDeleteMelody[] = {1000,800,600,400};

u8 output;
u8 result;
u8 tmpCode[3],tmpType,buttons;
u32 lightOffTimestamp;
u8 i;

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
        if (isKeyPushed()) {
            //Learning Preocess
            remoteDisable();
            delayMs(100);
            while(isKeyPushed());
            remoteEnable();
            while (1)
            {
                if (isRemoteCodeReceived()) {
                    remoteDisable();
                    remoteClearCodeFlag();
                    remoteCode[2] &= 0xF0;
                    result = remoteStoreAdd((u8*)remoteCode);
                    if (result == 0) {
                        //Success
                        beepPlay(beepLearnMelody,100);
                        ledBlink(4,100);
                    } else if (result == 1) {
                        //Exist
                        beepPlay(beepExistMelody,100);
                        for (i=0;i<5;i++) {
                            ledBlink(1,300);
                            if (isKeyPushed()) {
                                delayMs(100);
                                while (isKeyPushed());
                                result = remoteStoreRemove((u8*)remoteCode);
                                if (result == 0 ) beepPlay(beepDeleteMelody,100);
                                break;
                            }
                        }

                    } else {
                        //Memory Full
                        beepPlay(beepMemoryFullMelody,100);
                        ledBlink(3,300);
                    }
                    remoteEnable();
                    break;
                }
                if (isKeyPushed()) {
                    delayMs(100); 
                    while (isKeyPushed());
                    beepPlay(beepCancelMelody,100);   
                    break;                
                }
                ledBlink(1,100);
            }
            ledOn();
        }
        if(isRemoteCodeReceived()) {
            // Remote Pushed
            memcpy((u8*)tmpCode,(u8 *)remoteCode,3);
            tmpType = remoteType;
            
            buttons = tmpCode[2] & 0x0F;
            tmpCode[2] &= 0xF0;
            if (remoteStoreHas((u8*)tmpCode)) {
                // Exists in memory
                output ^= buttons;
                updateRelays();
                lightOn();
                lightOffTimestamp = timestamp + LIGHT_ON_DURATION;
                ledBlink(5,50);
            }
            remoteClearCodeFlag();
        }
        if (lightOffTimestamp && timestamp > lightOffTimestamp) {
            lightOff();
            lightOffTimestamp = 0;
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

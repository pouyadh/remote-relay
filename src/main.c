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
#include "store.h"
#include "init.h"

#define codecpy(t,s) t[0]=s[0];t[1]=s[1];t[2]=t[2];

const u16 beepStartupMelody[] = {500,900,4000,200};
const u16 beepLearnMelody[] = {400,600,800,1000};
const u16 beepClearMemoryMelody[] = {1000,500,300,2000};
const u16 beepExistMelody[] = {500,4000,500,1000};
const u16 beepMemoryFullMelody[] = {100,4000,1500,900};
const u16 beepCancelMelody[] = {100,500,100,100};
const u16 beepPushMelody[] = {100,100,100,100};
const u16 beepDeleteMelody[] = {1000,800,600,400};
const u16 beepNotFoundMelody[] = {500,4000,500,1000};
//const u16 beepTimeoutMelodyMelody[] = {500,4000,500,1000};

const u32 relaseDelay = 1000;

u8 relayInverse;
u8 result;
u8 lastButtonNumber,tmpButtonNumber,buttonMode;
u32 lightOffTimestamp;
u32 menuTimeoutTimestamp;
u8 i;
u8 *remoteptr , *tmpptr;
u32 activeRemoteTimeoutTimestamp = 0;

typedef enum {
    MENU_LEARN = 0,
    MENU_REMOVE = 1,
    MENU_SET_AS_MOMENTARY = 2,
    MENU_SET_AS_LATCH = 3,
    MENU_SET_AS_PUSHING = 4,
    MENU_SET_MOMENTARY_DELAY = 5,
    MENU_SET_LIGHT_ON_DURATION = 6,
} Menu;

Menu menu = MENU_LEARN;

u32 relayInverseTimestamp[4];
u32 relaseTimestamp;



void factoryReset();
void learn();
void remove();
void setMode(u8 mode);
void setMomentaryDelay();
void setLightOnDuration();

u8 getButtonNumber(u8 code[3]);
u8 getButtonMode(u8 *remoteptr,u8 buttonNumber);

void waitUntilKeyRelease();
void waitUntilKeyRelease() {
    delayMs(100);
    watchdogPause(); 
    while(isKeyPushed()); 
    watchdogRun();
    delayMs(100);
}

int main() {
    
    init();
    enableInterrupts();

    ledBlink(10,50);

    if (isKeyPushed()) {
        //Clear Memory
        factoryReset();
        waitUntilKeyRelease();
    }

    beepPlay(beepStartupMelody,100);
    ledOn();

    if (store.lightOnDuration == 0xFFFFFFFF) {
        store.lightOnDuration = 3000;
    }
    if (store.momentaryDuration == 0xFFFFFFFF) {
        store.momentaryDuration = 7000;
    }

    remoteEnable();
    remoteClearCodeFlag();

    

    
    
    while (1){
        watchdogRefresh();
        if (isKeyPushed()) {
            //Learning Preocess
            remoteDisable(); 
            menu = MENU_LEARN;
            i = 0;
            while (isKeyPushed()) {
                watchdogRefresh();
                delayMs(100);
                if (++i >= 10) {
                    i = 0;
                    if (menu<MENU_SET_LIGHT_ON_DURATION) {
                        beep(100);
                        menu++;
                    }
                }
            }
            remoteEnable();
            menuTimeoutTimestamp = timestamp + LEARN_TIMEOUT;
            switch (menu) {
            case MENU_LEARN: learn(); break;
            case MENU_REMOVE: remove(); break;
            case MENU_SET_AS_MOMENTARY: setMode(0x02); break;
            case MENU_SET_AS_LATCH: setMode(0x00); break;
            case MENU_SET_AS_PUSHING: setMode(0x01); break;
            case MENU_SET_MOMENTARY_DELAY: setMomentaryDelay();break;
            case MENU_SET_LIGHT_ON_DURATION: setLightOnDuration();break;
            default:
                break;
            }
            ledOn();
        }
        if(isRemoteCodeReceived()) {
            tmpptr = storeGet((u8*)remoteCode);
            tmpButtonNumber = getButtonNumber((u8*)remoteCode);

            if (tmpptr) {
                if (tmpButtonNumber != lastButtonNumber) relaseTimestamp = 0;
                lastButtonNumber = tmpButtonNumber;

                buttonMode = getButtonMode(tmpptr , tmpButtonNumber);
                if (buttonMode == BUTTON_MODE_LATCH) {
                    if (timestamp > relaseTimestamp) relayInverse = tmpButtonNumber;
                    relaseTimestamp = timestamp + relaseDelay;
                    relayInverseTimestamp[tmpButtonNumber -1] = 0;
                } else if (buttonMode == BUTTON_MODE_MOMENTARY) {
                    if (relayInverseTimestamp[tmpButtonNumber -1] == 0) relayInverse = tmpButtonNumber;
                    relayInverseTimestamp[tmpButtonNumber -1] = timestamp + store.momentaryDuration;
                } else if (buttonMode == BUTTON_MODE_PUSHING) {
                    if (relayInverseTimestamp[tmpButtonNumber -1] == 0) relayInverse = tmpButtonNumber;
                    relayInverseTimestamp[tmpButtonNumber -1] = timestamp + 1000;
                }
                
                remoteptr = tmpptr;
                lightOn();
                lightOffTimestamp = timestamp + store.lightOnDuration;
            }

            
            
            remoteClearCodeFlag();
        }

        if (lightOffTimestamp && timestamp > lightOffTimestamp) {
            lightOff();
            lightOffTimestamp = 0;
        }
        updateRelays();
    }
}

void updateRelays() {
    if (relayInverse == 0) {
        for(i=0;i<=3;i++) {
        if (relayInverseTimestamp[i] && timestamp > relayInverseTimestamp[i]) {
            relayInverse = i+1;
            relayInverseTimestamp[i] = 0;
            break;
        }
    }
    }
    if (relayInverse == 1) GPIO_WriteReverse(RELAY1_GPIO,RELAY1_GPIO_PIN);
    if (relayInverse == 2) GPIO_WriteReverse(RELAY2_GPIO,RELAY2_GPIO_PIN);
    if (relayInverse == 3) GPIO_WriteReverse(RELAY3_GPIO,RELAY3_GPIO_PIN);
    if (relayInverse == 4) GPIO_WriteReverse(RELAY4_GPIO,RELAY4_GPIO_PIN);
    relayInverse = 0;
}


void factoryReset() {
    storeErase();
    beepPlay(beepClearMemoryMelody,100);
}

void learn() {
    u32 timeoutT = timestamp + UNI_TIMEOUT;
    while(isRemoteCodeReceived() ==0 ) {
        ledBlink(1,100);
        if(isKeyPushed() || timestamp > timeoutT) {
            beep(1200);
            remoteClearCodeFlag();
            waitUntilKeyRelease();
            return;
        }
    }
    result = storeAdd((u8*)remoteCode);
    if (result == 0) {
        //Success
        beepPlay(beepLearnMelody,100);
        ledBlink(4,100);
    } else if (result == 1) {
        //Exist
        beepPlay(beepExistMelody,100);
        ledBlink(3,300);
    } else {
        //Memory Full
        beepPlay(beepMemoryFullMelody,100);
        ledBlink(3,300);
    }
    remoteClearCodeFlag();
}

void remove() {
    u32 timeoutT = timestamp + UNI_TIMEOUT;
    while(isRemoteCodeReceived() ==0 ) {
        ledBlink(1,100);
        if(isKeyPushed() || timestamp > timeoutT) {
            beep(1200);
            remoteClearCodeFlag();
            waitUntilKeyRelease();
            return;
        }
    }
    result = storeRemove((u8*)remoteCode);
    if (result == 1) {
        beepPlay(beepNotFoundMelody,100);
        ledBlink(3,300);
    } else {
        beepPlay(beepDeleteMelody,100);
        ledBlink(4,100);
    }
    remoteClearCodeFlag();
}

void setMode(u8 mode) {
    u32 timeoutT;
    againT:
    timeoutT = timestamp + UNI_TIMEOUT;
    again:
    remoteClearCodeFlag();
     while(isRemoteCodeReceived() ==0 ) {
        ledBlink(1,100);
        if(isKeyPushed() || timestamp > timeoutT) {
            beep(1200);
            remoteClearCodeFlag();
            waitUntilKeyRelease();
            return;
        }
    }
    remoteptr = storeGet((u8*)remoteCode);
    if (remoteptr == 0)  goto again;
    tmpButtonNumber = getButtonNumber((u8*)remoteCode);
    switch (tmpButtonNumber) {
    case 1:
        remoteptr[3] &= 0xFC;
        remoteptr[3] |= mode;
        break;
    case 2:
        remoteptr[3] &= 0xF3;
        remoteptr[3] |= mode << 2;
        break;
    case 3:
        remoteptr[3] &= 0xCF;
        remoteptr[3] |= mode << 4;
        break;
    case 4:
        remoteptr[3] &= 0x3F;
        remoteptr[3] |= mode << 6;
        break;
    default:
        break;
    }
    storeSave(remoteptr,4);
    beepPlay(beepLearnMelody,100);
    ledBlink(4,100);
    goto againT;
}

u8 getButtonNumber(u8 code[3]) {
    u8 btnBits = code[2] & 0x0F;
    switch (btnBits) {
    case 0x01:return 1;
    case 0x02:return 2;
    case 0x04:return 3;
    case 0x08:return 4;
    //case 0x0C:return 34;
    default: return 0xFF;
    }
}
u8 getButtonMode(u8 *remoteptr,u8 buttonNumber) {
    buttonNumber--;
    buttonNumber*=2;
    u8 mode = remoteptr[3] >> buttonNumber;
    mode &= 0x3;
    return mode;
}

void setMomentaryDelay() {
    u32 timeoutT = timestamp + UNI_TIMEOUT;
    u8 d = 0;
    u8 mm = 0;
    while (isKeyReleased()) {
        watchdogRefresh();
        if (timestamp > timeoutT) {
            beep(1200);
            return;
        }
    }
    while (isKeyPushed()) {
        watchdogRefresh();
        delayMs(100);
        if (++d >= 10) {
            d = 0;
            if (mm<255) {
                beep(200);
                mm++;
            }
        }
    }
    if (mm != 0) {
        store.momentaryDuration = mm * 1000;
        storeSave((u8*)&store.momentaryDuration,sizeof(store.momentaryDuration));
        ledBlink(4,100);
    } else {
        ledBlink(3,300);
    }
    
}

void setLightOnDuration() {
    u32 timeoutT = timestamp + UNI_TIMEOUT;
    u8 d = 0;
    u8 mm = 0;
    while (isKeyReleased()) {
        watchdogRefresh();
        if (timestamp > timeoutT) {
            beep(1200);
            return;
        }
    }
    while (isKeyPushed()) {
        watchdogRefresh();
        delayMs(100);
        if (++d >= 10) {
            d = 0;
            if (mm<255) {
                beep(200);
                mm++;
            }
        }
    }
    if (mm != 0) {
        store.lightOnDuration = mm * 1000;
        storeSave((u8*)&store.lightOnDuration,sizeof(store.lightOnDuration));
        ledBlink(4,100);
    } else {
        ledBlink(3,300);
    }
    
}

void assert_failed(uint8_t* file, uint32_t line) {}

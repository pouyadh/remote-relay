#include "init.h"
#include "stm8s_clk.h"
#include "timestamp.h"
#include "remote.h"
#include "remote-store.h"
#include "serial.h"
#include "led.h"
#include "main.h"

void clkInit();
void gpioInit();



void init() {
    u8 retry,res;
    clkInit();
    timestampInit();
    timestampStart();
    ledInit();
    serialInit();

    retry = 3; 
    do { res = remoteStoreInit(); } while (res && --retry);

    if (res) ledError(LED_CODE_EEPROM,TRUE);
    
    remoteInit();
    gpioInit();
}



void clkInit(void) {
    CLK_DeInit();                
    CLK_HSECmd(DISABLE);
    CLK_LSICmd(DISABLE);
    CLK_HSICmd(ENABLE);
    while(CLK_GetFlagStatus(CLK_FLAG_HSIRDY) == RESET);
    CLK_ClockSwitchCmd(ENABLE);
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
    CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);
    CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSI, 
    DISABLE, CLK_CURRENTCLOCKSTATE_ENABLE);
}

void gpioInit() {
    GPIO_Init(KEY_GPIO,KEY_GPIO_PIN,GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(RELAY1_GPIO,RELAY1_GPIO_PIN,GPIO_MODE_OUT_PP_LOW_FAST);
    // GPIO_Init(RELAY2_GPIO,RELAY2_GPIO_PIN,GPIO_MODE_OUT_PP_LOW_FAST);
    // GPIO_Init(RELAY3_GPIO,RELAY3_GPIO_PIN,GPIO_MODE_OUT_PP_LOW_FAST);
    // GPIO_Init(RELAY4_GPIO,RELAY4_GPIO_PIN,GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(LIGHT_GPIO,LIGHT_GPIO_PIN,GPIO_MODE_OUT_PP_LOW_FAST);
}
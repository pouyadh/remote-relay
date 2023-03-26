#include "serial.h"

void serialInit(void) {
    UART1_DeInit();
    UART1_Init(
        9600,
        UART1_WORDLENGTH_8D,
        UART1_STOPBITS_1,
        UART1_PARITY_NO,
        UART1_SYNCMODE_CLOCK_DISABLE,
        UART1_MODE_TX_ENABLE
    );
}

void serialSend(u8 *data, u8 len) {
    while (len--)
    {
        UART1_SendData8(*data);
        data++;
        while (!(UART1_GetFlagStatus(UART1_FLAG_TXE)));
    }
}
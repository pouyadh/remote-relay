#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "stm8s.h"
#include "stm8s_uart1.h"
#define serialSendChar(x) UART1_SendData8(x);

void serialInit(void);
void serialSend(u8 *data,u8 len);

#endif
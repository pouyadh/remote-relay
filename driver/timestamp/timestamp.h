#ifndef __TIMESTAMP_H
#define __TIMESTAMP_H

#include "stm8s.h"
extern volatile u32 timestamp;
void timestampInit(void);
void timestampStart(void);
void timestampStop(void);
void timestampClear(void);
void delayMs(u32 ms);

#endif
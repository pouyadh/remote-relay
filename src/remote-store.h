#ifndef __REMOTE_STORE_H
#define __REMOTE_STORE_H

#include "stm8s.h"
#include "eeprom.h"

#define REMOTE_STORE_SIZE 99


u8 remoteStoreInit();
u8 remoteStoreAdd(u8 code[3]);
u8 remoteStoreErase();
bool remoteStoreHas(u8 code[3]);

#endif 
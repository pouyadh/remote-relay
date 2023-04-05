#ifndef __REMOTE_STORE_H
#define __REMOTE_STORE_H

#include "stm8s.h"
#include "eeprom.h"

#define REMOTE_STORE_SIZE 99

typedef struct {
    u8 button;
    u8 mode;
} Remote;

typedef struct {
    u8 codes[REMOTE_STORE_SIZE*4];
    u32 momentaryDuration;
    u32 lightOnDuration;
} Store_t;

extern Store_t store;

u8 storeInit();
u8 storeAdd(u8 code[3]);
u8 storeErase();
bool storeHas(u8 code[3]);
u8 storeRemove(u8 code[3]);
u8* storeGet(u8 code[3]);
u8 storeSave(u8* ptr,u8 len);

#endif 
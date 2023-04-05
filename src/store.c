#include "store.h"

Store_t store;

u8 storeInit() {
    u8 tmp;
    tmp = eepromInit();
    if (tmp) return tmp;
    tmp = eepromRead((u8*)&store,0,sizeof(store));
    if (tmp) return tmp;
    return 0;
}

u8 storeAdd(u8 code[3]) {
    u8 code2 = code[2] & 0xF0;
    u8 *rptr;
    u8 *empty_rptr = 0;
    for(rptr=store.codes;rptr<=&store.codes[REMOTE_STORE_SIZE*4 - 1];rptr+=4) {
        if (
            !empty_rptr &&
            rptr[0] == 0xFF && 
            rptr[1] == 0xFF && 
            rptr[2] == 0xFF
        ) empty_rptr = rptr;
        if (
            rptr[0] == code[0] && 
            rptr[1] == code[1] && 
            rptr[2] == code2
        ) return 1;
    }
    if (!empty_rptr) return 2;

    empty_rptr[0] = code[0];
    empty_rptr[1] = code[1];
    empty_rptr[2] = code2;
    empty_rptr[3] = 0x00;

    if (eepromWrite(empty_rptr,(u16)(empty_rptr-(u8*)&store),4)) return 3;
    return 0;
}

u8 storeRemove(u8 code[3]) {
    u8 code2 = code[2] & 0xF0;
    u8 *rptr;
    for(rptr=store.codes;rptr<=&store.codes[REMOTE_STORE_SIZE*4 - 1];rptr+=4) {
        if (
            rptr[0] == code[0] && 
            rptr[1] == code[1] && 
            rptr[2] == code2
        ) {
            rptr[0] = 0xFF;
            rptr[1] = 0xFF;
            rptr[2] = 0xFF;
            rptr[3] = 0xFF;
            if (eepromWrite(rptr,(u16)(rptr-(u8*)&store),4)) return 3;
            return 0;
        }
    }
    return 1;
}

u8 storeErase() {
    u8 *ptr = (u8*)&store;
    u8 *ptrEnd = ptr + sizeof(store) - 1;
    do {
        *ptr = 0xFF;
    } while (++ptr <= ptrEnd);
    if (eepromWrite((u8*)&store,0,sizeof(store))) return 3;
    return 0;
}

bool storeHas(u8 code[3]) {
    u8 code2 = code[2] & 0xF0;
    u8 *rptr;
    for(rptr=store.codes;rptr<=&store.codes[REMOTE_STORE_SIZE*4 - 1];rptr+=4) {
        if (
            rptr[0] == code[0] && 
            rptr[1] == code[1] && 
            rptr[2] == code2
        ) return TRUE;
    }
    return FALSE;
}

u8* storeGet(u8 code[3]) {
    u8 code2 = code[2] & 0xF0;
    u8 *rptr;
    for(rptr=store.codes;rptr<=&store.codes[REMOTE_STORE_SIZE*4 - 1];rptr+=4) {
        if (
            rptr[0] == code[0] && 
            rptr[1] == code[1] && 
            rptr[2] == code2
        ) {
            return rptr;
        }
    }
    return 0;
}

u8 storeSave(u8* ptr,u8 len) {
    if (eepromWrite(ptr,(u16)(ptr-(u8*)&store),len)) return 3;
    return 0;
}
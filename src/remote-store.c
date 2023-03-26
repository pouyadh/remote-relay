#include "remote-store.h"

u8 remoteCodes[REMOTE_STORE_SIZE*3];

u8 remoteStoreInit() {
    u8 tmp;
    tmp = eepromInit();
    if (tmp) return tmp;
    tmp = REMOTE_STORE_SIZE - 1;
    tmp = eepromRead((u8 *)remoteCodes,0,sizeof(remoteCodes));
    if (tmp) return tmp;
    return 0;
}

u8 remoteStoreAdd(u8 code[3]) {
    u8 *rptr;
    u8 *empty_rptr = 0;
    for(rptr=remoteCodes;rptr<=&remoteCodes[REMOTE_STORE_SIZE-1];rptr+=3) {
        if (
            !empty_rptr &&
            rptr[0] == 0xFF && 
            rptr[1] == 0xFF && 
            rptr[2] == 0xFF
        ) empty_rptr = rptr;
        if (
            rptr[0] == code[0] && 
            rptr[1] == code[1] && 
            rptr[2] == code[2]
        ) return 1;
    }
    if (!empty_rptr) return 2;

    empty_rptr[0] = code[0];
    empty_rptr[1] = code[1];
    empty_rptr[2] = code[2];

    if (eepromWrite((u8*)code,(u16)(empty_rptr-remoteCodes),3)) return 3;
    return 0;
}

u8 remoteStoreErase() {
    u8 *rptr;
    for(rptr=remoteCodes;rptr<=&remoteCodes[REMOTE_STORE_SIZE-1];rptr+=3) {
        rptr[0] = 0xFF; rptr[1] = 0xFF; rptr[2] = 0xFF;
    }
    if (eepromWrite((u8*)remoteCodes,0,sizeof(remoteCodes))) return 3;
    return 0;
}

bool remoteStoreHas(u8 code[3]) {
    u8 *rptr;
    for(rptr=remoteCodes;rptr<=&remoteCodes[REMOTE_STORE_SIZE-1];rptr+=3) {
        if (
            rptr[0] == code[0] && 
            rptr[1] == code[1] && 
            rptr[2] == code[2]
        ) return TRUE;
    }
    return FALSE;
}
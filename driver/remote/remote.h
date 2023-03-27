#ifndef __REMOTE_H
#define __REMOTE_H

#include "stm8s.h"

#define REMOTE_VERIFY_COUNT 1
#define REMOTE_RELEASE_TIMEOUT 1000

#define REMOTE_STATUS_INIT ((u8)(1<<0))
#define REMOTE_STATUS_EN ((u8)(1<<1))
#define REMOTE_STATUS_CODE ((u8)(1<<2))

#define isRemoteCodeReceived() (remoteStatus & REMOTE_STATUS_CODE)
#define remoteClearCodeFlag() (remoteStatus &= (u8)(~REMOTE_STATUS_CODE))

void remoteInit(void);
void remoteEnable(void);
void remoteDisable(void);

typedef enum {
    REMOTE_TYPE_EV1527 = 0,
    REMOTE_TYPE_HT6P20 = 1,
} RemoteType_t;

extern volatile u8 remoteCode[3];
extern volatile RemoteType_t remoteType;
extern volatile u8 remoteStatus;

#endif
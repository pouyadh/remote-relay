#ifndef __EEPROM_H
#define __EEPROM_H

#include "stm8s.h"
#define EEPROM_ADDRESS 0x50
#define EEPROM_CLOCK 100000
#define EEPROM_TIMEOUT 20000

u8 eepromInit();
u8 eepromRead(u8 *buffer,u16 address, u16 len);
u8 eepromWrite(u8 *buffer,u16 address,u16 len);

#define EEPROM_RET_OK 0;
#define EEPROM_RET_TIMEOUT 1;
#define EEPROM_RET_ZERO_LENGTH 2;

#endif
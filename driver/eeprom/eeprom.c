
#include "eeprom.h"
#include "stm8s_i2c.h"
#include "stm8s_clk.h"
#include "led.h"
#include "utils.h"

u32 eepromT = 0;





u8 eepromInit() {

    I2C_DeInit();
    I2C_SoftwareResetCmd(ENABLE);
    I2C_SoftwareResetCmd(DISABLE);
    GPIO_Init(GPIOB,GPIO_PIN_4,GPIO_MODE_OUT_OD_HIZ_FAST); // SCL
    GPIO_Init(GPIOB,GPIO_PIN_5,GPIO_MODE_OUT_OD_HIZ_FAST);  //SDA  
    

    eepromT = 0;
    while ((u8)(GPIOB->IDR & GPIO_PIN_5) == 0)
    {
        GPIO_WriteLow(GPIOB,GPIO_PIN_4);
        delay(1000);
        GPIO_WriteHigh(GPIOB,GPIO_PIN_4);
        if(++eepromT > 9) return EEPROM_RET_TIMEOUT;
    };

    GPIO_Init(GPIOB,GPIO_PIN_4,GPIO_MODE_IN_FL_NO_IT); // SCL
    GPIO_Init(GPIOB,GPIO_PIN_5,GPIO_MODE_IN_FL_NO_IT);  //SDA 

    CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C,ENABLE);
    I2C_Init(
        EEPROM_CLOCK,
        0x01,
        I2C_DUTYCYCLE_2,
        I2C_ACK_CURR,
        I2C_ADDMODE_7BIT,
        CLK_GetClockFreq() / 1000000
    );
    I2C_Cmd(ENABLE);

    

    I2C_GenerateSTOP(ENABLE);
    I2C_GenerateSTART(ENABLE);

    
    if (flagTimeout(&I2C->SR1,I2C_SR1_SB,SET,10000)) return EEPROM_RET_TIMEOUT;
    I2C->DR = 0XFF;
    if (flagTimeout(&I2C->CR2,I2C_CR2_STOP,SET,10000)) return EEPROM_RET_TIMEOUT;
    
    return EEPROM_RET_OK;

}
u8 eepromRead(u8 *buffer,u16 address, u16 len) {
    if (len == 0) return EEPROM_RET_ZERO_LENGTH;
    u8 wordAddr = (u8)(address);
    u8 deviceAddr = (u8)(address >> 8);
    deviceAddr &= 0x01;
    deviceAddr |= EEPROM_ADDRESS;
    deviceAddr <<= 1;

    //u8 init = eepromInit();
    //if (init) return init;
    I2C_AcknowledgeConfig(I2C_ACK_CURR);

    I2C_GenerateSTART(ENABLE);
    if (flagTimeout(&I2C->SR1,I2C_SR1_SB,SET,10000)) return EEPROM_RET_TIMEOUT;

	I2C_Send7bitAddress(deviceAddr,I2C_DIRECTION_TX);
    if (flagTimeout(&I2C->SR1,I2C_SR1_ADDR,SET,10000)) return EEPROM_RET_TIMEOUT;

    (void)I2C->SR3;

    if (flagTimeout(&I2C->SR1,I2C_SR1_TXE,SET,10000)) return EEPROM_RET_TIMEOUT;

	I2C->DR = wordAddr;
    if (flagTimeout(&I2C->SR1,(I2C_SR1_TXE|I2C_SR1_BTF),SET,10000)) return EEPROM_RET_TIMEOUT;

	I2C->CR2 |= I2C_CR2_START;
    if (flagTimeout(&I2C->SR1,I2C_SR1_SB,SET,10000)) return EEPROM_RET_TIMEOUT;

	I2C_Send7bitAddress(deviceAddr,I2C_DIRECTION_RX);
    
    if (len > 2) {
        if (flagTimeout(&I2C->SR1,I2C_SR1_ADDR,SET,10000)) return EEPROM_RET_TIMEOUT;
        
        (void)I2C->SR3;

        while (len > 3)
        {
            if (flagTimeout(&I2C->SR1,I2C_SR1_RXNE,SET,10000)) return EEPROM_RET_TIMEOUT;
            
            *buffer = I2C->DR;
            buffer++;
            len--;
        }
        if (flagTimeout(&I2C->SR1,I2C_SR1_BTF,SET,10000)) return EEPROM_RET_TIMEOUT;
        
        I2C_AcknowledgeConfig(I2C_ACK_NONE);
        *buffer = I2C->DR; buffer++;
        I2C->CR2 |= I2C_CR2_STOP;
        *buffer = I2C->DR; buffer++;
        if (flagTimeout(&I2C->SR1,I2C_SR1_RXNE,SET,10000)) return EEPROM_RET_TIMEOUT;
        
        *buffer = I2C->DR; buffer++;
        len = 0;

    } else if (len == 2) {
        I2C_AcknowledgeConfig(I2C_ACK_NEXT);
        if (flagTimeout(&I2C->SR1,I2C_SR1_ADDR,SET,10000)) return EEPROM_RET_TIMEOUT;
        
        (void)I2C->SR1;
        (void)I2C->SR3;
        I2C_AcknowledgeConfig(I2C_ACK_NONE);
        if (flagTimeout(&I2C->SR1,I2C_SR1_BTF,SET,10000)) return EEPROM_RET_TIMEOUT;

        I2C->CR2 |= I2C_CR2_STOP;
        *buffer = I2C->DR; buffer++;
        *buffer = I2C->DR; buffer++;
        len = 0;

    } else if (len == 1) {
        if (flagTimeout(&I2C->SR1,I2C_SR1_ADDR,SET,10000)) return EEPROM_RET_TIMEOUT;
        
        I2C_AcknowledgeConfig(I2C_ACK_NONE);
        (void)I2C->SR1;
        (void)I2C->SR3;
        I2C->CR2 |= I2C_CR2_STOP;
        if (flagTimeout(&I2C->SR1,I2C_SR1_RXNE,SET,10000)) return EEPROM_RET_TIMEOUT;
        
        *buffer = I2C->DR; buffer++;
        len=0;
    }
    return EEPROM_RET_OK;
}
u8 eepromWrite(u8 *buffer,u16 address,u16 len) {

    eepromInit();
    uint8_t columnIndex = address % 16;
    while (len)
    {
        u8 wordAddr = (u8)(address);
        u8 deviceAddr = (u8)(address >> 8);
        deviceAddr &= 0x01;
        deviceAddr |= EEPROM_ADDRESS;
        deviceAddr <<= 1;
        if (flagTimeout(&I2C->SR3,I2C_SR3_BUSY,RESET,10000)) return EEPROM_RET_TIMEOUT;

        I2C_GenerateSTART(ENABLE);
        if (flagTimeout(&I2C->SR1,I2C_SR1_SB,SET,10000)) return EEPROM_RET_TIMEOUT;

        I2C_Send7bitAddress(deviceAddr, I2C_DIRECTION_TX);
        if (flagTimeout(&I2C->SR1,I2C_SR1_ADDR,SET,10000)) return EEPROM_RET_TIMEOUT;

        (void)I2C->SR3;
        if (flagTimeout(&I2C->SR1,I2C_SR1_TXE,SET,10000)) return EEPROM_RET_TIMEOUT;

        I2C_SendData(wordAddr);
        while (columnIndex < 16 && len)
        {
            if (flagTimeout(&I2C->SR1,I2C_SR1_TXE,SET,10000)) return EEPROM_RET_TIMEOUT;

            I2C_SendData(*buffer);
            buffer++;
            len--;
            columnIndex++;
            address++;
        }
        if (flagTimeout(&I2C->SR1,(I2C_SR1_TXE | I2C_SR1_BTF),SET,10000)) return EEPROM_RET_TIMEOUT;

        columnIndex = 0;
        I2C_GenerateSTOP(ENABLE);
        delay(10000);
    }
    return EEPROM_RET_OK;
}
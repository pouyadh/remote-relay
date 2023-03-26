#ifndef __MAIN_H
#define __MAIN_H

#define KEY_GPIO GPIOC
#define KEY_GPIO_PIN GPIO_PIN_4
#define KEY_PIN 4

#define RELAY1_GPIO GPIOD
#define RELAY1_GPIO_PIN GPIO_PIN_3
#define RELAY1_PIN 3

#define RELAY2_GPIO GPIOD
#define RELAY2_GPIO_PIN GPIO_PIN_2
#define RELAY2_PIN 2

#define RELAY3_GPIO GPIOD
#define RELAY3_GPIO_PIN GPIO_PIN_1
#define RELAY3_PIN 1

#define RELAY4_GPIO GPIOC
#define RELAY4_GPIO_PIN GPIO_PIN_7
#define RELAY4_PIN 7


#define pSet(port,pinNum) __BSET((u8 *)&GPIO##port->ODR,pinNum)
#define pClr(port,pinNum) __BRES((u8 *)&GPIO##port->ODR,pinNum)
#define pTgl(port,pinNum) __BCPL((u8 *)&GPIO##port->ODR,pinNum)
#define pGet(port,pinNum) (GPIO##port->IDR & (1<<pinNum))
#define isKeyPushed() !pGet(C,4)
#define isKeyReleased() pGet(C,4)


// void beepOn(u8 div, u8 sel);
// void beepOff();
// void beepPlay(const u16 *melody,u16 interval);
// void beepHz(u32 hz);

void updateRelays();


#endif
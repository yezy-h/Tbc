#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

#define DQ_PORT     GPIOB
#define DQ_PIN      GPIO_Pin_0
#define DQ_CLK      RCC_APB2Periph_GPIOB

uint8_t ds18b20_init(void);

short ds18b20_get_temperature(void);

#endif

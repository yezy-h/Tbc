#ifndef __FAN_H__
#define __FAN_H__

#include "stm32f10x.h"

void fan_init(int freq);
void fan_set(uint8_t val);

#endif

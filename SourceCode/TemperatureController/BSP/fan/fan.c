#include "fan.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"

TIM_TimeBaseInitTypeDef timBase;
TIM_OCInitTypeDef timOc;

int max_period = 0;

void fan_init(int freq)
{
    GPIO_InitTypeDef gpio;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    //init for GPIOB5
    gpio.GPIO_Pin = GPIO_Pin_5;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);
    //remap PB5 to TIM3_CH2
    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);

    //init for pwm
    timBase.TIM_Prescaler = 20000 / freq;
    timBase.TIM_CounterMode = TIM_CounterMode_Up;
    timBase.TIM_Period = 2400;
    max_period = 2400;
    timBase.TIM_ClockDivision = 0x20;
    timBase.TIM_RepetitionCounter = 0x00;
    TIM_TimeBaseInit(TIM3, &timBase);

    timOc.TIM_OCMode = TIM_OCMode_PWM2;
    timOc.TIM_OutputState = TIM_OutputState_Enable;
    timOc.TIM_Pulse = max_period * (100 - 0) / 100;
    timOc.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC2Init(TIM3, &timOc);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_Cmd(TIM3, ENABLE);
}

void fan_set(uint8_t val)
{
    uint8_t value = val;
    if(value > 100)
        value = 100;
    else if(value < 0)
        value = 0;

    timOc.TIM_OCMode = TIM_OCMode_PWM2;
    timOc.TIM_OutputState = TIM_OutputState_Enable;
  	timOc.TIM_Pulse = max_period * (100 - value) / 100;
  	timOc.TIM_OCPolarity = TIM_OCPolarity_Low; 
  	TIM_OC2Init(TIM3, &timOc);
}

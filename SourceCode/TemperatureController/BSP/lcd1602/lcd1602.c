#include "lcd1602.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "..\..\APP\includes.h"

#define RS_0        GPIO_ResetBits(GPIOC, GPIO_Pin_9)
#define RS_1        GPIO_SetBits(GPIOC, GPIO_Pin_9)
#define RW_0        GPIO_ResetBits(GPIOC, GPIO_Pin_8)
#define RW_1        GPIO_SetBits(GPIOC, GPIO_Pin_8)
#define EN_0        GPIO_ResetBits(GPIOC, GPIO_Pin_10)
#define EN_1        GPIO_SetBits(GPIOC, GPIO_Pin_10)

void lcd1602_init(void)
{
    //init gpio
    GPIO_InitTypeDef gpio;
    
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2
                  | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5
                  | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8
                  | GPIO_Pin_9 | GPIO_Pin_10;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &gpio);

    lcd1602_write_cmd(0x38);
    OSTimeDly(OS_TICKS_PER_SEC / 100);
    lcd1602_write_cmd(0x08);
    OSTimeDly(OS_TICKS_PER_SEC / 100);
    lcd1602_write_cmd(0x01);
    OSTimeDly(OS_TICKS_PER_SEC / 100);
    lcd1602_write_cmd(0x06);
    OSTimeDly(OS_TICKS_PER_SEC / 100);
    lcd1602_write_cmd(0x0c);
    OSTimeDly(OS_TICKS_PER_SEC / 100);

    printk("lcd1602 init done!\r\n");
}

void lcd_output(uint8_t val)
{
    uint16_t data = GPIO_ReadOutputData(GPIOC);
    data &= 0xff00;
    data |= val;
    GPIO_Write(GPIOC, data);
}

void lcd1602_write_cmd(uint8_t cmd)
{
    RS_0;
    OSTimeDly(OS_TICKS_PER_SEC / 1000);
    RW_0;
    OSTimeDly(OS_TICKS_PER_SEC / 1000);
    EN_1;
    lcd_output(cmd);
    OSTimeDly(OS_TICKS_PER_SEC / 100);
    EN_0;
}

void lcd1602_write_data(uint8_t data)
{
    RS_1;
    OSTimeDly(OS_TICKS_PER_SEC / 1000);
    RW_0;
    OSTimeDly(OS_TICKS_PER_SEC / 1000);
    EN_1;
    lcd_output(data);
    OSTimeDly(OS_TICKS_PER_SEC / 100);
    EN_0;
}

void lcd1602_write_char(uint8_t x, uint8_t y, char c)
{
    if(y == 0)
        lcd1602_write_cmd(0x80 + x);
    else
        lcd1602_write_cmd(0xc0 + x);
    lcd1602_write_data(c);
}

void lcd1602_write_string(uint8_t x, uint8_t y, char *str)
{
    uint8_t i = 0;

    if(y == 0)
        lcd1602_write_cmd(0x80 + x);
    else
        lcd1602_write_cmd(0xc0 + x);

    while(str[i] != '\0') {
        lcd1602_write_data(str[i]);
        ++i;
    }
}

void lcd1602_clear(void)
{
    lcd1602_write_cmd(0x01);
    OSTimeDly(100);
}

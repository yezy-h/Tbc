#include "ads1100.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"
#include "..\..\App\includes.h"

#define TWI_NOP     delay2()

uint16_t AD_Result[13];

#define ADS1100_WR_ADDRESS      0x90
#define ADS1100_RD_ADDRESS      0x91
#define ADS1100_CONFIG_REG      0x8c

void setSda(uint8_t id, uint8_t val)
{
    if(id == 0) {
        if(val == 0) {
            TWI_SDA0_LOW;
        } else if(val == 1) {
            TWI_SDA0_HIGH;
        }
    } else if(id == 1) {
        if(val == 0) {
            TWI_SDA1_LOW;
        } else if(val == 1) {
            TWI_SDA1_HIGH;
        }
    }
}

void setScl(uint8_t id, uint8_t val)
{
    if(id == 0) {
        if(val == 0) {
            TWI_SCL0_LOW;
        } else if(val == 1) {
            TWI_SCL0_HIGH;
        }
    } else if(id == 1) {
        if(val == 0) {
            TWI_SCL1_LOW;
        } else if(val == 1) {
            TWI_SCL1_HIGH;
        }
    }
}

int getSda(uint8_t id)
{
    if(id == 0) {
        return TWI_SDA0_STATE;
    } else {
        return TWI_SDA1_STATE;
    }
}

void delay2(void)
{
    uint32_t i = 15;

    while(i--);
}

void twi_init(void)
{
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_3;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOB, &gpio);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOD, &gpio);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOC, &gpio);

    setSda(0, 1);
    setSda(1, 1);
    setScl(0, 1);
    setScl(1, 1);

    printk("sda0 = %d, sda1 = %d\r\n", getSda(0), getSda(1));
//    while(1);
    ads1100_config(0);
    ads1100_config(1);
}

uint8_t twi_start(uint8_t id)
{
    setSda(id, 1);
    TWI_NOP;

    setScl(id, 1);
    TWI_NOP;

    if(!getSda(id)) {
        printk("%s: busy\r\n", __func__);
        return TWI_BUS_BUSY;
    }

    setSda(id, 0);
    TWI_NOP;

    if(getSda(id)) {
        printk("%s: bus error\r\n", __func__);
        return TWI_BUS_ERROR;
    }

    setSda(id, 0);
    TWI_NOP;

    if(getSda(id)) {
        printk("%s: bus error\r\n", __func__);
        return TWI_BUS_ERROR;
    }

    setScl(id, 0);
    TWI_NOP;

    return TWI_READY;
}

void twi_stop(uint8_t id)
{
    setScl(id, 0);
    TWI_NOP;

    setSda(id, 0);
    TWI_NOP;

    setScl(id, 1);
    TWI_NOP;

    setSda(id, 1);
    TWI_NOP;
}

void twi_send_ack(uint8_t id)
{
    setScl(id, 0);
    TWI_NOP;
    setSda(id, 0);
    TWI_NOP;
    setScl(id, 1);
    TWI_NOP;
    setScl(id, 0);
    TWI_NOP;
}

void twi_send_nack(uint8_t id)
{
    setScl(id, 0);
    TWI_NOP;

    setScl(id, 1);
    TWI_NOP;

    setSda(id, 1);
    TWI_NOP;

    setSda(id, 0);
    TWI_NOP;
}

uint8_t twi_send_byte(uint8_t id, uint8_t data)
{
    uint8_t i;

    setScl(id, 0);
    for(i = 0; i < 8; i++) {
        setScl(id, 0);
        TWI_NOP;
        //---------数据建立----------
        if(data & 0x80) {
            setSda(id, 1);
        } else {
            setSda(id, 0);
        }
        data <<= 1;
        TWI_NOP;
        //---数据建立保持一定延时----
        setScl(id, 1);;
        TWI_NOP;
    }
    setScl(id, 0);;
    if(getSda(id)) {
        return TWI_NACK;
    } else {
        return TWI_ACK;
    }
}

uint8_t twi_recv_byte(uint8_t id)
{
    uint8_t i, Dat;
    setSda(id, 1);
    Dat = 0;
    for(i = 0; i < 8; i++) {
        Dat <<= 1;
        setScl(id, 0);
        TWI_NOP;
        setScl(id, 1);
        TWI_NOP;
        if(getSda(id)) {
            Dat |= 0x01;
        }
        setScl(id, 0);//准备好再次接收数据
        //TWI_NOP;//等待数据准备好
    }
    return Dat;
}

uint8_t twi_wait_ack(uint8_t id)
{
    setScl(id, 0);
    TWI_NOP;
    setSda(id, 1);
    TWI_NOP;
    setScl(id, 1);
    TWI_NOP;
    if(getSda(id)) {
        setScl(id, 0);
        return 0x00;
    }

    setScl(id, 0);;
    return 0x01;
}

void ads1100_config(uint8_t id)
{
    twi_start(id);
    twi_send_byte(id, ADS1100_WR_ADDRESS);
    twi_send_nack(id);
    twi_send_byte(id, ADS1100_CONFIG_REG);
    twi_send_nack(id);
    twi_stop(id);
}

uint16_t ads1100_read(uint8_t id)
{
    uint16_t W_B1byte_high, W_B1byte_low,
             W_B1_word;

    twi_start(id);
    twi_send_byte(id, ADS1100_RD_ADDRESS);
    twi_send_nack(id);

    W_B1byte_high = twi_recv_byte(id);
    twi_send_ack(id);
    W_B1byte_low = twi_recv_byte(id);
    twi_send_ack(id);

    W_B1_word = twi_recv_byte(id);

    twi_stop(id);

    W_B1_word = (W_B1byte_high << 8) | W_B1byte_low;

    return W_B1_word;
}

uint16_t ads1100_get_result(uint8_t id)
{
    uint8_t i, j;
    uint16_t temp;

    for(i = 0; i < 13; i++) {
        AD_Result[i] = ads1100_read(id);
        OSTimeDly(1);
    }

    for(i = 1; i < 13; i++) {
        temp = AD_Result[i];
        for(j = i; j > 0 && temp < AD_Result[j - 1]; j--) {
            AD_Result[j] = AD_Result[j - 1];
        }
        AD_Result[j] = temp;
    }

    return AD_Result[7];
}


#ifndef __ADS1100_H__
#define __ADS1100_H__

#include "stm32f10x.h"

#define TWI_SCL1_LOW       GPIOD->BRR = GPIO_Pin_2
#define TWI_SCL1_HIGH       GPIOD->BSRR = GPIO_Pin_2
#define TWI_SDA1_LOW       GPIOB->BRR = GPIO_Pin_3
#define TWI_SDA1_HIGH       GPIOB->BSRR = GPIO_Pin_3
#define TWI_SDA1_STATE   (GPIOB->IDR & GPIO_Pin_3)

#define TWI_SCL0_LOW       GPIOC->BRR = GPIO_Pin_11
#define TWI_SCL0_HIGH       GPIOC->BSRR = GPIO_Pin_11
#define TWI_SDA0_LOW       GPIOC->BRR = GPIO_Pin_12
#define TWI_SDA0_HIGH       GPIOC->BSRR = GPIO_Pin_12
#define TWI_SDA0_STATE   (GPIOC->IDR & GPIO_Pin_12)

enum ENUM_TWI_REPLY {
    TWI_NACK = 0,
    TWI_ACK = 1
};

enum ENUM_TWI_BUS_STATE {
    TWI_READY = 0,
    TWI_BUS_BUSY = 1,
    TWI_BUS_ERROR = 2
};

#define TWI_RETRY_COUNT     3
void twi_init(void);
uint8_t twi_start(uint8_t id);
void twi_stop(uint8_t id);
uint8_t twi_send_byte(uint8_t id, uint8_t data);
uint8_t twi_recv_byte(uint8_t id);
uint8_t twi_wait_ack(uint8_t id);
void twi_send_ack(uint8_t id);
void twi_send_nack(uint8_t id);
void ads1100_config(uint8_t id);
uint16_t ads1100_read(uint8_t id);
uint16_t ads1100_get_result(uint8_t id);

#endif

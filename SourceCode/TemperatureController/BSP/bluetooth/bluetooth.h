#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__
#include "bsp.h"

#define RCC_APBxPeriph_BLE_USART_IO         RCC_APB2Periph_GPIOA
#define RCC_APBxPeriph_BLE_USART		    RCC_APB1Periph_USART2
#define BLE_USART_TXD				        GPIO_Pin_2
#define BLE_USART_RXD				        GPIO_Pin_3
#define BLE_USART_IO					    GPIOA
#define BLE_USART	                        USART2
#define BLE_PinRemap					    DISABLE
#define BLE_USARTAPB					    APB1
#define BLE_USART_IRQHandler			    USART2_IRQHandler

void ble_init(void);
void response(void);
void report_temperature(int time, int temperature1, int temperature2);
void rsp_with_one_byte(uint8_t cmd, uint8_t data);

#endif

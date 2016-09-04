#include "bluetooth.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "app.h"
#include "..\..\APP\includes.h"

extern OS_EVENT *atCmdMailbox;
__IO AppCmd cmd;
__IO uint8_t buffer[100];
__IO uint8_t index = 0;

void ble_init(void)
{
    u8 data;
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    if (BLE_USARTAPB == APB1)
    {
        RCC_APB2PeriphClockCmd(RCC_APBxPeriph_BLE_USART_IO | RCC_APB2Periph_AFIO,ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APBxPeriph_BLE_USART,ENABLE);
    }
    else
    {
        RCC_APB2PeriphClockCmd(RCC_APBxPeriph_BLE_USART_IO | RCC_APBxPeriph_BLE_USART | RCC_APB2Periph_AFIO,ENABLE);
    }
    if (BLE_PinRemap == ENABLE)
    {  				 
        GPIO_PinRemapConfig(GPIO_Remap_USART2,ENABLE);
    }
    GPIO_InitStructure.GPIO_Pin = BLE_USART_TXD;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BLE_USART_IO,&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = BLE_USART_RXD;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BLE_USART_IO,&GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None ;
    USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
    USART_Init(BLE_USART,&USART_InitStructure);
    data = data;
    data = BLE_USART->DR;
    data = BLE_USART->SR;
    USART_ITConfig(BLE_USART,USART_IT_RXNE,ENABLE);
    USART_Cmd(BLE_USART,ENABLE);

    response();
}

void clearBuffer(uint8_t *buf, int len)
{
    memset(buf, 0x00, len);
}

void send(uint8_t d)
{
    printf("%c", d);
    USART_SendData(BLE_USART, d);
    while(USART_GetFlagStatus(BLE_USART, USART_FLAG_TXE) == RESET){;}
}

void response(void)
{
    uint8_t i = 0;
    uint8_t resp[5] = {0x55, 0x55, 0x00, 0x00, 0x3e};
    for(i = 0; i < 5; i++) {
        send(resp[i]);
    }
}

void report_temperature(int time, int temperature1, int temperature2)
{
    uint8_t t_h, t_l, i, t2_h, t2_l;
    uint8_t data[14] = {0x55, 0x55, 0x10, 0x08,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x3e, 0x3e};
    int temp;
    temp = temperature1 + 20000;
    t_h = (temp >> 8) & 0xff;
    t_l = temp & 0xff;
    t2_h = (time >> 8) & 0xff;
    t2_l = time & 0xff;

    data[4] = t2_h;
    data[5] = t2_l;
    data[6] = t_h;
    data[7] = t_l;

    temp = temperature2 + 20000;
    t_h = (temp >> 8) & 0xff;
    t_l = temp & 0xff;
    t2_h = (time >> 8) & 0xff;
    t2_l = time & 0xff;

    data[8] = t2_h;
    data[9] = t2_l;
    data[10] = t_h;
    data[11] = t_l;

    for(i = 0; i < 14; i++) {
        send(data[i]);
    }
}

void rsp_with_one_byte(uint8_t cmd, uint8_t data)
{
    uint8_t i = 0;
    uint8_t rsp[6] = {0x55, 0x55, 0x00, 0x01, 0x00, 0x3e};
    rsp[2] = cmd;
    rsp[4] = data;

    for(;i < 6; i++) {
        send(rsp[i]);
    }
}

void handleData(uint8_t data)
{
	uint8_t i;
    uint8_t type;
    uint8_t len;
    if(data == 0x3e) {
        //process buffer
        //...
        if(buffer[0] == 0x55 && (buffer[1] == 0x55)) {
            type = buffer[2];
            len = buffer[3];
            cmd.type = type;
            cmd.len = len;
            if(len > 0) {
                for(i = 0; i < len; i++) {
                    cmd.data[i] = buffer[4 + i];
                }
            }
            clearBuffer((uint8_t *)buffer, 100);
            OSMboxPost(atCmdMailbox, (void *)&cmd);
            index = 0;
        }
    } else {
        buffer[index ++] = data;
    }
}

void BLE_USART_IRQHandler(void)
{
    uint8_t data;

    if(USART_GetITStatus(BLE_USART, USART_IT_RXNE)
            != RESET) {
        data = USART_ReceiveData(BLE_USART);
        handleData(data);
    }

    if(USART_GetITStatus(BLE_USART, USART_IT_TXE)
            != RESET) {
        USART_ITConfig(BLE_USART, USART_IT_TXE, DISABLE);
    }
}

void USART1_IRQHandler(void)
{
    uint8_t data;

    if(USART_GetITStatus(USART1, USART_IT_RXNE)
            != RESET) {
        data = USART_ReceiveData(USART1);
        handleData(data);
    }

    if(USART_GetITStatus(USART1, USART_IT_TXE)
            != RESET) {
        USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    }
}

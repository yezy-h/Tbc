#include "..\App\includes.h"
#include "ds18b20.h"
#include "fan.h"
#include "ads1100.h"
#include "bluetooth.h"
#include "m25p16.h"
#include "lcd1602.h"

#if 1
#pragma import(__use_no_semihosting)             

struct __FILE 
{ 
    int handle; 
    /* Whatever you require here. If the only file you are using is */ 
    /* standard output using printf() for debugging, no file handling */ 
    /* is required. */ 
}; 
/* FILE is typedef’ d in stdio.h. */ 
FILE __stdout;         
_sys_exit(int x) 
{ 
    x = x; 
} 

int fputc(int ch, FILE *f)
{   

    while((PC_USART->SR & 0x40) == 0);
    PC_USART->DR = (u8)ch;      
    return ch;
}
#endif

/*******************************************************************************
 * Function Name  : RCC_Configuration
 * Description    : Configures the different system clocks.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/

////////////////////////////////////////////////////////
void RCC_Configuration(void)
{	
    SystemInit(); 
}

//关闭调试接口，作GPIO使用
void UnableJTAG(void)
{
    /*
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;                     // enable clock for Alternate Function
    AFIO->MAPR &= ~(7UL<<24); // clear used bit
    AFIO->MAPR |= (4UL<<24); // set used bits
    */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}

void BSP_Init(void)
{
    /* System Clocks Configuration --72M*/
    RCC_Configuration();   
    UnableJTAG();
    GPIO_Configuration();
    USART_Config(115200);
    /* NVIC configuration */ 
    NVIC_Configuration();
    //初始化电磁阀开关控制液氮开关
    SSR_Init();
    //初始化风扇电机驱动
    fan_init(20);
    fan_set(0);
    //初始化蓝牙模块
    ble_init();
    //初始化flash存储模块
    //flash_init();
    //初始化lcd1602
    //lcd1602_init();
}

//电磁阀开关初始化
void SSR_Init(void)
{
    //init gpio A5, A6 to output
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
}

//电风扇驱动初始化
void FAN_Init(void)
{
}

void SSR1_Enable(int enable)
{
    if(enable) {
        GPIO_SetBits(GPIOA, GPIO_Pin_5);
    } else {
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);
    }
}

void SSR2_Enable(int enable)
{
    if(enable) {
        GPIO_SetBits(GPIOA, GPIO_Pin_6);
    } else {
        GPIO_ResetBits(GPIOA, GPIO_Pin_6);
    }
}

/*******************************************************************************
 * Function Name  : GPIO_Configuration
 * Description    : PB5: LED1 (mini and V3)
 PD6：LED2 (only V3)
 PD3：LED3 (only V3)
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void GPIO_Configuration(void)
{

}


/*******************************************************************************
 * Function Name  : NVIC_Configuration
 * Description    : Configures Vector Table base location.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef  NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void USART_Config(u32 baud)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    if (PC_USARTAPB == APB1)
    {
        RCC_APB2PeriphClockCmd(RCC_APBxPeriph_PC_USART_IO | RCC_APB2Periph_AFIO,ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APBxPeriph_PC_USART,ENABLE);
    }
    else
    {
        RCC_APB2PeriphClockCmd(RCC_APBxPeriph_PC_USART_IO | RCC_APBxPeriph_PC_USART | RCC_APB2Periph_AFIO,ENABLE);
    }
    if (PC_PinRemap == ENABLE)
    {  				 
        GPIO_PinRemapConfig(GPIO_Remap_USART2,ENABLE);
    }
    GPIO_InitStructure.GPIO_Pin = PC_USART_TXD;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PC_USART_IO,&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = PC_USART_RXD;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PC_USART_IO,&GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None ;
    USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
    USART_Init(PC_USART,&USART_InitStructure);
    //data = data;
    //data = PC_USART->DR;
    //data = PC_USART->SR;
    NVIC_Configuration();
    USART_ITConfig(PC_USART, USART_IT_RXNE, ENABLE);
    //USART_ITConfig(PC_USART, USART_IT_TXE, ENABLE);
    USART_Cmd(PC_USART,ENABLE);
}

void  OS_CPU_SysTickInit(void)
{
    RCC_ClocksTypeDef  rcc_clocks;
    INT32U         cnts;

    RCC_GetClocksFreq(&rcc_clocks);
    cnts = (INT32U)rcc_clocks.HCLK_Frequency / OS_TICKS_PER_SEC;
    SysTick_Config(cnts);
}

void delay(int ms)
{
    OSTimeDly(OS_TICKS_PER_SEC * ms / 1000);
}

#ifdef  DEBUG
/*******************************************************************************
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 *******************************************************************************/
void assert_failed(u8* file, u32 line)
{ 
    /* User can add his own implementation to report the file name and line number,
ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/


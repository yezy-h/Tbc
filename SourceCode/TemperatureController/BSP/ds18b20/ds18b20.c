#include "ds18b20.h"
#include "stdio.h"
#include "stm32f10x_gpio.h"
#include "..\..\App\includes.h"

void set_io_dir(int dir)
{
	GPIO_InitTypeDef gpio;
	RCC_APB2PeriphClockCmd(DQ_CLK, ENABLE);
	gpio.GPIO_Pin = DQ_PIN;
	if(dir == 0) {
		gpio.GPIO_Mode = GPIO_Mode_Out_PP;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
	} else {
		gpio.GPIO_Mode = GPIO_Mode_IPU;
	}
	GPIO_Init(DQ_PORT, &gpio);	
}

void set_io_high(void)
{
	GPIO_SetBits(DQ_PORT, DQ_PIN);
}

void set_io_low(void)
{
	GPIO_ResetBits(DQ_PORT, DQ_PIN);
}

uint8_t get_io(void)
{
	return GPIO_ReadInputDataBit(DQ_PORT, DQ_PIN);
}

void reset(void)
{
	set_io_dir(0);
	set_io_low();
	//750 us
	OSTimeDly(OS_TICKS_PER_SEC / 1333);
	set_io_high();
	//15 us
	OSTimeDly(OS_TICKS_PER_SEC / 66666);
}

uint8_t check(void)
{
	uint8_t retry = 0;
	set_io_dir(1);
	while(get_io() && retry < 200) {
		retry ++;
		//1us
		OSTimeDly(OS_TICKS_PER_SEC / 1000000);
	}

	if(retry >= 200)
		return 1;
	else
		retry = 0;

	while((!get_io()) && retry < 240) {
		retry ++;
		//1us
		OSTimeDly(OS_TICKS_PER_SEC / 1000000);
	}

	if(retry >= 240)
		return 1;
	
	return 0;
}

uint8_t read_bit(void)
{
	uint8_t data;
	set_io_dir(0);
	set_io_low();
	//2us
	OSTimeDly(OS_TICKS_PER_SEC / 500000);
	set_io_high();
	set_io_dir(1);

	if(get_io())
		data = 1;
	else
		data = 0;
	//50us
	OSTimeDly(OS_TICKS_PER_SEC / 20000);
	
	return data;
}

uint8_t read_byte(void)
{        
    uint8_t i,j,dat;
    dat=0;
	for (i=1;i<=8;i++) 
	{
        j=read_bit();
        dat=(j<<7)|(dat>>1);
    }						    
    return dat;
}

void write_byte(uint8_t dat)     
 {             
    uint8_t j;
    uint8_t testb;
	set_io_dir(0);
    for (j=1;j<=8;j++) 
	{
        testb=dat&0x01;
        dat=dat>>1;
        if (testb) 
        {
            set_io_low();
            //2us  
			OSTimeDly(OS_TICKS_PER_SEC / 500000);                          
            set_io_high();
            //60us
			OSTimeDly(OS_TICKS_PER_SEC / 16666);             
        }
        else 
        {
            set_io_low();
            //60us
			OSTimeDly(OS_TICKS_PER_SEC / 16666);             
            set_io_high();
            //2us
			OSTimeDly(OS_TICKS_PER_SEC / 500000);                          
        }
    }
}

void start(void)// ds1820 start convert
{   						               
    reset();	   
	check();	 
    write_byte(0xcc);// skip rom
    write_byte(0x44);// convert
}

uint8_t ds18b20_init(void)
{
	//delay_init();
	reset();
	
	return check();
}

short ds18b20_get_temperature(void)
{
    uint8_t temp;
    uint8_t TL, TH;
	short tem;
    start();                    // ds1820 start convert
    reset();
    check();	 
    write_byte(0xcc);// skip rom
    write_byte(0xbe);// convert	    
    TL = read_byte(); // LSB   
    TH = read_byte(); // MSB  

    if(TH > 7)
    {
        TH = ~TH;
        TL = ~TL; 
        temp = 0;//温度为负  
    } else { 
        temp = 1;//温度为正
    }
    tem = TH; //获得高八位
    tem <<= 8;    
    tem += TL;//获得底八位
    tem=(float)tem * 0.625;//转换     
	if(temp)
        return tem; //返回温度值
	else 
        return -tem;    
}

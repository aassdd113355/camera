#include "led.h" 
#include "delay.h" 

void GpioInit(void)
{    	 
	RCC->AHB1ENR|=1<<3;//使能PORTF时钟 
	GPIO_Set(GPIOD,PIN2,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PD2设置
	GPIO_Set(GPIOD,PIN3,GPIO_MODE_IN,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_NONE); //PD2设置
	On_Off=0;//LED0关闭
//	//LED1=1;//LED1关闭
//	delay_ms(3500);
//	On_Off=1;//LED0开启
//	delay_ms(3500);
//	On_Off=0;//LED0开启
}

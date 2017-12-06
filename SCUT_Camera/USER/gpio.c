#include "led.h" 
#include "delay.h" 

void GpioInit(void)
{    	 
	RCC->AHB1ENR|=1<<3;//使能PORTD时钟 
	GPIO_Set(GPIOD,PIN2,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PD2设置
	GPIO_Set(GPIOD,PIN3,GPIO_MODE_IN,0,0,GPIO_PUPD_NONE); //PD3设置
	On_Off=0;//LED0关闭

}

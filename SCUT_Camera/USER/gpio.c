#include "led.h" 
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

void GpioInit(void)
{    	 
	RCC->AHB1ENR|=1<<3;//ʹ��PORTFʱ�� 
	GPIO_Set(GPIOD,PIN2,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PF9,PF10����
	On_Off=1;//LED0�ر�
	//LED1=1;//LED1�ر�
}

#include "led.h" 
#include "delay.h" 

void GpioInit(void)
{    	 
	RCC->AHB1ENR|=1<<3;//ʹ��PORTFʱ�� 
	GPIO_Set(GPIOD,PIN2,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PD2����
	GPIO_Set(GPIOD,PIN3,GPIO_MODE_IN,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_NONE); //PD2����
	On_Off=0;//LED0�ر�
//	//LED1=1;//LED1�ر�
//	delay_ms(3500);
//	On_Off=1;//LED0����
//	delay_ms(3500);
//	On_Off=0;//LED0����
}

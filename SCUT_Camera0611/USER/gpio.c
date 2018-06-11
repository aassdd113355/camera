#include "led.h" 
#include "delay.h" 

void GpioInit(void)
{    	 
	RCC->AHB1ENR|=1<<3;//ʹ��PORTDʱ�� 
	RCC->AHB1ENR|=1<<1;//ʹ��PORTBʱ�� 
	RCC->AHB1ENR|=1<<2;//ʹ��PORTCʱ�� 
	GPIO_Set(GPIOD,PIN12,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PC11 ���������
	GPIO_Set(GPIOD,PIN2,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PD2����
	GPIO_Set(GPIOD,PIN6,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PD6����
	GPIO_Set(GPIOB,PIN3,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PB3����
	GPIO_Set(GPIOD,PIN4,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PD4���� //�ײ�ƹ�
	GPIO_Set(GPIOB,PIN4,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PB4����  //������������
	GPIO_Set(GPIOB,PIN4,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);
	GPIO_Set(GPIOD,PIN3,GPIO_MODE_IN,0,0,GPIO_PUPD_NONE); //PD3����
	GPIO_Set(GPIOD,PIN7,GPIO_MODE_IN,0,0,GPIO_PUPD_NONE); //PD7����
	
	GPIO_Set(GPIOA,PIN0,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PD);// PA0���� ������Echo�������벶��
	GPIO_Set(GPIOA,PIN1,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PD); //PA1���� ������trigger
	GPIO_AF_Set(GPIOA,0,1);	//PA0,AF1
	
	On_Off=0;
	Light_On=0;
	Light_On_Glass_Water = 0;
	Glass_Light_On = 0;
	TransmitSig = 0;
	TriggerOn = 0;
	
}

void UltrasonicTrigger() //����������
{
		TriggerOn=1;
		delay_us(20);                             //����һ��10us�ĸߵ�ƽ
		TriggerOn=0;
}

#ifndef __LED_H
#define __LED_H
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32F407������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	


//LED�˿ڶ���
#define On_Off PDout(2)	// PD2 ���Ƽ̵�������
#define GlassAreaAssitSignal PDin(7)	// PD7 ���ܹ��Թ����� 0ʱ������
#define GlassArea PDin(3)	// PD3 ���ܽӽ��������� 0ʱ������
#define Light_On PDout(5) //PD5 ���Ƶƹ�ļ̵������أ��ж�Ϊ��͸�����������������걭�ں�رյƹ�
#define Light_On_Glass_Water PBout(3) //PB3���Ƶƹ�ļ̵������أ��ж�Ϊ͸�����������걭��ʱ�������������Ƴ�ʱ�ر�
#define Glass_Light_On PDout(6) //PD6 ���Ƽ�ⲣ����ڭ�ƹ�ļ̵������أ��ж�Ϊ�������������겣������ر�
void GpioInit(void);//��ʼ��		 				    
#endif
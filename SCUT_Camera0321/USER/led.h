#ifndef __LED_H
#define __LED_H
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	


//LED端口定义
#define On_Off PDout(2)	// PD2 控制继电器开关
#define GlassAreaAssitSignal PDin(7)	// PD7 接受光电对管输入 0时有物体
#define GlassArea PDin(3)	// PD3 接受接近开关输入 0时有物体
#define Light_On PDout(5) //PD5 控制灯光的继电器开关，判断为非透明杯开启，在搜索完杯口后关闭灯光
#define Light_On_Glass_Water PBout(3) //PB3控制灯光的继电器开关，判断为透明杯且搜索完杯口时开启，玻璃杯移出时关闭
#define Glass_Light_On PDout(6) //PD6 控制检测玻璃笨诃灯光的继电器开关，判断为玻璃后开启，搜完玻璃杯后关闭
void GpioInit(void);//初始化		 				    
#endif

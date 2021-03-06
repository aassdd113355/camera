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
#define On_Off PDout(2)	// PD2 控制继电器出水开关
#define GlassAreaAssitSignal PDin(7)	// PD7 接受光电对管输入 0时有物体 区分玻璃杯与非玻璃杯
#define GlassArea PDin(3)	// PD3 接受超声波输出引脚输入 0时有物体
#define Light_On PDout(6) //PD6 控制灯光的继电器开关主杯口检测灯光，在搜索完杯口后关闭灯光
#define Light_On_Glass_Water PBout(3) //PB3控制灯光的继电器开关，判断为透明杯且搜索完杯口时开启，玻璃杯移出时关闭
#define Glass_Light_On PDout(4) //PD6 控制检测玻璃 辅助 灯光的继电器开关，判断为玻璃后开启，搜完玻璃杯后关闭
#define TransmitSig PBout(4) //PB4 红外发射，检测是否玻璃杯
#define TriggerOn PAout(1) //PA1 发送超声波trigger脉冲
#define GlassAreaOut PCout(11) //PC11 超声波输出引脚 与 PD3 硬件上相连
void GpioInit(void);//初始化		 	
void UltrasonicTrigger(void);
#endif

#ifndef _Drv_GPIO_H
#define _Drv_GPIO_H

#define Port_Set    FP00    //记忆键
#define Port_Test   FP01    //开关/测量键
#define Port_Mem    FP04    //设置键
#define Port_Earcap    FP17    //耳套键

// #define Port_On_text   FP01    //开机键&测量键
// #define Port_Voice     FP00    //声音键
// #define Port_earcap    FP05    //耳套模式键
// #define Port_Mode      FP04    //设置键

#define Port_Change_CF 	FP51	//高电平：可切换，低电平：不可切换
#define Port_CF 	FP52	//高电平：C，低电平：F
#define Port_Cal 	FP53	//校准模式检测入口
#define Port_Debug 	FP54	//绑定检测模式入口
#define Port_BZ     FP11    //蜂鸣口

void Cal_Inspect_Detect(void);
void CF_Check(void);
void GPIO_Init(void);
void GPIO_PowerDown( void );

#endif

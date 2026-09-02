/*
 * @Author: your name
 * @Date: 2020-11-17 15:44:46
 * @LastEditTime: 2020-11-19 18:51:34
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \et13\Include\App_Setmode.h
 */
#ifndef _App_SetMode_H
#define _App_SetMode_H

//设置态任务
typedef enum
{
	Set_Unit = 0,		//设置单位
	Set_Emission ,		//设置发射率
	Set_HumanRatio1 ,	//设置人体系数1 不戴耳套的
#if Func_Probecover
    Set_HumanRatio2 ,	//设置人体系数1 戴耳套的
    Set_Earcap10,		//设置cap15系数
    Set_Earcap15,		//设置cap15系数
	Set_Earcap20,		//设置cap20系数
	Set_Earcap25,		//设置cap25系数
	Set_Earcap30,		//设置cap30系数
	Set_Earcap35,		//设置cap35系数
	Set_Earcap40,		//设置cap40系数
#endif
	Set_TableNum ,		//设置黑体表格
	Set_End 			//设置退出保存
}eSetModeTask;

extern eSetModeTask eSetTask;
extern bit F_FirstEnter_SetMode;

void App_SetMode(void);

#endif

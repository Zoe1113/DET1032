/**************************************************************************
文件名称：	App_InitMode.c
说    明：	待机模式（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
备    注:
修订记录：
**************************************************************************/
#include "Include.h"

eInitModeTask eInitTask_Sta;

void App_InitMode(void)
{
	switch( eInitTask_Sta )
	{
		//初始化设置
		case Init_Set:
			// LVD_Init();		//低电压设置
			Drv_Adc_Init();	//AD设置
			Drv_Adc_Channel_Set(ACM_ACM);
			Delay1ms(5);

			#if Func_debug
				Drv_UartTX_Init();
			#endif

			//默认跳到下个状态
			eInitTask_Sta = Init_Disp;

			//EEPROM参数校验(如果校验和识别码都错则初始化数据，如果仅校验和错误则报错)
			//EEPROM参数校验(Er5无法启动语音播报，因为语种可能未读出)
			Parm_AutoCheck();
			break;

		//全显蜂鸣，并读取参数（实测500ms）
		case Init_Disp:
			//低电压检测
			#if Second_LVD == 1
			LBT_Chk();
			#else
			LVD_Init();
			LVD_Chk();
			#endif
			if( uStaFlag.bits.LowBat )
			{
				Clr_Disp();
				Disp_LowBat();	//显示低电压符号
				Auto_TurnOff_Time_Sel();	//关机时间选择
				eInitTask_Sta = Init_Err;
			}
			else
			{
				Disp_All();
				F_LED_Enable = Enable;	//开启背光
				BZ_Beep125();

				//默认跳到下个状态
				eInitTask_Sta = Init_ADDoff;
			}
			break;

		//采集Adc的Doff值
		case Init_ADDoff:
			//Adc Doff采集，14笔*0.016ms=224ms(采14，丢4，去最大最小，求8平均)，实测300ms
			if( Get_Adc_Avg() )
			{
				g_AdcDoff = g_AdcSum;
				Drv_PGA_Init(Adc_PGA_Gain);
				Drv_Adc_Channel_Set(ACM_ACM);
				eInitTask_Sta = Init_OpDoff;
			}
			break;

		//采集PGA的Doff值
		case Init_OpDoff:
			//Adc offset采集，14笔*0.016ms=224ms(采14，丢4，去最大最小，求8平均)，实测300ms
			if( Get_Adc_Avg() )
			{
				g_OpDoff = g_AdcSum;
				// Drv_Adc_Offset_Set( Offset_25 );
				// eInitTask_Sta = Init_OpOffsetDoff;
				Adc_Channel_Init(TPTONTC);	//ADC初始化和通道切换
				eInitTask_Sta = Init_Ntc;
			}
			break;

		// //目前测试不用偏压调整
		// //采集PGA偏压后的Doff值
		// case Init_OpOffsetDoff:
		// 	//Adc offset采集，14笔*0.016ms=224ms(采14，丢4，去最大最小，求8平均)，实测300ms
		// 	if( Get_Adc_Avg() )
		// 	{
		// 		g_OpOffsetDoff = g_AdcSum;
		// 		F_Enter_Get_Ntc = 0;	//使能ntc采集
		// 		eInitTask_Sta = Init_Ntc;
		// 	}
		// 	break;

		//采集thermistor温度
		case Init_Ntc:
			//计算ntc，保证环温可以显示，224*2=448ms（实测700ms）
			if( Get_Ntc_Count() )
			{
				Ntc_Caculate();
				NtcTable_Check();
				if( !uErrFlag.bits.Er2 )	//此处错误不做处理
				{
					NtcTable_Find();
				}
				eInitTask_Sta = Init_Wait;
			}
			break;

		//错误处理
		case Init_Err:
			//等待关机
			if ( uKeyPress.bits.TKeyPress && uKeyRelease.bits.TKeyRelease )
			{
				eInitTask_Sta = Init_Set;	//将当前状态设置为初始状态
				eMain_Task = Task_Sleepmode;
			}
			break;

		//1.5s全显背光等待
		case Init_Wait:
			if( !F_LED_Enable )
				eInitTask_Sta = Init_Key;
			break;

		//按键判断
		case Init_Key:
            if(uKeyPress.bits.TKeyPress)
            {
				HalKey_Set_KeyMode(Func_Short_Long, &sSetKey);	//设置键为短长按
                if( uKeyHold.bits.SKeyHold &&  uKeyRelease.bits.SKeyRelease )
                {
                    Clr_Disp();
					Disp_BadFace();
					Clr_All_Memory();
                    Auto_TurnOff_Time_Sel();
                    NtcTableWider_Check();
					uSetFlag.bits.Unit = Unit_C;	//进入检验态默认C
					//Adc_Channel_Init(TPTONTC);		//切换到NTC通道(AI2_ACM)，否则读TP通道导致Er2
					eTestmode_num = Insptectmode;	//非任务切换，仅改变测量模式而已
                }
				//检测等待测试/测量键抬起后退出检测，进入ready状态或关机
				if( uKeyRelease.bits.TKeyRelease )
				{
					HalKey_Set_KeyMode(Func_Long, &sSetKey);	//设置键为长按
					eInitTask_Sta = Init_End;
				}
            }
			else
			{
				HalKey_Set_KeyMode(Func_Long, &sSetKey);	//设置键为长按
				eInitTask_Sta = Init_End;
			}
			break;

		//初始化任务结束，还原当前任务为初始状态
		case Init_End:
            Auto_TurnOff_Time_Sel();
			HalKey_KeyClr();	//清除所有按键信息
            Clr_Disp();
            #if Func_Ble
                F_Ble_En = Enable;         //开启蓝牙
            #endif
			eInitTask_Sta = Init_Set;		//任务切换必须将当前任务状态设定为初始状态
			eMain_Task = Task_ReadyMode;	//切换到下一个任务
			if(eTestmode_num != Insptectmode)
			    eReadyTask_Sta = Ready_Init;
            else
                eReadyTask_Sta = Ready_Refresh;
			break;

			break;

		//默认保留
		default:
			break;
	}
}

void Set_Reset(void)
{
	g_Hour = 0;
	g_Minute = 0;
	g_Day = 1;
	g_Month = 1;
	g_Year = Default_Year;
	g_Second = 0;
	uSetFlag.bits.TimeFormat = TimeFormat_24H;	//默认24小时制
	#if CF_Change_Enable == 1
		uSetFlag.bits.Unit_Change = Unit_Change_En;	//默认单位可切换
	#else
		uSetFlag.bits.Unit_Change = Unit_Change_Dis;	//默认单位可切换
	#endif
	uSetFlag.bits.VoiceEnable = 1; 
	
	#if Func_Ble
		uSetFlag.bits.BleEnable = Enable;
	#endif

	CF_Check();
	
	Clr_All_Memory();
}
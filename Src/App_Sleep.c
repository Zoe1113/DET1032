/**************************************************************************
文件名称：	App_Sleep.c
说    明：	关机模式（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"

eSleep_Status eSleepTask_Sta;

void App_Sleep(void)
{
	static uint8 l_buf;
	switch ( eSleepTask_Sta )
	{
		//关机初始化
		case Sleep_false:
			//请养成习惯，还原主任务相关变量
			//如果进入过检验模式或黑体模式，则需初始化
			if( eTestmode_num == Insptectmode)
			{
				eTestmode_num = Earmode;
				Set_Reset();
				CF_Check();
				Clr_All_Memory();
			}
			if(eTestmode_num == Blackbodymode )
			{
				eTestmode_num = Earmode;
			}
			//主要变量初始化（错误清0）
			uErrFlag.g_ErrFlag = 0;

			//各任务状态初始化
			eReadyTask_Sta = Ready_Init;
			eInitTask_Sta = Init_Set;
			#if Func_Ble
				eBle_Sta = Ble_Disable;
			#endif	
			//记忆状态初始化
			F_Mem_FirstEnter = 0;
			//时间、日期、环温循环清0
			F_FirstEnter_SetMode = 0;

			eBeep_Status = Stop_beep;	//关蜂鸣
			Drv_BZ_Disable(); 
			LED_CloseAll();	//关背光
			Disp_OFF();
			Delay10ms(50);
			Clr_Disp();
            //Er1_Display_Sound(RESET);//复位Er2错误
            Er2_Display_Sound(RESET);//复位Er2错误
            #if Func_Obj	 
                Er6_Display_Sound(RESET);//复位Er6错误
            #endif
			eSleepTask_Sta = Sleep_waitkey;
			break;

		//等待按键释放
		case Sleep_waitkey:
			//如果关机时一直长按 关机&测量 则进入黑体模式(此处与原汇编不一样)
			if ( uKeyPress.bits.TKeyPress )
			{
				if( uKeyHold.bits.TKeyHold && !uStaFlag.bits.LowBat)
				{
					lcd_point_en();	//显示小数点

					BZ_Beep400();		//进入黑体模式，蜂鸣4声
					BZ_Beep400();
					BZ_Beep400();

					while( !Port_Test )	//等待释放进入
                    {
                        WDTR = 0x5A;        // Clear WDTR Dog
                    }
					Disp_CAL();	//显示CAL表示进入黑体模式
					Delay10ms(100);
					Disp_Code(Soft_Code);	//显示程序编码
                    Delay10ms(100);
					Disp_Version(Soft_External_Version);	//显示对外程序版本
					Delay10ms(100);
					HalKey_KeyClr();
					Adc_Channel_Init(TPTONTC);	//某些模式会只采集tp故必须切回ntc采集，ADC初始化和通道切换
                    Er2_Display_Sound(RESET);//复位Er2错误
					eTestmode_num = Blackbodymode;	//代表进入黑体模式
					eSleepTask_Sta = Sleep_false;	//设置当前任务状态为初始状态
					eMain_Task = Task_ReadyMode;
					eReadyTask_Sta = Ready_Init;	//进入ready模式
                    Auto_TurnOff_Time_Sel();
				}
			}
			else
			{
                eSleepTask_Sta = Sleep_true;
				uStaFlag.g_StatusFlag &= 0x02;
			}
			break;

		//完全关机状态
		case Sleep_true:
			#if Func_Ble
				F_Ble_En = Disable;
				Port_Ble_En = 0;		//蓝牙失能
				Port_Power = 1;		//关闭蓝牙供电使能（低电平有效）
			#endif
			Drv_UartTX_Disable();	//关UART TX
			Drv_UartRX_Disable();	//关UART RX
			I2C_Disable();		//关I2C
			FMSPWK = 0;		//禁止I2C总线唤醒，仅允许P0按键唤醒
			GPIO_PowerDown();	//IO口省电设置
            // Adc_Channel_Init(TPTONTC); //ADC初始化和通道切换

			FCLKMD = 1;			//切到Slow mode
			NOP(2);
			FSTPHX = 1;			//关IHRC
			NOP(2);
			FDA1EN = 0;			//关DAC1
			FDA2EN = 0;			//关DAC2
			FOPA1EN = 0;		//关OP1
			FOPA2EN = 0;		//关OP2
			FAMPEN = 0 ;		//关PGA
			FPCHPEN = 0 ;		//关PGA chopper
			FACHPEN = 0 ;		//关ADC chopper
			FADC1EN = 0 ;		//关ADC1
			FADC2EN = 0;		//关ADC2
			FAVEN = 0 ;			//关AVE电压
			FACMEN = 0 ;		//关ACM电压
			FAVDDREN = 0 ;		//关avddr电压
			FBGCHP = 0;			//关bandgap chooper
			FBGREN = 0 ;		//关bandgap电压
			FLCDBNK = 1 ;		//All of the LCD dots off
			FLCDEN = 0 ;		//关LCD
			FLCDMOD0 = 1 ;
			FLCDMOD1 = 1 ;		//LCD Mode All OFF
			FLBTEN = 0 ;		//关低电压检测
			FT0EN = 0;			//关机期间停止RTC，避免定时唤醒
			FT0IRQ = 0;		//清RTC中断请求
			FTC0ENB = 0;		//关TC0 timer
			FTC1ENB = 0;		//关TC1 timer
			FTC2ENB = 0;		//关TC2 timer
			INTEN0 = 0;			//所有中断除能
			INTEN1 = 0;			//所有中断除能
			FGIE = 0 ;			//关总中断
			FCPUM0 = 1;		//进入睡眠，等待P0按键唤醒
			NOP(2);

			FSTPHX = 0;		//按键唤醒后恢复IHRC
			NOP(2);
			FCLKMD = 0;		//恢复正常时钟
			NOP(2);
			//无按键按下
			l_buf = 1;
			
			// while(l_buf)
			// {
			// 	while( Port_Test && Port_Set )
			// 	{
			// 	}
			// 	Delay50us(4);	//按键去抖
				
			// 	if( !Port_Test || !Port_Set )
			// 	{
			// 		l_buf = 0;
			// 	}
			// }

			GPIO_Init();	//IO口设置
			Lcd_Init();		//Lcd设置
			TC1Init();		//TC1设置（10ms）
			T0Init();		//按键唤醒后恢复RTC计时
			FGIE = 1;

			HalKey_KeyClr();	//清所有按键信息
			HalKey_Set_KeyMode(Func_Long, &sTestKey);		//开机&测量键设为长按
			HalKey_Set_KeyMode(Func_Long, &sSetKey);		//设置键设为长按
			l_buf = 0;
			eSleepTask_Sta = Sleep_wakeup;
			break;

		//按键唤醒等待
		case Sleep_wakeup:
			l_buf ++;
			//去抖80ms，保证扫键程序执行
			if( l_buf > 8 )
			{
				//设置键长按3s进入模式切换
				if( uKeyPress.bits.SKeyPress )
				{
					 if( uKeyHold.bits.SKeyHold )
					{
                         //低电压检测
                       #if Second_LVD == 1
                           LBT_Chk();
                       #else
                           LVD_Init();
                           LVD_Chk();
                       #endif						//二级电压检测
                       if( uStaFlag.bits.LowBat )
                       {
                           eSleepTask_Sta = Sleep_true;
                       }
                       else
                       {
                            BZ_Beep125();
                            eSleepTask_Sta = Sleep_false;	//还原当前任务状态
                            eMain_Task = Task_Unitmode;
                            //进入设置态和快进为同一按键适用（当然非同一个按键也适用这样的写法）
                            HalKey_KeyClr();	//清除所有按键信息
                            App_SetMode();
                            while(!Port_Set)
                            {
                                WDTR = 0x5A;        // Clear WDTR Dog
                            }
                       }
					}
				}
				else
				{
					eSleepTask_Sta = Sleep_true;	//默认关机
					if( uKeyPress.bits.TKeyPress )
					{
						eSleepTask_Sta = Sleep_End;	//只有开机键运行开机
					}
				}
			}
			break;

		//结束睡眠
		case Sleep_End:
			eSleepTask_Sta = Sleep_false;
			eMain_Task = Task_InitMode;
			eTestmode_num = Earmode;							//开机默认耳温模式
            eAgemode_num = BigAge;
			HalKey_Set_KeyMode(Func_Long, &sTestKey);		//开机&测量键设为长按
			HalKey_Set_KeyMode(Func_Long, &sMemKey);			    //记忆键设为短长按
			HalKey_Set_KeyMode(Func_Short, &sEarcapKey);		    //耳套键为短按
			HalKey_Set_KeyMode(Func_Long, &sSetKey);				//设置键为长按

		default:
			break;
	}
}

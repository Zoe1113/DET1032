/**************************************************************************
文件名称：	ETH_2988_SJ_V1.0_20200704.c
说    明：	耳套可选,单位焊接选择,背光可选,物温可选
程序编码：	详见Variable.h
版    本：	V1.0
适用 PCB:	ETH PCB(A)
芯	  片:	SN8P2988
作    者:	Liaoy
修订记录：
1、在ET215-2988-242-V1.0-20190821基础上生成ETH程序；
V1.0：
采用宏定义合并程序，版本不升级，廖彦20200704
V1.0:
传感器表格也进行宏定义，与之PGA放大倍数也必须宏定义，版本升级为V1.6，廖彦20200723
**************************************************************************/
#include "Include.h"


//主程序
void main(void)
{
	//系统时钟设定
  	OSCM = 0x00;		//开启内置IHRC=8M，外置低速晶振32768，Fcpu=Normal mode
	Delay50us(100);

	//部分参数设置，如年月日时分秒
	g_Hour = 0;
	g_Minute = 0;
	g_Day = 1;
	g_Month = 1;
	g_Year = Default_Year;
	g_Second = 0;
	uSetFlag.bits.TimeFormat = TimeFormat_24H;	//默认24小时制
	uSetFlag.bits.Unit = Unit_C;	//默认C单位

	#if CF_Change_Enable == 1
		uSetFlag.bits.Unit_Change = Unit_Change_En;	//默认单位可切换
	#else
		uSetFlag.bits.Unit_Change = Unit_Change_Dis;	//默认单位可切换
	#endif

	uSetFlag.bits.VoiceEnable = 1;        //默认开启语音

	#if Func_Ble
		uSetFlag.bits.BleEnable = Enable;
	#endif
	
	GPIO_Init();//IO口设置
	Lcd_Init();	//Lcd设置
	CF_Check();	//单位状态确认

	//上电是否进入设置态
	#if Electricity_poweroff == 0
		eMain_Task = Task_Setmode;
	#else
		eMain_Task = Task_Sleepmode;
		eSleepTask_Sta = Sleep_true;
	#endif

	eTestmode_num = Earmode;	//默认耳温模式
    eAgemode_num = BigAge;

	//低电压检测
	#if Second_LVD == 1
		LBT_Chk();
	#else
		LVD_Init();	//低电压设置
		LVD_Chk();
	#endif
	if( uStaFlag.bits.LowBat )
	{
		Clr_Disp();
		lcd_bat_en();
		eMain_Task = Task_InitMode;
		eInitTask_Sta = Init_Err;
        while(!Port_Test)
        {
            WDTR = 0x5A;        // Clear WDTR Dog
        }
        uKeyRelease.bits.TKeyRelease = 1;

		eSleepTask_Sta = Sleep_false;
	}
	//EEPROM参数校验(如果校验和识别码都错则初始化数据，如果仅校验和错误则报错)
	Parm_AutoCheck();

	TC1Init();	//TC1设置（10ms）
	T0Init();	//开启RTC中断
	FGIE = 1;	//使能总中断等

	// 关机时间重置
	Auto_TurnOff_Time_Sel();	//关机时间选择
	// 初始化按键信息
	HalKey_Set_KeyMode(Func_Long, &sTestKey);		//开机&测量键设为长按
	HalKey_Set_KeyMode(Func_Long, &sMemKey);			    //记忆键设为长按
	HalKey_Set_KeyMode(Func_Short, &sEarcapKey);		    //耳套键为短按
	HalKey_Set_KeyMode(Func_Long, &sSetKey);				//设置键为长按

	// 绑定检测，因为修改上电进入模式的原因，移到此处；
	Cal_Inspect_Detect();
	
	// FP10 = 0;           // 拉低 → 点亮白灯（灌电流/低电平有效）
	// FP55=0;
	// while(1)
	// {
	// 	WDTR = 0x5A;        // Clear WDTR Dog
		
	// }
	while(1)
	{
		//10ms基本定时器
		if(F_10ms)
		{
            WDTR = 0x5A;        // Clear WDTR Dog
			F_10ms=0;
			Time_Creat_20ms_50ms();
			App_Beep();	//蜂鸣
            if (eMain_Task == Task_Memorymode || eMain_Task == Task_ReadyMode)
			{
    			ReadyMode_NtcMeas();            //Disable不需要切换通道
                if(uErrFlag.bits.Er2)
                {
                    if(eMain_Task == Task_Memorymode)
                        eMain_Task = Task_ReadyMode;
                    eReadyTask_Sta = Ready_DisEr2;
                }
			}
		}

		//20ms任务轮询
		if(F_20ms)
		{
			F_20ms = 0;

			//扫描按键状态
			HalKey_KeyScan();

			//三色背光
			if( F_LED_Enable )
				Light_RGB();

			if( eMain_Task == Task_ReadyMode || eMain_Task == Task_Memorymode)
			{
				//3s背光倒计时
				Led_CountDown_3s();
			}

			//自动关机
			if (eMain_Task == Task_Memorymode || eMain_Task == Task_ReadyMode || eMain_Task == Task_Setmode || eMain_Task == Task_InitMode || eMain_Task == Task_Unitmode ||  eMain_Task == Task_ParamModifymode )
				Auto_TurnOff();
		}

		//50ms更新显示时间
 		if(F_50ms)
		{
			F_50ms = 0;

			//蓝牙超时计数器
			#if Func_Ble
				g_ble_ack_timeout ++;
			#endif	
			if(eMain_Task == Task_ReadyMode)
				LCD_pc_Show(0);	

            #if Second_LVD == 1
			/*低电压显示*/
			/*初始化时不进行低电压符号操作*/
            #if ParamModif
			if( eMain_Task == Task_InitMode || eMain_Task == Task_BondTestmode || eMain_Task == Task_Calimode || eMain_Task == Task_ParamModifymode)
            #else
            if( eMain_Task == Task_InitMode || eMain_Task == Task_BondTestmode || eMain_Task == Task_Calimode)
            #endif
			{
			}
			/*设置和关机时不显示低电压符号*/
			else if( eMain_Task == Task_Setmode || eMain_Task == Task_Sleepmode ||eMain_Task == Task_Unitmode)
			{
				lcd_bat_clr();
				lcd_bat_full_clr();
			}
			else
			{
				LVD_Display();
			}	
			#endif				
		}

		//500ms更新显示
 		if(F_500ms)
		{
			F_500ms = 0;

			//系统时间更新
			Update_SysTime();

			//开机全显外都可以使能闪烁
			if( ( eMain_Task == Task_ReadyMode || eMain_Task == Task_Testingmode) && (eTestmode_num != Insptectmode))
				Disp_Colon();

			#if Func_Ble
				//蓝牙超时时间累加
				g_ble_timeout ++;
				//蓝牙标志闪烁
				if( F_Ble_Blink && F_Ble_En && eMain_Task != Task_BondTestmode && eMain_Task != Task_Sleepmode && eMain_Task != Task_Calimode)		//蓝牙连接，常亮
					lcd_ble_xor();
				if( Ble_TimeoutErr==eBle_Sta )    	//如果在断线状态，长时间拉低
				{
					Ble_Waittostart++;			  	//蓝牙断电时间累加
				}				
			#endif	

		}
		//仅用户模式的耳温、额温、物温启用蓝牙传输，其他均不启动蓝牙传输
		#if Func_Ble
			if( uSetFlag.bits.BleEnable == Enable )
			{
				if( eMain_Task != Task_BondTestmode && ( eTestmode_num == Earmode || eTestmode_num == Objectmode ) )
					App_BleMode();
			}
		#endif

		//主任务(10ms轮询，因为ADC基本是16ms，不可以20ms）
		if( F_10ms_task )
		{
			F_10ms_task = 0;

			switch(eMain_Task)
			{
				//开机初始化状态（即全显前的流程）
				case Task_InitMode:
					App_InitMode();
					break;

				//全显后测量前这一阶段等待就绪状态
				case Task_ReadyMode:
					App_ReadyMode();
					break;

				//测量模式任务（里面细分耳温、额温、物温、生产、黑体）
				case Task_Testingmode:
					App_TestingMode();
					break;

				//设置模式任务
				case Task_Setmode:
					App_SetMode();
					break;
                //单位切换模式任务
                case Task_Unitmode:
                    App_SetMode();
                    break;

				//校准模式任务
				case Task_Calimode:
					App_CaliMode();
					break;

				//绑定模式任务
				case Task_BondTestmode:
					App_BondTestMode();
					break;

				#if ParamModif
                // 参数调整模式任务
                case Task_ParamModifymode:
                    App_SetMode();
                    break;
                #endif

				//关机模式任务
				case Task_Sleepmode:
					App_Sleep();
					break;

                //记忆模式任务
				case Task_Memorymode:
					App_Memory();
					break;

				//标配保留
				default:
					break;
			}
		}
	}
}

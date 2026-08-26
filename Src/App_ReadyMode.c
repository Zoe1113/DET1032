/**************************************************************************
文件名称：	App_ReadyMode.c
说    明：	测量前等待就绪模式（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
备    注:
修订记录：
**************************************************************************/
#include "Include.h"

eReadyModeTask eReadyTask_Sta;
static uint8 Er1_Nb_InitDone = 0;

/*************** 关于本程序的说明 *******************
修改前请仔细阅读！！！
1、错误讯息规则：
Lo/HI/ER1/ER3/ER4：必须等待5s时间到，并更新ntc后，才会显示———，既可以关机、也可以进入记忆，也可以切换模式；
Er2：自更新，当错误消失后，无需等待5s，立即更新ntc后，才显示———，同时无法进入记忆模式，只能关机，也无法切换模式，同时也无法响应测量出Er1；
ER6：自更新，耳套拔出立即会恢复，无需等待5s，立即更新ntc后，才显示———，同时无法进入记忆模式，只能关机和切换模式，同时也无法响应测量出Er1；
正常测量结束（有温度显示）：汇编是测量结束立即采集ntc，5s完成后不会返回重新采集ntc，已修改成必须返回采集ntc，对临床无实质影响；
2、错误产生时机：
LO/HI/ER3/ER4发生在测量中，ER1/ER2/ER6发生在ready时，所以ER1/ER2/ER6时会刷新覆盖LO/HI/ER3/ER4，而ER2全显最高，其次ER6，再次ER1，最后LO/HI/ER3/ER4
3、错误讯息优先级：
Er2>Er6>Er1>Lo=Hi=Er3=Er4，这些错误可能会同时产生，但同时只会显示优先权高的错误，故可以用枚举替代
建议任何错误产生时都将上一个错误标志位清0，但本程序从长远考虑，不做枚举替代，防止出现错误同时产生，同时需要显示的情况---已修改都清0，因为ET不存在同时显示的情况
4、耳温模式：只采集一次ntc后，实时采集tp，其他模式均为实时采集ntc；
5、由于错误显示也可以进入记忆态，请特别注意第4点的通道切换；
6、尤其注意该程序这几个状态的跳转逻辑，修改程序必须测试该逻辑！
**************************************************/
uint16 g_NtcStepBuff[12];
uint16 g_NtcStepBuffCount = 0;
#define g_NtcStepBuffCountMax 10 	//NTC计数的最大次数值

void App_ReadyMode(void)
{	
    App_SKeyProcess();
    App_TKeyProcess();
    App_PCKeyProcess();
    Disp_Unit();	//显示单位
    App_MemKeyProcess();
    Cal_Inspect_Detect();                  //绑定检测模式、校准模式判断

    // Restart the Er1 scroll after another task interrupts it.
    if(eMain_Task != Task_ReadyMode)
    {
        Er1_Nb_InitDone = 0;
    }
    
    Disp_VoiceSign(uSetFlag.bits.VoiceEnable);
    switch( eReadyTask_Sta )
    {
        //初始化
        case Ready_Init:
            HalKey_KeyClr();	//清除所有按键信息
            
            g_50ms_Count = DispTime_Init;	//循环显示时间、日期、ntc计时器赋初值
            eReadyTask_Sta = Ready_Refresh;//开机直接刷新屏幕，不需要等待5S
            break;

        //5s超时判断
		case Ready_Timeout:
            if(Time_CountDown_5s_timeout(RUN))     //等待5s超时时间  
            {
                if(uErrFlag.bits.Lo || uErrFlag.bits.Hi || uErrFlag.bits.Er3)
                {
                    eReadyTask_Sta = Ready_Refresh;      //等待时间到后不需要更新显示
                }
                else
                {
                    eReadyTask_Sta = Ready_NoRefresh;      //等待时间到后不需要更新显示
                }
            }
            if(uErrFlag.bits.Er1)
            {
                eReadyTask_Sta = Ready_DisEr1;
            }
			break;

        case Ready_Refresh:     //需要跟新时的显示
            g_5s_Count = 0;       //这里复位5S等待，防止有Er1时出现Er2或者Er6.Er2恢复后马上测量出现Er1的问题
            Clr_Disp888();
			Disp_Ready();	//首次开机必须显示_ _._
            Disp_ModeSign();	//显示模式符号
            eReadyTask_Sta = Ready_WaitReady;
            break;
        
        case Ready_NoRefresh:     //不需要跟新时的显示
			Disp_ModeSign();	//显示模式符号
            eReadyTask_Sta = Ready_WaitReady;
            break;

        //等待就绪状态更新显示
        case Ready_WaitReady:
           
            if(eTestmode_num == Earmode || eTestmode_num == Blackbodymode)
            {
                Disp_Age_Segmentation();
            }
            else
            {
                lcd_age_clr();
            }
            LED_CloseAll();
            #if Func_White
                if(eTestmode_num != Foreheadmode)
				{
					LED_CloseAll();
					LED_White_En();
				}
            #else    
                if(eTestmode_num != Objectmode)//测量结束后有蜂鸣声
                {
                    LED_CloseAll();
                    
                    #if Func_3color
                        LED_Green_En();		//开启绿光
                    #endif
                }
            #endif
            g_3s_Count = CountDown_3s;	//开启背光3s倒计时
            
            BZ_Beep50();
            BZ_Beep50();		//蜂鸣2声
            
            //耳温切换到tp通道采集
            if (eTestmode_num == Earmode)
            {
                Adc_Channel_Init(NTCTOTP);
            }
            Auto_TurnOff_Time_Sel();
            eReadyTask_Sta = Ready_ReadyOk;		//当准备好，才可以跳到Ready_ReadyOk
            break;

        //已就绪状态
        case Ready_ReadyOk:
        
            uStaFlag.bits.Fever = 0;	//清发烧标志位（记忆可能退出）
            break;

        case Ready_DisEr1:	
        {
			if(!Er1_Nb_InitDone)
			{
				// 初始化 Er1 显示 + 非阻塞动画
				uErrFlag.g_ErrFlag = 0;
				uErrFlag.bits.Er1 = 1;
				Disp_FourSecLoop_Init();
				Er1_Nb_InitDone = 1;
			}

			if(Disp_FourSecLoop_Step())   // 返回 1 = 动画完成（5秒结束）
			{
				LED_CloseAll();
				#if Func_White
					LED_White_En();
				#elif Func_3color
					LED_Green_En();		//开启绿光
				#endif
				g_3s_Count = CountDown_3s;	//3秒背光倒计时
				Time_CountDown_5s_timeout(RESET);
                uErrFlag.bits.Er1 = 0;
                eReadyTask_Sta = Ready_Refresh;    //Er1恢复，更新显示
				eMain_Task = Task_ReadyMode;
				Er1_Nb_InitDone = 0;
			}
			break;
        }
            //Er1_Display_Sound(RUN);       //Er1的声音和显示
            //break;

        case Ready_DisEr2:
			#if Func_Obj	 
            	Er6_Display_Sound(RESET);     //复位Er6错误。防止Er6产生时，Er2也产生，但是Er2先恢复。会导致首次Er6无蜂鸣
			#endif
            Er2_Display_Sound(RUN);      //Er2的声音显示
            break;
        
        #if Func_Obj
        case Ready_DisEr6:
            Er2_Display_Sound(RESET);     //复位Er6错误。防止Er6产生时，Er2也产生，但是Er2先恢复。会导致首次Er6无蜂鸣
            Er6_Display_Sound(RUN);      //Er6的声音显示
            break;
        #endif

        case Ready_DispCap: 
        #if CAP_CHECK 
            Er2_Display_Sound(RESET);     //复位Er6错误。防止Er6产生时，Er2也产生，但是Er2先恢复。会导致首次Er6无蜂鸣
            CAP_Display_Sound(RUN);
        #endif
            break;
        default:
            break;
    }
}



uint16 NTC_StabilityRecurrence(void)		//NTC的稳定性递推函数
{
    int i;
    uint16 max=0,min=10000;

    for (i = 0; i < g_NtcStepBuffCountMax; i++) 
    {
        if (g_NtcStepBuff[i] > max) 
        {
            max = g_NtcStepBuff[i];
        }

        if (g_NtcStepBuff[i] < min) 
        {
            min = g_NtcStepBuff[i];
        }
    }
    return max-min;
}


/**************************************************************************
函数名称：	void Er1_Display_Sound(bit cmd)
函数功能：	Er1错误的蜂鸣和显示
输入参数：	cmd->   0：运行蜂鸣和等待恢复   1：复位Er1错误
输出参数：	无
返回值  ：	无
占用空间：	TBD
**************************************************************************/
void Er1_Display_Sound(bit cmd)
{
    static uint8 Er1_First_Enter = 0;
    if(cmd == RESET)
    {
        Er1_First_Enter = 0;                      //提前退出函数，防止在清零该标志位时会蜂鸣。
        goto END;                                  //使用跳转语句跳到函数末尾，复位程序时蜂鸣
    }
    Disp_ErrMsg();		//显示错误代码
    if(Er1_First_Enter == 0)                //首次进入需要蜂鸣
    {
        Er1_First_Enter = 1;

        if( uSetFlag.bits.VoiceEnable == 1 )
        {
            BZ_Beep50();
            BZ_Beep50();
            BZ_Beep50();
            BZ_Beep50();
        }
    }
    else                                  //不是首次进入，只需要等待错误恢复
    {
        if(Time_CountDown_5s_timeout(RUN))
        {
            Er1_First_Enter = 0;
            uErrFlag.bits.Er1 = 0;
            eReadyTask_Sta = Ready_Refresh;    //Er1恢复，更新显示
        }
    }
END: ;             //空语句，结束程序
}

/**************************************************************************
函数名称：	void Er2_Display_Sound(bit cmd)
函数功能：	Er2错误的蜂鸣和显示
输入参数：	cmd->   0：运行蜂鸣和等待恢复   1：复位Er2错误
输出参数：	无
返回值  ：	无
占用空间：	TBD
**************************************************************************/
void Er2_Display_Sound(bit cmd)
{
    static uint8 Er2_First_Enter = 0;      //注意该标志需要在关机时清零，否则在报Er2时关机，开机后第一次出现Er2时没有蜂鸣。
    if(cmd == RESET)
    {
        Er2_First_Enter = 0;             //提前退出函数，防止在清零该标志位时会蜂鸣。
        goto END;                         //使用跳转语句跳到函数末尾，复位程序时蜂鸣
    }
    Disp_ErrMsg();		//显示错误代码
    if(Er2_First_Enter == 0)
    {
        Er2_First_Enter = 1;
        lcd_ear_clr();
        lcd_obj_clr();
        lcd_age_clr();
        Auto_TurnOff_Time_Sel();
        /*********************************************************************************************************
        *加上这个是因为开机全显后需要清屏，产生Er2时会立即蜂鸣。这个蜂鸣会阻塞程序
        *不加上这些的话就会导致开机显示Er2的时候时间、蓝牙、电池符号的显示滞后
        *********************************************************************************************************/
		if(eTestmode_num != Insptectmode)
		{
	        LVD_Display();
		}
		Disp_Unit();	//显示单位
        /******************************************************************************************************/
        BZ_Beep50();
        BZ_Beep50();
        BZ_Beep50();
        BZ_Beep50();
    }
    else
    {
        if(uErrFlag.bits.Er2 == 0)
        {
            Er2_First_Enter = 0;
            eReadyTask_Sta = Ready_Refresh;         //Er2恢复，更新显示
            if(uErrFlag.bits.Er6 == 1)
            {
                eReadyTask_Sta = Ready_DisEr6;         //Er2恢复，更新显示
            }
        }
    }
END: ;        //跳转到此处。结束程序
}

/**************************************************************************
函数名称：	void Er6_Display_Sound(bit cmd)
函数功能：	Er6错误的蜂鸣和显示
输入参数：	cmd->   0：运行蜂鸣和等待恢复   1：复位Er6错误
输出参数：	无
返回值  ：	无
占用空间：	TBD
**************************************************************************/
#if Func_Obj	 
void Er6_Display_Sound(bit cmd)
{
    static uint8 Er6_First_Enter = 0;      //注意该标志需要在关机时清零，否则在报Er6时关机，开机后第一次出现Er6时没有蜂鸣。
    if(cmd == RESET)
    {
        Er6_First_Enter =0;              //提前退出函数，防止在清零该标志位时会蜂鸣。
        goto END;                          //使用跳转语句跳到函数末尾，复位程序时蜂鸣
    }
    Disp_ErrMsg();	//显示错误信息
    lcd_age_clr();
    if(Er6_First_Enter == 0)
    {
        Er6_First_Enter = 1;
        Auto_TurnOff_Time_Sel();
        lcd_pc_en();	//显示耳套符号
        lcd_earcap_en();//耳套
        lcd_ear_clr();
        lcd_obj_clr();
        if( uSetFlag.bits.VoiceEnable == 1 )
        {
            BZ_Beep50();
            BZ_Beep50();
            BZ_Beep50();
            BZ_Beep50();
        }
    }
    else
    {
        lcd_pc_en();
        if(uErrFlag.bits.Er6 == 0)
        {
            Er6_First_Enter = 0;
            lcd_pc_clr();	//消隐耳套符号
            eReadyTask_Sta = Ready_Refresh;
        }
    }
END: ;                           //空语句结束程序
}
#endif

/**************************************************************************
函数名称：	void CAP_Display_Sound(bit cmd)
函数功能：	CAP错误的蜂鸣和显示
输入参数：	cmd->   0：运行蜂鸣和等待恢复   1：复位Er6错误
输出参数：	无
返回值  ：	无
占用空间：	TBD
**************************************************************************/
#if CAP_CHECK	 
void CAP_Display_Sound(bit cmd)
{
    if(cmd == RESET)
    {
        Adc_Channel_Init(TPTONTC);		//重新采集ntc
        Disp_CAP();
        LED_CloseAll();
        #if Func_3color
			LED_Red_En();
		#endif
        g_3s_Count = CountDown_3s;	//开启背光3s倒计时
        Auto_TurnOff_Time_Sel();

        BZ_Beep50();    //CAP报错，蜂鸣4声
        BZ_Beep50();
        BZ_Beep50();
        BZ_Beep50();
    }
    else
    {
        Disp_CAP();
        if ( uStaFlag.bits.ProbeCover )
        {
            Clr_Disp888();
            lcd_pc_clr();
            lcd_pc_en();
            g_5s_Count = 0;	                //恢复无需等待5s
            uErrFlag.g_ErrFlag = 0;			//清除所有错误
            eReadyTask_Sta = Ready_Refresh;
        }
    }
}
#endif

/**************************************************************************
函数名称：	void Disp_VoiceSign(bit State)
函数功能：	Er6错误的蜂鸣和显示
输入参数：	State->   0：声音符号消隐   1：显示声音符号
输出参数：	无
返回值  ：	无
占用空间：	TBD
**************************************************************************/
void Disp_VoiceSign(bit State)
{
    if( State ) 
    {
        lcd_Voice_en();
    }
    else
    {
        lcd_Voice_clr();
    }
}

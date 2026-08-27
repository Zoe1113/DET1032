/**************************************************************************
文件名称：	Drv_Key.c
说    明：	按键处理相关函数集合（驱动层、应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
备	  注：	请留意本程序release的用法。
修订记录：
**************************************************************************/
#include "Include.h"

//按键状态eKeySta
enum eKeyStatus
{
    KeySta_Init = 0,	//按键初始化状态
    KeySta_Dither = 1,	//按键去抖状态
    KeySta_Comfirm = 2,	//按键确认状态
    KeySta_Release = 3	//按键释放状态
} eKeySta;

//按键键值eKeyVal
enum eKeyValue
{
    SetKey = 0x01,		 //设置键   p00
    TestKey = 0x02,		 //开机键&测量键   p01
    MemKey = 0x10,		 	 //记忆键  p04
    EarcapKey = 0x20,		 //耳套键  p17
    KeyMask = 0x33		 //按键掩码（p00 p01 p04 p17）
}eKeyVal;

uKey1 uKeyPress;
uKey2 uKeyHold;
uKey3 uKeyRelease;
uKey4 uKeyContinue;

strKey sMemKey, sTestKey, sSetKey, sEarcapKey;

/**************************************************************************
函数名称：	void App_KeyProcess(void)
函数功能：	按键功能组合处理子程序（应用层）
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	仅适用于Task_ReadyMode状态
与扫键程序不同的是，此函数属于应用层，用于识别不同事件产生对应的任务标志位
比如：
1、长按测量键开机3s进入单位切换；→ 在sleep中处理;ok
2、短按开机键开机；→ ok；
3、长按开机键3s进入设置，或长按设置键3s进入设置；→ 全显的在init模式处理，不全显的在sleep处理；ok
4、Ready模式长按开机键3s进入记忆；→ ok，在本程序中处理；ok
5、Ready模式长按开机键3s到关机再长按3s进入校准；→ ok，在sleep中处理；
6、Ready模式长按开机键并立即长按测量键1.2s进行耳物温切换；→ ok
7、Ready模式检测io进入绑定、系数调整等；ok
8、Ready模式检测耳套；ok
9、长按开机、两次按下测量进入生产态；ok；
**************************************************************************/

/**************************************************************************
函数名称：	void App_MemKeyProcess(void)
函数功能：	记忆键功能
输入参数：	uKeyRelease.bits.MemKeyRelease
输出参数：	无
返回值  ：	无
占用空间：	TBD
**************************************************************************/
void App_MemKeyProcess(void)
{
    static uint8 F_MemKey_Deal=0;		//记忆键长按处理

    //长按进入记忆后，松开事件由App_Memory处理；返回Ready时主动解除长按锁定
    if( !uKeyPress.bits.MemKeyPress && !uKeyHold.bits.MemKeyHold && !uKeyRelease.bits.MemKeyRelease )
    {
        F_MemKey_Deal = 0;
    }

    //记忆键长按3s进入记忆查看
    if( uKeyPress.bits.MemKeyPress )									//开机键按下
    {
        uKeyRelease.bits.MemKeyRelease = 0;
        if( uKeyHold.bits.MemKeyHold && !F_MemKey_Deal && !uErrFlag.bits.Er2 && !uErrFlag.bits.Er6 && eTestmode_num != Insptectmode )			//生产模式无记忆
        {
            F_MemKey_Deal = 1;
            uKeyHold.bits.MemKeyHold = 0;
			sMemKey.g_Key_Hold_cnt = 0;	//按键计时清0保证再次长按3s
			uKeyRelease.bits.MemKeyRelease = 0;
            Auto_TurnOff_Time_Sel();	//按下关机时间清0
            Time_CountDown_5s_timeout(RESET);		//记忆模式打断5s等待，清除倒计时相关标志位
            g_5s_Count = 0;
            F_Mem_FirstEnter = 0;	//清首次进入记忆模式标志位
            eMain_Task = Task_Memorymode;					//置进入记忆模式标志位
            uErrFlag.g_ErrFlag = 0;		//清错误标志位
            eReadyTask_Sta = Ready_ReadyOk;
            g_AgeNode_Men = eAgemode_num;	//进入记忆前记录年龄分段
        }
    }
    //记忆键短按开关蜂鸣
    else
    {
        if(uKeyRelease.bits.MemKeyRelease)
        {
            uKeyRelease.bits.MemKeyRelease = 0;
            if( !F_MemKey_Deal && !uErrFlag.bits.Er2 && !uErrFlag.bits.Er6 )			//短按释放
            {
                Auto_TurnOff_Time_Sel();
                uSetFlag.bits.VoiceEnable = !uSetFlag.bits.VoiceEnable;
                if( uSetFlag.bits.VoiceEnable ) 
                {
                    lcd_Voice_en();
                    BZ_Beep125();		//蜂鸣打开响一声
                }
                else
                {
                    lcd_Voice_clr();
                }
            }
            F_MemKey_Deal =0;
        }
    }
}

/**************************************************************************
函数名称：	void App_TKeyProcess(void)
函数功能：	开机/测量键功能
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
**************************************************************************/
void App_TKeyProcess(void)
{
    //测量键 短按测量开始测量
    if( uKeyRelease.bits.TKeyRelease && !uErrFlag.bits.Er2)
    {
        uKeyRelease.bits.TKeyRelease = 0;
        if(!uKeyPress.bits.TKeyPress)
        {
            if(eMain_Task == Task_Memorymode)
            {
                eAgemode_num = g_AgeNode_Men;   //退出记忆后，恢复
                eMain_Task = Task_ReadyMode;				//退出记忆模式
                lcd_mem_clr();
                g_5s_Count = 0;
                if(eTestmode_num == Earmode || eTestmode_num == Blackbodymode)    //耳温切换通道
                {
                    Disp_Age_Segmentation();
                    #if CAP_CHECK
                    if(uStaFlag.bits.ProbeCover)
                    {
                        Adc_Channel_Init(NTCTOTP);
                    }
                    #else
                        Adc_Channel_Init(NTCTOTP);
                    #endif
                }
                else
                {
                    lcd_age_clr();
                }
            }
            Auto_TurnOff_Time_Sel();	//按下关机时间清0
            
            if( eReadyTask_Sta == Ready_ReadyOk)
            {
                Time_CountDown_5s_timeout(RESET);
                eMain_Task = Task_Testingmode;
            }

            #if CAP_CHECK
                if((uStaFlag.bits.ProbeCover == 0) && (eTestmode_num  != Objectmode) && (eTestmode_num  != Insptectmode))
                {
                    CAP_Display_Sound(RESET);
                }
            #endif

            //未准备好按下则Er1报错
            if( eTestmode_num != Objectmode && !uErrFlag.bits.Er6 && eReadyTask_Sta != Ready_DispCap&&eReadyTask_Sta == Ready_Timeout)
            {
                uErrFlag.g_ErrFlag = 0;	//清其他错误标志位
                uErrFlag.bits.Er1 = 1;
                //切换模式状态标志处理
                Adc_Channel_Init(TPTONTC);			//切换到ntc通道
                eReadyTask_Sta = Ready_DisEr1;	//只要有错需调到错误处理状态
            }
        }								
    }
    else if(uKeyHold.bits.TKeyHold  )			//如果长按三秒
    {
        //长按3s进入关机模式，并清除必要设定
        sTestKey.g_Key_Hold_cnt = 0;	//按键计时清0保证再次长按3s
        uKeyHold.bits.TKeyHold = 0;
        //eReadyTask_Sta = Ready_Init;
        eMain_Task = Task_Sleepmode;
        eSleepTask_Sta = Sleep_false;
        HalKey_Set_KeyMode(Func_Super_Long, &sTestKey);	//测量键设为超长按
    }
}

/**************************************************************************
函数名称：	void SKeyProcess(void)
函数功能：	设置键功能
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
**************************************************************************/
void App_SKeyProcess(void)
{
    static uint8 F_SKey_Deal=0;		//设置键长按处理
    //设置键长按
    if( uKeyPress.bits.SKeyPress && eMain_Task != Task_Memorymode)									//设置键按下
    {
        uKeyRelease.bits.SKeyRelease = 0;
        #if Nation
        if(	uKeyHold.bits.SKeyHold && !uErrFlag.bits.Er2)			//如果长按三秒
        {
             Auto_TurnOff_Time_Sel();	//按下关机时间清0
            F_SKey_Deal = 1;
            sSetKey.g_Key_Hold_cnt = 0;	//按键计时清0保证再次长按3s
            uKeyHold.bits.SKeyHold = 0;
            #if Func_Obj
                if( (eTestmode_num == Earmode || eTestmode_num == Objectmode) )
                {
                    //切换模式状态标志处理
                    Adc_Channel_Init(TPTONTC);			//重新采集ntc
                    Er6_Display_Sound(RESET);
                    g_5s_Count = 0;       //切换模式，清零计数器。马上可以测量
                    uErrFlag.g_ErrFlag = 0;			//清除所有错误					
                    uErrFlag.bits.Er6 = 0;			//恢复原Er6产生的错误标志位
                    if( eTestmode_num == Earmode )
                    {
                        eTestmode_num = Objectmode;
                        lcd_pc_clr();
                    }
                    else
                    {
                        eTestmode_num = Earmode;
                    }	
                    eReadyTask_Sta = Ready_Refresh;
                }
            #endif	
        }	
        #endif
    }
    //设置键短按
    else
    {
        if( uKeyRelease.bits.SKeyRelease )							//判断开机键按下后是否抬起（3s内非长按抬起关机）
        {
            uKeyRelease.bits.SKeyRelease = 0;	
            if( !F_SKey_Deal && !uErrFlag.bits.Er2&& !uErrFlag.bits.Er1&&(eTestmode_num==Earmode||eTestmode_num==Blackbodymode) )
            {
                Auto_TurnOff_Time_Sel();	//按下关机时间清0
                eAgemode_num++;
                if(eAgemode_num > BigAge)
                {
                    eAgemode_num = LittleAge;
                }
                Disp_Age_Segmentation();
                if(eReadyTask_Sta == Ready_ReadyOk)
                    Disp_Ready();
                BZ_Beep125();       //蜂鸣打开响一声
            }	
            F_SKey_Deal =0;
        }
    }
}

/**************************************************************************
函数名称：	void App_PCKeyProcess(void)
函数功能：	ReadyMode和MemoryMode下的耳套检测
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
**************************************************************************/
void App_PCKeyProcess(void)
{
    static uint8 F_Enevt = 0, F_Lsat_Eenvt = 0;
    // 以下为耳套检测按键处理程序 
    if( uKeyPress.bits.EKeyPress )				//耳套键按下，
    {
        F_Enevt = 1;                            //戴耳套了
        uKeyRelease.bits.EKeyRelease = 0;		//清除按键抬起标志位
    }

    //等待模式检测耳套
    if(eReadyTask_Sta != Ready_Init )
    {
        //物温模式耳套按下报错，抬起恢复
        if (eTestmode_num == Objectmode)
        {
            if ( uKeyPress.bits.EKeyPress )
            {
                uErrFlag.g_ErrFlag = 0;	//清除其他错误
                uErrFlag.bits.Er6 = 1;
                eReadyTask_Sta = Ready_DisEr6;
            }
            else
            {
                uErrFlag.bits.Er6 = 0;
            }
        }
    }

    //耳温/黑体/生产模式耳套按下则表示带耳套，反之无耳套
    if( eTestmode_num != Objectmode )
    {
        if( uKeyPress.bits.EKeyPress )
        {
            LCD_pc_Show(2);
            lcd_pc_clr();
            lcd_pc_en();	//显示耳套符号
            lcd_earcap_en();//耳套
            uStaFlag.bits.ProbeCover = 1;
        }
        else
        {
            if(eTestmode_num == Earmode || eTestmode_num == Blackbodymode)
            {
                if(eMain_Task == Task_ReadyMode)
                    LCD_pc_Show(1);
                else
                    lcd_pc_en();	//显示耳温锥符号
                uStaFlag.bits.ProbeCover = 0;
            }
            else
            {
                lcd_pc_clr();	//消隐耳套符号
            }
            F_Enevt = 0;        //没带耳套
        }
        if(eMain_Task == Task_Memorymode)
        {
            if(!Port_Earcap && Port_Test)
            {
                lcd_pc_en();	//显示耳套符号
                lcd_earcap_en();//耳套
            }
            else
            {
                lcd_pc_clr();
                lcd_earcap_clr();
            }
        }
    }
    #if CAP_CHECK 
    if((eTestmode_num == Earmode || eTestmode_num == Blackbodymode) && (uErrFlag.bits.Er2 != 1) && (eReadyTask_Sta != Ready_Init))
    {
        if( uStaFlag.bits.ProbeCover == 0)           //第一次检测耳套未带上
        {    
            if(eReadyTask_Sta != Ready_DispCap)
            {
                Adc_Channel_Init(TPTONTC);			//重新采集ntc
            }
            eReadyTask_Sta = Ready_DispCap;	//只要有错需跳转到错误处理状态
        }
    }
    #endif
    if(F_Enevt != F_Lsat_Eenvt)
    {
        Auto_TurnOff_Time_Sel();	//关机时间选择
    }
    F_Lsat_Eenvt = F_Enevt;	                //保存本次耳套杆事件
}

/**************************************************************************
函数名称：	uint8 HalKey_ReadKeyVal(void)
函数功能：	读取按键键值
输入参数：	P0
输出参数：	无
返回值  ：	L_keydata（按键键值）
占用空间：	TBD
备    注：	无
**************************************************************************/
uint8 HalKey_ReadKeyVal(void)
{
	uint8 L_keydata;
	L_keydata = P0 & 0x13;	//只读取P00,P01,P04，P05不再使用
	L_keydata ^= 0x13;	//低电平有效按键取反
	
	//读取P17作为耳套键
	if(!FP17)	//P17低电平表示按键按下
	{
		L_keydata |= EarcapKey;	//耳套键标志
	}
	return L_keydata;
}

/**************************************************************************
函数名称：	void HalKey_KeyScan(void)
函数功能：	扫键函数
输入参数：	无
输出参数：	uOTKeyFlag、uSMKeyFlag
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void HalKey_KeyScan(void)
{
    uint8 L_Keydata;
    L_Keydata = HalKey_ReadKeyVal();				  //读取按键所在端口P0的状态和判断按键端口是否有按下
    HalKey_Scan(L_Keydata & MemKey , &sMemKey);  //判断记忆按键的状态
    HalKey_Scan(L_Keydata & TestKey , &sTestKey); //判断开机&测量键的状态
    HalKey_Scan(L_Keydata & SetKey ,  &sSetKey);    //判断设置按键的状态
    HalKey_Scan(L_Keydata & EarcapKey , &sEarcapKey); //判断耳套键的状态
}

/**************************************************************************
函数名称：	void HalKey_KeyScan(void)
函数功能：	扫键函数
输入参数：	无
输出参数：	uOTKeyFlag、uSMKeyFlag、uKeyRelease、g_KeyContinue_Flag、g_KeyPress_Flag、g_KeyHold_Flag
返回值  ：	无
占用空间：	TBD
备    注：	uKeyRelease必须手动清除
**************************************************************************/
void HalKey_Scan(uint8 L_Keydata, strKey *sKey)
{
    switch (sKey->g_Key_Status)
    {
        //按键等待状态
        case KeySta_Init:
            if (L_Keydata) //如有按键按下
            {
                sKey->g_Key_Hold_cnt = 0;
                sKey->g_Key_Val = L_Keydata;
                // uKeyRelease.g_KeyRelease_Flag &= ~L_Keydata;		//抬起必须手动清除
                sKey->g_Key_Status = KeySta_Dither;
            }
            break;
        //按键去抖状态
        case KeySta_Dither:
            if ( sKey->g_Key_Val == L_Keydata )
            {
                sKey->g_Key_Hold_cnt ++;
                if ( sKey->g_Key_Hold_cnt > CNT_Dither)
                {
                    sKey->g_Key_Status = KeySta_Comfirm;
                    uKeyPress.g_KeyPress_Flag |= L_Keydata;	//置已按下标志位
                }
            }
            else
            {
                sKey->g_Key_Status = KeySta_Init;
            }
            break;
        //按键确认状态
        case KeySta_Comfirm:
            if (L_Keydata == sKey->g_Key_Val)
            {
                if (sKey->g_Key_Hold_cnt < 0xFE) //不可超出否则从0开始
                {
                    sKey->g_Key_Hold_cnt ++;
                }
                if (sKey->g_Key_Hold_cnt > sKey->g_Key_preset_cnt)
                {
                    uKeyHold.g_KeyHold_Flag |= L_Keydata;	//置按键长按标志位
                    //如果使能加速按
                    if ( sKey->g_KeyFun & En_Cp )
                    {
                        uKeyContinue.g_KeyContinue_Flag |= L_Keydata;	//置按键加速按标志位
                        sKey->g_Key_Hold_cnt = sKey->g_Key_preset_cnt - CNT_CPInterval;
                    }
                }
            }
            else
            {
                sKey->g_Key_Status = KeySta_Release;
            }
            break;
        //按键释放状态
        case KeySta_Release:
            if (!L_Keydata)
            {
                uKeyHold.g_KeyHold_Flag &= ~sKey->g_Key_Val;
                uKeyPress.g_KeyPress_Flag &= ~sKey->g_Key_Val;
                uKeyContinue.g_KeyContinue_Flag &= ~sKey->g_Key_Val;
                uKeyRelease.g_KeyRelease_Flag |= sKey->g_Key_Val;
                sKey->g_Key_Status = KeySta_Init;
            }
            break;
        default:
            break;
    }
}

/**************************************************************************
函数名称：	void HalKey_Set_KeyMode(uint8 function, struct strKey *sKey )
函数功能：	开机键按键功能设置
输入参数：	mode（短按/长按/超长按/加速按/无设定默认为长按）
输出参数：	sKey->g_KeyFun（按键功能）、sKey->g_Key_preset_cnt（按键计数器初值）
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void HalKey_Set_KeyMode(uint8 function, strKey *sKey )
{
    switch (function)
    {
        case Func_Short:
            sKey->g_KeyFun &= ~En_Cp; 	//非加速按
            sKey->g_Key_preset_cnt = CNT_Invalid;
            break;
        case Func_Short_Long:
            sKey->g_KeyFun &= ~En_Cp;	//非加速按
            sKey->g_Key_preset_cnt = CNT_ShortLong;
            break;
        case Func_Long:
            sKey->g_KeyFun &= ~En_Cp;	//非加速按
            sKey->g_Key_preset_cnt = CNT_LongPress;
            break;
        case Func_Short_Continue:
            sKey->g_KeyFun |= En_Cp; 	//加速按
            sKey->g_Key_preset_cnt = CNT_EnterCP;
            break;
        case Func_Super_Long:
            sKey->g_KeyFun &= ~En_Cp;	//非加速按
            sKey->g_Key_preset_cnt = CNT_LLongPress;
            break;
        default:
            sKey->g_KeyFun &= ~En_Cp; 	//非加速按
            sKey->g_Key_preset_cnt = CNT_LongPress;
            break;
        }
}

/**************************************************************************
函数名称：	void HalKey_KeyClr(void)
函数功能：	清除所有按键信息（非所有，重点部分）
输入参数：	g_KeyHold_Flag、g_KeyPress_Flag、g_KeyRelease_Flag、g_KeyContinue_Flag、g_Key_Val（键值）、g_Key_Status（按键状态）
输出参数：	g_KeyHold_Flag、g_KeyPress_Flag、g_KeyRelease_Flag、g_KeyContinue_Flag、g_Key_Val（键值）、g_Key_Status（按键状态）
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void HalKey_KeyClr(void)
{
    uKeyHold.g_KeyHold_Flag = 0;
    uKeyPress.g_KeyPress_Flag = 0;
    uKeyRelease.g_KeyRelease_Flag = 0;
    uKeyContinue.g_KeyContinue_Flag = 0;

    sMemKey.g_Key_Val = 0;
    sTestKey.g_Key_Val = 0;
    sSetKey.g_Key_Val = 0;
    sEarcapKey.g_Key_Val = 0;

    sMemKey.g_Key_Status = 0;
    sTestKey.g_Key_Status = 0;
    sSetKey.g_Key_Status = 0;
    sEarcapKey.g_Key_Status = 0;
}

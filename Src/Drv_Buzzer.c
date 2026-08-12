/**************************************************************************
文件名称：	Drv_Buzzer.c
说    明：	蜂鸣相关函数集合（驱动层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"

eBeep eBeep_Status;
uint8 g_beep_time;	//蜂鸣时长（10ms时基）
uint8 g_beep_loop;	//蜂鸣循环次数

/**************************************************************************
函数名称：	void App_Beep(void)
函数功能：	蜂鸣
输入参数：	g_beep_time、g_beep_loop
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	10ms时基，对于连续蜂鸣(beep_loop > 1)，只可以修改首次蜂鸣时间（beep_time），其后连响的时间固定为140ms，如要修改，请增加形参变量
**************************************************************************/
void App_Beep(void)
{
	if( uSetFlag.bits.VoiceEnable )
	{
		switch (eBeep_Status)
		{
			case Stop_beep:
				break;

			case Enable_beep:
				Drv_BZ_Enable();
				eBeep_Status = Start_beep;
				break;

			case Start_beep:
				g_beep_time --;
				if ( !g_beep_time )
				{
					Drv_BZ_Disable();
					g_beep_time = 14;
					eBeep_Status = Disable_beep;
				}
				break;

			case Disable_beep:
				g_beep_time --;
				if ( !g_beep_time )
				{
					g_beep_loop --;
					if( !g_beep_loop )
					{
						eBeep_Status = Stop_beep;
					}
					else
					{
						g_beep_time = 14;
						eBeep_Status = Enable_beep;
					}
				}
				break;
		}		
	}
	else
	{
		Drv_BZ_Disable();
		eBeep_Status = Stop_beep;
	}	
}

#if BZ_HW

void Drv_BZ_Enable(void)
{
	TC2Init();
	FTC2ENB = 1 ;		//使能TC0定时器
	FPWM8OUT = 1 ;		//enable PT10 pwm 4KHZ输出
}

void Drv_BZ_Disable(void)
{
	FTC2ENB = 0 ;		//关闭TC0定时器
	FPWM8OUT = 0 ;		//关闭 PT10 pwm 4KHZ输出
	Port_BZ = 0;
}

#endif

void BZ_Beep50(void)
{
	if( uSetFlag.bits.VoiceEnable )
	{
		#if BZ_HW
			Drv_BZ_Enable();
			Delay10ms(14);		//延迟140ms
			Drv_BZ_Disable();
			Delay10ms(5);		//延迟50ms
		#else
			Drv_BZ_Enable();
			Delay10ms(14);		//延迟140ms
			Drv_BZ_Disable();
			Delay10ms(5);		//延迟50ms
		#endif		
	}

}

void BZ_Beep125(void)
{
	if( uSetFlag.bits.VoiceEnable )
	{
		#if BZ_HW
			Drv_BZ_Enable();
			Delay10ms(14);		//延迟140ms
			Drv_BZ_Disable();
			Delay10ms(14);		//延迟140ms
		#else
			Drv_BZ_Enable();
			Delay10ms(14);		//延迟140ms
			Drv_BZ_Disable();
			Delay10ms(14);		//延迟140ms
		#endif		
	}  

}

void BZ_Beep400(void)
{
	if( uSetFlag.bits.VoiceEnable )
	{
		#if BZ_HW
			Drv_BZ_Enable();
			Delay10ms(42);		//延迟420ms
			Drv_BZ_Disable();
			Delay10ms(14);		//延迟140ms
		#else
			Drv_BZ_Enable();
			Delay10ms(42);		//延迟420ms
			Drv_BZ_Disable();
			Delay10ms(14);		//延迟140ms
		#endif		
	} 

}

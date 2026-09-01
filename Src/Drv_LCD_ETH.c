/**************************************************************************
文件名称：	Drv_LCD_ET05.c
说    明：	液晶显示函数集合（驱动层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"



//常量定义
//后面应该用AbCdEF取代
uint16 __ROM	DispTable[10] =	 {	SA+SB+SC+SD+SE+SF,			//0		0
									SB+SC,						//1		1
									SA+SB+SD+SE+SG,				//2		2
									SA+SB+SC+SD+SG,				//3		3
									SB+SC+SF+SG,				//4		4
									SA+SC+SD+SF+SG,				//5		5
									SA+SC+SD+SE+SF+SG,			//6		6
									SA+SB+SC,					//7		7
									SA+SB+SC+SD+SE+SF+SG,		//8		8
									SA+SB+SC+SD+SF+SG,			//9		9
								} ;

/**************************************************************************
函数名称：	Lcd_Init()
函数功能：	Lcd初始化
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Lcd_Init( void )
{
	P4SEG = 0x00 ;
	P3SEG = 0X00 ;
	P2SEG = 0x1F ;			//P2.0-P2.4为GPIO，P2.5-P2.7为LCD

	LCDM1 = 0x03 ;			//1/3Bias,C-Type LCD Mode.
	LCDM2 = 0x04 ;			//VLCD = 3.0V,
	LCDM3 = 0X02 ;			//Disable LCD low power mode./0 = 4-COM
	Delay50us(100) ;
	FLCDEN = 1 ;			//使能LCD
}

/**************************************************************************
函数名称：	Disp_Temp()
函数功能：	显示测量值
输入参数：	point：0表示不显示小数点、1表示显示小数点
			High：0表示0.1显示模式，1表示0.01高精度模式
			Unit: 华氏＞=200判断标志位，此时High需置0
			temp：温度值（16进制）
输出参数：	LCD中188.88
返回值  ：	无
占用空间：	411words
备    注：	0.01精度可能需要在其他地方显示，视不同机型决定
			F华氏单位当>=200时需移位处理
			适合低精度显示、高精度显示、无小数点显示
**************************************************************************/
void Disp_Temp(bit Point, bit High, bit Unit, int16 Temp)
{
	uint16 R_LCD1,R_LCD2,R_LCD3,R_LCD4,R_LCD5;
	bit F_NegFlag = 0;

	//如果为负数转出正数，同时-20.00禁止显示高精度
	if ( Temp < 0 )
	{
		F_NegFlag = 1;
		Temp = ~Temp + 1;
		if (Temp == 2000)
		{
			Temp = Temp / 10;
			Point = 0;		//强行置零以防用户误输入
		}
	}

	//如果是华氏且大于199.99则禁止高精度显示
	if( Unit && (Temp > 0x4E1F) )
	{
		Temp = Temp / 10;
		Point = 0;		//强行置零以防用户误输入
	}

	//强制转换成无符号数
	Temp = (uint16)Temp;

	HexToBcd(Temp);

	//取百位
	R_LCD1 = Hex2Bcd[2] & 0x0F;
	//取十位
	R_LCD2 = Hex2Bcd[1] >> 4;
	//取个位
	R_LCD3 = Hex2Bcd[1] & 0x0F;
	//取0.1位
	R_LCD4 = Hex2Bcd[0] >> 4;
	//取0.01位
	R_LCD5 = Hex2Bcd[0] & 0x0F;

	//R_LCD1如果非0，不管R_LCD2是否为0都要显示，如果R_LCD1为0，那么R_LCD2非0才会显示
    lcd9 &= 0x01;
	if( R_LCD2 !=0 || R_LCD1!=0)
	{
		lcd9 = DispTable[ R_LCD2 ] >> 8;
    	lcd8 = DispTable[ R_LCD2 ];
	}
	else
	{
		lcd9 = 0x00;
		lcd8 = 0x00;
	}

	//对于温度来说8.8一定会显示的（当然要考虑校准态AD显示另算）
	lcd7 = DispTable[ R_LCD3 ] >> 8;
    lcd6 = DispTable[ R_LCD3 ];

    lcd5 &= 0x01;
	lcd5 = (DispTable[ R_LCD4 ] >> 8);
    lcd4 = DispTable[ R_LCD4 ];

	//最高位置1
	if( R_LCD1 )
	{
		lcd9 |= 0x01;
	}

	//小数点点亮
	if( Point )
	{
		lcd_point_en();
	}

	//高精显示 小于40.00，第一位不显示
	if( High )
	{
        lcd_point_clr();
        if(R_LCD2 > 3)
        {
            lcd9 = DispTable[ R_LCD3 ] >> 8;
            lcd9 |= 0x01;
    	    lcd8 = DispTable[ R_LCD3 ];
            lcd7 = DispTable[ R_LCD4 ] >> 8;
            lcd6 = DispTable[ R_LCD4 ];
            lcd5 &= 0x01;
            lcd5 = (DispTable[ R_LCD5 ] >> 8);
            lcd4 = DispTable[ R_LCD5 ];
        }
        else
        {
            lcd9 = DispTable[ R_LCD3 ] >> 8;
    	    lcd8 = DispTable[ R_LCD3 ];
            lcd7 = DispTable[ R_LCD4 ] >> 8;
            lcd6 = DispTable[ R_LCD4 ];
            lcd5 &= 0x01;
            lcd5 = (DispTable[ R_LCD5 ] >> 8);
            lcd4 = DispTable[ R_LCD5 ];
        }
	}

	//负数显示﹣
	if( F_NegFlag )
	{
		lcd9 |= lcd_dash;
	}
}

void Cal_Disp_Temp(int16 Temp)
{
	uint16 R_LCD1,R_LCD2,R_LCD3,R_LCD4,R_LCD5;
	bit F_NegFlag = 0;

	//如果为负数转出正数，同时-20.00禁止显示高精度
	if ( Temp < 0 )
	{
		F_NegFlag = 1;
		Temp = ~Temp + 1;
		if (Temp == 2000)
		{
			Temp = Temp / 10;
		}
	}
	//强制转换成无符号数
	Temp = (uint16)Temp;

	HexToBcd(Temp);

	//取百位
	R_LCD1 = Hex2Bcd[2] & 0x0F;
	//取十位
	R_LCD2 = Hex2Bcd[1] >> 4;
	//取个位
	R_LCD3 = Hex2Bcd[1] & 0x0F;
	//取0.1位
	R_LCD4 = Hex2Bcd[0] >> 4;
	//取0.01位
	R_LCD5 = Hex2Bcd[0] & 0x0F;

	//R_LCD1如果非0，不管R_LCD2是否为0都要显示，如果R_LCD1为0，那么R_LCD2非0才会显示
    lcd9 &= 0x01;
	if( R_LCD2 !=0 || R_LCD1!=0)
	{
		lcd9 = DispTable[ R_LCD2 ] >> 8;
    	lcd8 = DispTable[ R_LCD2 ];
	}
	else
	{
		lcd9 = 0x00;
		lcd8 = 0x00;
	}

	//最高位置1
	if( R_LCD1 )
	{
		lcd9 |= 0x01;
	}

    lcd_point_clr();
    if(R_LCD2 > 3)
    {
        lcd9 = DispTable[ R_LCD3 ] >> 8;
        lcd9 |= 0x01;
        lcd8 = DispTable[ R_LCD3 ];
        lcd7 = DispTable[ R_LCD4 ] >> 8;
        lcd6 = DispTable[ R_LCD4 ];
        lcd5 &= 0x01;
        lcd5 = (DispTable[ R_LCD5 ] >> 8);
        lcd4 = DispTable[ R_LCD5 ];
    }
    else
    {
        lcd9 = DispTable[ R_LCD3 ] >> 8;
        lcd8 = DispTable[ R_LCD3 ];
        lcd7 = DispTable[ R_LCD4 ] >> 8;
        lcd6 = DispTable[ R_LCD4 ];
        lcd5 &= 0x01;
        lcd5 = (DispTable[ R_LCD5 ] >> 8);
        lcd4 = DispTable[ R_LCD5 ];
    }

	//负数显示﹣
	if( F_NegFlag )
	{
		lcd_obj_en();
	}
    else
    {
        lcd_obj_clr();
    }
}

/**************************************************************************
函数名称：	Disp_Code()
函数功能：	显示程序编码
输入参数：	程序编码（如142，则输入142）
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Code(uint16 num)
{
	uint16 R_LCD1,R_LCD2,R_LCD3;

	//取百位
	R_LCD1 = num / 100;
	//取十位
	num = num % 100;
	R_LCD2 = num / 10;
	//取个位
	R_LCD3 = num % 10;

	//对于程序编码888一定会显示的
	lcd9 = DispTable[ R_LCD1 ] >> 8;
    lcd8 = DispTable[ R_LCD1 ];
	lcd7 = DispTable[ R_LCD2 ] >> 8;
    lcd6 = DispTable[ R_LCD2 ];
	lcd5 = DispTable[ R_LCD3 ] >> 8;
    lcd4 = DispTable[ R_LCD3 ];
}

/**************************************************************************
函数名称：	Disp_Version()
函数功能：	显示程序版本
输入参数：	格式为U1.0，U和.为固定显示，10为输入的2位数字
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Version(uint16 num)
{
	uint16 R_LCD1,R_LCD2;

	//取十位
	R_LCD1 = num / 10;
	//取个位
	R_LCD2 = num % 10;

	lcd9 = S_U>>8;
    lcd8 = S_U;
	lcd7 = DispTable[ R_LCD1 ] >> 8;
    lcd6 = DispTable[ R_LCD1 ];
	lcd5 = DispTable[ R_LCD2 ] >> 8;
    lcd_point_en();
    lcd4 = DispTable[ R_LCD2 ];
}

/**************************************************************************
函数名称：	Clr_Disp()
函数功能：	完全清屏
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Clr_Disp(void)
{
	lcd0 = 0x00;
    lcd1 = 0x00;
    lcd2 = 0x00;
    lcd3 = 0x00;
    lcd4 = 0x00;
    lcd5 = 0x00;
    lcd6 = 0x00;
    lcd7 = 0x00;
    lcd8 = 0x00;
    lcd9 = 0x00;
	lcd10 = 0x00;
    lcd11 = 0x00;
    lcd12 = 0x00;
    lcd13 = 0x00;
    lcd14 = 0x00;

}

/**************************************************************************
函数名称：	Clr_Disp888()
函数功能：	清除温度的3个888
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Clr_Disp888(void)
{
	lcd4 = 0x00;
    lcd5 = 0x00;
    lcd6 = 0x00;
    lcd7 = 0x00;
	lcd8 = 0x00;
    lcd9 = 0x00;
}

/**************************************************************************
函数名称：	Disp_All()
函数功能：	全显
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_All(void)
{
    lcd0 = 0xff;
    lcd1 = 0xff;
    lcd2 = 0xff;
    lcd3 = 0xff;
    lcd4 = 0xff;
    lcd5 = 0xff;
    lcd6 = 0xff;
    lcd7 = 0xff;
    lcd8 = 0xff;
    lcd9 = 0xff;
    lcd10 = 0xff;
    lcd11 = 0xff;
    lcd12 = 0xff;
    lcd13 = 0xff;
    lcd14 = 0xff;
	if( uSetFlag.bits.Unit_Change )
	{
		Disp_Unit();
	}

	#if Second_LVD != 1
					//国内电池图标内部三个组件屏蔽
	#endif

	#if !Func_Obj
		lcd_obj_clr();
	#endif

	#if !Func_Ble
		lcd_ble_clr();
	#endif
}

/**************************************************************************
函数名称：	Disp_Unit()
函数功能：	根据单位设置显示对应单位
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Unit(void)
{
	lcd_unit_c_en();
	if( uSetFlag.bits.Unit )
	{
		lcd_unit_f_en();
	}
}

/**************************************************************************
函数名称：	Disp_SmileFace()
函数功能：	仅显示笑脸
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_SmileFace(void)
{
	lcd_badface_clr();	//清哭脸
	lcd_smileface_en();	//显笑脸
}

/**************************************************************************
函数名称：	void Disp_BadFace(void)
函数功能：	仅显示哭脸
输入参数：	无
输出参数：	LCD16
返回值  ：	无
占用空间：	TBD
备    注：	生产检验模式、黑体模式使用
**************************************************************************/
void Disp_BadFace(void)
{
	lcd_smileface_clr();	//清笑脸
	lcd_badface_en();	//显哭脸
}

/**************************************************************************
函数名称：	void Disp_ModeSign(void)
函数功能：	根据不同模式显示不同标志
输入参数：	无
输出参数：	LCD16
返回值  ：	无
占用空间：	TBD
备    注：	生产检验模式、黑体模式使用
**************************************************************************/
void Disp_ModeSign(void)
{
	lcd_ear_clr();		//清除耳温标志
	lcd_obj_clr();		//清除物温标志
	switch ( eTestmode_num )
	{
		case Earmode:
			lcd_ear_en();
			break;

		case Objectmode:
		case Insptectmode:
		case Blackbodymode:
			lcd_obj_en();
			break;
	}
}

/**************************************************************************
函数名称：	void Disp_Age_Segmentation(void)
函数功能：	根据不同模式显示不同标志
输入参数：	无
输出参数：	LCD16
返回值  ：	无
占用空间：	TBD
备    注：	生产检验模式、黑体模式使用
**************************************************************************/
void Disp_Age_Segmentation(void)
{
	lcd_age_en();
	switch ( eAgemode_num )
	{
		case LittleAge:
			lcd_little_age_en();
			break;
		case MidAge:
            lcd_mid_age_en();
            break;
		case BigAge:
			lcd_big_age_en();
			break;
	}
}

/**************************************************************************
函数名称：	void Clr_ModeSign(void)
函数功能：	清除额温、耳温、物温标志
输入参数：	无
输出参数：	LCD16
返回值  ：	无
占用空间：	TBD
备    注：	生产检验模式、黑体模式使用
**************************************************************************/
void Clr_ModeSign(void)
{
	lcd_ear_clr();		//清除耳温标志
	lcd_obj_clr();		//清除物温标志
}

/**************************************************************************
函数名称：	Disp_LowBat()
函数功能：	仅显示低电压符号
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_LowBat(void)
{
	lcd_bat_en();	//显示低电压符号；
}

/**************************************************************************
函数名称：	Disp_OFF()
函数功能：	显示Off
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_OFF(void)
{
	Clr_Disp();
	lcd9 = DispTable[0]>>8;
    lcd8 = DispTable[0];
    lcd7 = S_F>>8;
    lcd6 = S_F;
    lcd5 = S_F>>8;
    lcd4 = S_F;
}

/**************************************************************************
函数名称：	Disp_Lo()
函数功能：	显示Lo
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Lo(void)
{
    lcd9 = 0x00;
    lcd8 = 0x00;
    lcd7 = S_L>>8;
    lcd6 = S_L;
    lcd5 = S_o>>8;
    lcd4 = S_o;
    lcd_badface_clr();      //清笑脸
    lcd_smileface_clr();    //清哭脸
}

/**************************************************************************
函数名称：	Disp_Hi()
函数功能：	显示Hi
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Hi(void)
{
    lcd9 = 0x00;
    lcd8 = 0x00;
    lcd7 = S_H>>8;
    lcd6 = S_H;
    lcd5 = S_i>>8;
    lcd4 = S_i;
    lcd_badface_clr();      //清笑脸
    lcd_smileface_clr();    //清哭脸
}

/**************************************************************************
函数名称：	Disp_Ready()
函数功能：	显示_ _ . _
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Ready(void)
{
    lcd9 = 0x00;
    lcd8 = 0x01;
    lcd7 = 0x00;			//不清楚为何打开记忆符号 以关闭
    lcd6 = 0x01;
    lcd5 = 0x01;
    lcd4 = 0x01;
	lcd_smileface_clr();	//清哭笑脸（从测试态退出）
	lcd_badface_clr();
    lcd_mem_clr();   //清M标志（从记忆态退出）
}

/**************************************************************************
函数名称：	Disp_FourSecLoop_Init() / Disp_FourSecLoop_Step()
函数功能：	每 ~10ms 由主循环调用一次 Disp_FourSecLoop_Step()，内部按阶段推进动画。
			总时长 = g_5s_Count / 100 个周期，每周期 4 阶段 × 25 次调用 = 1 秒。
			返回 0=动画进行中，1=动画完成（5秒结束）。
输入参数：	无
输出参数：	LCD
返回值  ：	uint8 — 0：运行中 / 1：完成
占用空间：	TBD
备    注：	用于 Ready_DisEr1 状态，替代原阻塞版本，使 5 秒等待期间
			主循环仍可处理按键（SkeyProcess 等）。
**************************************************************************/
static uint8  nb_FourSecPhase = 0;   // 动画阶段 0-3
static uint8  nb_FourSecTick  = 0;   // 当前阶段内滴答计数
static uint8  nb_FourSecCycle = 0;   // 已完成周期数
static uint8  nb_FourSecTotal = 0;   // 总周期数

void Disp_FourSecLoop_Init(void)
{
	nb_FourSecPhase = 0;
	nb_FourSecTick  = 0;
	nb_FourSecCycle = 0;
	nb_FourSecTotal = g_5s_Count / 100;
	if(nb_FourSecTotal == 0) nb_FourSecTotal = 1;
}

uint8 Disp_FourSecLoop_Step(void)
{
	if(nb_FourSecCycle >= nb_FourSecTotal)
	{
		return 1;   // 动画完成
	}
		

	switch(nb_FourSecPhase)
	{
		case 0:   // 点亮 第一个 横杠 + 耳温/物温图标
			if(nb_FourSecTick == 0)
			{
				Clr_Disp888();
				lcd_ear_clr();
				lcd_obj_clr();
				lcd9 = 0x04;
				lcd8 = 0x00;
			if(eTestmode_num == Insptectmode || eTestmode_num == Blackbodymode)
			{
				lcd_obj_en();
			}
			else if(eTestmode_num == Earmode)
			{
				lcd_ear_en();
			}
			}
			if(++nb_FourSecTick >= 25)   // 250ms / 10ms = 25 次
			{
				nb_FourSecTick = 0;
				nb_FourSecPhase = 1;
			}
			break;

		case 1:   // 点亮 第二个 横杠，清除图标
			if(nb_FourSecTick == 0)
			{
				lcd7 = 0x04;
				lcd6 = 0x00;
			if(eTestmode_num == Insptectmode || eTestmode_num == Blackbodymode)
			{
				lcd_obj_clr();
			}
			else if(eTestmode_num == Earmode)
			{
				lcd_ear_clr();
			}
			}
			if(++nb_FourSecTick >= 25)
			{
				nb_FourSecTick = 0;
				nb_FourSecPhase = 2;
			}
			break;

		case 2:   // 点亮 第三个 横杠
			if(nb_FourSecTick == 0)
			{
				lcd5 = 0x04;
				lcd4 = 0x00;
			}
			if(++nb_FourSecTick >= 25)
			{
				nb_FourSecTick = 0;
				nb_FourSecPhase = 3;
			}
			break;

		case 3:   // 清屏
			if(nb_FourSecTick == 0)
			{
				Clr_Disp888();
				lcd_ear_clr();
				lcd_obj_clr();
			}
			if(++nb_FourSecTick >= 25)
			{
				nb_FourSecTick = 0;
				nb_FourSecPhase = 0;
				nb_FourSecCycle++;
			}
			break;
	}

	return (nb_FourSecCycle >= nb_FourSecTotal) ? 1 : 0;
}

/**************************************************************************
函数名称：	Disp_Null()
函数功能：	记忆模式显示- - -
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Null(void)
{
    Clr_Disp888();
	lcd_pc_clr();
    lcd9 = 0x04;
    lcd8 = 0x00;
    lcd7 = 0x04;
    lcd6 = 0x00;
    lcd5 = 0x04;
    lcd4 = 0x00;
    lcd_mem_en();  //显示M标志
    lcd_age_clr(); //
}

/**************************************************************************
函数名称：	Disp_ErN()
函数功能：	显示错误代码ErN，n可以为0-9的值
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_ErN(uint8 num)
{
	// if( eTestmode_num == Insptectmode )
	// {
	// 	lcd1 &= 0x01;
	// 	lcd0 = 0;				
	// }	
	lcd9 = S_E>>8;
    lcd8 = S_E;
    lcd7 = S_r>>8;
    lcd6 = S_r;
	lcd5 = (DispTable[ num ] >> 8);
    lcd4 = DispTable[ num ];
}

/**************************************************************************
函数名称：	void Disp_ErrMsg(uint8 ErrNum)
函数功能：	显示错误菜单
输入参数：	uErrFlag.g_ErrFlag
输出参数：	LCD
返回值  ：	无
占用空间：	28 words
备    注：	错误存在优先级（Er2>Lo/Hi>Er3/Er4），优先级可以在本程序定义，也可以有根据错误发生的先后决定
**************************************************************************/
void Disp_ErrMsg(void)
{
	lcd_badface_clr();
	lcd_smileface_clr();
	switch (uErrFlag.g_ErrFlag)
	{
		case 1:
			Disp_ErN(1);
			break;
		case 2:
			Disp_ErN(2);
			break;
		case 4:
			Disp_ErN(3);
			break;
		case 8:
			Disp_ErN(4);
			break;
		case 0x10:
			Disp_ErN(5);
			break;
		case 0x20:
			Disp_ErN(6);
			break;
		case 0x40:
			Disp_Lo();
			break;
		case 0x80:
			Disp_Hi();
			break;
		default:
			break;
	}
}

/**************************************************************************
函数名称：	Disp_CAL()
函数功能：	黑体模式显示CAL
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_CAL(void)
{
	Clr_Disp();
	lcd9 = S_C>>8;
    lcd8 = S_C;
    lcd7 = S_A>>8;
    lcd6 = S_A;
    lcd5 = S_L>>8;
    lcd4 = S_L;
}

/**************************************************************************
函数名称：	Disp_Ab()
函数功能：	调试模式显示Ab
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Ab(void)
{
	Clr_Disp();
	lcd9 = 0x00;
    lcd8 = 0x00;
    lcd7 = S_A>>8;
    lcd6 = S_A;
    lcd5 = S_b>>8;
    lcd4 = S_b;
}

/**************************************************************************
函数名称：	Disp_PAS()
函数功能：	调试模式显示PAS
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_PAS(void)
{
	Clr_Disp();
	lcd9 = S_P>>8;
    lcd8 = S_P;
    lcd7 = S_A>>8;
    lcd6 = S_A;
    lcd5 = S_S>>8;
    lcd4 = S_S;
}

/**************************************************************************
函数名称：	Disp_Err()
函数功能：	调试模式显示Err
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Err(void)
{
	lcd9 = S_E>>8;
    lcd8 = S_E;
    lcd7 = S_r>>8;
    lcd6 = S_r;
    lcd5 = S_r>>8;
    lcd4 = S_r;
}

/**************************************************************************
函数名称：	Disp_Debug1()
函数功能：	绑定检测模式显示测试画面1
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Debug1(void)
{
	lcd0 = 0x0A;
    lcd1 = 0x07;
    lcd2 = 0x0A;
    lcd3 = 0x06;
    lcd4 = 0x0A;
    lcd5 = 0x07;
    lcd6 = 0x0A;
    lcd7 = 0x06;
    lcd8 = 0x05;
    lcd9 = 0x03;
    lcd10 = 0x0A;
    lcd11 = 0x06;
    lcd12 = 0x0A;
    lcd13 = 0x06;
    lcd14 = 0x0A;

}

/**************************************************************************
函数名称：	Disp_Debug2()
函数功能：	绑定检测模式显示测试画面2
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Debug2(void)
{
	lcd0 = 0x05;
    lcd1 = 0x08;
    lcd2 = 0x05;
    lcd3 = 0x09;
    lcd4 = 0x05;
    lcd5 = 0x08;
    lcd6 = 0x05;
    lcd7 = 0x09;
    lcd8 = 0x0A;
    lcd9 = 0x0C;
    lcd10 = 0x05;
    lcd11 = 0x09;
    lcd12 = 0x05;
    lcd13 = 0x09;
    lcd14 = 0x05;

}

/**************************************************************************
函数名称：	Disp_Table1()
函数功能：	显示Table1
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Table1(void)
{
	Clr_Disp();
    lcd7 = S_t>>8;
    lcd6 = S_t;
    lcd5 = DispTable[1]>>8;
    lcd4 = DispTable[1];
}

/**************************************************************************
函数名称：	Disp_Table2()
函数功能：	显示Table2
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Table2(void)
{
	Clr_Disp();
    lcd7 = S_t>>8;
    lcd6 = S_t;
    lcd5 = DispTable[2]>>8;
    lcd4 = DispTable[2];
}

/**************************************************************************
函数名称：	LCD_pc_Show()
函数功能：	显示耳杯的动画
输入参数：	Earcap 0：移除耳套特效 1：插入耳套特效 2:清零计数器
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void LCD_pc_Show(uint8 Earcap)
{
	static uint8 L_Mode=0; 
	if(!Earcap)
	{
        if(uErrFlag.bits.Er6)
        {
            lcd1 &= 0x01;

            if( L_Mode<16 )
                lcd1 |= 0x02;
            else if( L_Mode<32 )
                lcd1 |= 0x04;
            else if( L_Mode<48 )
                lcd1 |= 0x08;

            L_Mode++;
            L_Mode %= 48;
        }
	}
    else if(1 == Earcap)
    {
        lcd1 = 0x01;

        if( L_Mode<48 )
            lcd1 |= 0x08;
        else if( L_Mode<96 )
            lcd1 |= 0x04;
        else if( L_Mode<144 )
            lcd1 |= 0x02;

        L_Mode++;
        L_Mode %= 144;
    }
    else if(2 == Earcap)
    {
        L_Mode = 0;
    }
}

/**************************************************************************
函数名称：	Disp_CAP(void)
函数功能：	显示耳杯的动画
输入参数：	Earcap 0：移除耳套特效 1：插入耳套特效 2:清零计数器
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_CAP(void)
{
	Clr_Disp888();
	Disp_ModeSign();
	lcd_smileface_clr();	//清哭笑脸（从测试态退出）
	lcd_badface_clr();
    lcd_mem_clr();   //清M标志（从记忆态退出）
	lcd9 = S_C>>8;
    lcd8 = S_C;
    lcd7 = S_A>>8;
    lcd6 = S_A;
    lcd5 = S_P>>8;
    lcd4 = S_P;
}


/**************************************************************************
函数名称：	void Disp_Ear(uint8 L_buf)
函数功能：	显示耳套系数温度
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Ear(uint8 L_buf)
{
	uint8 R_LCD1,R_LCD2;
	//取十位
	R_LCD1 = L_buf /10;
	//取个位
	R_LCD2 = L_buf % 10;

	lcd7 = S_E >> 8;
    lcd6 = S_E;
	lcd5 = S_r >> 8;
    lcd4 = S_r;
	lcd3 = DispTable[ R_LCD1 ] >> 8;
    lcd2 = DispTable[ R_LCD1 ];
	lcd1 = DispTable[ R_LCD2 ] >> 8;
	lcd0 = DispTable[ R_LCD2 ];
}


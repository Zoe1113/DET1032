/**************************************************************************
文件名称：	App_Memory.c
说    明：	记忆程序集合（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"

//记忆显示的几种方法：
//1、有几组只显示到几组（完全不显示--- 或 如果全空，则显示序号1和---，两种二选一）
//2、10组全部显示，无记忆时都显示---；（该方法只有A/D/H在用）
//3、有几组只显示到几组，但必须显示后一个为空的记忆序号；意义不大，和用户说明什么呢？

//只在本文件内使用的宏定义


//变量定义
volatile bit F_MemNull;		//记忆为空标志位，1：空，0：非空
volatile bit F_Mem_FirstEnter;	//首次进入记忆模式标志位
static bit F_MemNo_Disp;	//记忆序号更新标志位
static bit F_Mem_Disp;		//记忆值更新标志位

static volatile uint8 g_MemNo;			//记忆地址记录号
static volatile uint8 g_MemTotalNo;		//记忆总记录号
static volatile uint8 g_MemCount;		//记忆数循环计数器

uint8 g_HourMem;		//用于表示记忆态下显示测量时间小时寄存器
uint8 g_MinuteMem;		//用于表示记忆态下显示测量时间分钟寄存器
uint8 g_DayMem;			//用于表示记忆态下显示测量时间日期寄存器
uint8 g_MonthMem;		//用于表示记忆态下显示测量时间月份寄存器
uint16 g_YearMem;		//用于表示记忆态下显示测量时间年份寄存器
uint8 g_MonthMem_momeory;  //记忆更新下的模式记忆

/**************************************************************************
函数名称：	App_Memory()
函数功能：	记忆处理子程序
输入参数：	无
输出参数：	记忆序号、记忆值
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void App_Memory(void)
{
	int16 L_Temp;

    // App_PCKeyProcess();
    App_TKeyProcess();
    //App_SKeyProcess();
	//首次进入吗？
	if(F_Mem_FirstEnter==0)
	{
		F_Mem_FirstEnter = 1;	//置已进入标志位
		F_MemNo_Disp = 0;		//
		Mem_Init();			//初始化记忆参数

		//首次进入记忆模式，直接显示第一组数据（序号+温度值）
		if(!F_MemNull)
		{
			//显示记忆序号（第一组：M 1）
			lcd_obj_clr();										//清除物温标志
			lcd_ear_clr();									//清除耳温标志
			Disp_MemNo();
			LED_CloseAll();
			#if Func_3color
				LED_Green_En();
			#endif
			
			g_3s_Count = CountDown_3s;	//开启背光3s倒计时

			//显示记忆值（第一组温度）
			L_Temp = Disp_Mem();
			if( L_Temp )
			{
				if( g_MonthMem_momeory & 0x20 )                  //耳温
					Fever_alarm(L_Temp);
				else
				{
					LED_CloseAll();
					#if Func_3color
						LED_Green_En();
					#endif
					
				}
				g_3s_Count = CountDown_3s;		//开启背光3s倒计时
			}
		}
		else
		{
			//记忆为空，显示---
			Disp_MemNo();
		}
		//设置标志位，等效于完成一次"按下→抬起"循环
		F_MemNo_Disp = 0;	//允许下次按键按下时刷新序号
		F_Mem_Disp = 1;		//防止残留的MemKeyRelease重复触发显示
	}

	//如果按下，且未更新过记忆序号，则更新记忆序号
	if ((uKeyPress.bits.MemKeyPress && !F_MemNo_Disp&&!uKeyHold.bits.MemKeyHold ))
	{
        Auto_TurnOff_Time_Sel();	//按下关机时间清0
		lcd_obj_clr();										//清除物温标志
		lcd_ear_clr();									//清除耳温标志			
		F_MemNo_Disp = 1;		//清记忆值刷新标志位
		F_Mem_Disp = 0;
		Disp_MemNo();
		LED_CloseAll();
		#if Func_White
			LED_White_En();
		#elif Func_3color
			LED_Green_En();		//开启绿光
		#endif
		g_3s_Count = CountDown_3s;	//开启背光3s倒计时
	}

	//如果抬起，且未更新过记忆值，则更新记忆值
	else if ( uKeyRelease.bits.MemKeyRelease && !F_Mem_Disp &&!uKeyHold.bits.MemKeyHold)
	{
		uKeyRelease.bits.MemKeyRelease = 0;
		F_MemNo_Disp = 0;		//清记忆序号显示刷新标志位
		F_Mem_Disp = 1;
		L_Temp = Disp_Mem();
		if( L_Temp )
		{
			if( g_MonthMem_momeory & 0x20 )                  //耳温
				Fever_alarm(L_Temp);
			else
			{
				LED_CloseAll();
				#if Func_White
					LED_White_En();
				#elif Func_3color
					LED_Green_En();		//开启绿光
				#endif

			}
			g_3s_Count = CountDown_3s;		//开启背光3s倒计时
		}
	}
	else if(uKeyHold.bits.MemKeyHold)
	{
		//长按3s进入关机模式，并清除必要设定
        sMemKey.g_Key_Hold_cnt = 0;	//按键计时清0保证再次长按3s
        uKeyHold.bits.MemKeyHold = 0;
        eReadyTask_Sta = Ready_Init;
        eMain_Task = Task_Sleepmode;
	}
}

/**************************************************************************
函数名称：	Mem_Init()
函数功能：	记忆初始化子程序
输入参数：	无
输出参数：	F_MemNull（是否为空）、g_MemCount（记忆序号）
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
static void Mem_Init(void)
{
	//读取总记录号和记忆号
	Read_Mem_RecordNo();

	//有几组显示几组的方法，当完全为空时，只显示第1组---
	g_MemCount = 0;		//清记忆循环计数器
	F_MemNull = 1;		//默认记忆为空
	if( g_MemTotalNo )
	{
		F_MemNull = 0;		//R_MemTotalNo非0表示不为空，有记忆
		g_MemCount = g_MemTotalNo;
	}
}

/**************************************************************************
函数名称：	Disp_MemNo()
函数功能：	显示记忆序号（有几组记忆显示几组的写法）
输入参数：	g_MemCount
输出参数：	LCD记忆序号
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
static void Disp_MemNo(void)
{
	uint8 i,j;

	//有几组显示几组的方法，当完全为空时，就只显示---
	if(F_MemNull)
	{
		Disp_Null();
	}
	else
	{
		if( g_MemCount == 0 )
		{
			g_MemCount = g_MemTotalNo;
		}
		g_MemCount--;
		i = g_MemTotalNo - g_MemCount;

		Clr_Disp();
		#if Func_Ble
		if( !Port_Ble_Link && (eTestmode_num == Earmode || eTestmode_num == Objectmode))
			lcd_ble_en();//防止连接上之后进入记忆模式蓝牙符号丢失
		#endif

		#if Second_LVD == 1
		/*防止二级电压闪烁*/ 
		/*如果满电则显示满电符号*/ 
		if(uStaFlag.bits.MidBat!=1 && uStaFlag.bits.LowBat!=1)
        {
            lcd_bat_full_en();
            lcd_bat_en();
        }
		/*如果电压高于2.6v，但是低于2.7v*/
		else if(uStaFlag.bits.LowBat != 1 && uStaFlag.bits.MidBat == 1)
		{
			lcd_bat_en();
			lcd_bat_lack_en();
		}
		#endif

		if( uSetFlag.bits.VoiceEnable ){
			lcd_Voice_en();
		} 	
		else{
			lcd_Voice_clr();
		}	
		j = i/10;
		if( j )
		{
			lcd7 = DispTable[ j ] >> 8;
			lcd6 = DispTable[ j ];
		}
		j = i%10;

		lcd5 = ((DispTable[ j ] >> 8) );
		lcd4 = DispTable[ j ];
		lcd_mem_en();	//M点亮
		//Disp_ModeSign();
	}
}

/**************************************************************************
函数名称：	static int16 Disp_Mem(void)
函数功能：	显示记忆值
输入参数：	Mem_EarAdd、Mem_ForeAdd、Mem_ObjAdd
输出参数：	LCD记忆值
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
static int16 Disp_Mem(void)
{
	int16 L_Temp = 0;
	uint8 L_MemAdd;

	if(!F_MemNull)
	{
		//读取记忆地址
		L_MemAdd = Read_Mem_Address(g_MemNo);

		//获取温度
		I2C_masterInit();
		L_Temp = I2C_Random_R(L_MemAdd);
		L_Temp = L_Temp << 8;
		L_MemAdd++;
		L_Temp |= I2C_Random_R(L_MemAdd);
		//获取时
		L_MemAdd++;
		g_HourMem = I2C_Random_R(L_MemAdd);
		//获取分
		L_MemAdd++;
		g_MinuteMem = I2C_Random_R(L_MemAdd);
		//获取月
		L_MemAdd++;
		g_MonthMem = I2C_Random_R(L_MemAdd);
		g_MonthMem_momeory = g_MonthMem;
		if( g_MonthMem & 0x20 )        //耳温   
		{
			lcd_obj_clr();		//清除物温标志
			lcd_ear_en();	//显示耳温标志
		}
		else                                      //物体温度标志
		{
			lcd_ear_clr();	//清除耳温标志
			lcd_obj_en();       //显示物体温度			
		}
		g_MonthMem &= 0x0F;	
		//获取日
		L_MemAdd++;
		g_DayMem=I2C_Random_R(L_MemAdd);
		eAgemode_num =  (g_DayMem & 0xC0)>>6;
		g_DayMem &= 0x3F;
		I2C_Disable();

		//显示记忆值
		g_50ms_Count = DispTime_Init;	//循环显示时间、日期、ntc计时器赋初值
		L_Temp = CToF(L_Temp);
		if( eTestmode_num == Insptectmode )
			Disp_Temp(1,1,uSetFlag.bits.Unit,L_Temp);	//显示温度
		else
		{
			L_Temp = Temp_Resolution_Adjust(L_Temp);
			Disp_Temp(1,0,uSetFlag.bits.Unit,L_Temp);	//显示温度
		}
		lcd_mem_en();	//M点亮
        Disp_Unit();
        if(g_MonthMem_momeory & 0x20)   //耳温
        {
            Disp_Age_Segmentation();
        }
        else
        {
            lcd_age_clr();
        }
		//Disp_ModeSign();	//显示模式符号

		//计算下个查询地址
		g_MemNo --;
		if( g_MemNo == 0 )
		{
			//如果总记录数小于10时，说明总记录数和记录号相等，否则最大为10
			if( g_MemTotalNo < MemMaxArray )
			{
				g_MemNo	= g_MemTotalNo;
			}
			else
			{
				g_MemNo = MemMaxArray;
			}
		}		
	}
	return L_Temp;
}

/**************************************************************************
函数名称：	Mem_Store()
函数功能：	测量值存储
输入参数：	L_Temp、g_MemTotalNo、g_MemNo
输出参数：	EEPROM
返回值  ：	无
占用空间：	TBD
备    注：	存储时从前往后
**************************************************************************/
void Mem_Store(int16 L_Temp)
{
	uint8 L_MemAdd;
	uint8 L_tempbuf;
	Mem_Init();
	if ( g_MemTotalNo < MemMaxArray )
	{
		g_MemTotalNo ++;
		g_MemNo	++;
	}
	else
	{
		g_MemNo ++;
		if ( g_MemNo == (g_MemTotalNo + 1) )
		{
			g_MemNo = 0x01;
		}
	}

	L_MemAdd = I2C_Add_EarMem;                  //读取地址选择为耳温的总记录号
	I2C_masterInit();
	Delay1ms(10);

	//存储记录总数和记录号
	I2C_Byte_W(L_MemAdd, g_MemTotalNo);
	Delay1ms(5);

	L_MemAdd = L_MemAdd + I2C_Add_Offset;
	I2C_Byte_W(L_MemAdd, g_MemNo);
	Delay1ms(5);

	//读取记忆地址
	L_MemAdd = Read_Mem_Address(g_MemNo);

	//存储温度
	I2C_Byte_W(L_MemAdd, L_Temp >> 8);
	Delay1ms(5);
	L_MemAdd ++;
	I2C_Byte_W(L_MemAdd, L_Temp);
	Delay1ms(5);
	//存储时
	L_MemAdd++;
	I2C_Byte_W(L_MemAdd, g_Hour);
	Delay1ms(5);
	//存储分
	L_MemAdd++;
	I2C_Byte_W(L_MemAdd, g_Minute);
	Delay1ms(5);
	//存储月
	L_MemAdd++;
	switch (eTestmode_num)
	{
		case Earmode:
			L_tempbuf = g_Month | 0x20;
			break;
		case Objectmode:
			L_tempbuf = g_Month;
			break;
		//生产、黑体模式均放obj存储区域
		default:
			L_tempbuf = g_Month;
			break;
	}
	I2C_Byte_W(L_MemAdd, L_tempbuf);
	Delay1ms(5);

	//存储日
	L_MemAdd++;
	// I2C_Byte_W(L_MemAdd, g_Day);
    //I2C_Byte_W(L_MemAdd, eAgemode_num);
	//日期数值最大是31占5位，最高的2位存年龄分组
	I2C_Byte_W(L_MemAdd, g_Day + (eAgemode_num << 6));
	Delay1ms(5);
	//存储年(存放格式为g_Year-2000)
	L_MemAdd++;
	I2C_Byte_W(L_MemAdd, g_Year-2000);
	Delay1ms(5);
	I2C_Disable();
}

/**************************************************************************
函数名称：	void Read_Mem_RecordNo(void)
函数功能：	读取记忆记录号
输入参数：	无
输出参数：	g_MemTotalNo、g_MemNo
返回值  ：	无
占用空间：	TBD
备    注：
**************************************************************************/
void Read_Mem_RecordNo(void)
{
	uint8 L_MemAdd;

	L_MemAdd = I2C_Add_EarMem;                   //此记忆缓存的程序将耳闻的地址作为总记录号
	I2C_masterInit();
	g_MemTotalNo = I2C_Random_R(L_MemAdd);		//读取总记录号
	g_MemNo = I2C_Random_R(L_MemAdd + I2C_Add_Offset);		//读取记忆号
	I2C_Disable();
}

/**************************************************************************
函数名称：	uint8 Read_Mem_Address(uint8 L_MemNo)
函数功能：	读取记忆地址
输入参数：	L_MemNo（记忆序号）
输出参数：	L_MemAdd（记忆地址）
返回值  ：	无
占用空间：	TBD
备    注：
**************************************************************************/
uint8 Read_Mem_Address(uint8 L_MemNo)
{
	uint8 L_MemAdd;

	//计算地址
	L_MemAdd = (0x20+(L_MemNo - 1) * 7);
	return L_MemAdd;
}

/**************************************************************************
函数名称：	void Clr_All_Memory(void)
函数功能：	清除所有记忆
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：
**************************************************************************/
void Clr_All_Memory(void)
{
	I2C_masterInit();                       //使能IIC功能
	I2C_Byte_W(I2C_Add_EarMem, 0x00);		//存记忆总记录数
	Delay1ms(5);

	I2C_Byte_W(I2C_Add_EarMem + I2C_Add_Offset, 0x00);	//存记忆记录号
	Delay1ms(5);

	I2C_Disable();                          //关闭IIC功能
}

/**************************************************************************
文件名称：	Drv_GPIO.c
说    明：	GPIO初始化函数集合（驱动层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"

/**************************************************************************
函数名称：	Cal_Inspect_Detect()
函数功能：	校准模式、绑定模式入口检测
输入参数：	Port_Debug、Port_Cal
输出参数：	uSetFlag
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Cal_Inspect_Detect(void)
{
	//绑定检测模式、校温检测模式、系数调整模式判断
	if( !Port_Debug && Port_Cal )
	{
		if ( eTestmode_num == Insptectmode )
		{
			 #if ParamModif
			LED_CloseAll();
			eMain_Task = Task_ParamModifymode;	//生产模式短路debug口则进入系数调整模式
            #endif
        }
		else
		{
			eSetTask = Set_Unit;		//设置态恢复状态
			F_FirstEnter_SetMode = 0;
			eMain_Task = Task_BondTestmode;	//用户模式下短路debug进入绑定检测模式
		}
	}

	//设置态决不允许进入校温，汇编和C的程序结构不一样，未初始化adc
	if( !Port_Cal && !Port_Debug && eMain_Task == Task_ReadyMode )
	{
        #if Func_Ble
            F_Ble_En = Disable;         //进入校准关闭蓝牙
            Port_Ble_En = 0;            //蓝牙失能
        #endif
		eMain_Task = Task_Calimode;		//用户模式下短路debug和cal进入校温模式
	}
}

/**************************************************************************
函数名称：	CF_Check()
函数功能：	单位状态、单位可切换状态查询
输入参数：	Port_CF、Port_Change_CF
输出参数：	uSetFlag
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void CF_Check(void)
{
	//单位状态检测
	uSetFlag.bits.Unit = Unit_C;
	if( !Port_CF )
	{
		uSetFlag.bits.Unit = Unit_F;
	}

}

/**************************************************************************
函数名称：	GPIO_Init()
函数功能：	IO口初始化设置
输入参数：	P0、P1、P2、P3、P4、P5
输出参数：	P0、P1、P2、P3、P4、P5
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void GPIO_Init(void)
{
	//SDA/SCL输出为0(因为采用的是硬体I2C),按键输入上拉高电平,XIN/XOUT输入不上拉0
	//0: input mode, 1: output mode
	P0M = 0x0C;			//0b 0000 1100
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P0UR = 0x13;		//0b 0001 0011
	//0: low level, 1: high level
	P0 = 0x1F;			//0b 0001 1111
 
	//白灯输出不上拉高电平,MOSI输出不上拉高电平,MISO输入不上拉高电平,SCK输出不上拉高电平,TX输入不上拉高电平（防止向BLE供电）,RX输入不上拉,VOC_EN,BLE_LINK输入不上拉1
	//0: input mode, 1: output mode
	P1M = 0x4F;			//0b0100 1111
	//0: LCD functon pin, 1: IO pin
	P1SEG = 0xFF;		//均为普通IO口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P1UR = 0xC0;		//0b1100 0000
	//0: low level, 1: high lDevel
	P1 = 0xB9;			//0b1011 1001

	//P25-P27为seg口，LINK脚输入 不上拉 高,EN输出 上拉 低，BUSY输入不上拉高，G320_EN输出不上拉高，VOC_EN输出不上拉低
	//0: input mode, 1: output mode
	P2M = 0x0B;			//0b0000 1011
	//0: LCD functon pin, 1: IO pin
	P2SEG = 0x1F;		//0b0001 1111,P25-P27均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P2UR = 0x08;		//0b0000 1000
	//0: low level, 1: high level
	P2 = 0x16;			//0b0001 0110

	//C/、CF、CAL、Debug输入上拉高
	//0: input mode, 1: output mode
	P5M = 0xE0;			//0b1110 0000
	//0: LCD functon pin, 1: IO pin
	P5SEG = 0xFF;		//0b1111 1111
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P5UR = 0x1E;		//0b0001 1110
	//0: low level, 1: high level
	P5 = 0xFE;			//0b1111 1110

	//0: input mode, 1: output mode
	P3M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P3SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P3UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P3 = 0x00;			//均为LCD口

	//0: input mode, 1: output mode
	P4M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P4SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P4UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P4 = 0x00;			//均为LCD口
}


/**************************************************************************
函数名称：	GPIO_PowerDown()
函数功能：	IO口初始化设置
输入参数：	P0、P1、P2、P3、P4、P5
输出参数：	P0、P1、P2、P3、P4、P5
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void GPIO_PowerDown( void )
{
	//按键输入上拉高电平,XIN/XOUT输入不上拉0,E2供电口输出上拉高
	//0: input mode, 1: output mode
	P0M = 0x2C;			//0b0010 1100
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P0UR = 0x13;		//0b0001 0011
	//0: low level, 1: high level
	P0 = 0x1F;			//0b0001 1111

	//BZ不上拉输出0,TX输出不上拉低电平,RX为I2C供电口不上拉输出0,其余未使用输入上拉高

#if !Func_Ble
	//白灯输入上拉高，按键输入上拉高电平,BZ/MOSI/MISO/TX/RX/SCK均输出不上拉低，
	//0: input mode, 1: output mode
	P1M = 0xFE;			//0b 1111 1110
	//0: LCD functon pin, 1: IO pin
	P1SEG = 0xFF;		//均为普通IO口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P1UR = 0x01;		//0b 0100 0001
	//0: low level, 1: high level
	P1 = 0x01;			//0b 0100 0001

	//P25-P27为seg口，BLE_Link、BLE_EN、BLE_BUSY、G320_EN输出不上拉低，VOC_EN输出上拉高
	//0: input mode, 1: output mode
	P2M = 0x03;			//0b 0000 0011
	//0: LCD functon pin, 1: IO pin
	P2SEG = 0x1F;		//0b 0001 1111,P25-P27均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P2UR = 0x1D;		//0b 0001 1101
	//0: low level, 1: high level
	P2 = 0x1D;			//0b 0001 1101
#else
	//按键输出不上拉低,，SCK/RX1/TX1/MISO/MOSI/BZ输出不上拉低,白灯输入上拉高，
	//0: input mode, 1: output mode
	P1M = 0xFE;			//0b 1111 1110
	//0: LCD functon pin, 1: IO pin
	P1SEG = 0xFF;		//均为普通IO口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P1UR = 0x01;		//0b 0000 0001
	//0: low level, 1: high level
	P1 = 0x01;			//0b 0000 0001

	//P25-P27为seg口，BLE_Link、BLE_EN、BLE_BUSY、G320_EN输出不上拉低，VOC_EN输出上拉高
	//0: input mode, 1: output mode
	P2M = 0x1F;			//0b 0001 1111
	//0: LCD functon pin, 1: IO pin
	P2SEG = 0x1F;		//0b 0001 1111,P25-P27均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P2UR = 0x01;		//0b 0000 0001
	//0: low level, 1: high level
	P2 = 0x01;			//0b 0000 0001
#endif

	//RGB输入上拉高，CAL/Debug输入上拉高电平,C/F、CF输出不上拉0，MOTOR输出不上拉低
    //0: input mode, 1: output mode
    P5M = 0x07;			//0b0000 0111
    //0: LCD functon pin, 1: IO pin
    P5SEG = 0xFF;		//0b1111 1111
    //0: disable pullup, 1: enable pullup, pull resistor = 200k
    P5UR = 0xF8;		//0b1111 1000
    //0: low level, 1: high level
    P5 = 0xF8;			//0b1111 1000

	//0: input mode, 1: output mode
	P3M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P3SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P3UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P3 = 0x00;			//均为LCD口

	//0: input mode, 1: output mode
	P4M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P4SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P4UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P4 = 0x00;			//均为LCD口
}

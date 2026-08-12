/**************************************************************************
文件名称：	Drv_LVD.c
说    明：	低电压处理函数集合（驱动层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"

/**************************************************************************
函数名称：	void LVD_Init(void)
函数功能：	low voltage初始化设置
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void LVD_Init(void)
{
	//internal lvd
	LBTM = 0x10 ;		//低电压<2.6V，内部电压检测
	Delay50us(1) ;
	FLBTEN = 1 ;		//enable low-voltage detect
}

/**************************************************************************
函数名称：	void LVD_Chk(void)
函数功能：	low voltage检测
输入参数：	无
输出参数：	uStaFlag.bits.LowBat
返回值  ：	无
占用空间：	TBD
备    注：	100ms（Fcup=2M）
**************************************************************************/
void LVD_Chk(void)
{
	uint8 i ;		//count with low-voltage

	for( i=0; i<10; i++ )	//若低电压连续有十次，则置低电压标志F_Lo为1，清低电压计数
	{
		Delay10ms(1) ;		//延迟10ms稳定
		if( FLBTO )			//FLBTO 1:低电压 0：正常电压
		{
			uStaFlag.bits.LowBat = 1 ;
		}
		else
		{
			uStaFlag.bits.LowBat = 0 ;
			break;
		}
	}
}


/**************************************************************************
函数名称：	void LBT_Chk(void)
函数功能：	二级电压检测
输入参数：	无
输出参数：	uStaFlag.bits.LowBat,uStaFlag.bits.MidBat
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void LBT_Chk(void)
{
	uint8 i=0 ;				

	/*先用2.7V的比较电压检测*/
	LBTM = 0x14 ;		
	Delay50us(1) ;
	//enable low-voltage detect	
	FLBTEN = 1 ;		

	/*若低电压连续有十次，则置低电压标志为1，清低电压计数*/
	for( i=0; i<10; i++ )	
	{
		/*延迟10ms稳定*/
		Delay10ms(1) ;		
		/* FLBTO 1:低电压 0：正常电压 */
		if( FLBTO )			
		{
			/*二级电压第一级标志位打开*/
			uStaFlag.bits.MidBat = 1;
		}
		/*如果有一次未检测到*/
		else
		{
			/*清除标志位并返回*/
			uStaFlag.bits.MidBat = 0;		
			uStaFlag.bits.LowBat = 0 ;
			return;		
		}
	}

	/*如果连续十次小于2.7v，判断有没有低于2.6v*/
	/*选择2.6v作为比较电压*/
	LBTM = 0x10 ;		
	Delay50us(1) ;
	//enable low-voltage detect	
	FLBTEN = 1 ;		

	for( i=0; i<10; i++ )	
	{
		/*延迟10ms稳定*/
		Delay10ms(1) ;	
		/*FLBTO 1:低电压 0：正常电压*/
		if( FLBTO )			
		{
			/*二级电压第二级标志位打开*/
			uStaFlag.bits.LowBat = 1 ;	
		}
		else
		{
			/*如果有一次大于2.6V*/
			uStaFlag.bits.LowBat = 0 ;
			return;		
		}
	}

	/*如果二级电压第二级标志位打开*/
	if(uStaFlag.bits.LowBat == 1)
	{
		/*二级电压第一级标志位关闭*/
		uStaFlag.bits.MidBat = 0;
	}
	return;
}
/*************************************************************************/

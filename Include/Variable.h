#ifndef _Variable_H
#define _Variable_H

/***********************************************/
//Warning: 更改传感器可能需要同步修改放大倍数
//Warning: 对于额温计STP9CF55、J11、ZXH、10DF55已大货验证，11DF55未验证，不可选择
//Warning: 对于耳温计STP9CF55、J11已验证，其余均未验证，不可以选择
#define Sensor 0	//用于传感器预编译选择

//传感器列表选项
#define STP9CF55 0	//放大倍数64倍
#define J11	1		//放大倍数64倍
#define ZXH	2		//放大倍数32倍
#define STP10DF55 3		//放大倍数64倍
#define STP11DF55 4		//放大倍数32倍
#define Eselida 5

//传感器放大倍数设定
#if ( (Sensor == STP9CF55) || (Sensor == J11) || (Sensor == STP10DF55) )
    #define Adc_PGA_Gain	PGA_x64
#endif

#if ( (Sensor == ZXH) || (Sensor == STP11DF55) )
    #define Adc_PGA_Gain	PGA_x32
#endif
/***********************************************/
#define Func_debug      0 	//1：开启debug测试，0：未开启debug测试
#define Have_Voice_Func 0 	//H款无语音
//请仔细核对基础特性编码表！！！！！！
#define Ble_Name 1032	//型号名（不可超过4字节）
#define Soft_Internal_Version 10 //对内程序版本
#define Soft_External_Version 10 //对外程序版本

#define ET_FT 0 	//0:耳温枪，1:额温枪，2：耳额温             //ET_FT定义为1时，必须再加Distence_En的宏定义
#define OffsetTable2open 0   //0:关闭第二张黑体表格   1:打开第二张黑体表格
#define ParamModif         1  //0:关闭后门调参节约空间   1:打开后门调参
#if ET_FT
#define Forehead_Temple_Offset         0  //额温补偿部位为太阳穴(接触非接触) 
#define Forehead_Central_Offset         1  //额温补偿部位为额头中央   ，测量部位为太阳穴时是接触的，测量部位为额头时是非接触的 
#define Ear_Forehead_Temple_Offset         2  // 耳额温枪的额温模式补偿部位为太阳穴
#define Ear_Forehead_Central_Offset         3  // 耳额温枪的额温模式补偿部位为额头中央
#define ForeheadOffset         1   //选择补偿模式 ，如果同一程序含两个及以上补偿方式，此定义可放进每个程序code
#endif
#define CAP_CHECK               0

#define Soft_Code 684

//国外,有耳套自动侦测,四色背光,有物温;人体系数由20
#if Soft_Code == 684
    #define Nation                  1	        //1：国外 0：国内
    #define Second_LVD              1           //0:一级电压 1：二级电压
    #define Func_Ble                1			//0：关闭蓝牙 1：开启蓝牙
    #define Func_Obj	            1 	        //1：有物温功能，0：无物温功能
    #define Func_3color	            1	        //1：开启3色背光，0：无背光或单色背光
    #define Func_White				1			//1：开启白色背光，0：关闭白色背光
    #define Func_Probecover 		1 	        //1：有耳套功能，0：无耳套功能
	#define CF_Change_Enable        1           //1：单位可切换，0：单位不可切换
    #define Electricity_poweroff    1			//0:上电不自动关机，并进入设置态； 1.上电自动关机
    #define HumanRatio_Num          0           //1：棒米算法45 0：健拓常规20
#endif


//增加程序命令宏
#define RUN 0
#define RESET 1

#define NTCTOTP 0
#define TPTONTC 1

#define Enable 1
#define Disable 0

#define Unit_F 		1
#define Unit_C 		0
#define Unit_Change_En 		0   //0：单位可切换
#define Unit_Change_Dis		1   //1：单位不可切换
#define TimeFormat_24H	0 	//0：24小时
#define TimeFormat_12H	1 	//1：12小时

#define Default_Year    2023   //上电默认年份

#define uint8 	 unsigned char
#define uint16 	 unsigned long
#define uint32 	 unsigned long long
#define int8 	 char
#define int16 	 long
#define int32 	 long long

/**************************开机不可清除RAM区******************************/
extern volatile uint8 g_Hour;		//用于表示时间-小时寄存器
extern volatile uint8 g_Minute;		//用于表示时间-分钟寄存器
extern volatile uint8 g_Day;		//用于表示时间-天数寄存器
extern volatile uint8 g_Month;		//用于表示时间-月份寄存器
extern volatile uint16 g_Year;		//用于表示时间-年份寄存器
extern volatile uint8 g_Second;		//用于表示时间-秒数寄存器
extern int16 g_RelateTemp;	//用于关联
extern uint8 g_AgeNode_Men;	//用于记忆年龄分段

/****************************校准数据RAM区********************************/
extern int16 g_Cali25TP;		//校准环温相对25.0℃的TP值
extern int16 g_Cali37Data;		//37.0℃黑体校准值
extern int16 g_Cali41Data;		//41.0℃黑体校准值
extern int16 g_AdcDoff;			//ADC通道DOFF的高字节
extern int16 g_OpDoff;			//PGA Doff
extern int16 g_OpOffsetDoff;	//PGA Offset Doff
extern uint16 g_AIN1Count;		//AIN1→ACM通道ADC COUNT
extern uint16 g_AIN2Count;		//AIN2→ACM通道ADC COUNT
extern uint16 g_NtcCount;		//NTC ADC COUNT
extern int16 g_TpCount;			//TP ADC COUNT
extern uint16 g_NtcStep;		//NTC查表步长的高字节(NTC温度)
extern int16 g_TpStep;			//TP查表步长的高字节(目标温度)
//extern uint8 F_Ble_FirstEnter;

//掉电不能清除
typedef union
{
    uint8 g_SetFlag;
    struct
    {
        unsigned Unit:1;		//用于表示单位标志位，0：C，1：F
        unsigned TimeFormat:1;	//用于表示时制标志位，0: 24小时制，1：12小时制
        unsigned Unit_Change:1;	//用于表示单位可切换，0：可切换，1：不可切换
        unsigned VoiceEnable:1;	//用于表示语音开启还是关闭，0：未开启，1：开启；
        unsigned X4:1;			//用于表示
        unsigned BleEnable:1;	//用于表示BLE开启还是关闭，0：未开启，1：开启
        unsigned X6:1;			//用于表示
        unsigned X7:1;			//用于表示
    }bits;
}uSet;

extern uSet uSetFlag;

//主任务
typedef enum
{
    Task_InitMode = 0,			//开机初始化模式任务
    Task_ReadyMode,				//准备就绪模式任务
    Task_Testingmode,			//测量进行模式任务
    Task_Setmode,				//设置模式任务
    Task_Unitmode,				//单位切换模式任务
    Task_Calimode,				//校准模式任务
    Task_BondTestmode,			//绑定测试模式任务
    #if ParamModif
	Task_ParamModifymode, //系数修改模式任务
    #endif
	Task_Sleepmode,				//睡眠模式任务
    Task_Memorymode             //记忆模式
}eTask;

extern eTask eMain_Task;

//状态标志位
typedef union
{
    uint8 g_StatusFlag;
    struct
    {
        unsigned Fever:1;		//用于表示发烧标志位，0：不发烧，1：发烧；
        unsigned LowBat:1;		//用于表示低电压标志位，0：不低电压，1：低电压；
        unsigned TableNum:1;	//用于表示黑体表格序号标志位，0：表格1，1：表格2；
        unsigned Identify:1;	//用户表示EEPROM识别码，0：相等，1：不相等
        unsigned ProbeCover:1;	//用于表示耳套标志位，0：不带耳套，1：带耳套
        unsigned MidBat:1;		//一级电压警告标志，0：不低电压，1：低电压；
        unsigned Er7:1;			//用于表示Er7错误标志位(表示校准时37和41校准值均为负数异常)
        unsigned Er8:1;			//用于表示Er8错误标志位(表示校准时ADC抖动厉害不稳)
    }bits;
}uStatus;

extern uStatus uStaFlag;

//ErN错误联合体（可以用枚举替代，这样子可以直接赋值给Disp_ErN(g_ErrFlag)，节省大量程序 if( !g_ErrFlag ) Disp_ErN(g_ErrFlag)
//错误显示机制永远以新发生的错误为第一优先级，故需清除所有标志位g_ErrFlag=0；
typedef union
{
    uint8 g_ErrFlag;

    struct
    {
        unsigned Er1:1;		//用于表示Er1错误标志位(表示耳温计未准备就绪就按下测量键)
        unsigned Er2:1;		//用于表示Er2错误标志位(表示环境温度不在10-40℃内)
        unsigned Er3:1;		//用于表示Er3错误标志位(表示采集时AD不稳,也表示用户没有拿稳耳温枪)
        unsigned Er4:1;		//用于表示Er4错误标志位(表示环境温度变化过快)
        unsigned Er5:1;		//用于表示Er5错误标志位(表示存储器读取错误)
        unsigned Er6:1;		//用于表示Er6错误标志位(表示物温模式带上耳套)
        unsigned Lo:1;		//用于表示温度低于显示范围标志位，0：在显示范围内，1：低于显示范围下限；
        unsigned Hi:1;		//用于表示温度高于显示范围标志位，0：在显示范围内，1：高于显示范围上限；
    }bits;
}uErr;

extern uErr uErrFlag;

//年龄模式
typedef enum
{
	LittleAge = 0,	
	MidAge,	
	BigAge,		
}eAgemode;

extern eAgemode eAgemode_num;


#endif

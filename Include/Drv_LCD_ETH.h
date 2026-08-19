#ifndef _Drv_LCD_ETH_H
#define _Drv_LCD_ETH_H

//底层驱动  //跟段码表对应关系+5
// sfr	lcd0 = 0xf00 ;
// sfr	lcd1 = 0xf01 ;
// sfr	lcd2 = 0xf02 ;
// sfr	lcd3 = 0xf03 ;
// sfr	lcd4 = 0xf04 ;
// sfr	lcd5 = 0xf05 ;
// sfr	lcd6 = 0xf06 ;
// sfr	lcd7 = 0xf07 ;
// sfr	lcd8 = 0xf08 ;
// sfr	lcd9 = 0xf09 ;
// sfr	lcd10 = 0xf0a ;
// sfr	lcd11 = 0xf0b ;
// sfr	lcd12 = 0xf0c ;
// sfr	lcd13 = 0xf0d ;
// sfr	lcd14 = 0xf0e ;

//镜像驱动
sfr	lcd0 = 0xf0e ;
sfr	lcd1 = 0xf0d ;
sfr	lcd2 = 0xf0c ;
sfr	lcd3 = 0xf0b ;
sfr	lcd4 = 0xf0a ;
sfr	lcd5 = 0xf09 ;
sfr	lcd6 = 0xf08 ;
sfr	lcd7 = 0xf07 ;
sfr	lcd8 = 0xf06 ;
sfr	lcd9 = 0xf05 ;
sfr	lcd10 = 0xf04 ;
sfr	lcd11 = 0xf03 ;
sfr	lcd12 = 0xf02 ;
sfr	lcd13 = 0xf01 ;
sfr	lcd14 = 0xf00 ;
//只在本文件内使用的宏定义
#define	SA	8			//0000000000001000B
#define	SB	4			//0000000000000100B
#define	SC	2			//0000000000000010B
#define	SD	1			//0000000000000001B
#define	SE	512			//0000001000000000B
#define	SF	2048		//0000100000000000B
#define	SG	1024		//0000010000000000B
#define	P	256			//0000000100000000B

#define S_A (SA+SB+SC+SE+SF+SG)
#define S_C (SA+SF+SE+SD)
#define S_c (SG+SE+Sd)
#define S_d (SB+SC+SD+SE+SG)
#define S_E (SA+SF+SE+SG+SD)
#define S_F (SA+SF+SE+SG)
#define S_H (SB+SF+SE+SG+SC)
#define S_r (SE+SG)
#define S_t (SE+SG+SF+SD)
#define S_P (SE+SG+SF+SA+SB)
#define S_S (SA+SF+SG+SC+SD)
#define S_b (SE+SF+SG+SC+SD)
#define S_i (SE)
#define S_L (SE+SF+SD)
#define S_o (SE+SG+SD+SC)
#define S_U (SE+SF+SD+SC+SB)
#define S_J (SD+SC+SB)

//常用lcd符号宏定义
#define lcd_mem             0x01        //记忆符号 S4
#define lcd_bat             0x01        //电池符号 N4
#define lcd_bat_left        0x08        //满电压符号1 N1
#define lcd_bat_right       0x04        //满电压符号2 N3
#define lcd_bat_mid         0x02        //满电压符号3 N2
#define lcd_pc              0x01        //耳道探头 S8
#define lcd_earcap          0x02        //耳套 S7
#define lcd_dash            0x04        //温度的负号 3G
#define lcd_point           0x01        //温度的点的符号 P1
#define lcd_badface         0x08        //哭脸符号 S12
#define lcd_smileface       0x04        //笑脸符号 S11
#define lcd_ear             0x01        //耳朵符号 S9
#define lcd_obj             0x02        //物温符号 S10
#define lcd_Voice           0x04        //语音符号 S2
#define lcd_ble             0x08        //蓝牙符号 S1

#define lcd_age 0x08        //年龄符号边框 S17
#define lcd_little_age 0x04     //小年龄符号 W1
#define lcd_mid_age 0x02        //中年龄符号 W2
#define lcd_big_age 0x01        //大年龄符号 W3

//显示指定lcd图标
#define lcd_mem_en()        { lcd0 |= lcd_mem; }	    //M点亮
#define lcd_bat_en()        { lcd11 |= lcd_bat; }
#define lcd_bat_full_en()    { lcd11 |= lcd_bat_left|lcd_bat_right|lcd_bat_mid; }
#define lcd_bat_lack_en()    { lcd11 &= lcd_bat;lcd11 |= lcd_bat_right; }
#define lcd_pc_en()         { lcd1 |= lcd_pc; }
#define lcd_earcap_en()         { lcd1 |= lcd_earcap; }
#define lcd_point_en()      { lcd5 |= lcd_point; }
#define lcd_badface_en()    { lcd2 |= lcd_badface; }
#define lcd_smileface_en()  { lcd2 |= lcd_smileface; }
#define lcd_ear_en()        { lcd2 |= lcd_ear; }
#define lcd_obj_en()        { lcd2 |= lcd_obj; }
#define lcd_unit_c_en()     { lcd3 &= 0x00; lcd3 |= 0x0A; } //S13 S15
#define lcd_unit_f_en()     { lcd3 &= 0x00; lcd3 |= 0x06; } //S13 S14
#define lcd_Voice_en()      { lcd0 &= 0X09;lcd0 |= lcd_Voice; }
#define lcd_ble_en()        { lcd0 |= lcd_ble; }

#define lcd_age_en()        { lcd10 = lcd_age; }
#define lcd_big_age_en()        { lcd10 = lcd_age; lcd10 |= lcd_big_age; }
#define lcd_little_age_en()        { lcd10 = lcd_age; lcd10 |= lcd_little_age; }
#define lcd_mid_age_en()        { lcd10 = lcd_age; lcd10 |= lcd_mid_age; }


//清除指定lcd图标
#define lcd_mem_clr() 	    { lcd0 &= ~lcd_mem; }	//M点亮
#define lcd_bat_clr()       { lcd11 &= ~lcd_bat; }
#define lcd_bat_full_clr()    { lcd11 &= ~(lcd_bat_left|lcd_bat_right|lcd_bat_mid); }
#define lcd_pc_clr()        { lcd1 = 0x00; }
#define lcd_earcap_clr()        { lcd1 = 0x00; }
#define lcd_point_clr()     { lcd5 &= ~lcd_point; }
#define lcd_badface_clr()   { lcd2 &= ~lcd_badface; }
#define lcd_smileface_clr() { lcd2 &= ~lcd_smileface; }
#define lcd_ear_clr()       { lcd2 &= ~lcd_ear; }
#define lcd_obj_clr()       { lcd2 &= ~lcd_obj; }
#define lcd_unit_c_clr()    { lcd3 &= 0x00; }
#define lcd_unit_f_clr()    { lcd3 &= 0x00; }
#define lcd_Voice_clr()     { lcd0 |= 0X06; }
#define lcd_ble_clr()       { lcd0 &= ~lcd_ble; }
#define lcd_age_clr()        { lcd10 = 0x00;}

//消隐指定lcd图标
// #define lcd_colon_xor()     { lcd5 ^= lcd_colon; }
#define lcd_ble_xor()       { lcd0 ^= lcd_ble; }
// #define lcd_bat_xor()       { lcd17 ^= lcd_bat; }

extern uint16 __ROM	DispTable[];

// 显示函数
void Lcd_Init(void);
void Disp_Version(uint16 num);
void Disp_Code(uint16 num);
void Disp_All(void);
void Disp_Unit(void);
void Disp_LowBat(void);
void Disp_BadFace(void);
void Disp_SmileFace(void);
void Disp_ModeSign(void);
void Disp_OFF(void);
void Disp_Lo(void);
void Disp_Hi(void);
void Disp_Ready(void);
void Disp_Null(void);
void Disp_ErN(uint8 num);
void Disp_ErrMsg(void);
void Disp_CAL(void);
void Disp_Ab(void);
void Disp_PAS(void);
void Disp_Err(void);
void Disp_Debug1(void);
void Disp_Debug2(void);
// void Disp_DebugPASn(uint8 num);
void Disp_12H(void);
void Disp_24H(void);
void Disp_Table1(void);
void Disp_Table2(void);
void Disp_Temp(bit Point, bit High, bit Unit, int16 Temp);
void Disp_Year(uint16 L_buf);
void LCD_pc_Show(uint8 Earcap);
void Disp_CAP(void);
void Disp_Age_Segmentation(void);
void Cal_Disp_Temp(int16 Temp);
void Disp_Ear(uint8 L_buf);

// 清除函数
void Clr_Disp(void);
void Clr_Disp888(void);
void Clr_ModeSign(void);
void Clr_SetTime(uint8 L_Blink);

void Disp_FourSecLoop_Init(void);
uint8 Disp_FourSecLoop_Step(void);

#endif
/*************************************************************************/
#ifndef _App_Function_H
#define _App_Function_H

//????????
#define DispRange_HumanUp		0x10CC      //43.00
#define DispRange_HumanDown     0x0C80      //32.00
#define DispRange_ObjUp         0x2710      //100.00
#define DispRange_ObjDown       0x0000		//0.00

extern uint8 Hex2Bcd[3];	//hex¡Á?bcd?¨®bcd????????

void Tone_Init(void);
void Fever_alarm(int16 Temp);
void Select_Age0_alarm(int16 L_Temp);
void Select_Age3_alarm(int16 L_Temp);
void Select_Age36_alarm(int16 L_Temp);
uint8 LeapYear_Judge( uint16 L_Buf );
void Temp_Relate(void);
void Emissivity_correction(void);
void Body_MeasureRange_Check(void);
void Obj_MeasureRange_Check(void);
void NtcTableWider_Check(void);
void NtcTable_Check(void);
int16 CToF(int16 L_Temp);
int16 Temp_Resolution_Adjust(int16 L_Temp);
void HexToBcd(uint16 Hex_Value);
void LVD_Display(void);

#endif

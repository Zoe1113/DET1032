#ifndef _App_ETOffset_H
#define _App_ETOffset_H

void Forehead_Compensate(void);
void LowTemp_Compensate(void);
uint16 S_NtcFTValue(uint8 R_NtcSection, uint8 R_FTSection);
void Ear_Compensate(void);
void Probecover_compensate(void);

#endif
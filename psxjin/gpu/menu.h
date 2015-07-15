#ifndef _GPU_MENU_H_
#define _GPU_MENU_H_

#include "psxcommon.h"

void DisplayText(void);
void DisplayFrames(void);
void DisplayLag(void);
void DisplayInput(short P1, short P2);
void DisplayAnalog(PadDataS padd1, PadDataS padd2);
void DisplayRecording(int RecNum, int MaxPlayer);
void DisplayMovMode(void);
void CloseMenu(void);
void InitMenu(void);
void BuildDispMenu(int iInc);
void SwitchDispMenu(int iStep);

#endif // _GPU_MENU_H_

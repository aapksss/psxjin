#include "PSXCommon.h"

long PAD1_readPort1(PadDataS* pads);
long PAD2_readPort2(PadDataS* pads);
LRESULT WINAPI ConfigurePADDlgProc (const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam);
void PADconfigure (void);
s32 PADopen (HWND hWnd);
unsigned char PAD1_poll(unsigned char value);
unsigned char PAD2_poll(unsigned char value);
unsigned char PAD1_startPoll(int pad);
unsigned char PAD2_startPoll(int pad);
u8 PADpoll_SSS (u8 value);
void UpdateState (const int pad);
int PadFreeze(gzFile f, int Mode);

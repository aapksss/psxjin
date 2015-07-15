#ifndef _RECORD_H_
#define _RECORD_H_

#include "stdafx.h"
#include <vfw.h>

extern BOOL				RECORD_RECORDING;
extern BOOL				RUN_ONCE;
extern BITMAPINFOHEADER	RECORD_BI;
extern unsigned char	RECORD_BUFFER[1600*1200*3];
extern unsigned long	RECORD_INDEX;
extern unsigned long	RECORD_RECORDING_MODE;
extern unsigned long	RECORD_VIDEO_SIZE;
extern unsigned long	RECORD_RECORDING_WIDTH;
extern unsigned long	RECORD_RECORDING_HEIGHT;
extern unsigned long	RECORD_FRAME_RATE_SCALE;
extern unsigned long	RECORD_COMPRESSION_MODE;
extern COMPVARS			RECORD_COMPRESSION2;
extern unsigned char	RECORD_COMPRESSION_STATE2[1048576];

BOOL RECORD_Start();
void RECORD_Stop();
BOOL RECORD_WriteFrame();
BOOL RECORD_GetFrame();

#endif

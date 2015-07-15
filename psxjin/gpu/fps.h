#ifndef _FPS_INTERNALS_H
#define _FPS_INTERNALS_H

void FrameCap(void);
void FrameCapSSSPSX (void);
void FrameSkip(void);
void calcfps(void);
void PCFrameCap (void);
void PCcalcfps(void);
void SetAutoFrameCap(void);
void SetFPSHandler(void);
void InitFPS(void);
void CheckFrameRate(void);

#endif // _FPS_INTERNALS_H

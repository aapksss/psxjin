#include "stdafx.h"
#include <algorithm>

#define _IN_FPS

#include "externals.h"
#include "fps.h"
#include "gpu.h"

// FPS stuff

LARGE_INTEGER CPUFrequency, PerformanceCounter;

float          fFrameRateHz=0;
DWORD          dwFrameRateTicks=16;
float          fFrameRate;
int            iFrameLimit;
int            UseFrameLimit=0;
int            UseFrameSkip=0;
BOOL           bSSSPSXLimit=FALSE;

// FPS skipping / limit

BOOL   bInitCap = TRUE;
float  fps_skip = 0;
float  fps_cur  = 0;

#define MAXLACE 16

void CheckFrameRate(void)
{
	if (UseFrameSkip)                                     // Skipping mode?
	{
		if (!(dwActFixes&0x80))                             // Not old skipping mode?
		{
			dwLaceCnt++;                                      // Store cnt of Vsync between frames
			if (dwLaceCnt>=MAXLACE && UseFrameLimit)          // If there are many laces without screen toggling,
			{                                                                             // do standard frame limitation
				if (dwLaceCnt==MAXLACE) bInitCap=TRUE;

				if (bSSSPSXLimit) FrameCapSSSPSX();
				else             FrameCap();
			}
		}
		else
			if (UseFrameLimit)
			{
				if (bSSSPSXLimit) FrameCapSSSPSX();
				else             FrameCap();
			}
		calcfps();                                          // Calculate FPS display in skipping mode
	}
	else                                                  // Non-skipping mode
	{
		if (UseFrameLimit) FrameCap();                      // Do it
		if (ulKeybits&KEY_SHOWFPS) calcfps();               // And calculate FPS display
	}
}

BOOL           IsPerformanceCounter = FALSE;

void FrameCap (void)                                   // Frame limit functionality
{
	static DWORD curticks, lastticks, _ticks_since_last_update;
	static DWORD TicksToWait = 0;
	static LARGE_INTEGER  CurrentTime;
	static LARGE_INTEGER  LastTime;
//	static BOOL SkipNextWait = FALSE;
	BOOL Waiting = TRUE;

	if (bInitCap)
	{
		bInitCap=FALSE;
		if (IsPerformanceCounter)
			QueryPerformanceCounter(&LastTime);
		lastticks = timeGetTime();
		TicksToWait=0;
		return;
	}

	if (IsPerformanceCounter)
	{
		QueryPerformanceCounter(&CurrentTime);
		_ticks_since_last_update = CurrentTime.LowPart - LastTime.LowPart;

		curticks = timeGetTime();
		if (_ticks_since_last_update>(CPUFrequency.LowPart>>1))
		{
			if (curticks < lastticks)
				_ticks_since_last_update = dwFrameRateTicks+TicksToWait+1;
			else _ticks_since_last_update = (CPUFrequency.LowPart * (curticks - lastticks))/1000;
		}

		if ((_ticks_since_last_update > TicksToWait) ||
		    (CurrentTime.LowPart < LastTime.LowPart))
		{
			LastTime.HighPart = CurrentTime.HighPart;
			LastTime.LowPart  = CurrentTime.LowPart;

			lastticks=curticks;

			if ((_ticks_since_last_update-TicksToWait) > dwFrameRateTicks)
				TicksToWait=0;
			else TicksToWait=dwFrameRateTicks-(_ticks_since_last_update-TicksToWait);
		}
		else
		{
			while (Waiting)
			{
				QueryPerformanceCounter(&CurrentTime);
				_ticks_since_last_update = CurrentTime.LowPart - LastTime.LowPart;

				curticks = timeGetTime();
				if (_ticks_since_last_update>(CPUFrequency.LowPart>>1))
				{
					if (curticks < lastticks)
						_ticks_since_last_update = TicksToWait+1;
					else _ticks_since_last_update = (CPUFrequency.LowPart * (curticks - lastticks))/1000;
				}

				if ((_ticks_since_last_update > TicksToWait) ||
				    (CurrentTime.LowPart < LastTime.LowPart))
				{
					Waiting = FALSE;

					lastticks=curticks;

					LastTime.HighPart = CurrentTime.HighPart;
					LastTime.LowPart = CurrentTime.LowPart;
					TicksToWait = dwFrameRateTicks;
				}
			}
		}
	}
	else
	{
		curticks = timeGetTime();
		_ticks_since_last_update = curticks - lastticks;

		if ((_ticks_since_last_update > TicksToWait) ||
		    (curticks < lastticks))
		{
			lastticks = curticks;

			if ((_ticks_since_last_update-TicksToWait) > dwFrameRateTicks)
				TicksToWait=0;
			else TicksToWait=dwFrameRateTicks-(_ticks_since_last_update-TicksToWait);
		}
		else
		{
			while (Waiting)
			{
				curticks = timeGetTime();
				_ticks_since_last_update = curticks - lastticks;
				if ((_ticks_since_last_update > TicksToWait) ||
				    (curticks < lastticks))
				{
					Waiting = FALSE;
					lastticks = curticks;
					TicksToWait = dwFrameRateTicks;
				}
			}
		}
	}
}

void FrameCapSSSPSX(void)                              // Frame limit functionality from SSSPSX
{
	static DWORD reqticks, curticks;
	static double offset;

	if (bInitCap)
	{
		bInitCap=FALSE;
		reqticks = curticks = timeGetTime();
		offset = 0;
		return;
	}

	offset+=1000/fFrameRateHz;
	reqticks+=(DWORD)offset;
	offset-=(DWORD)offset;

	curticks = timeGetTime();
	if ((signed int)(reqticks - curticks) > 60)
		Sleep ((reqticks - curticks) / 2);                // Pete: a simple Sleep doesn't burn 100% CPU cycles, but it isn't as exact as a brute force loop
	if ((signed int)(curticks - reqticks) > 60)
		reqticks += (curticks - reqticks) / 2;
}

#define MAXSKIP 120

void FrameSkip(void)
{
	static int   iNumSkips=0,iAdditionalSkip=0;           // Number of additional frames to skip
	static DWORD dwLastLace=0;                            // Helper variable for frame limitation
	static DWORD curticks, lastticks, _ticks_since_last_update;
	static LARGE_INTEGER  CurrentTime;
	static LARGE_INTEGER  LastTime;

	if (!dwLaceCnt) return;                               // Important: if no update lace happened, we ignore it completely

	if (iNumSkips)                                        // We are in pure skipping mode?
	{
		dwLastLace+=dwLaceCnt;                              // Calculate frame limit helper (number of laces)
		bSkipNextFrame = TRUE;                              // We skip next frame as well
		iNumSkips--;                                        // OK, one done
	}
	else                                                  // OK, no additional skipping has to be done
	{                                                    // We check now, if some limitation is needed, or a new skipping has to get started
		DWORD dwWaitTime;

		if (bInitCap || bSkipNextFrame)                     // First time or we skipped before?
		{
			if (UseFrameLimit && !bInitCap)                   // Frame limit wanted and not first time called?
			{
				DWORD dwT=_ticks_since_last_update;             // That's the time of the last drawn frame
				dwLastLace+=dwLaceCnt;                          // And that's the number of update lace since the start of the last drawn frame

				if (IsPerformanceCounter)                       // Now we calculate the time of the last drawn frame + the time we spent skipping
				{
					QueryPerformanceCounter(&CurrentTime);
					_ticks_since_last_update= dwT+CurrentTime.LowPart - LastTime.LowPart;
				}
				else
				{
					curticks = timeGetTime();
					_ticks_since_last_update= dwT+curticks - lastticks;
				}

				dwWaitTime=dwLastLace*dwFrameRateTicks;         // And now we calculate the time the real PS1 would have needed

				if (_ticks_since_last_update<dwWaitTime)        // We were too fast?
				{
					if ((dwWaitTime-_ticks_since_last_update)>    // Some more security, to prevent
					    (60*dwFrameRateTicks))                           // wrong waiting times
						_ticks_since_last_update=dwWaitTime;

					while (_ticks_since_last_update<dwWaitTime)   // Loop until we have reached the real PS1 time
					{                                                                     // That's the additional limitation
						if (IsPerformanceCounter)
						{
							QueryPerformanceCounter(&CurrentTime);
							_ticks_since_last_update = dwT+CurrentTime.LowPart - LastTime.LowPart;
						}
						else
						{
							curticks = timeGetTime();
							_ticks_since_last_update = dwT+curticks - lastticks;
						}
					}
				}
				else                                            // We were still too slow?!
				{
					if (iAdditionalSkip<MAXSKIP)                  // Well, then we really have to stop skipping on very slow systems
					{
						iAdditionalSkip++;                          // Include our watchdog variable
						dwLaceCnt=0;                                // Reset lace count
						if (IsPerformanceCounter)                   // OK, start time of the next frame
							QueryPerformanceCounter(&LastTime);
						lastticks = timeGetTime();
						return;                                     // Done, we will skip next frame to get more speed (SkipNextFrame is still TRUE)
					}
				}
			}

			bInitCap=FALSE;                                   // OK, we have initialized the frameskip function
			iAdditionalSkip=0;                                // Initialized additional skip
			bSkipNextFrame=FALSE;                             // We don't skip the next frame
			if (IsPerformanceCounter)                         // We store the start time of the next frame
				QueryPerformanceCounter(&LastTime);
			lastticks = timeGetTime();
			dwLaceCnt=0;                                      // And we start to count the laces
			dwLastLace=0;
			_ticks_since_last_update=0;
			return;                                           // Done, the next frame will get drawn
		}

		bSkipNextFrame=FALSE;                               // Initialize the frame skip signal to 'no skipping' first

		if (IsPerformanceCounter)                           // Get the current time (we are now at the end of one drawn frame)
		{
			QueryPerformanceCounter(&CurrentTime);
			_ticks_since_last_update = CurrentTime.LowPart - LastTime.LowPart;
		}
		else
		{
			curticks = timeGetTime();
			_ticks_since_last_update = curticks - lastticks;
		}

		dwLastLace=dwLaceCnt;                               // Store current count (frame limitation helper)
		dwWaitTime=dwLaceCnt*dwFrameRateTicks;              // Calculate the 'real PS1 lace time'

		if (_ticks_since_last_update>dwWaitTime)            // Hey, we needed way too long for that frame
		{
			if (UseFrameLimit)                                // If limitation, we skip just next frame,
			{                                                // And decide after, if we need to do more
				iNumSkips=0;
			}
			else
			{
				iNumSkips=_ticks_since_last_update/dwWaitTime;  // Calculate number of frames to skip to catch up
				iNumSkips--;                                    // Since we already skip next frame, one down
				if (iNumSkips>MAXSKIP) iNumSkips=MAXSKIP;       // Well, somewhere we have to draw a line
			}
			bSkipNextFrame = TRUE;                            // Signal for skipping the next frame
		}
		else                                                // We were faster than real PS1?
			if (UseFrameLimit)                                  // Frame limit used? So we wait until the 'real PS1 time' has been reached
			{
				if (dwLaceCnt>MAXLACE)                            // Security check
					_ticks_since_last_update=dwWaitTime;

				while (_ticks_since_last_update<dwWaitTime)       // Just do a waiting loop
				{
					if (IsPerformanceCounter)
					{
						QueryPerformanceCounter(&CurrentTime);
						_ticks_since_last_update = CurrentTime.LowPart - LastTime.LowPart;
					}
					else
					{
						curticks = timeGetTime();
						_ticks_since_last_update = curticks - lastticks;
					}
				}
			}

		if (IsPerformanceCounter)                           // OK, start time of the next frame
			QueryPerformanceCounter(&LastTime);
		lastticks = timeGetTime();
	}

	dwLaceCnt=0;                                          // Initialize lace counter
}

////////////////////////////////////////////////////////////////////////

void calcfps(void)                                     // FPS calculations
{
	static DWORD curticks,_ticks_since_last_update,lastticks;
	static long   fps_cnt = 0;
	static DWORD  fps_tck = 1;
	static LARGE_INTEGER  CurrentTime;
	static LARGE_INTEGER  LastTime;
	static long   fpsskip_cnt = 0;
	static DWORD  fpsskip_tck = 1;

	if (IsPerformanceCounter)
	{
		QueryPerformanceCounter(&CurrentTime);
		_ticks_since_last_update=CurrentTime.LowPart-LastTime.LowPart;

		curticks = timeGetTime();
		if (_ticks_since_last_update>(CPUFrequency.LowPart>>1))
			_ticks_since_last_update = (CPUFrequency.LowPart * (curticks - lastticks))/1000;
		lastticks=curticks;

		if (UseFrameSkip && !UseFrameLimit && _ticks_since_last_update)
			fps_skip=std::min(fps_skip,(((float)CPUFrequency.LowPart) / ((float)_ticks_since_last_update) +1.0f));

		LastTime.HighPart = CurrentTime.HighPart;
		LastTime.LowPart = CurrentTime.LowPart;
	}
	else
	{
		curticks = timeGetTime();
		_ticks_since_last_update=curticks-lastticks;

		if (UseFrameSkip && !UseFrameLimit && _ticks_since_last_update)
			fps_skip=std::min(fps_skip,((float)1000/(float)_ticks_since_last_update+1.0f));

		lastticks = curticks;
	}

	if (UseFrameSkip && UseFrameLimit)
	{
		fpsskip_tck += _ticks_since_last_update;

		if (++fpsskip_cnt==2)
		{
			if (IsPerformanceCounter)
				fps_skip = ((float)CPUFrequency.LowPart) / ((float)fpsskip_tck) *2.0f;
			else
				fps_skip = (float)2000/(float)fpsskip_tck;

			fps_skip +=6.0f;

			fpsskip_cnt = 0;
			fpsskip_tck = 1;
		}
	}

	fps_tck += _ticks_since_last_update;

	if (++fps_cnt==10)
	{
		if (IsPerformanceCounter)
			fps_cur = ((float)CPUFrequency.LowPart) / ((float)fps_tck) *10.0f;
		else
			fps_cur = (float)10000/(float)fps_tck;

		fps_cnt = 0;
		fps_tck = 1;

		if (UseFrameLimit && fps_cur>fFrameRateHz)          // Optical adjust to avoid flickering FPS display
			fps_cur=fFrameRateHz;
	}
}

// PC FPS skipping / limit

void PCFrameCap (void)
{
	static DWORD curticks, lastticks, _ticks_since_last_update;
	static DWORD TicksToWait = 0;
	static LARGE_INTEGER  CurrentTime;
	static LARGE_INTEGER  LastTime;
	BOOL Waiting = TRUE;

	while (Waiting)
	{
		if (IsPerformanceCounter)
		{
			QueryPerformanceCounter(&CurrentTime);
			_ticks_since_last_update = CurrentTime.LowPart - LastTime.LowPart;

			curticks = timeGetTime();
			if (_ticks_since_last_update>(CPUFrequency.LowPart>>1))
			{
				if (curticks < lastticks)
					_ticks_since_last_update = TicksToWait+1;
				else _ticks_since_last_update = (CPUFrequency.LowPart * (curticks - lastticks))/1000;
			}

			if ((_ticks_since_last_update > TicksToWait) ||
			    (CurrentTime.LowPart < LastTime.LowPart))
			{
				Waiting = FALSE;
				lastticks=curticks;
				LastTime.HighPart = CurrentTime.HighPart;
				LastTime.LowPart = CurrentTime.LowPart;
				TicksToWait = (unsigned long)(CPUFrequency.LowPart / fFrameRateHz);
			}
		}
		else
		{
			curticks = timeGetTime();
			_ticks_since_last_update = curticks - lastticks;
			if ((_ticks_since_last_update > TicksToWait) ||
			    (curticks < lastticks))
			{
				Waiting = FALSE;
				lastticks = curticks;
				TicksToWait = (1000 / (DWORD)fFrameRateHz);
			}
		}
	}
}

void PCcalcfps(void)
{
	static DWORD curticks,_ticks_since_last_update,lastticks;
	static long  fps_cnt = 0;
	static float fps_acc = 0;
	static LARGE_INTEGER  CurrentTime;
	static LARGE_INTEGER  LastTime;
	float CurrentFPS=0;

	if (IsPerformanceCounter)
	{
		QueryPerformanceCounter(&CurrentTime);
		_ticks_since_last_update=CurrentTime.LowPart-LastTime.LowPart;

		curticks = timeGetTime();
		if (_ticks_since_last_update>(CPUFrequency.LowPart>>1))
			_ticks_since_last_update = (CPUFrequency.LowPart * (curticks - lastticks))/1000;
		lastticks=curticks;

		if (_ticks_since_last_update)
		{
			CurrentFPS = ((float)CPUFrequency.LowPart) / ((float)_ticks_since_last_update);
		}
		else CurrentFPS = 0;
		LastTime.HighPart = CurrentTime.HighPart;
		LastTime.LowPart = CurrentTime.LowPart;
	}
	else
	{
		curticks = timeGetTime();
		if ((_ticks_since_last_update=curticks-lastticks))
			CurrentFPS=(float)1000/(float)_ticks_since_last_update;
		else CurrentFPS = 0;
		lastticks = curticks;
	}

	fps_acc += CurrentFPS;

	if (++fps_cnt==10)
	{
		fps_cur = fps_acc / 10;
		fps_acc = 0;
		fps_cnt = 0;
	}

	fps_skip=CurrentFPS+1.0f;
}

void SetAutoFrameCap(void)
{
	if (iFrameLimit==1)
	{
		fFrameRateHz = fFrameRate;
		if (IsPerformanceCounter)
			dwFrameRateTicks=(DWORD)(CPUFrequency.LowPart / fFrameRateHz);
		else dwFrameRateTicks=(1000 / (DWORD)fFrameRateHz);
		return;
	}

	if (dwActFixes&32)
	{
		if (PSXDisplay.Interlaced)
			fFrameRateHz = PSXDisplay.PAL?50.0f:60.0f;
		else fFrameRateHz = PSXDisplay.PAL?25.0f:30.0f;
	}
	else
	{
		//fFrameRateHz = PSXDisplay.PAL?50.0f:59.94f;
		if (PSXDisplay.PAL)
		{
			if (lGPUstatusRet&GPUSTATUS_INTERLACED)
				fFrameRateHz=33868800.0f/677343.75f;        // 50.00238
			else fFrameRateHz=33868800.0f/680595.00f;        // 49.76351
		}
		else
		{
			if (lGPUstatusRet&GPUSTATUS_INTERLACED)
				fFrameRateHz=33868800.0f/565031.25f;        // 59.94146
			else fFrameRateHz=33868800.0f/566107.50f;        // 59.82750
		}

		if (IsPerformanceCounter)
			dwFrameRateTicks=(DWORD)(CPUFrequency.LowPart / fFrameRateHz);
		else dwFrameRateTicks=(1000 / (DWORD)fFrameRateHz);
	}
}

void SetFPSHandler(void)
{
	if (QueryPerformanceFrequency (&CPUFrequency))        // Timer mode
		IsPerformanceCounter = TRUE;
	else IsPerformanceCounter = FALSE;
}

void InitFPS(void)
{
	bInitCap = TRUE;

	if (fFrameRateHz==0)
	{
		if (iFrameLimit==2) fFrameRateHz=59.94f;          // Automatic framerate? Set some initial value (no PAL/NTSC known yet)
		else               fFrameRateHz=fFrameRate;       // Else set user framerate
	}

	if (IsPerformanceCounter)
		dwFrameRateTicks=(DWORD)(CPUFrequency.LowPart / fFrameRateHz);
	else dwFrameRateTicks=(1000 / (DWORD)fFrameRateHz);
}

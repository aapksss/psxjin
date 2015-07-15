#include "stdafx.h"

#ifdef _WINDOWS

#include <mmsystem.h>
#include "resource.h"
#include "externals.h"

#define _IN_RECORD

#include "record.h"
#include "psxcommon.h"

int      iDoRecord=0;
HMMIO    hWaveFile=NULL;
MMCKINFO mmckMain;
MMCKINFO mmckData;
char     szRecFileName[MAX_PATH];
unsigned long TotBytes;
int FileCount;
unsigned long MBYTES = 1024*1024;

void RecordStart()
{
	WAVEFORMATEX pcmwf;
	TotBytes = 0;
	
	// Setup header in the same format as our DirectSound stream
	
	memset(&pcmwf,0,sizeof(WAVEFORMATEX));
	pcmwf.wFormatTag      = WAVE_FORMAT_PCM;

	pcmwf.nChannels       = 2;
	pcmwf.nBlockAlign     = 4;
	pcmwf.nSamplesPerSec  = 44100;
	pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
	pcmwf.wBitsPerSample  = 16;

	// Create file
	
    sprintf(&szRecFileName[0],"%s%s%03d_%s.wav",Movie.AviDrive,Movie.AviDirectory,Movie.AviCount,Movie.AviFnameShort);
	hWaveFile=mmioOpen(szRecFileName,NULL,MMIO_CREATE|MMIO_WRITE|MMIO_EXCLUSIVE | MMIO_ALLOCBUF);
	if (!hWaveFile) return;

	// Setup WAVE, fmt, and data chunks
	
	memset(&mmckMain,0,sizeof(MMCKINFO));
	mmckMain.fccType = mmioFOURCC('W','A','V','E');

	mmioCreateChunk(hWaveFile,&mmckMain,MMIO_CREATERIFF);

	memset(&mmckData,0,sizeof(MMCKINFO));
	mmckData.ckid    = mmioFOURCC('f','m','t',' ');
	mmckData.cksize  = sizeof(WAVEFORMATEX);

	mmioCreateChunk(hWaveFile,&mmckData,0);
	mmioWrite(hWaveFile,(char*)&pcmwf,sizeof(WAVEFORMATEX));
	mmioAscend(hWaveFile,&mmckData,0);

	mmckData.ckid = mmioFOURCC('d','a','t','a');
	mmioCreateChunk(hWaveFile,&mmckData,0);
}

void RecordStop()
{
	// First some checks, if recording is running
	
	iDoRecord=0;
	if (!hWaveFile) return;

	// Now finish writing and close the wave file
	
	mmioAscend(hWaveFile,&mmckData,0);
	mmioAscend(hWaveFile,&mmckMain,0);
	mmioClose(hWaveFile,0);

	// Initialize variable
	hWaveFile=NULL;
}

void RecordBuffer(s16* pSound,long lBytes)
{
	// Write the samples
	
	if (hWaveFile) mmioWrite(hWaveFile,(char*)pSound,lBytes);
	TotBytes += lBytes;
	if (TotBytes > 2000*MBYTES)
	{
		RecordStop();
		FileCount++;
		RecordStart();
	}

}

BOOL CALLBACK RecordDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	// Initialize
	case WM_INITDIALOG:
	{
		SetDlgItemText(hW,IDC_WAVFILE,"C:\PEOPS.WAV");   // Initialize filename edit
		ShowCursor(TRUE);                                 // Who is hiding it? The main emulator? tsts (What is tsts?)
		return TRUE;
	}
	// Destroy
	case WM_DESTROY:
	{
		RecordStop();
	}
	break;
	// Command
	case WM_COMMAND:
	{
		if (wParam==IDCANCEL) iRecordMode=2;              // Cancel? Raise flag for destroying the dialog

		if (wParam==IDC_RECORD)                           // Record start/stop?
		{
			if (IsWindowEnabled(GetDlgItem(hW,IDC_WAVFILE))) // Not started yet (edit is not disabled):
			{
				GetDlgItemText(hW,IDC_WAVFILE,szRecFileName,255); // Get filename

				RecordStart();                                // Start recording

				if (hWaveFile)                                // Start was OK?
				
				{                                            // Disable filename edit, change text, raise flag
				
					EnableWindow(GetDlgItem(hW,IDC_WAVFILE),FALSE);
					SetDlgItemText(hW,IDC_RECORD,"Stop recording");
					iDoRecord=1;
				}
				else MessageBeep(0xFFFFFFFF);                 // Error starting recording? BEEP
			}
			else                                            // Stop recording?
			{
				RecordStop();                                 // Just do it
				EnableWindow(GetDlgItem(hW,IDC_WAVFILE),TRUE);// Enable filename edit again
				SetDlgItemText(hW,IDC_RECORD,"Start recording");
			}
			SetFocus(hWMain);
		}
	}
	break;
	// Size
	case WM_SIZE:
		if (wParam==SIZE_MINIMIZED) SetFocus(hWMain);      // If we get minimized, set the focus to the main window
		break;
		// Set cursor
	case WM_SETCURSOR:
	{
		SetCursor(LoadCursor(NULL,IDC_ARROW));            // Force the arrow
		return TRUE;
	}

	}
	return FALSE;
}

#endif

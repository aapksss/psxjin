#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <string>
#include "../plugins.h"
#include "../psxcommon.h"
#include "../movie.h"
#include "../cheat.h"
#include "../r3000a.h"
#include "movie.h"
#include "padwin.h"
#include "win32.h"
#include "resource.h"
#include "maphkeys.h"
#include "ramsearch.h"
#include "ramwatch.h"
#include "../spu/spu.h"
#include "analog.h"

extern HWND LuaConsoleHWnd;
extern INT_PTR CALLBACK DlgLuaScriptDialog(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

extern int iTurboMode;
int iSpeedMode = 7;

int ShowPic=0;
char Text[255];
int ret;
extern int dispAnalog;

void PlayMovieFromBeginning()
{
	if (Movie.mode != MOVIEMODE_INACTIVE) {
		MOV_StopMovie();
		WIN32_StartMovieReplay(Movie.movieFilename);
	}
	else
		GPUdisplayText("*PSXjin*: ERROR: no movie to play");
	return;
}

void ReadonlyToggle()
{
	Movie.readOnly^=1;
	if (Movie.readOnly)
		GPUdisplayText("*PSXjin*: read-only mode");
	else
		GPUdisplayText("*PSXjin*: read+write mode");
	return;
}

void CDOpenClose()
{
	if (Movie.mode == MOVIEMODE_RECORD)
	{
		MovieControl.cdCase ^= 1; // Flip Flag - Not actual status
		if (cdOpenCase < 0) { // Check if open/closed
			if (MovieControl.cdCase) // Notifiers of whether or not we are reversing the process. 
				GPUdisplayText("*PSXjin*: CD case will close on next frame");
			else
				GPUdisplayText("*PSXjin*: CD case won't close on next frame");
		}
		else {
			if (MovieControl.cdCase)
				GPUdisplayText("*PSXjin*: CD case will open on next frame");
			else
				GPUdisplayText("*PSXjin*: CD case won't open on next frame");
		}
	}
	else 
	{
		cdOpenCase ^= -1;
		if (cdOpenCase < 0) {
			GPUdisplayText(_("*PSXjin*: CD case opened"));
			iPause = 1;					
			SwapCD(IsoFile, &Movie.CdromIds[0]);	
			Movie.CDSwap = true;
		}
		else {
			GPUdisplayText(_("*PSXjin*: CD case closed"));
			Movie.CDSwap = false;
			CDRclose();
			CDRopen(IsoFile);
			CheckCdrom();
			if (LoadCdrom() == -1)
				SysMessage(_("Could not load CD-ROM"));
		}
	}
		return;
}

void gpuShowPic() {
	gzFile f;

	if (!ShowPic) {
		unsigned char *pMem;

		pMem = (unsigned char *) malloc(128*96*3);
		if (pMem == NULL) return;
		sprintf(Text, "sstates\\%10.10s.%3.3d", CdromLabel, StatesC);

		GPUfreeze(2, (GPUFreeze_t *)&StatesC);

		f = gzopen(Text, "rb");
		if (f != NULL) {
			gzseek(f, 32, SEEK_SET); // Skip header
			gzread(f, pMem, 128*96*3);
			gzclose(f);
		}
		GPUshowScreenPic(pMem);

		free(pMem);
		ShowPic = 1;
	} else { GPUshowScreenPic(NULL); ShowPic = 0; }
}

void WIN32_SaveState(int newState) {
	StatesC=newState-1;
	if (StatesC == -1) StatesC = 9;
	if (Movie.mode == MOVIEMODE_RECORD)
	{
		MOV_WriteMovieFile();
	}
	if (Movie.mode != MOVIEMODE_INACTIVE)
		sprintf(Text, "%ssstates\\%s.pjm.%3.3d", szCurrentPath, Movie.movieFilenameMini, StatesC+1);
	else
		sprintf(Text, "%ssstates\\%10.10s.%3.3d", szCurrentPath, CdromLabel, StatesC+1);
	GPUfreeze(2, (GPUFreeze_t *)&StatesC);
	ret = SaveState(Text);
	if (ret == 0)
		 sprintf(Text, _("*PSXjin*: saved state %d"), StatesC+1);
	else sprintf(Text, _("*PSXjin*: Error saving state %d"), StatesC+1);
	GPUdisplayText(Text);
	if (ShowPic) { ShowPic = 0; gpuShowPic(); }
}

void WIN32_LoadState(int newState) {
	int previousMode = Movie.mode;
	if (Movie.mode == MOVIEMODE_RECORD) {		
		if (Movie.readOnly) {		
			MOV_WriteMovieFile();
			Movie.mode = MOVIEMODE_PLAY;
		}
	}
	else if (Movie.mode == MOVIEMODE_PLAY) {
		if (!Movie.readOnly) Movie.mode = MOVIEMODE_RECORD;
	}
	StatesC=newState-1;
	if (StatesC == -1) StatesC = 9;
	if (Movie.mode != MOVIEMODE_INACTIVE)
		sprintf(Text, "%ssstates\\%s.pjm.%3.3d", szCurrentPath, Movie.movieFilenameMini, StatesC+1);
	else
		sprintf(Text, "%ssstates\\%10.10s.%3.3d", szCurrentPath, CdromLabel, StatesC+1);
	ret = LoadState(Text);
	if (ret == 0)
		sprintf(Text, _("*PSXjin*: loaded state %d"), StatesC+1);
	else {
		sprintf(Text, _("*PSXjin*: Error loading state %d"), StatesC+1);
		Movie.mode = previousMode;
	}
	GPUdisplayText(Text);
	UpdateToolWindows();
}

char *GetSavestateFilename(int newState) {
	if (Movie.mode != MOVIEMODE_INACTIVE)
		sprintf(Text, "%ssstates\\%s.pjm.%3.3d", szCurrentPath, Movie.movieFilenameMini, newState);
	else
		sprintf(Text, "%ssstates\\%10.10s.%3.3d", szCurrentPath, CdromLabel, newState);
	return Text;
}

void PADhandleKey(int key) {
	char tempstr[1024];
	const int Frates[] = {1, 4, 9, 15, 22, 30, 60, 75, 90, 120, 240, 480, 9999};
	int i;
	int modifiers = 0;
	if(GetAsyncKeyState(VK_CONTROL))
		modifiers = VK_CONTROL;
	else if(GetAsyncKeyState(VK_MENU))
		modifiers = VK_MENU;
	else if(GetAsyncKeyState(VK_SHIFT))
		modifiers = VK_SHIFT;

	for (i = EMUCMD_LOADSTATE1; i <= EMUCMD_LOADSTATE1+9; i++) {
		if(key == EmuCommandTable[i].key
		&& modifiers == EmuCommandTable[i].keymod)
		{
			iLoadStateFrom=i-EMUCMD_LOADSTATE1+1;
			return;
		}
	}

	for (i = EMUCMD_SAVESTATE1; i <= EMUCMD_SAVESTATE1+9; i++) {
		if(key == EmuCommandTable[i].key
		&& modifiers == EmuCommandTable[i].keymod)
		{
			iSaveStateTo=i-EMUCMD_SAVESTATE1+1;
			return;
		}
	}

	for (i = EMUCMD_SELECTSTATE1; i <= EMUCMD_SELECTSTATE1+9; i++) {
		if(key == EmuCommandTable[i].key
		&& modifiers == EmuCommandTable[i].keymod)
		{
			StatesC=i-EMUCMD_SELECTSTATE1;
			GPUfreeze(2, (GPUFreeze_t *)&StatesC);
			if (ShowPic) { ShowPic = 0; gpuShowPic(); }
			sprintf(Text, "*PSXjin*: state %d selected", StatesC+1);
			GPUdisplayText(Text);
			return;
		}
	}

	if(key == EmuCommandTable[EMUCMD_PREVIOUSSTATE].key
	&& modifiers == EmuCommandTable[EMUCMD_PREVIOUSSTATE].keymod)
	{
		if (StatesC == 0)
			StatesC = 9;
		else
			StatesC--;

		GPUfreeze(2, (GPUFreeze_t *)&StatesC);
		if (ShowPic) { ShowPic = 0; gpuShowPic(); }
		sprintf(Text, "*PSXjin*: state %d selected", StatesC+1);
		GPUdisplayText(Text);
		return;
	}

	if(key == EmuCommandTable[EMUCMD_NEXTSTATE].key
	&& modifiers == EmuCommandTable[EMUCMD_NEXTSTATE].keymod)
	{
		if (StatesC == 9)
			StatesC = 0;
		else
			StatesC++;

		GPUfreeze(2, (GPUFreeze_t *)&StatesC);
		if (ShowPic) { ShowPic = 0; gpuShowPic(); }
		sprintf(Text, "*PSXjin*: state %d selected", StatesC+1);
		GPUdisplayText(Text);
		return;
	}

	if(key == EmuCommandTable[EMUCMD_LOADSTATE].key
	&& modifiers == EmuCommandTable[EMUCMD_LOADSTATE].keymod)
	{
		iLoadStateFrom=StatesC+1;
		return;
	}

	if(key == EmuCommandTable[EMUCMD_SAVESTATE].key
	&& modifiers == EmuCommandTable[EMUCMD_SAVESTATE].keymod)
	{
		iSaveStateTo=StatesC+1;
		return;
	}

	if(key == EmuCommandTable[EMUCMD_OPENCD].key
	&& modifiers == EmuCommandTable[EMUCMD_OPENCD].keymod)
	{
		SendMessage(gApp.hWnd, WM_COMMAND, (WPARAM)ID_FILE_RUN_CD,(LPARAM)(NULL));
		return;
	}

	if(key == EmuCommandTable[EMUCMD_PAUSE].key
	&& modifiers == EmuCommandTable[EMUCMD_PAUSE].keymod)
	{
		if (!iPause)
			iDoPauseAtVSync=1;
		else
			iPause=0;
		return;
	}

	if(key == EmuCommandTable[EMUCMD_FRAMEADVANCE].key
	&& modifiers == EmuCommandTable[EMUCMD_FRAMEADVANCE].keymod)
	{
		iFrameAdvance=1;
		return;
	}

	if(key == EmuCommandTable[EMUCMD_ANALOGTOGGLE].key
	&& modifiers == EmuCommandTable[EMUCMD_ANALOGTOGGLE].keymod)
	{
		dispAnalog ^= 1;
		GPUshowAnalog();
		return;
	}

	if(key == EmuCommandTable[EMUCMD_RWTOGGLE].key
	&& modifiers == EmuCommandTable[EMUCMD_RWTOGGLE].keymod)
	{
		ReadonlyToggle();
	}

	if(key == EmuCommandTable[EMUCMD_LAGCOUNTERRESET].key
	&& modifiers == EmuCommandTable[EMUCMD_LAGCOUNTERRESET].keymod)
	{
		Movie.lagCounter=0;
		GPUsetlagcounter(Movie.lagCounter);
		return;
	}

	if(key == EmuCommandTable[EMUCMD_SCREENSHOT].key
	&& modifiers == EmuCommandTable[EMUCMD_SCREENSHOT].keymod)
	{
		GPUmakeSnapshot();
		return;
	}

	if(key == EmuCommandTable[EMUCMD_SPEEDNORMAL].key
	&& modifiers == EmuCommandTable[EMUCMD_SPEEDNORMAL].keymod)
	{
		SetEmulationSpeed(EMUSPEED_NORMAL);
		GPUdisplayText("Speed at 100%");
		return;
	}

	if(key == EmuCommandTable[EMUCMD_SPEEDTURBO].key
	&& modifiers == EmuCommandTable[EMUCMD_SPEEDTURBO].keymod)
	{
		SetEmulationSpeed(EMUSPEED_TURBO);
		return;
	}

	if(key == EmuCommandTable[EMUCMD_SPEEDMAXIMUM].key
	&& modifiers == EmuCommandTable[EMUCMD_SPEEDMAXIMUM].keymod)
	{
		SetEmulationSpeed(EMUSPEED_MAXIMUM);
		return;
	}

	if(key == EmuCommandTable[EMUCMD_TURBOMODE].key
	&& modifiers == EmuCommandTable[EMUCMD_TURBOMODE].keymod)
	{
		if (!iTurboMode) SetEmulationSpeed(EMUSPEED_TURBO);
		iTurboMode = 1;
		return;
	}

	if(key == EmuCommandTable[EMUCMD_SPEEDDEC].key
	&& modifiers == EmuCommandTable[EMUCMD_SPEEDDEC].keymod)
	{		
		SetEmulationSpeed(EMUSPEED_SLOWER);
		sprintf(tempstr,"Speed set at %d%%",(100 * Frates[iSpeedMode-1])/60);
		GPUdisplayText(tempstr);
		return;
	}

	if(key == EmuCommandTable[EMUCMD_SPEEDINC].key
	&& modifiers == EmuCommandTable[EMUCMD_SPEEDINC].keymod)
	{
		SetEmulationSpeed(EMUSPEED_FASTER);
		sprintf(tempstr,"Speed set at %d%%",(100 * Frates[iSpeedMode-1])/60);
		GPUdisplayText(tempstr);
		return;
	}

	if(key == EmuCommandTable[EMUCMD_FRAMECOUNTER].key
	&& modifiers == EmuCommandTable[EMUCMD_FRAMECOUNTER].keymod)
	{
		GPUshowframecounter();
		return;
	}
	if(key == EmuCommandTable[EMUCMD_INPUTDISPLAY].key
	&& modifiers == EmuCommandTable[EMUCMD_INPUTDISPLAY].keymod)
	{
		GPUshowInput();
		return;
	}
	if(key == EmuCommandTable[EMUCMD_CONFCPU].key
	&& modifiers == EmuCommandTable[EMUCMD_CONFCPU].keymod)
	{
		DialogBox(gApp.hInstance, MAKEINTRESOURCE(IDD_CPUCONF), gApp.hWnd, (DLGPROC)ConfigureCpuDlgProc);
		return;
	}

	if(key == EmuCommandTable[EMUCMD_RAMWATCH].key
	&& modifiers == EmuCommandTable[EMUCMD_RAMWATCH].keymod)
	{
		if(!RamWatchHWnd)
		{
			RamWatchHWnd = CreateDialog(gApp.hInstance, MAKEINTRESOURCE(IDD_RAMWATCH), NULL, (DLGPROC) RamWatchProc);
		}
		else
			SetForegroundWindow(RamWatchHWnd);
		return;
	}

	if(key == EmuCommandTable[EMUCMD_CHEATEDITOR].key
	&& modifiers == EmuCommandTable[EMUCMD_CHEATEDITOR].keymod)
	{
		PSXjinRemoveCheats();
		CreateCheatEditor();
		PSXjinApplyCheats();
		return;
	}

	if(key == EmuCommandTable[EMUCMD_RAMSEARCH].key
	&& modifiers == EmuCommandTable[EMUCMD_RAMSEARCH].keymod)
	{
		if(!RamSearchHWnd)
		{
			reset_address_info();
			RamSearchHWnd = CreateDialog(gApp.hInstance, MAKEINTRESOURCE(IDD_RAMSEARCH), NULL, (DLGPROC) RamSearchProc);
		}
		else
			SetForegroundWindow(RamSearchHWnd);
		return;
	}

	if(key == EmuCommandTable[EMUCMD_RAMPOKE].key
	&& modifiers == EmuCommandTable[EMUCMD_RAMPOKE].keymod)
	{
		CreateMemPoke();
		return;
	}

	if(key == EmuCommandTable[EMUCMD_CONFGPU].key
	&& modifiers == EmuCommandTable[EMUCMD_CONFGPU].keymod)
	{
		GPUconfigure();
		return;
	}

	if(key == EmuCommandTable[EMUCMD_CONFSPU].key
	&& modifiers == EmuCommandTable[EMUCMD_CONFSPU].keymod)
	{
		SPUconfigure();
		return;
	}

	if(key == EmuCommandTable[EMUCMD_HOTKEYS].key
	&& modifiers == EmuCommandTable[EMUCMD_HOTKEYS].keymod)
	{
		MHkeysCreate();
		return;
	}

	if(key == EmuCommandTable[EMUCMD_CONFPAD].key
	&& modifiers == EmuCommandTable[EMUCMD_CONFPAD].keymod)
	{
		
		PADconfigure();
		return;
	}

	if(key == EmuCommandTable[EMUCMD_MEMORYCARDS].key
	&& modifiers == EmuCommandTable[EMUCMD_MEMORYCARDS].keymod)
	{
		DialogBox(gApp.hInstance, MAKEINTRESOURCE(IDD_MCDCONF), gApp.hWnd, (DLGPROC)ConfigureMcdsDlgProc);
		return;
	}

	if(key == EmuCommandTable[EMUCMD_STARTRECORDING].key
	&& modifiers == EmuCommandTable[EMUCMD_STARTRECORDING].keymod)
	{
		if (Movie.mode == MOVIEMODE_INACTIVE)
			WIN32_StartMovieRecord();
		else
			GPUdisplayText("*PSXjin*: ERROR: movie already active");
		return;
	}

	if(key == EmuCommandTable[EMUCMD_STARTPLAYBACK].key
	&& modifiers == EmuCommandTable[EMUCMD_STARTPLAYBACK].keymod)
	{
		if (Movie.mode == MOVIEMODE_INACTIVE)
			WIN32_StartMovieReplay(0);
		else
			GPUdisplayText("*PSXjin*: ERROR: movie already active");
		return;
	}

	if(key == EmuCommandTable[EMUCMD_PLAYFROMBEGINNING].key
	&& modifiers == EmuCommandTable[EMUCMD_PLAYFROMBEGINNING].keymod)
	{
		PlayMovieFromBeginning();
	}

	if(key == EmuCommandTable[EMUCMD_STOPMOVIE].key
	&& modifiers == EmuCommandTable[EMUCMD_STOPMOVIE].keymod)
	{
		if (Movie.mode != MOVIEMODE_INACTIVE) {
			MOV_StopMovie();
			GPUdisplayText("*PSXjin*: stop movie");
			EnableMenuItem(gApp.hMenu,ID_FILE_RECORD_MOVIE,MF_ENABLED);
			EnableMenuItem(gApp.hMenu,ID_FILE_REPLAY_MOVIE,MF_ENABLED);
			EnableMenuItem(gApp.hMenu,ID_FILE_STOP_MOVIE,MF_GRAYED);
		}
		else
			GPUdisplayText("*PSXjin*: ERROR: no movie to stop");
		return;
	}

	if(key == EmuCommandTable[EMUCMD_CHEATTOGLE].key
	&& modifiers == EmuCommandTable[EMUCMD_CHEATTOGLE].keymod)
	{
		if (Movie.mode == MOVIEMODE_RECORD || Movie.mode == MOVIEMODE_PLAY) {
			if (Movie.cheatListIncluded) {
				MovieControl.cheats ^= 1;
				if (!cheatsEnabled) {
					if (MovieControl.cheats)
						GPUdisplayText("*PSXjin*: Cheats will activate on next frame");
					else
						GPUdisplayText("*PSXjin*: Cheats won't activate on next frame");
				}
				else {
					if (MovieControl.cheats)
						GPUdisplayText("*PSXjin*: Cheats will deactivate on next frame");
					else
						GPUdisplayText("*PSXjin*: Cheats won't deactivate on next frame");
				}
			}
		}
		else {
			cheatsEnabled ^= 1;
			if (cheatsEnabled)
				GPUdisplayText(_("*PSXjin*: Cheats enabled"));
			else
				GPUdisplayText(_("*PSXjin*: Cheats disabled"));
			PSXjinApplyCheats();
		}
		return;
	}

	if(key == EmuCommandTable[EMUCMD_SIOIRQ].key
	&& modifiers == EmuCommandTable[EMUCMD_SIOIRQ].keymod)
	{
		if (Movie.mode == MOVIEMODE_RECORD || Movie.mode == MOVIEMODE_PLAY) {
			MovieControl.sioIrq ^= 1;
			if (!Config.Sio) {
				if (MovieControl.sioIrq)
					GPUdisplayText("*PSXjin*: SIO IRQ will activate on next frame");
				else
					GPUdisplayText("*PSXjin*: SIO IRQ won't activate on next frame");
			}
			else {
				if (MovieControl.sioIrq)
					GPUdisplayText("*PSXjin*: SIO IRQ will deactivate on next frame");
				else
					GPUdisplayText("*PSXjin*: SIO IRQ won't deactivate on next frame");
			}
		}
		else {
			Config.Sio ^= 0x1;
			if (Config.Sio)
				GPUdisplayText(_("*PSXjin*: SIO IRQ always enabled"));
			else
				GPUdisplayText(_("*PSXjin*: SIO IRQ not always enabled"));
		}
		return;
	}

	if(key == EmuCommandTable[EMUCMD_RCNTFIX].key
	&& modifiers == EmuCommandTable[EMUCMD_RCNTFIX].keymod)
	{
		if (Movie.mode == MOVIEMODE_RECORD || Movie.mode == MOVIEMODE_PLAY) {
			MovieControl.RCntFix ^= 1;
			if (!Config.RCntFix) {
				if (MovieControl.RCntFix)
					GPUdisplayText("*PSXjin*: Parasite Eve 2 hack will activate on next frame");
				else
					GPUdisplayText("*PSXjin*: Parasite Eve 2 hack won't activate on next frame");
			}
			else {
				if (MovieControl.RCntFix)
					GPUdisplayText("*PSXjin*: Parasite Eve 2 hack will deactivate on next frame");
				else
					GPUdisplayText("*PSXjin*: Parasite Eve 2 hack won't deactivate on next frame");
			}
		}
		else {
			Config.RCntFix ^= 0x1;
			if (Config.RCntFix)
				GPUdisplayText(_("*PSXjin*: Parasite Eve 2 hack enabled"));
			else
				GPUdisplayText(_("*PSXjin*: Parasite Eve 2 hack disabled"));
		}
		return;
	}

	if(key == EmuCommandTable[EMUCMD_VSYNCWA].key
	&& modifiers == EmuCommandTable[EMUCMD_VSYNCWA].keymod)
	{
		if (Movie.mode == MOVIEMODE_RECORD || Movie.mode == MOVIEMODE_PLAY) {
			MovieControl.VSyncWA ^= 1;
			if (!Config.VSyncWA) {
				if (MovieControl.VSyncWA)
					GPUdisplayText("*PSXjin*: Resident Evil 2/3 hack will activate on next frame");
				else
					GPUdisplayText("*PSXjin*: Resident Evil 2/3 hack won't activate on next frame");
			}
			else {
				if (MovieControl.VSyncWA)
					GPUdisplayText("*PSXjin*: Resident Evil 2/3 hack will deactivate on next frame");
				else
					GPUdisplayText("*PSXjin*: Resident Evil 2/3 hack won't deactivate on next frame");
			}
		}
		else {
			Config.VSyncWA ^= 0x1;
			if (Config.VSyncWA)
				GPUdisplayText(_("*PSXjin*: Resident Evil 2/3 hack enabled"));
			else
				GPUdisplayText(_("*PSXjin*: Resident Evil 2/3 hack disabled"));
		}
		return;
	}

	if(key == EmuCommandTable[EMUCMD_LUA_OPEN].key
	&& modifiers == EmuCommandTable[EMUCMD_LUA_OPEN].keymod)
	{
		if(!LuaConsoleHWnd)
		{
			LuaConsoleHWnd = CreateDialog(gApp.hInstance, MAKEINTRESOURCE(IDD_LUA), NULL, (DLGPROC) DlgLuaScriptDialog);
		}
		else
			SetForegroundWindow(LuaConsoleHWnd);
		return;
	}

	if(key == EmuCommandTable[EMUCMD_LUA_STOP].key
	&& modifiers == EmuCommandTable[EMUCMD_LUA_STOP].keymod)
	{
		PSXjin_LuaStop();
		return;
	}

	if(key == EmuCommandTable[EMUCMD_LUA_RELOAD].key
	&& modifiers == EmuCommandTable[EMUCMD_LUA_RELOAD].keymod)
	{
		PSXjin_ReloadLuaCode();
		return;
	}

	if(key == EmuCommandTable[EMUCMD_VOLUMEUP].key
	&& modifiers == EmuCommandTable[EMUCMD_VOLUMEUP].keymod)
	{
		if (iVolume < 5)
			iVolume++;
		else if (iVolume > 5)
			iVolume = 5;		// Just in case
		sprintf(Text, "Sound volume: %d", 5 - iVolume);
		GPUdisplayText(_(Text));
		return;
	}

	if(key == EmuCommandTable[EMUCMD_VOLUMEDOWN].key
	&& modifiers == EmuCommandTable[EMUCMD_VOLUMEDOWN].keymod)
	{
		if (iVolume > 0)
			iVolume--;
		if (iVolume < 0)
			iVolume = 0;		// Just in case
		sprintf(Text, "Sound volume: %d", 5 - iVolume);
		GPUdisplayText(_(Text));
		return;
	}

	if(key == EmuCommandTable[EMUCMD_ANALOG].key
	&& modifiers == EmuCommandTable[EMUCMD_ANALOG].keymod)
	{
		OpenAnalogControl();
	}

	if(key == EmuCommandTable[EMUCMD_RESET].key
	&& modifiers == EmuCommandTable[EMUCMD_RESET].keymod)
	{
		ResetGame();
		return;
	}

	if(key == EmuCommandTable[EMUCMD_CDCASE].key
	&& modifiers == EmuCommandTable[EMUCMD_CDCASE].keymod)
	{
		CDOpenClose();
	}

	if(key == EmuCommandTable[EMUCMD_STARTAVI].key
	&& modifiers == EmuCommandTable[EMUCMD_STARTAVI].keymod)
	{
		WIN32_StartAviRecord();
		return;
	}

	if(key == EmuCommandTable[EMUCMD_STOPAVI].key
	&& modifiers == EmuCommandTable[EMUCMD_STOPAVI].keymod)
	{
		WIN32_StopAviRecord();
		return;
	}
	
	if(key == EmuCommandTable[EMUCMD_MTRACK].key
	&& modifiers == EmuCommandTable[EMUCMD_MTRACK].keymod)
	{
		if (!IsMovieLoaded())
		{
			Movie.MultiTrack = !Movie.MultiTrack;
			if (Movie.MultiTrack)
			{
				Movie.RecordPlayer = Movie.NumPlayers+1;
				GPUdisplayText("*PSXjin*: Multi-track enabled");
			}
			else
				GPUdisplayText("*PSXjin*: Multi-track disabled");
		}
		else GPUdisplayText("*PSXjin*: Movie must be loaded to enable multi-track");
		return;
	}
	if(key == EmuCommandTable[EMUCMD_INCPLAYER].key
	&& modifiers == EmuCommandTable[EMUCMD_INCPLAYER].keymod)
	{
		Movie.RecordPlayer += 1;
		Movie.RecordPlayer = (Movie.RecordPlayer > Movie.NumPlayers+2)? (1):(Movie.RecordPlayer);
		return;
	}
	if(key == EmuCommandTable[EMUCMD_DECPLAYER].key
	&& modifiers == EmuCommandTable[EMUCMD_DECPLAYER].keymod)
	{
		Movie.RecordPlayer -= 1;
		Movie.RecordPlayer = (Movie.RecordPlayer == 0)? (Movie.NumPlayers+2):(Movie.RecordPlayer);
		return;
	}
	if(key == EmuCommandTable[EMUCMD_SELECTALL].key
	&& modifiers == EmuCommandTable[EMUCMD_SELECTALL].keymod)
	{
		Movie.RecordPlayer = Movie.NumPlayers+2;		
		return;
	}
	if(key == EmuCommandTable[EMUCMD_SELECTNONE].key
	&& modifiers == EmuCommandTable[EMUCMD_SELECTNONE].keymod)
	{
		Movie.RecordPlayer = Movie.NumPlayers+1;		
		return;
	}

	if(key == EmuCommandTable[EMUCMD_AUTOHOLD].key
	&& modifiers == EmuCommandTable[EMUCMD_AUTOHOLD].keymod)
	{
		Config.GetAutoHold = true;
		return;
	}

	if(key == EmuCommandTable[EMUCMD_AUTOHOLDCLEAR].key
	&& modifiers == EmuCommandTable[EMUCMD_AUTOHOLDCLEAR].keymod)
	{
		Config.Pad1AutoHold = 0;
		Config.Pad2AutoHold = 0;
		Config.EnableAutoHold = false;
		Config.EnableAutoFire = false;
		Config.Pad1AutoFire = 0;
		Config.Pad2AutoFire = 0;
		return;
	}
	
	if(key == EmuCommandTable[EMUCMD_AUTOFIRE].key
	&& modifiers == EmuCommandTable[EMUCMD_AUTOFIRE].keymod)
	{
		Config.GetAutoFire = true;
		return;
	}
}

long CALLBACK GPUopen(HWND hwndGPU);
bool OpenPlugins(HWND hWnd) {
	int ret;

	//GPUclearDynarec(clearDynarec);

	if(strcmp(IsoFile,""))
	{
		ret = CDRopen(IsoFile);
		if (ret == 2) return false;	// Using 2 to mean "do nothing" for when the user cancels the open file dialog
		if (ret < 0) {/* SysMessage (_("Error opening CDR plugin"));*/ return false; }
	}

	SetCurrentDirectory(PSXjinDir);
	
	ret = GPUopen(hWnd);
	if (ret < 0) { SysMessage (_("Error opening GPU plugin (%d)"), ret); return false; }
	ret = SPUopen(hWnd);
	if (ret < 0) { SysMessage (_("Error opening SPU plugin (%d)"), ret); return false; }
	ret = PADopen(hWnd);
	if (ret < 0) { SysMessage (_("Error opening PAD1 plugin (%d)"), ret); }
	

	SetCurrentDirectory(PSXjinDir);
	//ShowCursor(FALSE);
	GPUsendFpLuaGui(PSXjin_LuaGui);

	// It is important to call this because plugins expect it
	ResetPlugins();
	return true;
}

void ClosePlugins() {
	int ret;

	UpdateMenuSlots();
	ret = CDRclose();
	if (ret < 0) { SysMessage (_("Error closing CDR plugin")); return; }
	ret = GPUclose();
	if (ret < 0) { SysMessage (_("Error closing GPU plugin")); return; }
}

void ResetPlugins() {
	int ret;

	CDRshutdown();
	GPUshutdown();
	SPUshutdown();

	ret = CDRinit();
	if (ret != 0) { SysMessage (_("CDRinit error: %d"), ret); return; }
	ret = GPUinit();
	if (ret != 0) { SysMessage (_("GPUinit error: %d"), ret); return; }
	ret = SPUinit();
	if (ret != 0) { SysMessage (_("SPUinit error: %d"), ret); return; }	
}

void SetEmulationSpeed(int cmd) {
	if(cmd == EMUSPEED_TURBO)
		GPUsetspeedmode(0);
	else if(cmd == EMUSPEED_MAXIMUM)
		GPUsetspeedmode(13);
	else if(cmd == EMUSPEED_NORMAL)
		GPUsetspeedmode(7);
	else if(cmd == EMUSPEED_FASTEST)
		GPUsetspeedmode(12);
	else if(cmd == EMUSPEED_SLOWEST)
		GPUsetspeedmode(1);
	else if(cmd == EMUSPEED_SLOWER) {
		if (iSpeedMode>1) iSpeedMode--;
		GPUsetspeedmode(iSpeedMode);
	}
	else if(cmd == EMUSPEED_FASTER) {
		if (iSpeedMode<12) iSpeedMode++;
		GPUsetspeedmode(iSpeedMode);
	}
}

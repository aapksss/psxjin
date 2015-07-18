/*  PSXjin - Pc Psx Emulator
 *    PSXjin Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "psxcommon.h"
#include "plugin.h"
#include "resource.h"
#include "win32.h"
#include "maphkeys.h"
#include "padwin.h"

int tempDest; // This is for the compiler to not throw in a million of warnings (Sounds like a hack, so maybe check that out)

// Changed from storing into the registry to saving into a configure file

void SaveConfig()
{
	char Str_Tmp[1024];
	char Conf_File[1024] = ".\\PSXjin.ini";	// To do: make a global for other files
	
	WritePrivateProfileString("Plugins", "Bios", Config.Bios, Conf_File);
	WritePrivateProfileString("Plugins", "MCD1", Config.Mcd1 , Conf_File);
	WritePrivateProfileString("Plugins", "MCD2", Config.Mcd2 , Conf_File);
	wsprintf(Str_Tmp, "%d", Config.Xa);
	WritePrivateProfileString("Plugins", "Xa", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Config.Sio);
	WritePrivateProfileString("Plugins", "Sio", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Config.Mdec);
	WritePrivateProfileString("Plugins", "Mdec", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Config.PsxAuto);
	WritePrivateProfileString("Plugins", "PsxAuto", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Config.PsxType);
	WritePrivateProfileString("Plugins", "PsxType", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Config.QKeys);
	WritePrivateProfileString("Plugins", "QKeys", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Config.Cdda);
	WritePrivateProfileString("Plugins", "Cdda", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Config.PauseAfterPlayback);
	WritePrivateProfileString("Plugins", "PauseAfterPlayback", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Config.PsxOut);
	WritePrivateProfileString("Plugins", "PsxOut", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Config.RCntFix);
	WritePrivateProfileString("Plugins", "RCntFix", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Config.VSyncWA);
	WritePrivateProfileString("Plugins", "VSyncWA", Str_Tmp, Conf_File);
	SavePADConfig();	
	for (int i = 0; i <= EMUCMDMAX; i++) 
	{
		wsprintf(Str_Tmp, "%d", EmuCommandTable[i].key);
		WritePrivateProfileString("Hotkeys", EmuCommandTable[i].name, Str_Tmp, Conf_File);
	}

	for (int i = 0; i <= EMUCMDMAX; i++)
	{
		wsprintf(Str_Tmp, "%d", EmuCommandTable[i].keymod);
		WritePrivateProfileString("HotkeysKeyMods", EmuCommandTable[i].name, Str_Tmp, Conf_File);
	}
}

void LoadConfig()
{
	char Conf_File[1024] = ".\\PSXjin.ini";	// To do: make a global for other files

	GetPrivateProfileString("Plugins", "Bios", "scph1001.bin", &Config.Bios[0], 256, Conf_File);	
	GetPrivateProfileString("Plugins", "MCD1", "", &Config.Mcd1[0], 256, Conf_File);
	GetPrivateProfileString("Plugins", "MCD2", "", &Config.Mcd2[0], 256, Conf_File);
	Config.Xa = GetPrivateProfileInt("Plugins", "Xa", 0, Conf_File);
	Config.Mdec = GetPrivateProfileInt("Plugins", "Mdec", 0, Conf_File);
	Config.PsxAuto = GetPrivateProfileInt("Plugins", "PsxAuto", 0, Conf_File);
	Config.PsxType = GetPrivateProfileInt("Plugins", "PsxType", 0, Conf_File);
	Config.QKeys = GetPrivateProfileInt("Plugins", "QKeys", 0, Conf_File);
	Config.Cdda = GetPrivateProfileInt("Plugins", "Cdda", 0, Conf_File);
	Config.PauseAfterPlayback = GetPrivateProfileInt("Plugins", "PauseAfterPlayback", 0, Conf_File);
	Config.PsxOut = GetPrivateProfileInt("Plugins", "PsxOut", 0, Conf_File);
	Config.RCntFix = GetPrivateProfileInt("Plugins", "RCntFix", 0, Conf_File);
	Config.VSyncWA = GetPrivateProfileInt("Plugins", "VSyncWA", 0, Conf_File);
	LoadPADConfig();
	int temp;
	for (int i = 0; i <= EMUCMDMAX-1; i++)
	{
		temp = GetPrivateProfileInt("Hotkeys", EmuCommandTable[i].name, 65535, Conf_File);
		if (temp != 65535)
			EmuCommandTable[i].key = temp;
	}
	
	for (int i = 0; i <= EMUCMDMAX-1; i++) 
	{
		temp = GetPrivateProfileInt("HotkeysKeyMods", EmuCommandTable[i].name, 65535, Conf_File);
		if (temp != 65535)
			EmuCommandTable[i].keymod = temp;
	}
}

#define ComboAddPlugin(hw, str) { \
	lp = (char *)malloc(strlen(FindData.cFileName)+8); \
	sprintf(lp, "%s", FindData.cFileName); \
	i = ComboBox_AddString(hw, tmpStr); \
	tempDest = ComboBox_SetItemData(hw, i, lp); \
	if (_stricmp(str, lp)==0) \
		tempDest = ComboBox_SetCurSel(hw, i); \
}

BOOL OnConfigurePluginsDialog(HWND hW) {
	WIN32_FIND_DATA FindData;
	HANDLE Find;
	HWND hWC_BIOS=GetDlgItem(hW,IDC_LISTBIOS);
	char tmpStr[256];
	char *lp;
	int i;

// BIOS

	strcpy(tmpStr, Config.BiosDir);
	strcat(tmpStr, "*");
	Find=FindFirstFile(tmpStr, &FindData);
 	do {
		if (Find==INVALID_HANDLE_VALUE) break;
		if (!strcmp(FindData.cFileName, ".")) continue;
		if (!strcmp(FindData.cFileName, "..")) continue;
		if (FindData.nFileSizeLow != 1024 * 512) continue;
		lp = (char *)malloc(strlen(FindData.cFileName)+8);
		sprintf(lp, "%s", (char *)FindData.cFileName);
		i = ComboBox_AddString(hWC_BIOS, FindData.cFileName);
		tempDest = ComboBox_SetItemData(hWC_BIOS, i, lp);
		if (_stricmp(Config.Bios, FindData.cFileName)==0)
			tempDest = ComboBox_SetCurSel(hWC_BIOS, i);
	} while (FindNextFile(Find,&FindData));
    
	if (Find!=INVALID_HANDLE_VALUE) FindClose(Find);
	if (ComboBox_GetCurSel(hWC_BIOS) == -1)
		tempDest = ComboBox_SetCurSel(hWC_BIOS, 0);

	return TRUE;
}
	
#define CleanCombo(item) \
	hWC = GetDlgItem(hW, item); \
	iCnt = ComboBox_GetCount(hWC); \
	for (i=0; i<iCnt; i++) { \
		lp = (char *)ComboBox_GetItemData(hWC, i); \
		if (lp) free(lp); \
	} \
	tempDest = ComboBox_ResetContent(hWC);

void CleanUpCombos(HWND hW) {
	int i,iCnt;HWND hWC;char * lp;
	CleanCombo(IDC_LISTBIOS);
}

void OnCancel(HWND hW) {
	CleanUpCombos(hW);
	EndDialog(hW,FALSE);
}

char *GetSelBIOS(HWND hW,int id) {
	HWND hWC = GetDlgItem(hW,id);
	int iSel;
	iSel = ComboBox_GetCurSel(hWC);
	if (iSel<0) return NULL;
	return (char *)ComboBox_GetItemData(hWC, iSel);
}

void OnOK(HWND hW) {
	char * biosFILE=GetSelBIOS(hW,IDC_LISTBIOS);

    if  (biosFILE==NULL) {
		MessageBox(hW,"BIOS not selected","Error",MB_OK|MB_ICONERROR);
	}
	else
	{
		strcpy(Config.Bios, biosFILE);
	}

	SaveConfig();

	CleanUpCombos(hW);

	if (!ConfPlug) {
		NeedReset = 1;
		ReleasePlugins();
		LoadPlugins();
	}
	EndDialog(hW,TRUE);
}

BOOL CALLBACK ConfigurePluginsDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_INITDIALOG:
			SetWindowText(hW, _("Configuration"));

			Button_SetText(GetDlgItem(hW, IDOK), _("OK"));
			Button_SetText(GetDlgItem(hW, IDCANCEL), _("Cancel"));
			Static_SetText(GetDlgItem(hW, IDC_BIOS), _("BIOS"));			
			return OnConfigurePluginsDialog(hW);

		case WM_COMMAND:
			switch(LOWORD(wParam)) {				
				case IDCANCEL: 
					OnCancel(hW); 
					if (CancelQuit) {
						SysClose(); exit(1);
					}
					return TRUE;
				case IDOK:     
					OnOK(hW);     
					return TRUE;
			}
	}
	return FALSE;
}

void ConfigurePlugins(HWND hWnd) {
    DialogBox(gApp.hInstance,
              MAKEINTRESOURCE(IDD_CONFIG),
              hWnd,  
              (DLGPROC)ConfigurePluginsDlgProc);
}

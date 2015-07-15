#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include "../soundtouch/soundtouch.h"

//#ifdef _WIN32
//#	include "WinConfig.h"
//#else
//#	include "LnxConfig.h"
//#endif
//
//namespace DebugConfig
//{
//	extern void ReadSettings();
//	extern void WriteSettings();
//	extern void OpenDialog();
//	extern void EnableControls( HWND hWnd );
//}

namespace SoundtouchCfg
{
	extern void ReadSettings();
	extern void WriteSettings();
	extern void OpenDialog( HWND hWnd );
	extern BOOL CALLBACK DialogProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	extern void ApplySettings( soundtouch::SoundTouch& sndtouch );
}


//extern int		SendDialogMsg( HWND hwnd, int dlgId, UINT code, WPARAM wParam, LPARAM lParam);
//extern HRESULT	GUIDFromString( const char *str, LPGUID guid );

//extern void		AssignSliderValue( HWND idcwnd, HWND hwndDisplay, int value );
//extern void		AssignSliderValue( HWND hWnd, int idc, int editbox, int value );
//extern int		GetSliderValue( HWND hWnd, int idc );
//extern BOOL		DoHandleScrollMessage( HWND hwndDisplay, WPARAM wParam, LPARAM lParam );

//extern bool		CfgFindName( const TCHAR *Section, const TCHAR* Name);

//extern void		CfgWriteBool(const TCHAR* Section, const TCHAR* Name, bool Value);
//extern void		CfgWriteInt(const TCHAR* Section, const TCHAR* Name, int Value);
//extern void		CfgWriteStr(const TCHAR* Section, const TCHAR* Name, const wstring& Data);

//extern bool		CfgReadBool(const TCHAR *Section,const TCHAR* Name, bool Default);
//extern void		CfgReadStr(const TCHAR* Section, const TCHAR* Name, wstring& Data, int DataSize, const TCHAR* Default);
//extern void		CfgReadStr(const TCHAR* Section, const TCHAR* Name, TCHAR* Data, int DataSize, const TCHAR* Default);
//extern int		CfgReadInt(const TCHAR* Section, const TCHAR* Name,int Default);

//// Items Specific to DirectSound
//#define STRFY(x) #x
//#define verifyc(x) Verifyc(x,STRFY(x))
//
//extern void Verifyc(HRESULT hr, const char* fn);
//
//struct ds_device_data
//{
//	std::wstring name;
//	GUID guid;
//	bool hasGuid;
//};
//

#include "stdafx.h"

HINSTANCE hInst=NULL;

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	hInst=(HINSTANCE)hModule;
	return TRUE;
}

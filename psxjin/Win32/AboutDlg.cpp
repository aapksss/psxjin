#include <windows.h>
#include <windowsx.h>
#include "resource.h"
#include "aboutdlg.h"
#include "psxcommon.h"

LRESULT WINAPI AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_INITDIALOG:
			SetWindowText(hDlg, _("About"));

			Button_SetText(GetDlgItem(hDlg, IDOK), _("OK"));
			Static_SetText(GetDlgItem(hDlg, IDC_PSXjin_ABOUT_TEXT), _("PSXjin\n"));
			Static_SetText(GetDlgItem(hDlg, IDC_PSXjin_ABOUT_AUTHORS), _(LabelAuthors));
			Static_SetText(GetDlgItem(hDlg, IDC_PSXjin_ABOUT_GREETS), _(LabelGreets));
			Button_SetText(GetDlgItem(hDlg,IDOK), _("OK"));
			return TRUE;

		case WM_COMMAND:
			switch(wParam) {
				case IDOK:
				case IDCANCEL:
					EndDialog(hDlg, TRUE);
					return TRUE;
			}
			break;

		case WM_CLOSE:
			EndDialog(hDlg, TRUE);
			break;
	}
	return FALSE;
}

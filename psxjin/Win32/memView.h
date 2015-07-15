#ifndef MEM_VIEW_H
#define MEM_VIEW_H

INT_PTR CALLBACK MemView_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MemView_ViewBoxProc(HWND hCtl, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CMemView : public CToolWindow
{
public:
	CMemView();
	~CMemView();

	HFONT font;

	u32 region;
	u32 address;
	u32 viewMode;

	BOOL sel;
	u32 selPart;
	u32 selAddress;
	u32 selNewVal;
};

#endif

void ReadConfig(void);

#ifdef _WINDOWS
BOOL CALLBACK AboutDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DSoundDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam);
#else
void StartCfgTool(char * pCmdLine);
#endif

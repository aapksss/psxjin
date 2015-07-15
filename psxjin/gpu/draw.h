#ifndef _GPU_DRAW_H_
#define _GPU_DRAW_H_

void          DoBufferSwap(void);
void          DoClearScreenBuffer(void);
void          DoClearFrontBuffer(void);
unsigned long ulInitDisplay(void);
void          CloseDisplay(void);
void          CreatePic(unsigned char * pMem);
void          DestroyPic(void);
void          DisplayPic(void);
void          ShowGpuPic(void);
void          ShowTextGpuPic(void);
void          MoveScanLineArea(HWND hwnd);
void		  SetRes(int X, int Y);

#endif // _GPU_DRAW_H_

#ifndef _GPU_INTERNALS_H
#define _GPU_INTERNALS_H

#define OPAQUEON   10
#define OPAQUEOFF  11

#define KEY_RESETTEXSTORE 1
#define KEY_SHOWFPS       2
#define KEY_RESETOPAQUE   4
#define KEY_RESETDITHER   8
#define KEY_RESETFILTER   16
#define KEY_RESETADVBLEND 32
//#define KEY_BLACKWHITE    64
#define KEY_BADTEXTURES   128
#define KEY_CHECKTHISOUT  256
#define KEY_SHOWINPUT	  512
#define KEY_SHOWFCOUNT	  1024
#define KEY_SHOWANALOG	  2048

#ifndef _FPSE
#define RED(x) (x & 0xff)
#define BLUE(x) ((x>>16) & 0xff)
#define GREEN(x) ((x>>8) & 0xff)
#define COLOR(x) (x & 0xffffff)
#else
#define BLUE(x) (x & 0xff)
#define RED(x) ((x>>16) & 0xff)
#define GREEN(x) ((x>>8) & 0xff)
#define COLOR(x) (x & 0xffffff)
#endif

void           updateDisplay(void);
void           SetAutoFrameCap(void);
void           SetFixes(void);
void           speedModifier(unsigned long option);
void           makeNormalSnapshotPNG(void);
void           makeNormalSnapshotBMP(void);
void           makeVramSnapshot(void);
void           makeFullVramSnapshot(void);
extern void           (*fpPSXjin_LuaGui)(void *s, int width, int height, int bpp, int pitch);

#endif // _GPU_INTERNALS_H

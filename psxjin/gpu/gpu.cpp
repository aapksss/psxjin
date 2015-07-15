#include "stdafx.h"
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "resource.h"
#include <png.h>
#include "win32.h"

#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL

#define _IN_GPU

#include "gpurecord.h"
#include "externals.h"
#include "gpu.h"
#include "draw.h"
#include "gpucfg.h"
#include "prim.h"
#include "psemu.h"
#include "menu.h"
#include "fps.h"
#include "../plugins.h"

unsigned long dwGPUVersion=0;
int           iGPUHeight=512;
int           iGPUHeightMask=511;
int           GlobalTextIL=0;
int           iTileCheat=0;
unsigned long          ulKeybits=0;

void (*fpPSXjin_LuaGui)(void *s, int width, int height, int bpp, int pitch);
int iMaximumSpeed=0;

// PPDK developer must change libraryName field and can change revision and build

const  unsigned char version  = 1;    // Do not touch - library for PSEmu 1.x
const  unsigned char revision = 0;
const  unsigned char build    = 2;   // Increase that with each version

static char *libraryName      = "TAS Soft Graphics Plugin";

static char *PluginAuthor     = "Pete Bernert and the P.E.Op.S. team";

// Memory image of the PS1 VRAM

unsigned char  *psxVSecure;
unsigned char  *psxVub;
signed   char  *psxVsb;
unsigned short *psxVuw;
unsigned short *psxVuw_eom;
signed   short *psxVsw;
unsigned long  *psxVul;
signed   long  *psxVsl;

// GPU globals

static long       lGPUdataRet;
long              lGPUstatusRet; //!
char              szDispBuf[64];
char              szMenuBuf[36];
char			  szInputBuf[36];
char              szDebugText[512];
unsigned long     ulStatusControl[256];
int               currentFrame=0;
int               totalFrames=0;
int               currentLag=0;
int               currentInput=0;
int               ThereIsLag=0;
char              modeFlags;

unsigned char BMP_BUFFER[1600*1200*3];

static unsigned   long gpuDataM[256];
static unsigned   char gpuCommand = 0;
static long       gpuDataC = 0;
static long       gpuDataP = 0;

VRAMLoad_t        VRAMWrite;
VRAMLoad_t        VRAMRead;
DATAREGISTERMODES DataWriteMode; //!
DATAREGISTERMODES DataReadMode; //!

BOOL              bSkipNextFrame = FALSE;
DWORD             dwLaceCnt=0;
int               iColDepth;
int               iWindowMode;
static int				  dispInput = 0;
short             sDispWidths[8] = {256,320,512,640,368,384,512,640};
PSXDisplay_t      PSXDisplay; //!
PSXDisplay_t      PreviousPSXDisplay; //!
long              lSelectedSlot=0;
BOOL              bChangeWinMode=FALSE;
BOOL              bDoLazyUpdate=FALSE;
unsigned long     lGPUInfoVals[16]; //!
int               iFakePrimBusy=0;
int               iRumbleVal=0;
int               iRumbleTime=0;

// Screensaver stuff: dynamically load kernel32.dll to avoid export dependency

int				  iStopSaver=0;
HINSTANCE kernel32LibHandle = NULL;

// A stub function, that does nothing .... but it does "nothing" well
EXECUTION_STATE WINAPI STUB_SetThreadExecutionState(EXECUTION_STATE esFlags)
{
	return esFlags;
}

// The dynamic version of the system call is perpended with a "D_"
EXECUTION_STATE (WINAPI *D_SetThreadExecutionState)(EXECUTION_STATE esFlags) = STUB_SetThreadExecutionState;

BOOL LoadKernel32(void)
{
	// Get a handle to the kernel32.dll (which is actually already loaded)
	kernel32LibHandle = LoadLibrary("kernel32.dll");

	// If we've got a handle, then locate the entry point for the SetThreadExecutionState function
	if (kernel32LibHandle != NULL)
	{
		if ((D_SetThreadExecutionState = (EXECUTION_STATE (WINAPI *)(EXECUTION_STATE))GetProcAddress (kernel32LibHandle, "SetThreadExecutionState")) == NULL)
			D_SetThreadExecutionState = STUB_SetThreadExecutionState;
	}

	return TRUE;
}

BOOL FreeKernel32(void)
{
	// Release the handle to kernel32.dll
	if (kernel32LibHandle != NULL)
		FreeLibrary(kernel32LibHandle);

	// Set to stub function, to avoid nasty surprises if called
	D_SetThreadExecutionState = STUB_SetThreadExecutionState;

	return TRUE;
}

// Some misc external display functions

/*
unsigned long PCADDR;
void CALLBACK GPUdebugSetPC(unsigned long addr)
{
	PCADDR=addr;
}
*/

#include <time.h>
time_t tStart;

void CALLBACK GPUdisplayText(char * pText)             // Some debug function
{
	if (!pText)
	{
		szDebugText[0]=0;
		return;
	}
	if (strlen(pText)>511) return;
	time(&tStart);
	strcpy(szDebugText,pText);
}

void CALLBACK GPUdisplayFlags(unsigned long dwFlags)   // Some info function
{
	dwCoreFlags=dwFlags;
	BuildDispMenu(0);
}

// Screenshot function

char * pGetConfigInfos(int iCfg)
{
	char szO[2][4]={"off","on "};
	char szTxt[256];
	char * pB=(char *)malloc(32767);

	if (!pB) return NULL;
	*pB=0;

	sprintf(szTxt,"Plugin: %s %d.%d.%d\r\n",libraryName,version,revision,build);
	strcat(pB,szTxt);
	sprintf(szTxt,"Author: %s\r\n\r\n",PluginAuthor);
	strcat(pB,szTxt);

	if (iCfg && iWindowMode)
		sprintf(szTxt,"Resolution/Color:\r\n- %dx%d ",iResX,iResY);
	else
		sprintf(szTxt,"Resolution/Color:\r\n- %dx%d ",iResX,iResY);
	strcat(pB,szTxt);
	if (iWindowMode && iCfg)
		strcpy(szTxt,"Window mode\r\n");
	else
		if (iWindowMode)
			sprintf(szTxt,"Window mode - [%d Bit]\r\n",iDesktopCol);
		else
			sprintf(szTxt,"Fullscreen - [%d Bit]\r\n",iColDepth);
	strcat(pB,szTxt);

	sprintf(szTxt,"Stretch mode: %d\r\n",iUseNoStretchBlt);
	strcat(pB,szTxt);
	sprintf(szTxt,"Dither mode: %d\r\n\r\n",iUseDither);
	strcat(pB,szTxt);

	sprintf(szTxt,"Framerate:\r\n- FPS limit: %s\r\n",szO[UseFrameLimit]);
	strcat(pB,szTxt);
	sprintf(szTxt,"- Frame skipping: %s",szO[UseFrameSkip]);
	strcat(pB,szTxt);
	if (iFastFwd) strcat(pB," (fast forward)");
	strcat(pB,"\r\n");
	if (iFrameLimit==2)
		strcpy(szTxt,"- FPS limit: Auto\r\n\r\n");
	else sprintf(szTxt,"- FPS limit: %.1f\r\n\r\n",fFrameRate);
	strcat(pB,szTxt);

	strcpy(szTxt,"Miscellaneous:\r\n- Scanlines: ");
	if (iUseScanLines==0) strcat(szTxt,"disabled");
	else
		if (iUseScanLines==1) strcat(szTxt,"standard");
		else
			if (iUseScanLines==2) strcat(szTxt,"double blitting");
	strcat(szTxt,"\r\n");
	strcat(pB,szTxt);
	sprintf(szTxt,"- Game fixes: %s [%08lx]\r\n",szO[iUseFixes],dwCfgFixes);
	strcat(pB,szTxt);

	return pB;
}

void DoTextSnapShot(int iNum)
{
	FILE *txtfile;
	char szTxt[256];
	char * pB;

	sprintf(szTxt,"SNAP\\PEOPSSOFT%03d.txt",iNum);

	if ((txtfile=fopen(szTxt,"wb"))==NULL)
		return;

	pB=pGetConfigInfos(0);
	if (pB)
	{
		fwrite(pB,strlen(pB),1,txtfile);
		free(pB);
	}
	fclose(txtfile);
}

void CALLBACK GPUmakeSnapshot(void)
{
	makeNormalSnapshotPNG();
}

void makeNormalSnapshotPNG(void)                    // Screenshot of current screen
{
	static unsigned short *srcs,*src,cs;
	static unsigned char *srcc,*destc;
	static long x,y,cx,cy,ax,ay;
	static unsigned long cl;
	char sendThisText[50];
	BITMAPINFOHEADER BMP_INFO = {40,0,0,1,16,0,0,2048,2048,0,0};
	FILE *bmpfile;
	char filename[256];
	unsigned long snapshotnr = 0;
	png_structp png_ptr;
	png_infop info_ptr;

	BMP_INFO.biWidth = PSXDisplay.DisplayMode.x;
	BMP_INFO.biHeight = PSXDisplay.DisplayMode.y;
	BMP_INFO.biBitCount = 16;
	BMP_INFO.biSizeImage = BMP_INFO.biWidth*BMP_INFO.biHeight*3;

	do
	{
		snapshotnr++;

		sprintf(filename,"SNAP\\snap_%03lu.png",snapshotnr);

		bmpfile=fopen(filename,"rb");
		if (bmpfile == NULL) break;
		fclose(bmpfile);
	}
	while (TRUE);

	srcs = (unsigned short*)&psxVuw[PSXDisplay.DisplayPosition.x+(PSXDisplay.DisplayPosition.y<<10)];
	destc = (unsigned char*)BMP_BUFFER;
	ax = (BMP_INFO.biWidth*(s64)65536)/BMP_INFO.biWidth;
	ay = (BMP_INFO.biHeight*(s64)65536)/BMP_INFO.biHeight;
	cy = (BMP_INFO.biHeight-1)<<16;

	if (PSXDisplay.RGB24)
	{
		if (iFPSEInterface)
		{
			for (y=0;y<BMP_INFO.biHeight;y++)
			{
				srcc = (unsigned char*)&srcs[(cy&0xffff0000)>>6];
				cx = 0;
				for (x=0;x<BMP_INFO.biWidth;x++)
				{
					cl = *((unsigned long*)&srcc[(cx>>16)*3]);
					*(destc++) = (unsigned char)((cl&0xff0000)>>16);
					*(destc++) = (unsigned char)((cl&0xff00)>>8);
					*(destc++) = (unsigned char)(cl&0xff);
					cx += ax;
				}
				cy -= ay;
				if (cy<0) cy=0;
			}
		}
		else
		{
			for (y=0;y<BMP_INFO.biHeight;y++)
			{
				srcc = (unsigned char*)&srcs[(cy&0xffff0000)>>6];
				cx = 0;
				for (x=0;x<BMP_INFO.biWidth;x++)
				{
					cl = *((unsigned long*)&srcc[(cx>>16)*3]);
					*(destc++) = (unsigned char)(cl&0xff);
					*(destc++) = (unsigned char)((cl&0xff00)>>8);
					*(destc++) = (unsigned char)((cl&0xff0000)>>16);
					cx += ax;
				}
				cy -= ay;
				if (cy<0) cy=0;
			}
		}
	}
	else
	{
		for (y=0;y<BMP_INFO.biHeight;y++)
		{
			src = &srcs[(cy&0xffff0000)>>6];
			cx = 0;
			for (x=0;x<BMP_INFO.biWidth;x++)
			{
				cs = src[cx>>16];
				*(destc++) = (unsigned char)((cs&0x001f)<<3);
				*(destc++) = (unsigned char)((cs&0x03e0)>>2);
				*(destc++) = (unsigned char)((cs&0x7c00)>>7);
				cx += ax;
			}
			cy -= ay;
			if (cy<0) cy=0;
		}
	}

	if ((bmpfile=fopen(filename,"wb"))==NULL)
		return;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	if(png_ptr == NULL) {
		fclose(bmpfile);
		return;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == NULL) {
		fclose(bmpfile);
		png_destroy_write_struct(&png_ptr, png_infopp_NULL);
		return;
	}
	if(setjmp(png_jmpbuf(png_ptr))) {
		fclose(bmpfile);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return;
	}
	png_init_io(png_ptr, bmpfile);

	png_set_IHDR(png_ptr, info_ptr, BMP_INFO.biWidth, BMP_INFO.biHeight, 8, PNG_COLOR_TYPE_RGB,
	             PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);
	{
		png_bytepp row_pointers = (png_bytepp) calloc(BMP_INFO.biHeight, sizeof(png_bytep));
		if(row_pointers == NULL) {
			fclose(bmpfile);
			png_destroy_write_struct(&png_ptr, png_infopp_NULL);
			return;
		}
		destc = (unsigned char*)BMP_BUFFER;
		for (y = 0; y < BMP_INFO.biHeight; y++)
			row_pointers[BMP_INFO.biHeight-y-1] = destc + y*BMP_INFO.biWidth*3;
		png_write_image(png_ptr, row_pointers);
		free(row_pointers);
	}
	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	sprintf(sendThisText,"Snap saved as \"snap_%03lu.png\".",snapshotnr);
	GPUdisplayText(sendThisText);
	fclose(bmpfile);
}

void makeNormalSnapshotBMP(void)                    // Screenshot of current screen
{
	static unsigned short *srcs,*src,cs;
	static unsigned char *srcc,*destc;
	static long x,y,cx,cy,ax,ay;
	static unsigned long cl;
	char sendThisText[50];
	unsigned char empty[2]={0,0};
	BITMAPINFOHEADER BMP_INFO = {40,0,0,1,16,0,0,2048,2048,0,0};

	FILE *bmpfile;
	char filename[256];
	char header[0x36];
	unsigned long snapshotnr = 0;
	long size;

	BMP_INFO.biWidth = PSXDisplay.DisplayMode.x;
	BMP_INFO.biHeight = PSXDisplay.DisplayMode.y;
	BMP_INFO.biBitCount = 24;
	BMP_INFO.biSizeImage = BMP_INFO.biWidth*BMP_INFO.biHeight*3;
	size=BMP_INFO.biSizeImage+0x38;

	// Hard coded BMP header
	memset(header,0,0x36);
	header[0]='B';
	header[1]='M';
	header[2]=size&0xff;
	header[3]=(size>>8)&0xff;
	header[4]=(size>>16)&0xff;
	header[5]=(size>>24)&0xff;
	header[0x0a]=0x36;
	header[0x0e]=0x28;
	header[0x12]=BMP_INFO.biWidth%256;
	header[0x13]=BMP_INFO.biWidth/256;
	header[0x16]=BMP_INFO.biHeight%256;
	header[0x17]=BMP_INFO.biHeight/256;
	header[0x1a]=0x01;
	header[0x1c]=0x18;
	header[0x26]=0x12;
	header[0x27]=0x0B;
	header[0x2A]=0x12;
	header[0x2B]=0x0B;

	do
	{
		snapshotnr++;

		sprintf(filename,"SNAP\\snap_%03lu.bmp",snapshotnr);

		bmpfile=fopen(filename,"rb");
		if (bmpfile == NULL) break;
		fclose(bmpfile);
	}
	while (TRUE);

	// Try opening new screenshot file
	if ((bmpfile=fopen(filename,"wb"))==NULL)
		return;

	fwrite(header,0x36,1,bmpfile);

	srcs = (unsigned short*)&psxVuw[PSXDisplay.DisplayPosition.x+(PSXDisplay.DisplayPosition.y<<10)];
	destc = (unsigned char*)BMP_BUFFER;
	ax = (BMP_INFO.biWidth*65535L)/BMP_INFO.biWidth;
	ay = (BMP_INFO.biHeight*65535L)/BMP_INFO.biHeight;
	cy = (BMP_INFO.biHeight-1)<<16;

	if (PSXDisplay.RGB24)
	{
		if (iFPSEInterface)
		{
			for (y=0;y<BMP_INFO.biHeight;y++)
			{
				srcc = (unsigned char*)&srcs[(cy&0xffff0000)>>6];
				cx = 0;
				for (x=0;x<BMP_INFO.biWidth;x++)
				{
					cl = *((unsigned long*)&srcc[(cx>>16)*3]);
					*(destc++) = (unsigned char)(cl&0xff);
					*(destc++) = (unsigned char)((cl&0xff00)>>8);
					*(destc++) = (unsigned char)((cl&0xff0000)>>16);
					cx += ax;
				}
				cy -= ay;
				if (cy<0) cy=0;
			}
		}
		else
		{
			for (y=0;y<BMP_INFO.biHeight;y++)
			{
				srcc = (unsigned char*)&srcs[(cy&0xffff0000)>>6];
				cx = 0;
				for (x=0;x<BMP_INFO.biWidth;x++)
				{
					cl = *((unsigned long*)&srcc[(cx>>16)*3]);
					*(destc++) = (unsigned char)((cl&0xff0000)>>16);
					*(destc++) = (unsigned char)((cl&0xff00)>>8);
					*(destc++) = (unsigned char)(cl&0xff);
					cx += ax;
				}
				cy -= ay;
				if (cy<0) cy=0;
			}
		}
	}
	else
	{
		for (y=0;y<BMP_INFO.biHeight;y++)
		{
			src = &srcs[(cy&0xffff0000)>>6];
			cx = 0;
			for (x=0;x<BMP_INFO.biWidth;x++)
			{
				cs = src[cx>>16];
				*(destc++) = (unsigned char)((cs&0x7c00)>>7);
				*(destc++) = (unsigned char)((cs&0x03e0)>>2);
				*(destc++) = (unsigned char)((cs&0x001f)<<3);
				cx += ax;
			}
			cy -= ay;
			if (cy<0) cy=0;
		}
	}


	destc = (unsigned char*)BMP_BUFFER;
	for (y=0;y<BMP_INFO.biHeight;y++) {
		fwrite(destc+(y*BMP_INFO.biWidth*3),BMP_INFO.biWidth*3,1,bmpfile);
	}
	fwrite(empty,0x2,1,bmpfile);
	fclose(bmpfile);

	sprintf(sendThisText,"Snap saved as \"snap_%03lu.bmp\".",snapshotnr);
	GPUdisplayText(sendThisText);
}

void makeVramSnapshot(void)                    // Screenshot of current screen
{
	char sendThisText[50];
	FILE *bmpfile;
	char filename[256];
	unsigned char header[0x36];
	long size,height,width,start_x,start_y;
	unsigned char * line;
	short i,j;
	unsigned char empty[2]={0,0};
	unsigned short color;
	unsigned long snapshotnr = 0;
	start_x = PSXDisplay.DisplayPosition.x;
	start_y = PSXDisplay.DisplayPosition.y;
	width=PSXDisplay.DisplayMode.x;
	height=PSXDisplay.DisplayMode.y;
	line = (unsigned char*) malloc(width*3);
	size=height*width*3+0x38;

	// Fill in proper values for BMP

	// Hard coded BMP header
	memset(header,0,0x36);
	header[0]='B';
	header[1]='M';
	header[2]=size&0xff;
	header[3]=(size>>8)&0xff;
	header[4]=(size>>16)&0xff;
	header[5]=(size>>24)&0xff;
	header[0x0a]=0x36;
	header[0x0e]=0x28;
	header[0x12]=width%256;
	header[0x13]=width/256;
	header[0x16]=height%256;
	header[0x17]=height/256;
	header[0x1a]=0x01;
	header[0x1c]=0x18;
	header[0x26]=0x12;
	header[0x27]=0x0B;
	header[0x2A]=0x12;
	header[0x2B]=0x0B;

	// Increment screenshot value and try to get filename
	do
	{
		snapshotnr++;

		sprintf(filename,"SNAP\\snap_%03lu.bmp",snapshotnr);

		bmpfile=fopen(filename,"rb");
		if (bmpfile == NULL) break;
		fclose(bmpfile);
	}
	while (TRUE);

	// Try opening new screenshot file
	if ((bmpfile=fopen(filename,"wb"))==NULL)
		return;

	fwrite(header,0x36,1,bmpfile);
	for (i=start_y+height-1;i>=start_y;i--)
	{
		for (j=start_x;j<width+start_x;j++)
		{
			color=psxVuw[i*1024+j];
			line[(j-start_x)*3+2]=(color<<3)&0xf1;
			line[(j-start_x)*3+1]=(color>>2)&0xf1;
			line[(j-start_x)*3+0]=(color>>7)&0xf1;
		}
		fwrite(line,width*3,1,bmpfile);
	}
	fwrite(empty,0x2,1,bmpfile);
	fclose(bmpfile);
	free(line);

	sprintf(sendThisText,"Snap saved as \"snap_%03lu.bmp\".",snapshotnr);
	GPUdisplayText(sendThisText);
}

void makeFullVramSnapshot(void)                    // screenshot of whole VRAM
{
	FILE *bmpfile;
	char filename[256];
	unsigned char header[0x36];
	long size,height;
	unsigned char line[1024*3];
	short i,j;
	unsigned char empty[2]={0,0};
	unsigned short color;
	unsigned long snapshotnr = 0;

	height=iGPUHeight;

	size=height*1024*3+0x38;

	// Fill in proper values for BMP

	// Hard coded BMP header
	memset(header,0,0x36);
	header[0]='B';
	header[1]='M';
	header[2]=size&0xff;
	header[3]=(size>>8)&0xff;
	header[4]=(size>>16)&0xff;
	header[5]=(size>>24)&0xff;
	header[0x0a]=0x36;
	header[0x0e]=0x28;
	header[0x12]=1024%256;
	header[0x13]=1024/256;
	header[0x16]=height%256;
	header[0x17]=height/256;
	header[0x1a]=0x01;
	header[0x1c]=0x18;
	header[0x26]=0x12;
	header[0x27]=0x0B;
	header[0x2A]=0x12;
	header[0x2B]=0x0B;

	// Increment screenshot value and try to get filename
	do
	{
		snapshotnr++;
		sprintf(filename,"SNAP\\PEOPSSOFT%03lu.bmp",snapshotnr);
		bmpfile=fopen(filename,"rb");
		if (bmpfile == NULL) break;
		fclose(bmpfile);
	}
	while (TRUE);

	// Try opening new screenshot file
	
	if ((bmpfile=fopen(filename,"wb"))==NULL)
		return;

	fwrite(header,0x36,1,bmpfile);
	for (i=height-1;i>=0;i--)
	{
		for (j=0;j<1024;j++)
		{
			color=psxVuw[i*1024+j];
			line[j*3+2]=(color<<3)&0xf1;
			line[j*3+1]=(color>>2)&0xf1;
			line[j*3+0]=(color>>7)&0xf1;
		}
		fwrite(line,1024*3,1,bmpfile);
	}
	fwrite(empty,0x2,1,bmpfile);
	fclose(bmpfile);

//	DoTextSnapShot(snapshotnr);
}

// Initialization, will be called after library load. Well, just do some variable initialization

long CALLBACK GPUinit()                                // GPU initialization
{
	memset(ulStatusControl,0,256*sizeof(unsigned long));  // Initialize save state scontrol field

	szDebugText[0]=0;                                     // Initialize debug text buffer

	psxVSecure=(unsigned char *)malloc((iGPUHeight*2)*1024 + (1024*1024)); // Always allocate one extra MB for soft drawing function security
	if (!psxVSecure) return -1;

	//!!! ATTENTION !!!
	psxVub=psxVSecure+512*1024;                           // Security offset into double sized PS1 VRAM

	psxVsb=(signed char *)psxVub;                         // Different ways of accessing PS1 VRAM
	psxVsw=(signed short *)psxVub;
	psxVsl=(signed long *)psxVub;
	psxVuw=(unsigned short *)psxVub;
	psxVul=(unsigned long *)psxVub;

	psxVuw_eom=psxVuw+1024*iGPUHeight;                    // Pre-calculated of end of VRAM

	memset(psxVSecure,0x00,(iGPUHeight*2)*1024 + (1024*1024));
	memset(lGPUInfoVals,0x00,16*sizeof(unsigned long));

	SetFPSHandler();

	PSXDisplay.RGB24        = FALSE;                      // Initialize some stuff
	PSXDisplay.Interlaced   = FALSE;
	PSXDisplay.DrawOffset.x = 0;
	PSXDisplay.DrawOffset.y = 0;
	PSXDisplay.DisplayMode.x= 320;
	PSXDisplay.DisplayMode.y= 240;
	PreviousPSXDisplay.DisplayMode.x= 320;
	PreviousPSXDisplay.DisplayMode.y= 240;
	PSXDisplay.Disabled     = FALSE;
	PreviousPSXDisplay.Range.x0 =0;
	PreviousPSXDisplay.Range.y0 =0;
	PSXDisplay.Range.x0=0;
	PSXDisplay.Range.x1=0;
	PreviousPSXDisplay.DisplayModeNew.y=0;
	PSXDisplay.Double=1;
	lGPUdataRet=0x400;

	lLowerpart = 0;
	dwLaceCnt = 0;
	bCheckMask = 0;
	sSetMask = 0;
	lSetMask = 0;
	memset(&gpuDataM[0],0,sizeof(gpuDataM));
	gpuCommand = 0;
	gpuDataC = 0;
	gpuDataP = 0;
	DataReadMode = DR_NORMAL;
	memset(lGPUInfoVals,0x00,16*sizeof(unsigned long));
	bUsingTWin = FALSE;
	memset(&TWin,0,sizeof(TWin));
	clutid = 0;
	usMirror=0;
	iDither = 0;
	drawX=drawY=drawW=drawH=0;
	g_m1=g_m2=g_m3=0;
	DrawSemiTrans=0;
		extern short Ymin;
	extern short Ymax;
	Ymin=0;
	Ymax=0;
	ly0=lx0=ly1=lx1=ly2=lx2=ly3=lx3=0;
	GlobalTextAddrX=GlobalTextAddrY=GlobalTextTP=0;
	GlobalTextREST=GlobalTextABR=GlobalTextPAGE=0;
	InitFPS();
	// Just to be sure:  copied from GPU reset
	memset(lGPUInfoVals,0x00,16*sizeof(unsigned long));
	lGPUstatusRet=0x14802000;
	PSXDisplay.Disabled=1;
	DataWriteMode=DataReadMode=DR_NORMAL;
	PSXDisplay.DrawOffset.x=PSXDisplay.DrawOffset.y=0;
	drawX=drawY=0;
	drawW=drawH=0;
	sSetMask=0;
	lSetMask=0;
	bCheckMask=FALSE;
	usMirror=0;
	GlobalTextAddrX=0;
	GlobalTextAddrY=0;
	GlobalTextTP=0;
	GlobalTextABR=0;
	PSXDisplay.RGB24=FALSE;
	PSXDisplay.Interlaced=FALSE;
	bUsingTWin = FALSE;

	DataWriteMode = DR_NORMAL;


	// Reset transfer values, to prevent mis-transfer of data
	memset(&VRAMWrite,0,sizeof(VRAMLoad_t));
	memset(&VRAMRead,0,sizeof(VRAMLoad_t));

	// Device initialized already
	lGPUstatusRet = 0x14802000;
	GPUIsIdle;
	GPUIsReadyForCommands;
	bDoVSyncUpdate=TRUE;

	// Get a handle for kernel32.dll, and access the required export function
	
	LoadKernel32();

	return 0;
}

// Here it begins

void gpu_ReadConfig(void);

long CALLBACK GPUopen(HWND hwndGPU)                    // GPU open
{
	hWGPU = hwndGPU;                                      // Store hardware window

	if (bChangeWinMode) ReadWinSizeConfig();              // ALT and Enter toggle?
	else                                                  // Or first time startup?
	{
		gpu_ReadConfig();                                       // Read configuration
		InitFPS();
	}

	bIsFirstFrame  = TRUE;                                // We have to initialize later
	bDoVSyncUpdate = TRUE;

	ulInitDisplay();                                      // Setup DirectDraw

	if (iStopSaver)
		D_SetThreadExecutionState(ES_SYSTEM_REQUIRED|ES_DISPLAY_REQUIRED|ES_CONTINUOUS);


	return 0;
}

// Time to leave

long CALLBACK GPUclose()                               // GPU Close
{
	if (RECORD_RECORDING==TRUE)
	{
		RECORD_Stop();
		RECORD_RECORDING=FALSE;
		BuildDispMenu(0);
	}

	CloseDisplay();                                       // Shutdown DirectDraw

	if (iStopSaver)
		D_SetThreadExecutionState(ES_SYSTEM_REQUIRED|ES_DISPLAY_REQUIRED);

	return 0;
}

// I shot the sheriff (but I did not shoot the deputy) (also, why is this here? LOL)

long CALLBACK GPUshutdown()                            // GPU shutdown
{
	// Screensaver: release the handle for kernel32.dll
	FreeKernel32();

	free(psxVSecure);

	return 0;                                             // Nothing to do
}

// Update display (swap buffers)

void updateDisplay(void)                               // Update display
{
	 // We never want a clear front buffer
	/*if (PSXDisplay.Disabled)                              // Disable?
	{
		DoClearFrontBuffer();                               // Clear front buffer
		return;                                             // Done
	}*/

	if (dwActFixes&32)                                    // PC FPS calculation fix
	{
		if (UseFrameLimit) PCFrameCap();                    // Break
		if (UseFrameSkip || ulKeybits&KEY_SHOWFPS)
			PCcalcfps();
	}

	if (ulKeybits&KEY_SHOWFPS)                            // Make FPS display buffer
	{
		sprintf(szDispBuf,"FPS %06.2f",fps_cur);
	}

	if (iFastFwd)                                         // Fast forward?
	{
		static int fpscount;
		UseFrameSkip=1;
		if (iMaximumSpeed)
			bSkipNextFrame=1;

		if (!bSkipNextFrame) DoBufferSwap();                // To skip or not to skip
		if (fpscount%6 || iMaximumSpeed)                    // Skip 6/7 frames
			bSkipNextFrame = TRUE;
		else bSkipNextFrame = FALSE;
		fpscount++;
		if (fpscount >= (int)fFrameRateHz) fpscount = 0;
		return;
	}

	if (UseFrameSkip)                                     // Skip?
	{
		if (!bSkipNextFrame) DoBufferSwap();                // To skip or not to skip
		if (dwActFixes&0xa0)                                // PC FPS calculation fix/old skipping fix
		{
			if ((fps_skip < fFrameRateHz) && !(bSkipNextFrame)) // Skip max one in a row
			{
				bSkipNextFrame = TRUE;
				fps_skip=fFrameRateHz;
			}
			else bSkipNextFrame = FALSE;
		}
		else FrameSkip();
	}
	else                                                  // No skip?
	{
		DoBufferSwap();                                     // Swap
	}
}

// Roughly emulated screen centering bits (not complete)

void ChangeDispOffsetsX(void)                          // X Center
{
	long lx,l;

	if (!PSXDisplay.Range.x1) return;

	l=PreviousPSXDisplay.DisplayMode.x;

	l*=(long)PSXDisplay.Range.x1;
	l/=2560;
	lx=l;
	l&=0xfffffff8;

	if (l==PreviousPSXDisplay.Range.y1) return;           // Abusing range.y1 for
	PreviousPSXDisplay.Range.y1=(short)l;                 // storing last x range and test

	if (lx>=PreviousPSXDisplay.DisplayMode.x)
	{
		PreviousPSXDisplay.Range.x1=
		  (short)PreviousPSXDisplay.DisplayMode.x;
		PreviousPSXDisplay.Range.x0=0;
	}
	else
	{
		PreviousPSXDisplay.Range.x1=(short)l;

		PreviousPSXDisplay.Range.x0=
		  (PSXDisplay.Range.x0-500)/8;

		if (PreviousPSXDisplay.Range.x0<0)
			PreviousPSXDisplay.Range.x0=0;

		if ((PreviousPSXDisplay.Range.x0+lx)>
		    PreviousPSXDisplay.DisplayMode.x)
		{
			PreviousPSXDisplay.Range.x0=
			  (short)(PreviousPSXDisplay.DisplayMode.x-lx);
			PreviousPSXDisplay.Range.x0+=2; //???

			PreviousPSXDisplay.Range.x1+=(short)(lx-l);
		}

		DoClearScreenBuffer();
	}

	bDoVSyncUpdate=TRUE;
}

void ChangeDispOffsetsY(void)                          // Y Center
{
	int iT,iO=PreviousPSXDisplay.Range.y0;
	int iOldYOffset=PreviousPSXDisplay.DisplayModeNew.y;

// New

	if ((PreviousPSXDisplay.DisplayModeNew.x+PSXDisplay.DisplayModeNew.y)>iGPUHeight)
	{
		int dy1=iGPUHeight-PreviousPSXDisplay.DisplayModeNew.x;
		int dy2=(PreviousPSXDisplay.DisplayModeNew.x+PSXDisplay.DisplayModeNew.y)-iGPUHeight;

		if (dy1>=dy2)
		{
			PreviousPSXDisplay.DisplayModeNew.y=-dy2;
		}
		else
		{
			PSXDisplay.DisplayPosition.y=0;
			PreviousPSXDisplay.DisplayModeNew.y=-dy1;
		}
	}
	else PreviousPSXDisplay.DisplayModeNew.y=0;

// Eon

	if (PreviousPSXDisplay.DisplayModeNew.y!=iOldYOffset) // If old offset!=new offset: recalculate height
	{
		PSXDisplay.Height = PSXDisplay.Range.y1 -
		                    PSXDisplay.Range.y0 +
		                    PreviousPSXDisplay.DisplayModeNew.y;
		PSXDisplay.DisplayModeNew.y=PSXDisplay.Height*PSXDisplay.Double;
	}

	if (PSXDisplay.PAL) iT=48;
	else iT=28;

	if (PSXDisplay.Range.y0>=iT)
	{
		PreviousPSXDisplay.Range.y0=
		  (short)((PSXDisplay.Range.y0-iT-4)*PSXDisplay.Double);
		if (PreviousPSXDisplay.Range.y0<0)
			PreviousPSXDisplay.Range.y0=0;
		PSXDisplay.DisplayModeNew.y+=
		  PreviousPSXDisplay.Range.y0;
	}
	else
		PreviousPSXDisplay.Range.y0=0;

	if (iO!=PreviousPSXDisplay.Range.y0)
	{
		DoClearScreenBuffer();
	}
}

// Check if update needed

void updateDisplayIfChanged(void)                      // Update display if changed
{
	if ((PSXDisplay.DisplayMode.y == PSXDisplay.DisplayModeNew.y) &&
	    (PSXDisplay.DisplayMode.x == PSXDisplay.DisplayModeNew.x))
	{
		if ((PSXDisplay.RGB24      == PSXDisplay.RGB24New) &&
		    (PSXDisplay.Interlaced == PSXDisplay.InterlacedNew)) return;
	}

	PSXDisplay.RGB24         = PSXDisplay.RGB24New;       // Get new infos

	PSXDisplay.DisplayMode.y = PSXDisplay.DisplayModeNew.y;
	PSXDisplay.DisplayMode.x = PSXDisplay.DisplayModeNew.x;
	PreviousPSXDisplay.DisplayMode.x=                     // Previous will hold
		std::min(640L,PSXDisplay.DisplayMode.x);                   // Maximum 640x512...that's
	PreviousPSXDisplay.DisplayMode.y=                     // the size of my
	  std::min(512L,PSXDisplay.DisplayMode.y);                   // back buffer surface
	PSXDisplay.Interlaced    = PSXDisplay.InterlacedNew;

	PSXDisplay.DisplayEnd.x=                              // Calculate end of display
	  PSXDisplay.DisplayPosition.x+ PSXDisplay.DisplayMode.x;
	PSXDisplay.DisplayEnd.y=
	  PSXDisplay.DisplayPosition.y+ PSXDisplay.DisplayMode.y+PreviousPSXDisplay.DisplayModeNew.y;
	PreviousPSXDisplay.DisplayEnd.x=
	  PreviousPSXDisplay.DisplayPosition.x+ PSXDisplay.DisplayMode.x;
	PreviousPSXDisplay.DisplayEnd.y=
	  PreviousPSXDisplay.DisplayPosition.y+ PSXDisplay.DisplayMode.y+PreviousPSXDisplay.DisplayModeNew.y;

	ChangeDispOffsetsX();

	if (iFrameLimit==2) SetAutoFrameCap();                // Set it

	if (UseFrameSkip) updateDisplay();                    // Stupid stuff when frame skipping enabled
}

void ChangeWindowMode(void)                            // Toggle fullscreen or windowed mode
{
	GPUclose();
	iWindowMode=!iWindowMode;
	GPUopen(hWGPU);
	bChangeWinMode=FALSE;
	bDoVSyncUpdate=TRUE;
}

// Gun cursor function: player=0-7, x=0-511, y=0-255

void CALLBACK GPUcursor(int iPlayer,int x,int y)
{
	if (iPlayer<0) return;
	if (iPlayer>7) return;

	usCursorActive|=(1<<iPlayer);

	if (x<0)       x=0;
	if (x>511)     x=511;
	if (y<0)       y=0;
	if (y>255)     y=255;

	ptCursorPoint[iPlayer].x=x;
	ptCursorPoint[iPlayer].y=y;
}

// Update lace is called every VSync

void CALLBACK GPUupdateLace(void)                      // VSync
{
	if (!(dwActFixes&1))
		lGPUstatusRet^=0x80000000;                           // Odd/even bit

	if (!(dwActFixes&32))                                 // Standard FPS limitation?
		CheckFrameRate();

	if (!UseFrameSkip)
		updateDisplay();

//	if(PSXDisplay.Interlaced)                             // Interlaced mode?
//		{
//			if(bDoVSyncUpdate && PSXDisplay.DisplayMode.x>0 && PSXDisplay.DisplayMode.y>0)
//				{
//					updateDisplay();
//				}
//		}
//	else                                                  // Non-interlaced?
//		{
	if (dwActFixes&64)                                  // Lazy screen update fix
	{
//					if(bDoLazyUpdate && !UseFrameSkip)
//						updateDisplay();
		bDoLazyUpdate=FALSE;
	}
//			else
//				{
//					if(bDoVSyncUpdate && !UseFrameSkip)               // Some primitives drawn?
//						updateDisplay();                                 // Update display
//				}
//		}

	if (RECORD_RECORDING)
		if (RECORD_WriteFrame()==FALSE)
		{
			RECORD_RECORDING=FALSE;
			RECORD_Stop();
		}

	if (bChangeWinMode) ChangeWindowMode();               // Toggle fullscreen or windowed mode

	bDoVSyncUpdate=FALSE;                                 // VSync done
}

// Process read request from GPU status register

unsigned long CALLBACK GPUreadStatus(void)             // Read status
{
	if (dwActFixes&1)
	{
		static int iNumRead=0;                              // Odd/even hack
		if ((iNumRead++)==2)
		{
			iNumRead=0;
			lGPUstatusRet^=0x80000000;                        // Interlaced bit toggle...we do it on every 3 read status'...needed by some games (like Chrono Cross) with old ePSXe versions (1.5.2 and older)
		}
	}

// if(GetAsyncKeyState(VK_SHIFT)&32768) auxprintf("1 %08x\n",lGPUstatusRet);

	if (iFakePrimBusy)                                    // 27.10.2007 - Pete : emulating some 'busy' while drawing
	{
		iFakePrimBusy--;

		if (iFakePrimBusy&1)                                // We do a busy-idle-busy-idle sequence after/while drawing primitives
		{
			GPUIsBusy;
			GPUIsNotReadyForCommands;
		}
		else
		{
			GPUIsIdle;
			GPUIsReadyForCommands;
		}
//   auxprintf("2 %08x\n",lGPUstatusRet);
	}

	return lGPUstatusRet;
}

// Processes data sent to GPU status register
// these are always single packet commands

void CALLBACK GPUwriteStatus(unsigned long gdata)      // Write status
{
	unsigned long lCommand=(gdata>>24)&0xff;

	ulStatusControl[lCommand]=gdata;                      // Store command for freezing

	switch (lCommand)
	{
		// Reset GPU
	case 0x00:
		memset(lGPUInfoVals,0x00,16*sizeof(unsigned long));
		lGPUstatusRet=0x14802000;
		PSXDisplay.Disabled=1;
		DataWriteMode=DataReadMode=DR_NORMAL;
		PSXDisplay.DrawOffset.x=PSXDisplay.DrawOffset.y=0;
		drawX=drawY=0;
		drawW=drawH=0;
		sSetMask=0;
		lSetMask=0;
		bCheckMask=FALSE;
		usMirror=0;
		GlobalTextAddrX=0;
		GlobalTextAddrY=0;
		GlobalTextTP=0;
		GlobalTextABR=0;
		PSXDisplay.RGB24=FALSE;
		PSXDisplay.Interlaced=FALSE;
		bUsingTWin = FALSE;
		return;
		
		// Disable/enable display
		
	case 0x03:

		PreviousPSXDisplay.Disabled = PSXDisplay.Disabled;
		PSXDisplay.Disabled = (gdata & 1);

		if (PSXDisplay.Disabled)
			lGPUstatusRet|=GPUSTATUS_DISPLAYDISABLED;
		else lGPUstatusRet&=~GPUSTATUS_DISPLAYDISABLED;
		return;

		// Setting transfer mode
		
	case 0x04:
		gdata &= 0x03;                                     // Only want the lower two bits

		DataWriteMode=DataReadMode=DR_NORMAL;
		if (gdata==0x02) DataWriteMode=DR_VRAMTRANSFER;
		if (gdata==0x03) DataReadMode =DR_VRAMTRANSFER;
		lGPUstatusRet&=~GPUSTATUS_DMABITS;                 // Clear the current settings of the DMA bits
		lGPUstatusRet|=(gdata << 29);                      // Set the DMA bits according to the received data

		return;

		// Setting display position
	case 0x05:
	{
		PreviousPSXDisplay.DisplayPosition.x = PSXDisplay.DisplayPosition.x;
		PreviousPSXDisplay.DisplayPosition.y = PSXDisplay.DisplayPosition.y;

		/*
							PSXDisplay.DisplayPosition.y = (short)((gdata>>10)&0x3ff);
							if (PSXDisplay.DisplayPosition.y & 0x200)
								PSXDisplay.DisplayPosition.y |= 0xfffffc00;
							if(PSXDisplay.DisplayPosition.y<0)
								{
									PreviousPSXDisplay.DisplayModeNew.y=PSXDisplay.DisplayPosition.y/PSXDisplay.Double;
									PSXDisplay.DisplayPosition.y=0;
								}
							else PreviousPSXDisplay.DisplayModeNew.y=0;
		*/
		
// new

		if (iGPUHeight==1024)
		{
			if (dwGPUVersion==2)
				PSXDisplay.DisplayPosition.y = (short)((gdata>>12)&0x3ff);
			else PSXDisplay.DisplayPosition.y = (short)((gdata>>10)&0x3ff);
		}
		else PSXDisplay.DisplayPosition.y = (short)((gdata>>10)&0x1ff);

		// Store the same value in some helper variable, we need it in later compare code
		PreviousPSXDisplay.DisplayModeNew.x=PSXDisplay.DisplayPosition.y;

		if ((PSXDisplay.DisplayPosition.y+PSXDisplay.DisplayMode.y)>iGPUHeight)
		{
			int dy1=iGPUHeight-PSXDisplay.DisplayPosition.y;
			int dy2=(PSXDisplay.DisplayPosition.y+PSXDisplay.DisplayMode.y)-iGPUHeight;

			if (dy1>=dy2)
			{
				PreviousPSXDisplay.DisplayModeNew.y=-dy2;
			}
			else
			{
				PSXDisplay.DisplayPosition.y=0;
				PreviousPSXDisplay.DisplayModeNew.y=-dy1;
			}
		}
		else PreviousPSXDisplay.DisplayModeNew.y=0;
		
// Eon

		PSXDisplay.DisplayPosition.x = (short)(gdata & 0x3ff);
		PSXDisplay.DisplayEnd.x=
		  PSXDisplay.DisplayPosition.x+ PSXDisplay.DisplayMode.x;
		PSXDisplay.DisplayEnd.y=
		  PSXDisplay.DisplayPosition.y+ PSXDisplay.DisplayMode.y + PreviousPSXDisplay.DisplayModeNew.y;
		PreviousPSXDisplay.DisplayEnd.x=
		  PreviousPSXDisplay.DisplayPosition.x+ PSXDisplay.DisplayMode.x;
		PreviousPSXDisplay.DisplayEnd.y=
		  PreviousPSXDisplay.DisplayPosition.y+ PSXDisplay.DisplayMode.y + PreviousPSXDisplay.DisplayModeNew.y;

		bDoVSyncUpdate=TRUE;

		if (!(PSXDisplay.Interlaced))                      // Stupid frame skipping option
		{
			if (UseFrameSkip)  updateDisplay();
			if (dwActFixes&64) bDoLazyUpdate=TRUE;
		}
	}
	return;
	
	// Setting width
	
	case 0x06:

		PSXDisplay.Range.x0=(short)(gdata & 0x7ff);
		PSXDisplay.Range.x1=(short)((gdata>>12) & 0xfff);

		PSXDisplay.Range.x1-=PSXDisplay.Range.x0;

		ChangeDispOffsetsX();

		return;
		
		// Setting height
		
	case 0x07:
	{

		PSXDisplay.Range.y0=(short)(gdata & 0x3ff);
		PSXDisplay.Range.y1=(short)((gdata>>10) & 0x3ff);

		PreviousPSXDisplay.Height = PSXDisplay.Height;

		PSXDisplay.Height = PSXDisplay.Range.y1 -
		                    PSXDisplay.Range.y0 +
		                    PreviousPSXDisplay.DisplayModeNew.y;

		if (PreviousPSXDisplay.Height!=PSXDisplay.Height)
		{
			PSXDisplay.DisplayModeNew.y=PSXDisplay.Height*PSXDisplay.Double;

			ChangeDispOffsetsY();

			updateDisplayIfChanged();
		}
		return;
	}
	
	// Setting display info
	
	case 0x08:

		PSXDisplay.DisplayModeNew.x =
		  sDispWidths[(gdata & 0x03) | ((gdata & 0x40) >> 4)];

		if (gdata&0x04) PSXDisplay.Double=2;
		else            PSXDisplay.Double=1;

		PSXDisplay.DisplayModeNew.y = PSXDisplay.Height*PSXDisplay.Double;

		ChangeDispOffsetsY();

		PSXDisplay.PAL           = (gdata & 0x08)?TRUE:FALSE; // if 1 - PAL mode, else NTSC
		PSXDisplay.RGB24New      = (gdata & 0x10)?TRUE:FALSE; // if 1 - True color
		PSXDisplay.InterlacedNew = (gdata & 0x20)?TRUE:FALSE; // if 1 - Interlace

		lGPUstatusRet&=~GPUSTATUS_WIDTHBITS;                   // Clear the width bits
		lGPUstatusRet|=
		  (((gdata & 0x03) << 17) |
		   ((gdata & 0x40) << 10));                // Set the width bits

		if (PSXDisplay.InterlacedNew)
		{
			if (!PSXDisplay.Interlaced)
			{
				PreviousPSXDisplay.DisplayPosition.x = PSXDisplay.DisplayPosition.x;
				PreviousPSXDisplay.DisplayPosition.y = PSXDisplay.DisplayPosition.y;
			}
			lGPUstatusRet|=GPUSTATUS_INTERLACED;
		}
		else lGPUstatusRet&=~GPUSTATUS_INTERLACED;

		if (PSXDisplay.PAL)
			lGPUstatusRet|=GPUSTATUS_PAL;
		else lGPUstatusRet&=~GPUSTATUS_PAL;

		if (PSXDisplay.Double==2)
			lGPUstatusRet|=GPUSTATUS_DOUBLEHEIGHT;
		else lGPUstatusRet&=~GPUSTATUS_DOUBLEHEIGHT;

		if (PSXDisplay.RGB24New)
			lGPUstatusRet|=GPUSTATUS_RGB24;
		else lGPUstatusRet&=~GPUSTATUS_RGB24;

		updateDisplayIfChanged();

		return;

		// Ask about GPU version and other stuff
		
	case 0x10:

		gdata&=0xff;

		switch (gdata)
		{
		case 0x02:
			lGPUdataRet=lGPUInfoVals[INFO_TW];              // TW info
			return;
		case 0x03:
			lGPUdataRet=lGPUInfoVals[INFO_DRAWSTART];       // Draw start
			return;
		case 0x04:
			lGPUdataRet=lGPUInfoVals[INFO_DRAWEND];         // Draw end
			return;
		case 0x05:
		case 0x06:
			lGPUdataRet=lGPUInfoVals[INFO_DRAWOFF];         // Draw offset
			return;
		case 0x07:
			if (dwGPUVersion==2)
				lGPUdataRet=0x01;
			else lGPUdataRet=0x02;                          // GPU type
			return;
		case 0x08:
		case 0x0F:                                       // Some BIOS addr?
			lGPUdataRet=0xBFC03720;
			return;
		}
		return;

	}
}

// VRAM read/write helpers, needed by LEWPY's optimized VRAM read/write

__inline void FinishedVRAMWrite(void)
{
	/*
	// NEWX
		if(!PSXDisplay.Interlaced && UseFrameSkip)            // Stupid frame skipping
			{
				VRAMWrite.Width +=VRAMWrite.x;
				VRAMWrite.Height+=VRAMWrite.y;
				if(VRAMWrite.x<PSXDisplay.DisplayEnd.x &&
							VRAMWrite.Width >=PSXDisplay.DisplayPosition.x &&
							VRAMWrite.y<PSXDisplay.DisplayEnd.y &&
							VRAMWrite.Height>=PSXDisplay.DisplayPosition.y)
					updateDisplay();
			}
	*/

	// Set register to normal operation
	DataWriteMode = DR_NORMAL;
	// Reset transfer values, to prevent mis-transfer of data
	VRAMWrite.x = 0;
	VRAMWrite.y = 0;
	VRAMWrite.Width = 0;
	VRAMWrite.Height = 0;
	VRAMWrite.ColsRemaining = 0;
	VRAMWrite.RowsRemaining = 0;
}

__inline void FinishedVRAMRead(void)
{
	// Set register to normal operation
	DataReadMode = DR_NORMAL;
	// Reset transfer values, to prevent mis-transfer of data
	VRAMRead.x = 0;
	VRAMRead.y = 0;
	VRAMRead.Width = 0;
	VRAMRead.Height = 0;
	VRAMRead.ColsRemaining = 0;
	VRAMRead.RowsRemaining = 0;

	// Indicate GPU is no longer ready for VRAM data in the STATUS REGISTER
	lGPUstatusRet&=~GPUSTATUS_READYFORVRAM;
}

// Core read from VRAM

void CALLBACK GPUreadDataMem(u32 * pMem, int iSize)
{
	int i;

	if (DataReadMode!=DR_VRAMTRANSFER) return;

	GPUIsBusy;

	// Adjust read ptr, if necessary
	
	while (VRAMRead.ImagePtr>=psxVuw_eom)
		VRAMRead.ImagePtr-=iGPUHeight*1024;
	while (VRAMRead.ImagePtr<psxVuw)
		VRAMRead.ImagePtr+=iGPUHeight*1024;

	for (i=0;i<iSize;i++)
	{
		// Do 2 separate 16-bit reads for compatibility (wrap issues)
		
		if ((VRAMRead.ColsRemaining > 0) && (VRAMRead.RowsRemaining > 0))
		{
			// Lower 16-bit
			
			lGPUdataRet=(unsigned long)*VRAMRead.ImagePtr;

			VRAMRead.ImagePtr++;
			if (VRAMRead.ImagePtr>=psxVuw_eom) VRAMRead.ImagePtr-=iGPUHeight*1024;
			VRAMRead.RowsRemaining --;

			if (VRAMRead.RowsRemaining<=0)
			{
				VRAMRead.RowsRemaining = VRAMRead.Width;
				VRAMRead.ColsRemaining--;
				VRAMRead.ImagePtr += 1024 - VRAMRead.Width;
				if (VRAMRead.ImagePtr>=psxVuw_eom) VRAMRead.ImagePtr-=iGPUHeight*1024;
			}

			// Higher 16-bit (always, even if it's an odd width)
			
			lGPUdataRet|=(unsigned long)(*VRAMRead.ImagePtr)<<16;

			*pMem++=lGPUdataRet;

			if (VRAMRead.ColsRemaining <= 0)
			{
				FinishedVRAMRead();
				goto ENDREAD;
			}

			VRAMRead.ImagePtr++;
			if (VRAMRead.ImagePtr>=psxVuw_eom) VRAMRead.ImagePtr-=iGPUHeight*1024;
			VRAMRead.RowsRemaining--;
			if (VRAMRead.RowsRemaining<=0)
			{
				VRAMRead.RowsRemaining = VRAMRead.Width;
				VRAMRead.ColsRemaining--;
				VRAMRead.ImagePtr += 1024 - VRAMRead.Width;
				if (VRAMRead.ImagePtr>=psxVuw_eom) VRAMRead.ImagePtr-=iGPUHeight*1024;
			}
			if (VRAMRead.ColsRemaining <= 0)
			{
				FinishedVRAMRead();
				goto ENDREAD;
			}
		}
		else
		{
			FinishedVRAMRead();
			goto ENDREAD;
		}
	}

ENDREAD:
	GPUIsIdle;
}

unsigned long CALLBACK GPUreadData(void)
{
	u32 l;
	GPUreadDataMem(&l,1);
	return (long)l;
}

// Processes data send to GPU data register
// extra table entries for fixing polyline troubles

const unsigned char primTableCX[256] =
{
	// 00
	0,0,3,0,0,0,0,0,
	// 08
	0,0,0,0,0,0,0,0,
	// 10
	0,0,0,0,0,0,0,0,
	// 18
	0,0,0,0,0,0,0,0,
	// 20
	4,4,4,4,7,7,7,7,
	// 28
	5,5,5,5,9,9,9,9,
	// 30
	6,6,6,6,9,9,9,9,
	// 38
	8,8,8,8,12,12,12,12,
	// 40
	3,3,3,3,0,0,0,0,
	// 48
//  5,5,5,5,6,6,6,6,    // FLINE
	254,254,254,254,254,254,254,254,
	// 50
	4,4,4,4,0,0,0,0,
	// 58
//  7,7,7,7,9,9,9,9,    // GLINE
	255,255,255,255,255,255,255,255,
	// 60
	3,3,3,3,4,4,4,4,
	// 68
	2,2,2,2,3,3,3,3,    // 3=SPRITE1?
	// 70
	2,2,2,2,3,3,3,3,
	// 78
	2,2,2,2,3,3,3,3,
	// 80
	4,0,0,0,0,0,0,0,
	// 88
	0,0,0,0,0,0,0,0,
	// 90
	0,0,0,0,0,0,0,0,
	// 98
	0,0,0,0,0,0,0,0,
	// a0
	3,0,0,0,0,0,0,0,
	// a8
	0,0,0,0,0,0,0,0,
	// b0
	0,0,0,0,0,0,0,0,
	// b8
	0,0,0,0,0,0,0,0,
	// c0
	3,0,0,0,0,0,0,0,
	// c8
	0,0,0,0,0,0,0,0,
	// d0
	0,0,0,0,0,0,0,0,
	// d8
	0,0,0,0,0,0,0,0,
	// e0
	0,1,1,1,1,1,1,0,
	// e8
	0,0,0,0,0,0,0,0,
	// f0
	0,0,0,0,0,0,0,0,
	// f8
	0,0,0,0,0,0,0,0
};

void CALLBACK GPUwriteDataMem(u32* pMem, int iSize)
{
	unsigned char command;
	unsigned long gdata=0;
	int i=0;

	GPUIsBusy;
	GPUIsNotReadyForCommands;

STARTVRAM:

	if (DataWriteMode==DR_VRAMTRANSFER)
	{
		BOOL bFinished=FALSE;

		// Make sure we are in VRAM
		while (VRAMWrite.ImagePtr>=psxVuw_eom)
			VRAMWrite.ImagePtr-=iGPUHeight*1024;
		while (VRAMWrite.ImagePtr<psxVuw)
			VRAMWrite.ImagePtr+=iGPUHeight*1024;

		// Now do the loop
		while (VRAMWrite.ColsRemaining>0)
		{
			while (VRAMWrite.RowsRemaining>0)
			{
				if (i>=iSize)
				{
					goto ENDVRAM;
				}
				i++;

				gdata=*pMem++;

				*VRAMWrite.ImagePtr++ = (unsigned short)gdata;
				if (VRAMWrite.ImagePtr>=psxVuw_eom) VRAMWrite.ImagePtr-=iGPUHeight*1024;
				VRAMWrite.RowsRemaining --;

				if (VRAMWrite.RowsRemaining <= 0)
				{
					VRAMWrite.ColsRemaining--;
					if (VRAMWrite.ColsRemaining <= 0)             // Last pixel is odd width
					{
						gdata=(gdata&0xFFFF)|(((unsigned long)(*VRAMWrite.ImagePtr))<<16);
						FinishedVRAMWrite();
						bDoVSyncUpdate=TRUE;
						goto ENDVRAM;
					}
					VRAMWrite.RowsRemaining = VRAMWrite.Width;
					VRAMWrite.ImagePtr += 1024 - VRAMWrite.Width;
				}

				*VRAMWrite.ImagePtr++ = (unsigned short)(gdata>>16);
				if (VRAMWrite.ImagePtr>=psxVuw_eom) VRAMWrite.ImagePtr-=iGPUHeight*1024;
				VRAMWrite.RowsRemaining --;
			}

			VRAMWrite.RowsRemaining = VRAMWrite.Width;
			VRAMWrite.ColsRemaining--;
			VRAMWrite.ImagePtr += 1024 - VRAMWrite.Width;
			bFinished=TRUE;
		}

		FinishedVRAMWrite();
		if (bFinished) bDoVSyncUpdate=TRUE;
	}

ENDVRAM:

	if (DataWriteMode==DR_NORMAL)
	{
		void (* *primFunc)(unsigned char *);
		/*if (bSkipNextFrame)
			primFunc=primTableSkip; // causes desync if frame skip is enabled
		else*/
			primFunc=primTableJ;

		for (;i<iSize;)
		{
			if (DataWriteMode==DR_VRAMTRANSFER) goto STARTVRAM;

			gdata=*pMem++;
			i++;

			if (gpuDataC == 0)
			{
				command = (unsigned char)((gdata>>24) & 0xff);

//if(command>=0xb0 && command<0xc0) auxprintf("b0 %x!!!!!!!!!\n",command);

				if (primTableCX[command])
				{
					gpuDataC = primTableCX[command];
					gpuCommand = command;
					gpuDataM[0] = gdata;
					gpuDataP = 1;
				}
				else continue;
			}
			else
			{
				gpuDataM[gpuDataP] = gdata;
				if (gpuDataC>128)
				{
					if ((gpuDataC==254 && gpuDataP>=3) ||
					    (gpuDataC==255 && gpuDataP>=4 && !(gpuDataP&1)))
					{
						if ((gpuDataM[gpuDataP] & 0xF000F000) == 0x50005000)
							gpuDataP=gpuDataC-1;
					}
				}
				gpuDataP++;
			}

			if (gpuDataP == gpuDataC)
			{
				gpuDataC=gpuDataP=0;
				primFunc[gpuCommand]((unsigned char *)gpuDataM);

				if (dwEmuFixes&0x0001 || dwActFixes&0x0400)     // Hack for emulating "GPU busy" in some games (try to remove in the future)
					iFakePrimBusy=4;
			}
		}
	}

	lGPUdataRet=gdata;

	GPUIsReadyForCommands;
	GPUIsIdle;
}

////////////////////////////////////////////////////////////////////////

void CALLBACK GPUwriteData(unsigned long gdata)
{
	u32 temp = gdata;
	GPUwriteDataMem(&temp,1);
}

// These functions will be removed soon...not really needed, but some emulators want them (they will be removed later)

void CALLBACK GPUsetMode(unsigned long gdata)
{
	
// Peops does nothing here
// DataWriteMode=(gdata&1)?DR_VRAMTRANSFER:DR_NORMAL;
// DataReadMode =(gdata&2)?DR_VRAMTRANSFER:DR_NORMAL;

}

long CALLBACK GPUgetMode(void)
{
	long iT=0;

	if (DataWriteMode==DR_VRAMTRANSFER) iT|=0x1;
	if (DataReadMode ==DR_VRAMTRANSFER) iT|=0x2;
	return iT;
}

// Call configuration dialog

long CALLBACK GPUconfigure(void)
{
	HWND hWP=GetActiveWindow();
	DialogBox(hInst,MAKEINTRESOURCE(IDD_CFGSOFT),
	          hWP,(DLGPROC)SoftDlgProc);
	return 0;
}

// Sets all kind of ACT fixes

void SetFixes(void)
{
	BOOL bOldPerformanceCounter=IsPerformanceCounter;    // Store current timer mode

	if (dwActFixes&0x10)                                 // Check fix 0x10
		IsPerformanceCounter=FALSE;
	else SetFPSHandler();

	if (bOldPerformanceCounter!=IsPerformanceCounter)    // We have to change it?
		InitFPS();                                          // Initialize FPS again

	if (dwActFixes&0x02) sDispWidths[4]=384;
	else                sDispWidths[4]=368;
}

// Process GPU commands

unsigned long lUsedAddr[3];

__inline BOOL CheckForEndlessLoop(unsigned long laddr)
{
	if (laddr==lUsedAddr[1]) return TRUE;
	if (laddr==lUsedAddr[2]) return TRUE;

	if (laddr<lUsedAddr[0]) lUsedAddr[1]=laddr;
	else                   lUsedAddr[2]=laddr;
	lUsedAddr[0]=laddr;
	return FALSE;
}

long CALLBACK GPUdmaChain(u32* baseAddrL, unsigned long addr)
{
	unsigned long dmaMem;
	unsigned char * baseAddrB;
	short count;
	unsigned int DMACommandCounter = 0;

	GPUIsBusy;

	lUsedAddr[0]=lUsedAddr[1]=lUsedAddr[2]=0xffffff;

	baseAddrB = (unsigned char*) baseAddrL;

	do
	{
		if (iGPUHeight==512) addr&=0x1FFFFC;
		if (DMACommandCounter++ > 2000000) break;
		if (CheckForEndlessLoop(addr)) break;

		count = baseAddrB[addr+3];

		dmaMem=addr+4;

		if (count>0) GPUwriteDataMem((u32*)&baseAddrL[dmaMem>>2],count);

		addr = baseAddrL[addr>>2]&0xffffff;
	}
	while (addr != 0xffffff);

	GPUIsIdle;

	return 0;
}

// Show about dialog

BOOL CALLBACK AboutDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hW,TRUE);
			return TRUE;
		}
	}
	}
	return FALSE;
}

void CALLBACK GPUabout(void)                           // About
{
	HWND hWP=GetActiveWindow();                           // To be sure
	DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),
	          hWP,(DLGPROC)AboutDlgProc);
	return;
}

// We are fine

long CALLBACK GPUtest(void)
{
	// If test fails, this function should return negative value for error (unable to continue)
	// and positive value for warning (can continue but output might be wrong)
	return 0;
}

// Freeze

struct FreezeExtra
{
	long           lLowerpart;
	BOOL           bCheckMask;
	unsigned short sSetMask;
	unsigned long  lSetMask;

	long       lGPUdataRet;
	long              lGPUstatusRet;

	unsigned   long gpuDataM[256];
	unsigned   char gpuCommand;
	long       gpuDataC;
	long       gpuDataP;
	VRAMLoad_t        VRAMWrite;
	VRAMLoad_t        VRAMRead;
	DATAREGISTERMODES DataWriteMode;
	DATAREGISTERMODES DataReadMode;
	DWORD             dwLaceCnt;

	PSXDisplay_t      PSXDisplay;
	PSXDisplay_t      PreviousPSXDisplay; 

	unsigned long     lGPUInfoVals[16];


	BOOL           bUsingTWin;
	TWin_t         TWin;
	unsigned long  clutid;                                 // Global clut
	unsigned short usMirror;                             // Sprite mirror
	int            iDither;
	long           drawX;
	long           drawY;
	long           drawW;
	long           drawH;

	short g_m1,g_m2,g_m3;
	short DrawSemiTrans;
	short Ymin;
	short Ymax;

	short          ly0,lx0,ly1,lx1,ly2,lx2,ly3,lx3;        // Global PS1 vertex coordinates
	long           GlobalTextAddrX,GlobalTextAddrY,GlobalTextTP; 
	long           GlobalTextREST,GlobalTextABR,GlobalTextPAGE;
};

// VRAMWrite.ImagePtr and VRAMRead.ImagePtr point near to psxVuw
// We don't want to store pointers into the savestate, so the savestate
// stores (ImagePtr - psxVuw) instead of ImagePtr
// The pointers start out NULL, so we treat that as a special case

#define NULL_IMAGEPTR 0x80000000U
void vuw_to_int(unsigned short **ptr) {
	if (*ptr == NULL) {
		*ptr = (unsigned short *)NULL_IMAGEPTR;
	} else {
		*ptr = (unsigned short *)(*ptr - psxVuw);
	}
}

void int_to_vuw(unsigned short **ptr) {
	if (u32(*ptr) == NULL_IMAGEPTR) {
		*ptr = NULL;
	} else {
		*ptr = psxVuw + s32(*ptr);
	}
}

void FreezeExtra_save(struct FreezeExtra* extra)
{
	extern short g_m1,g_m2,g_m3;
	extern short DrawSemiTrans;
	extern short Ymin;
	extern short Ymax;

	extern short          ly0,lx0,ly1,lx1,ly2,lx2,ly3,lx3;        // Global PS1 vertex coordinates
	extern long           GlobalTextAddrX,GlobalTextAddrY,GlobalTextTP; //!
	extern long           GlobalTextREST,GlobalTextABR,GlobalTextPAGE;

	extra->lLowerpart = lLowerpart;
	extra->bCheckMask= bCheckMask;
	extra->sSetMask = sSetMask;
	extra->lSetMask = lSetMask;

	extra->lGPUdataRet = lGPUdataRet;
	extra->lGPUstatusRet = lGPUstatusRet;

	memcpy(extra->gpuDataM,gpuDataM,sizeof(gpuDataM));
	extra->gpuCommand = gpuCommand;
	extra->gpuDataC = gpuDataC;
	extra->gpuDataP = gpuDataP;
	extra->VRAMWrite = VRAMWrite;
	extra->VRAMRead = VRAMRead;
	vuw_to_int(&extra->VRAMWrite.ImagePtr);
	vuw_to_int(&extra->VRAMRead.ImagePtr);
	extra->DataWriteMode = DataWriteMode;
	extra->DataReadMode = DataReadMode;

	//zero 05-mar-2011 - We don't need to save this, it is intimately tied to the throttle and isn't part of the game state, even though it sounds like something that may be a HW attribute
	//extra->dwLaceCnt = dwLaceCnt;
	extra->dwLaceCnt = 0;

	extra->PSXDisplay = PSXDisplay;
	extra->PreviousPSXDisplay = PreviousPSXDisplay; 

	memcpy(extra->lGPUInfoVals,lGPUInfoVals,sizeof(lGPUInfoVals));


	extra->bUsingTWin = bUsingTWin;
	extra->TWin = TWin;
	extra->clutid = clutid;                                 // Global clut
	extra->usMirror = usMirror;                             // Sprite mirror
	extra->iDither = iDither;
	extra->drawX = drawX;
	extra->drawY = drawY;
	extra->drawW = drawW;
	extra->drawH = drawH;

	extra->g_m1 = g_m1;
	extra->g_m2 = g_m2;
	extra->g_m3 = g_m3;
	extra->DrawSemiTrans = DrawSemiTrans;
	extra->Ymin = Ymin;
	extra->Ymax = Ymax;

	extra->ly0 = ly0;
	extra->lx0 = lx0;
	extra->ly1 = ly1;
	extra->lx1 = lx1;
	extra->ly2 = ly2;
	extra->lx2 = lx2;
	extra->ly3 = ly3;
	extra->lx3 = lx3;
	extra->GlobalTextAddrX = GlobalTextAddrX;
	extra->GlobalTextAddrY = GlobalTextAddrY;
	extra->GlobalTextTP = GlobalTextTP; 
	extra->GlobalTextREST = GlobalTextREST;
	extra->GlobalTextABR = GlobalTextABR;
	extra->GlobalTextPAGE = GlobalTextPAGE;
};

void FreezeExtra_load(struct FreezeExtra* extra)
{
	extern short g_m1,g_m2,g_m3;
	extern short DrawSemiTrans;
	extern short Ymin;
	extern short Ymax;

	extern short          ly0,lx0,ly1,lx1,ly2,lx2,ly3,lx3;        // Global PS1 vertex coordinates
	extern long           GlobalTextAddrX,GlobalTextAddrY,GlobalTextTP; //!
	extern long           GlobalTextREST,GlobalTextABR,GlobalTextPAGE;

	lLowerpart = extra->lLowerpart;
	bCheckMask= extra->bCheckMask;
	sSetMask = extra->sSetMask;
	lSetMask = extra->lSetMask;

	lGPUdataRet = extra->lGPUdataRet;
	lGPUstatusRet = extra->lGPUstatusRet;

	memcpy(gpuDataM,extra->gpuDataM,sizeof(gpuDataM));
	gpuCommand = extra->gpuCommand;
	gpuDataC = extra->gpuDataC;
	gpuDataP = extra->gpuDataP;
	VRAMWrite = extra->VRAMWrite;
	VRAMRead = extra->VRAMRead;
	int_to_vuw(&VRAMWrite.ImagePtr);
	int_to_vuw(&VRAMRead.ImagePtr);
	DataWriteMode = extra->DataWriteMode;
	DataReadMode = extra->DataReadMode;
	
	// Don't save/restore dwLaceCnt
	// dwLaceCnt = extra->dwLaceCnt;

	PSXDisplay = extra->PSXDisplay;
	PreviousPSXDisplay = extra->PreviousPSXDisplay; 

	memcpy(lGPUInfoVals,extra->lGPUInfoVals,sizeof(lGPUInfoVals));


	bUsingTWin = extra->bUsingTWin;
	TWin = extra->TWin;
	clutid = extra->clutid;                                 // Global clut
	usMirror = extra->usMirror;                             // Sprite mirror
	iDither = extra->iDither;
	drawX = extra->drawX;
	drawY = extra->drawY;
	drawW = extra->drawW;
	drawH = extra->drawH;

	g_m1 = extra->g_m1;
	g_m2 = extra->g_m2;
	g_m3 = extra->g_m3;
	DrawSemiTrans = extra->DrawSemiTrans;
	Ymin = extra->Ymin;
	Ymax = extra->Ymax;

	ly0 = extra->ly0;
	lx0 = extra->lx0;
	ly1 = extra->ly1;
	lx1 = extra->lx1;
	ly2 = extra->ly2;
	lx2 = extra->lx2;
	ly3 = extra->ly3;
	lx3 = extra->lx3;
	GlobalTextAddrX = extra->GlobalTextAddrX;
	GlobalTextAddrY = extra->GlobalTextAddrY;
	GlobalTextTP = extra->GlobalTextTP; 
	GlobalTextREST = extra->GlobalTextREST;
	GlobalTextABR = extra->GlobalTextABR;
	GlobalTextPAGE = extra->GlobalTextPAGE;
};

long CALLBACK GPUfreeze(unsigned long ulGetFreezeData,GPUFreeze_t * pF)
{
	if(ulGetFreezeData==3)
	{
		free(pF->extraData);
		return 1;
	}

	if (ulGetFreezeData==2)                               // 2: info, which save slot is selected (just for display)?
	{
		long lSlotNum=*((long *)pF);
		if (lSlotNum<0) return 0;
		if (lSlotNum>8) return 0;
		lSelectedSlot=lSlotNum+1;
		BuildDispMenu(0);
		return 1;
	}

	if (!pF)                    return 0;                 // Some checks
	if (pF->ulFreezeVersion!=1) return 0;

	if (ulGetFreezeData==1)                               // 1: Get data
	{
		pF->ulStatus=lGPUstatusRet;
		memcpy(pF->ulControl,ulStatusControl,256*sizeof(unsigned long));
		memcpy(pF->psxVRam,  psxVub,         1024*iGPUHeight*2);
		pF->extraDataSize = sizeof(struct FreezeExtra);
		pF->extraData = malloc(pF->extraDataSize);
		memset(pF->extraData, 0, pF->extraDataSize);
		FreezeExtra_save((struct FreezeExtra*)pF->extraData);

		return 1;
	}

	if (ulGetFreezeData!=0) return 0;                     // 0: Set data

	// loadstate:

	lGPUstatusRet=pF->ulStatus;
	memcpy(ulStatusControl,pF->ulControl,256*sizeof(unsigned long));
	memcpy(psxVub,         pF->psxVRam,  1024*iGPUHeight*2);

	FreezeExtra_load((struct FreezeExtra*)pF->extraData);

// RESET TEXTURE STORE HERE, IF YOU USE SOMETHING LIKE THAT

	//GPUwriteStatus(ulStatusControl[0]);
	//GPUwriteStatus(ulStatusControl[1]);
	//GPUwriteStatus(ulStatusControl[2]);
	//GPUwriteStatus(ulStatusControl[3]);
	//GPUwriteStatus(ulStatusControl[8]);                   // Try to repair things
	//GPUwriteStatus(ulStatusControl[6]);
	//GPUwriteStatus(ulStatusControl[7]);
	//GPUwriteStatus(ulStatusControl[5]);
	//GPUwriteStatus(ulStatusControl[4]);

	return 1;
}

// Save state display stuff

// font 0-9, 24x20 pixels, 1 byte = 4 dots
// 00 = black
// 01 = white
// 10 = red
// 11 = transparent

unsigned char cFont[10][120]=
{
// 0
	{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
		0x80,0x00,0x00,0x00,0x00,0x00,
		0x80,0x00,0x00,0x00,0x00,0x00,
		0x80,0x00,0x00,0x00,0x00,0x00,
		0x80,0x00,0x00,0x00,0x00,0x00,
		0x80,0x00,0x05,0x54,0x00,0x00,
		0x80,0x00,0x14,0x05,0x00,0x00,
		0x80,0x00,0x14,0x05,0x00,0x00,
		0x80,0x00,0x14,0x05,0x00,0x00,
		0x80,0x00,0x14,0x05,0x00,0x00,
		0x80,0x00,0x14,0x05,0x00,0x00,
		0x80,0x00,0x14,0x05,0x00,0x00,
		0x80,0x00,0x14,0x05,0x00,0x00,
		0x80,0x00,0x14,0x05,0x00,0x00,
		0x80,0x00,0x05,0x54,0x00,0x00,
		0x80,0x00,0x00,0x00,0x00,0x00,
		0x80,0x00,0x00,0x00,0x00,0x00,
		0x80,0x00,0x00,0x00,0x00,0x00,
		0x80,0x00,0x00,0x00,0x00,0x00,
		0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
	},
// 1
	{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x50,0x00,0x00,
	 0x80,0x00,0x05,0x50,0x00,0x00,
	 0x80,0x00,0x00,0x50,0x00,0x00,
	 0x80,0x00,0x00,0x50,0x00,0x00,
	 0x80,0x00,0x00,0x50,0x00,0x00,
	 0x80,0x00,0x00,0x50,0x00,0x00,
	 0x80,0x00,0x00,0x50,0x00,0x00,
	 0x80,0x00,0x00,0x50,0x00,0x00,
	 0x80,0x00,0x00,0x50,0x00,0x00,
	 0x80,0x00,0x05,0x55,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
	},
// 2
	{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x05,0x54,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x00,0x05,0x00,0x00,
	 0x80,0x00,0x00,0x05,0x00,0x00,
	 0x80,0x00,0x00,0x14,0x00,0x00,
	 0x80,0x00,0x00,0x50,0x00,0x00,
	 0x80,0x00,0x01,0x40,0x00,0x00,
	 0x80,0x00,0x05,0x00,0x00,0x00,
	 0x80,0x00,0x14,0x00,0x00,0x00,
	 0x80,0x00,0x15,0x55,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
	},
// 3
	{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x05,0x54,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x00,0x05,0x00,0x00,
	 0x80,0x00,0x00,0x05,0x00,0x00,
	 0x80,0x00,0x01,0x54,0x00,0x00,
	 0x80,0x00,0x00,0x05,0x00,0x00,
	 0x80,0x00,0x00,0x05,0x00,0x00,
	 0x80,0x00,0x00,0x05,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x05,0x54,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
	},
// 4
	{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x14,0x00,0x00,
	 0x80,0x00,0x00,0x54,0x00,0x00,
	 0x80,0x00,0x01,0x54,0x00,0x00,
	 0x80,0x00,0x01,0x54,0x00,0x00,
	 0x80,0x00,0x05,0x14,0x00,0x00,
	 0x80,0x00,0x14,0x14,0x00,0x00,
	 0x80,0x00,0x15,0x55,0x00,0x00,
	 0x80,0x00,0x00,0x14,0x00,0x00,
	 0x80,0x00,0x00,0x14,0x00,0x00,
	 0x80,0x00,0x00,0x55,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
	},
// 5
	{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x15,0x55,0x00,0x00,
	 0x80,0x00,0x14,0x00,0x00,0x00,
	 0x80,0x00,0x14,0x00,0x00,0x00,
	 0x80,0x00,0x14,0x00,0x00,0x00,
	 0x80,0x00,0x15,0x54,0x00,0x00,
	 0x80,0x00,0x00,0x05,0x00,0x00,
	 0x80,0x00,0x00,0x05,0x00,0x00,
	 0x80,0x00,0x00,0x05,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x05,0x54,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
	},
// 6
	{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x01,0x54,0x00,0x00,
	 0x80,0x00,0x05,0x00,0x00,0x00,
	 0x80,0x00,0x14,0x00,0x00,0x00,
	 0x80,0x00,0x14,0x00,0x00,0x00,
	 0x80,0x00,0x15,0x54,0x00,0x00,
	 0x80,0x00,0x15,0x05,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x05,0x54,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
	},
// 7
	{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x15,0x55,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x00,0x14,0x00,0x00,
	 0x80,0x00,0x00,0x14,0x00,0x00,
	 0x80,0x00,0x00,0x50,0x00,0x00,
	 0x80,0x00,0x00,0x50,0x00,0x00,
	 0x80,0x00,0x01,0x40,0x00,0x00,
	 0x80,0x00,0x01,0x40,0x00,0x00,
	 0x80,0x00,0x05,0x00,0x00,0x00,
	 0x80,0x00,0x05,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
	},
// 8
	{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x05,0x54,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x05,0x54,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x05,0x54,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
	},
// 9
	{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x05,0x54,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x14,0x05,0x00,0x00,
	 0x80,0x00,0x14,0x15,0x00,0x00,
	 0x80,0x00,0x05,0x55,0x00,0x00,
	 0x80,0x00,0x00,0x05,0x00,0x00,
	 0x80,0x00,0x00,0x05,0x00,0x00,
	 0x80,0x00,0x00,0x14,0x00,0x00,
	 0x80,0x00,0x05,0x50,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0x80,0x00,0x00,0x00,0x00,0x00,
	 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
	}
};

void PaintPicDot(unsigned char * p,unsigned char c)
{

	if (c==0)
	{
		*p++=0x00;  // black
		*p++=0x00;
		*p=0x00;
		return;
	}
	if (c==1)
	{
		*p++=0xff;  // white
		*p++=0xff;
		*p=0xff;
		return;
	}
	if (c==2)
	{
		*p++=0x00;  // red
		*p++=0x00;
		*p=0xff;
		return;
	}
	// transparent
}

// The main emulator allocates 128x96x3 bytes, and passes a ptr
// to it in pMem...the plugin has to fill it with
// 8-8-8 bit BGR screen data (Windows 24-bit BMP format
// without header
// Beware: the function can be called at any time,
// so you have to use the front buffer to get a fully
// rendered picture

void CALLBACK GPUgetScreenPic(unsigned char * pMem)
{
	HRESULT ddrval;
	DDSURFACEDESC xddsd;
	unsigned char * pf;
	int x,y,c,v,iCol;
	RECT r,rt;
	float XS,YS;

	// Pete: creating a temporary surface, blitting primary surface into it, getting data from temporary, and finally delete temporary...
	// seems to be better in Vista (a Vista hack should be removed as soon as possible, even if it isn't that bad)
	DDPIXELFORMAT dd;
	LPDIRECTDRAWSURFACE DDSSave;

	memset(&xddsd, 0, sizeof(DDSURFACEDESC));
	xddsd.dwSize = sizeof(DDSURFACEDESC);
	xddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	xddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	xddsd.dwWidth        = iResX;
	xddsd.dwHeight       = iResY;

	if (IDirectDraw_CreateSurface(DX.DD,&xddsd, &DDSSave, NULL)) // Create temp surface
		return;

	dd.dwSize=sizeof(DDPIXELFORMAT);                      // Check out, what color we have
	IDirectDrawSurface_GetPixelFormat(DDSSave,&dd);

	if (dd.dwRBitMask==0x00007c00 &&
	    dd.dwGBitMask==0x000003e0 &&
	    dd.dwBBitMask==0x0000001f)       iCol=15;
	else
		if (dd.dwRBitMask==0x0000f800 &&
		    dd.dwGBitMask==0x000007e0 &&
		    dd.dwBBitMask==0x0000001f)       iCol=16;
		else                                iCol=32;

	r.left=0;
	r.right =iResX;                             // Get blitting rectangles
	r.top=0;
	r.bottom=iResY;
	rt.left=0;
	rt.right =iResX;
	rt.top=0;
	rt.bottom=iResY;
	if (iWindowMode)
	{
		POINT Point={0,0};
		ClientToScreen(DX.hWnd,&Point);
		rt.left+=Point.x;
		rt.right+=Point.x;
		rt.top+=Point.y;
		rt.bottom+=Point.y;
	}

	IDirectDrawSurface_Blt(DDSSave,&r,DX.DDSPrimary,&rt,  // And blit from primary into temporary
	                       DDBLT_WAIT,NULL);

	memset(&xddsd, 0, sizeof(DDSURFACEDESC));
	xddsd.dwSize   = sizeof(DDSURFACEDESC);
	xddsd.dwFlags  = DDSD_WIDTH | DDSD_HEIGHT;
	xddsd.dwWidth  = iResX;
	xddsd.dwHeight = iResY;

	XS=(float)iResX/128;
	YS=(float)iResY/96;

	ddrval=IDirectDrawSurface_Lock(DDSSave,NULL, &xddsd, DDLOCK_WAIT|DDLOCK_READONLY, NULL);

	if (ddrval==DDERR_SURFACELOST) IDirectDrawSurface_Restore(DDSSave);

	pf=pMem;

	if (ddrval==DD_OK)
	{
		unsigned char * ps=(unsigned char *)xddsd.lpSurface;

		if (iCol==16)
		{
			unsigned short sx;
			for (y=0;y<96;y++)
			{
				for (x=0;x<128;x++)
				{
					sx=*((unsigned short *)((ps)+
					                        r.top*xddsd.lPitch+
					                        (((int)((float)y*YS))*xddsd.lPitch)+
					                        r.left*2+
					                        ((int)((float)x*XS))*2));
					*(pf+0)=(sx&0x1f)<<3;
					*(pf+1)=(sx&0x7e0)>>3;
					*(pf+2)=(sx&0xf800)>>8;
					pf+=3;
				}
			}
		}
		else
			if (iCol==15)
			{
				unsigned short sx;
				for (y=0;y<96;y++)
				{
					for (x=0;x<128;x++)
					{
						sx=*((unsigned short *)((ps)+
						                        r.top*xddsd.lPitch+
						                        (((int)((float)y*YS))*xddsd.lPitch)+
						                        r.left*2+
						                        ((int)((float)x*XS))*2));
						*(pf+0)=(sx&0x1f)<<3;
						*(pf+1)=(sx&0x3e0)>>2;
						*(pf+2)=(sx&0x7c00)>>7;
						pf+=3;
					}
				}
			}
			else
			{
				unsigned long sx;
				for (y=0;y<96;y++)
				{
					for (x=0;x<128;x++)
					{
						sx=*((unsigned long *)((ps)+
						                       r.top*xddsd.lPitch+
						                       (((int)((float)y*YS))*xddsd.lPitch)+
						                       r.left*4+
						                       ((int)((float)x*XS))*4));
						*(pf+0)=(unsigned char)((sx&0xff));
						*(pf+1)=(unsigned char)((sx&0xff00)>>8);
						*(pf+2)=(unsigned char)((sx&0xff0000)>>16);
						pf+=3;
					}
				}
			}
	}

	IDirectDrawSurface_Unlock(DDSSave,&xddsd);
	IDirectDrawSurface_Release(DDSSave);

	/*
		HRESULT ddrval;DDSURFACEDESC xddsd;unsigned char * pf;
		int x,y,c,v;RECT r;
		float XS,YS;

		memset(&xddsd, 0, sizeof(DDSURFACEDESC));
		xddsd.dwSize   = sizeof(DDSURFACEDESC);
		xddsd.dwFlags  = DDSD_WIDTH | DDSD_HEIGHT;
		xddsd.dwWidth  = iResX;
		xddsd.dwHeight = iResY;

		r.left=0; r.right =iResX;
		r.top=0;  r.bottom=iResY;

		if(iWindowMode)
			{
				POINT Point={0,0};
				ClientToScreen(DX.hWnd,&Point);
				r.left+=Point.x;r.right+=Point.x;
				r.top+=Point.y;r.bottom+=Point.y;
			}

		XS=(float)iResX/128;
		YS=(float)iResY/96;

		ddrval=IDirectDrawSurface_Lock(DX.DDSPrimary,NULL, &xddsd, DDLOCK_WAIT|DDLOCK_READONLY, NULL);

		if(ddrval==DDERR_SURFACELOST) IDirectDrawSurface_Restore(DX.DDSPrimary);

		pf=pMem;

		if(ddrval==DD_OK)
			{
				unsigned char * ps=(unsigned char *)xddsd.lpSurface;

				if(iDesktopCol==16)
					{
						unsigned short sx;
						for(y=0;y<96;y++)
							{
								for(x=0;x<128;x++)
									{
										sx=*((unsigned short *)((ps)+
															r.top*xddsd.lPitch+
															(((int)((float)y*YS))*xddsd.lPitch)+
																r.left*2+
																((int)((float)x*XS))*2));
										*(pf+0)=(sx&0x1f)<<3;
										*(pf+1)=(sx&0x7e0)>>3;
										*(pf+2)=(sx&0xf800)>>8;
										pf+=3;
									}
							}
					}
				else
				if(iDesktopCol==15)
					{
						unsigned short sx;
						for(y=0;y<96;y++)
							{
								for(x=0;x<128;x++)
									{
										sx=*((unsigned short *)((ps)+
															r.top*xddsd.lPitch+
															(((int)((float)y*YS))*xddsd.lPitch)+
																r.left*2+
																((int)((float)x*XS))*2));
										*(pf+0)=(sx&0x1f)<<3;
										*(pf+1)=(sx&0x3e0)>>2;
										*(pf+2)=(sx&0x7c00)>>7;
										pf+=3;
									}
							}
					}
				else
					{
						unsigned long sx;
						for(y=0;y<96;y++)
							{
								for(x=0;x<128;x++)
									{
										sx=*((unsigned long *)((ps)+
															r.top*xddsd.lPitch+
															(((int)((float)y*YS))*xddsd.lPitch)+
																r.left*4+
																((int)((float)x*XS))*4));
										*(pf+0)=(unsigned char)((sx&0xff));
										*(pf+1)=(unsigned char)((sx&0xff00)>>8);
										*(pf+2)=(unsigned char)((sx&0xff0000)>>16);
										pf+=3;
									}
							}
					}
			}

		IDirectDrawSurface_Unlock(DX.DDSPrimary,&xddsd);
	*/

	// Generic number/border painter

	pf=pMem+(103*3);                                      // Offset to numbered rectangle

	for (y=0;y<20;y++)                                    // Loop the numbered rectangle pixel
	{
		for (x=0;x<6;x++)
		{
			c=cFont[lSelectedSlot][x+y*6];                    // Get 4 character dot info at once (number depends on selected slot)
			v=(c&0xc0)>>6;
			PaintPicDot(pf,(unsigned char)v);
			pf+=3;                // Paint the dots into the rectangle
			v=(c&0x30)>>4;
			PaintPicDot(pf,(unsigned char)v);
			pf+=3;
			v=(c&0x0c)>>2;
			PaintPicDot(pf,(unsigned char)v);
			pf+=3;
			v=c&0x03;
			PaintPicDot(pf,(unsigned char)v);
			pf+=3;
		}
		pf+=104*3;                                          // Next rectangle y line
	}

	pf=pMem;                                              // ptr to first position in 128x96 picture
	for (x=0;x<128;x++)                                   // Loop top/bottom line
	{
		*(pf+(95*128*3))=0x00;
		*pf++=0x00;
		*(pf+(95*128*3))=0x00;
		*pf++=0x00;                   // Paint it red
		*(pf+(95*128*3))=0xff;
		*pf++=0xff;
	}
	pf=pMem;                                              // ptr to first position
	for (y=0;y<96;y++)                                    // Loop left/right line
	{
		*(pf+(127*3))=0x00;
		*pf++=0x00;
		*(pf+(127*3))=0x00;
		*pf++=0x00;                      // Paint it red
		*(pf+(127*3))=0xff;
		*pf++=0xff;
		pf+=127*3;                                          // Offset to next line
	}
}

// Function will be called with 128x96x3 BGR data
// The plugin has to store the data and display
// it in the upper right corner.
// If the function is called with a NULL ptr, you can
// release your picture data and stop displaying
// the screen picture

void CALLBACK GPUshowScreenPic(unsigned char * pMem)
{
	DestroyPic();                                         // Destroy old picture data
	if (pMem==0) return;                                  // Done
	CreatePic(pMem);                                      // Create new picture...don't free pMem...just read from it
}

void CALLBACK GPUsetfix(unsigned long dwFixBits)
{
	dwEmuFixes=dwFixBits;
}

void CALLBACK GPUsetframelimit(unsigned long option)
{
	bInitCap = TRUE;

	if (option==1)
	{
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=2;
		SetAutoFrameCap();
		BuildDispMenu(0);
	}
	else
	{
		UseFrameLimit=0;
	}
}

void CALLBACK GPUsetspeedmode(unsigned long newSpeedMode)
{
	bInitCap = TRUE;

	switch (newSpeedMode)
	{
	case 1:
		bSkipNextFrame = FALSE;
		fFrameRate=1.0f;
		iFastFwd=0;
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=1;
		SetAutoFrameCap();
		BuildDispMenu(0);
		break;
	case 2:
		bSkipNextFrame = FALSE;
		fFrameRate=4.0f;
		iFastFwd=0;
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=1;
		SetAutoFrameCap();
		BuildDispMenu(0);
		break;
	case 3:
		bSkipNextFrame = FALSE;
		fFrameRate=9.0f;
		iFastFwd=0;
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=1;
		SetAutoFrameCap();
		BuildDispMenu(0);
		break;
	case 4:
		bSkipNextFrame = FALSE;
		fFrameRate=15.0f;
		iFastFwd=0;
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=1;
		SetAutoFrameCap();
		BuildDispMenu(0);
		break;
	case 5:
		bSkipNextFrame = FALSE;
		fFrameRate=22.0f;
		iFastFwd=0;
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=1;
		SetAutoFrameCap();
		BuildDispMenu(0);
		break;
	case 6:
		bSkipNextFrame = FALSE;
		fFrameRate=30.0f;
		iFastFwd=0;
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=1;
		SetAutoFrameCap();
		BuildDispMenu(0);
		break;
	case 7:
		bSkipNextFrame = FALSE;
		iFastFwd=0;
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=2;
		SetAutoFrameCap();
		BuildDispMenu(0);
		break;
	case 8:
		bSkipNextFrame = FALSE;
		fFrameRate=75.0f;
		iFastFwd=0;
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=1;
		SetAutoFrameCap();
		BuildDispMenu(0);
		break;
	case 9:
		bSkipNextFrame = FALSE;
		fFrameRate=90.0f;
		iFastFwd=0;
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=1;
		SetAutoFrameCap();
		BuildDispMenu(0);
		break;
	case 10:
		bSkipNextFrame = FALSE;
		fFrameRate=120.0f;
		iFastFwd=0;
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=1;
		SetAutoFrameCap();
		BuildDispMenu(0);
		break;
	case 11:
		bSkipNextFrame = FALSE;
		fFrameRate=240.0f;
		iFastFwd=0;
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=1;
		SetAutoFrameCap();
		BuildDispMenu(0);
		break;
	case 12:
		bSkipNextFrame = FALSE;
		fFrameRate=480.0f;
		iFastFwd=0;
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=1;
		SetAutoFrameCap();
		BuildDispMenu(0);
		break;
	case 13:
		iFastFwd=1;
		iMaximumSpeed=1;
		bSkipNextFrame=FALSE;
		UseFrameSkip=1;
		UseFrameLimit=0;
		break;
	default:
		iFastFwd=1;
		iMaximumSpeed=0;
		bSkipNextFrame=FALSE;
		UseFrameSkip=1;
		UseFrameLimit=0;
		BuildDispMenu(0);
	}
}

void CALLBACK GPUvisualVibration(unsigned long iSmall, unsigned long iBig)
{
	int iVibVal;

	if (PreviousPSXDisplay.DisplayMode.x)                 // Calculate minimum "shake pixel" from screen width
		iVibVal=std::max(1L,iResX/PreviousPSXDisplay.DisplayMode.x);
	else iVibVal=1;
	// Big rumble: 4...15 sp ; small rumble 1...3 sp
	if (iBig) iRumbleVal=std::max(4*iVibVal,std::min(15*iVibVal,((int)iBig  *iVibVal)/10));
	else     iRumbleVal=std::max(1*iVibVal,std::min( 3*iVibVal,((int)iSmall*iVibVal)/10));

	srand(timeGetTime());                                 // Initialize random (will be used in BufferSwap)

	iRumbleTime=15;                                       // Let the rumble last 16 buffer swaps
}

void CALLBACK GPUsetframecounter(unsigned long newCurrentFrame,unsigned long newTotalFrames)
{
	currentFrame=newCurrentFrame;
	totalFrames=newTotalFrames;
	BuildDispMenu(0);
	ThereIsLag=0;
}

void CALLBACK GPUsetlagcounter(unsigned long newCurrentLag)
{
	currentLag=newCurrentLag;
	ThereIsLag = 1;
	BuildDispMenu(0);
}

void CALLBACK GPUinputdisplay(unsigned long newCurrentInput)
{
	currentInput=newCurrentInput;
	BuildDispMenu(0);
}

void CALLBACK GPUupdateframe()
{
	updateDisplay();
}

void CALLBACK GPUshowframecounter()
{
	if (ulKeybits&KEY_SHOWFCOUNT)
	{
		DestroyPic();
		ulKeybits&=~KEY_SHOWFCOUNT;
		DoClearScreenBuffer();
	}
	else
	{
		ulKeybits|=KEY_SHOWFCOUNT;
		szDispBuf[0]=0;
		BuildDispMenu(0);
	}
}

void CALLBACK GPUshowInput()
{
	if (ulKeybits&KEY_SHOWINPUT)
	{
		DestroyPic();
		ulKeybits&=~KEY_SHOWINPUT;
		DoClearScreenBuffer();
	}
	else
	{
		ulKeybits|=KEY_SHOWINPUT;
		szDispBuf[0]=0;
		BuildDispMenu(0);
	}
}

void CALLBACK GPUshowAnalog()
{
	if (ulKeybits&KEY_SHOWANALOG)
	{
		DestroyPic();
		ulKeybits&=~KEY_SHOWANALOG;
		DoClearScreenBuffer();
	}
	else
	{
		ulKeybits|=KEY_SHOWANALOG;
		szDispBuf[0]=0;
		BuildDispMenu(0);
	}
}

void CALLBACK GPUshowALL()
{	
	if (ulKeybits&KEY_SHOWFPS)
	{
		DestroyPic();
		ulKeybits&=~KEY_SHOWFPS;
		DoClearScreenBuffer();
	}
	else
	{
		ulKeybits|=KEY_SHOWFPS;
		szDispBuf[0]=0;
		BuildDispMenu(0);
	}
}

void CALLBACK GPUsetcurrentmode(char newModeFlags)
{
	modeFlags = newModeFlags;
	BuildDispMenu(0);
}
void OnRecording(HWND hW);


void CALLBACK GPUstartAvi(char* filename)
{
	if (!RECORD_RECORDING)
	{
		bool proceed = true;
		HWND hWP=GetActiveWindow();
		extern bool HACK_CODEC_CHOOSE(HWND hW);
		if(HACK_CODEC_CHOOSE(GetMainWindow()))
		{
			RECORD_RECORDING=TRUE;
			RECORD_Start();
		}
		else 
		{
			RECORD_RECORDING=FALSE;
		}
		BuildDispMenu(0);
	}
}

void CALLBACK GPUrestartAVINewRes(void)
{
	RECORD_Stop();	
	RECORD_Start();	
}

void CALLBACK GPUstopAvi(void)
{
	if (RECORD_RECORDING)
	{		
		RECORD_RECORDING=FALSE;
		RECORD_Stop();
		BuildDispMenu(0);		
	}	
}

void CALLBACK GPUsendFpLuaGui(void (*fpPSXjin_LuaGuiTemp)(void *s, int width, int height, int bpp, int pitch))
{
	fpPSXjin_LuaGui = fpPSXjin_LuaGuiTemp;
}

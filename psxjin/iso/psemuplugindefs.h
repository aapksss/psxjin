#ifndef _PSEMU_PLUGIN_DEFS_H
#define _PSEMU_PLUGIN_DEFS_H

// Header version

#define _PPDK_HEADER_VERSION		3

#define PLUGIN_VERSION				1

// Plugin type returned by PSEgetLibType (types can be merged if plugin is multi type!)

#define PSE_LT_CDR					1
#define PSE_LT_GPU					2
#define PSE_LT_SPU					4
#define PSE_LT_PAD					8
#define PSE_LT_NET					16

// Every function in DLL if completed successfully should return this value

#define PSE_ERR_SUCCESS				0

// Undefined error but fatal one, that kills all functionality

#define PSE_ERR_FATAL				-1

// XXX_Init return values
// Those return values apply to all libraries
// Currently obsolete - preserved for compatibility

// Initialization went OK

#define PSE_INIT_ERR_SUCCESS		0

// This driver is not configured

#define PSE_INIT_ERR_NOTCONFIGURED	-2

// This driver can not operate properly on this hardware or hardware is not detected

#define PSE_INIT_ERR_NOHARDWARE		-3

// GPU plugin

//  GPU_Test return values

// Success, everything configured, and went OK

#define PSE_GPU_ERR_SUCCESS			0

// Errors
// This error might be returned as a critical error, but none of the below

#define PSE_GPU_ERR					-20

// This driver is not configured

#define PSE_GPU_ERR_NOTCONFIGURED	PSE_GPU_ERR - 1

// This driver failed initialization

#define PSE_GPU_ERR_INIT			PSE_GPU_ERR - 2

// Warnings
// This warning might be returned as an undefined warning, but allowing driver to continue

#define PSE_GPU_WARN				20

//  GPU_Query - will be implemented soon

typedef struct
{
	unsigned long	flags;
	unsigned long	status;
	HWND			window;
	unsigned char reserved[100];
} gpuQueryS;

// gpuQueryS.flags
// If driver can operate in both modes, it must support GPU_changeMode();
// This driver can operate in fullscreen mode

#define PSE_GPU_FLAGS_FULLSCREEN		1

// This driver can operate in windowed mode

#define PSE_GPU_FLAGS_WINDOWED			2


// gpuQueryS.status
// This driver cannot operate in this windowed mode

#define PSE_GPU_STATUS_WINDOWWRONG	1

// GPU_Query End - will be implemented

// CDR plugin

// CDR_Test return values

// Success, everything configured, and went OK

#define PSE_CDR_ERR_SUCCESS			0

// General failure (error undefined)

#define PSE_CDR_ERR_FAILURE			-1

// Errors

#define PSE_CDR_ERR -40

// This driver is not configured

#define PSE_CDR_ERR_NOTCONFIGURED	PSE_CDR_ERR - 0

// If this driver is unable to read data from medium

#define PSE_CDR_ERR_NOREAD			PSE_CDR_ERR - 1

// Warnings

#define PSE_CDR_WARN 40

// If this driver emulates lame mode (if it can read only 2048 tracks and sector header is emulated)
// This might happen to CDROMS that do not support RAW mode reading - it may reduce compatibility

#define PSE_CDR_WARN_LAMECD			PSE_CDR_WARN + 0

// SPU Plugin

// Some info restricted (what?)

// Success, everything configured, and went OK

#define PSE_SPU_ERR_SUCCESS 0

// Errors
// This error might be returned as critical error, but none of the below

#define PSE_SPU_ERR					-60

// This driver is not configured

#define PSE_SPU_ERR_NOTCONFIGURED	PSE_SPU_ERR - 1

// This driver failed initialization

#define PSE_SPU_ERR_INIT			PSE_SPU_ERR - 2

// Warnings
// This warning might be returned as an undefined warning, but allowing driver to continue

#define PSE_SPU_WARN				60

// Input plugin

/*

  functions that must be exported from PAD Plugin
  
  long	PADinit(long flags);	// called only once when PSEmu Starts
  void	PADshutdown(void);		// called when PSEmu exits
  long	PADopen(PadInitS *);	// called when PSEmu is running program
  long	PADclose(void);
  long	PADconfigure(void);
  void  PADabout(void);
  long  PADtest(void);			// called from Configure Dialog and after PADopen();
  long	PADquery(void);

  long	PADreadPort1(PadDataS *);
  long	PADreadPort2(PadDataS *);

*/

// PADquery responses (notice - values ORed)
// PSEmu will use them also in PADinit to tell plugin which ports will use
// Notice that PSEmu will call PADinit and PADopen only once when they are from
// same plugin

// Might be used in port 1 (must support PADreadPort1() function)

#define PSE_PAD_USE_PORT1			1

// Might be used in port 2 (must support PADreadPort2() function)

#define PSE_PAD_USE_PORT2			2

// Mouse SCPH-1030

#define PSE_PAD_TYPE_MOUSE			1

// NegCon - 16 button analog controller SLPH-00001

#define PSE_PAD_TYPE_NEGCON			2

// Gun controller - SLPH-00014 from Konami

#define PSE_PAD_TYPE_GUN			3

// Standard controller SCPH-1080, SCPH-1150

#define PSE_PAD_TYPE_STANDARD		4

// Analog controller SCPH-1110

#define PSE_PAD_TYPE_ANALOGJOY		5

// GunCon - gun controller SLPH-00034 from Namco

#define PSE_PAD_TYPE_GUNCON			6

// Analog controller SCPH-1150

#define PSE_PAD_TYPE_ANALOGPAD		7

// Success, everything configured, and went OK

#define PSE_PAD_ERR_SUCCESS			0

// General plugin failure (undefined error)

#define PSE_PAD_ERR_FAILURE			-1

// Errors

// This error might be returned as a critical error, but none of the below

#define PSE_PAD_ERR					-80

// This driver is not configured

#define PSE_PAD_ERR_NOTCONFIGURED	PSE_PAD_ERR - 1

// This driver failed initialization

#define PSE_PAD_ERR_INIT			PSE_PAD_ERR - 2

// Warnings

// This warning might be returned as an undefined warning, but allowing the driver to continue

#define PSE_PAD_WARN				80

typedef struct
{
	// Controller type - fill it with the predefined values above
	
	unsigned char controllerType;
	
	// Status of buttons - every controller fills this field
	
	unsigned short buttonStatus;
	
	// For analog controller fill those next 4 bytes
	// Values are analog in range 0-255 where 128 is center position
	
	unsigned char rightJoyX, rightJoyY, leftJoyX, leftJoyY;

	// For mouse, fill those next 2 bytes
	// Values are in range -128 - 127
	
	unsigned char moveX, moveY;

	unsigned char reserved[91];

} PadDataS;

// NET plugin v2

/* Modes bits for NETsendData/NETrecvData */
#define PSE_NET_BLOCKING	0x00000000
#define PSE_NET_NONBLOCKING	0x00000001

/*
typedef struct { 
	char EmuName[32];
	char CdromID[32];
	char CdromLabel[32];
	void *psxMem;
	GPUshowScreenPic PAD_showScreenPic;
	GPUdisplayText PAD_displayText;
	PADsetSensitive PAD_setSensitive;
	void unused[1024];
} netInfo;
*/

/*
  basic funcs:

   long NETopen(HWND hWnd)
    opens the connection.
    shall return 0 on success, else -1.
    -1 is also returned if the user selects offline mode.

   long NETclose()
    closes the connection.
    shall return 0 on success, else -1.

   void NETpause()
    this is called when the user paused the emulator.

   void NETresume()
    this is called when the user resumed the emulator.

   long NETqueryPlayer()
    returns player number

   long NETsendPadData(void *pData, int Size)
    this should be called for the first pad only on each side.

   long NETrecvPadData(void *pData, int Pad)
    call this for Pad 1/2 to get the data sent by the above func.

  extended funcs:

   long NETsendData(void *pData, int Size, int Mode)
    sends Size bytes from pData to the other side.

   long NETrecvData(void *pData, int Size, int Mode)
    receives Size bytes from pData to the other side.

   void NETsetInfo(netInfo *info);
    sets the netInfo struct.

   void NETvsync()
    called every vsync (before GPUupdateLace).

   void NETkeypressed(int key)
    key is on win32 a VK_?? keycode, and over linux a XK_?? (X11) keycode.
*/

#endif // _PSEMU_PLUGIN_DEFS_H

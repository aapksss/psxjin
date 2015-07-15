/*
	PSEmu Plugin Developer Kit Header definition

	(C)1998 Vision Thing

	This file can be used only to develop PSEmu Plugins
	Other usage is highly prohibited.
	
	IMPORTANT!!!
	if you want to add return codes (any errors or warnings) just drop an email to
	plugin@psemu.com
	
	NOTE: is the email/website still active? Is PSEmu still active?
*/
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

// Every function in the DLL (if completed successfully) should return this value
#define PSE_ERR_SUCCESS				0

// Undefined error, albeit a fatal one, that kills all functionality

#define PSE_ERR_FATAL				-1

// XXX_Init return values
// These return values apply to all libraries
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
// This error might be returned as critical error but none of below

#define PSE_GPU_ERR					-20

// This driver is not configured

#define PSE_GPU_ERR_NOTCONFIGURED	PSE_GPU_ERR - 1

// This driver failed initialization

#define PSE_GPU_ERR_INIT			PSE_GPU_ERR - 2

// Warnings
// This warning might be returned as undefined. Warning, but allowing driver to continue
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

// GPU_Query End - will be implemented soon

// CDR plugin

//	CDR_Test return values

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

// If this driver emulates lame mode (meaning it can read only 2048 tracks and sector header is emulated)
// This might happen to CD-ROMs that do not support RAW mode reading - compatibility will be affected

#define PSE_CDR_WARN_LAMECD			PSE_CDR_WARN + 0

// SPU plugin

// Some info restricted (what?)

// Success, everything configured, and went OK

#define PSE_SPU_ERR_SUCCESS 0

// Errors
// This error might be returned as a critical error but none of the below

#define PSE_SPU_ERR					-60

// This driver is not configured

#define PSE_SPU_ERR_NOTCONFIGURED	PSE_SPU_ERR - 1

// This driver failed initialization

#define PSE_SPU_ERR_INIT			PSE_SPU_ERR - 2

// Warnings
// This warning might be returned as an undefined warning, but allowing driver to continue
#define PSE_SPU_WARN				60

// Might be used in port 1 (must support PADreadPort1() function)

#define PSE_PAD_USE_PORT1			1

// Might be used in port 2 (must support PADreadPort2() function)

#define PSE_PAD_USE_PORT2			2

// Mouse SCPH-1030
#define PSE_PAD_TYPE_MOUSE			1
// NegCon - 16 button analog controller SLPH-00001
#define PSE_PAD_TYPE_NEGCON			2
// Gun controller - gun controller SLPH-00014 from Konami
#define PSE_PAD_TYPE_GUN			3
// Standard controller SCPH-1080, SCPH-1150
#define PSE_PAD_TYPE_STANDARD		4
// Analog controller SCPH-1110
#define PSE_PAD_TYPE_ANALOGJOY		5
// GunCon - gun controller SLPH-00034 from Namco
#define PSE_PAD_TYPE_GUNCON			6
// Analog Controller SCPH-1150
#define PSE_PAD_TYPE_ANALOGPAD		7

// No controller

// Success, everything configured, and went OK

#define PSE_PAD_ERR_SUCCESS			0

// General plugin failure (undefined error)

#define PSE_PAD_ERR_FAILURE			-1

// Errors
// This error might be returned as a critical error but none of the below

#define PSE_PAD_ERR					-80

// This driver is not configured

#define PSE_PAD_ERR_NOTCONFIGURED	PSE_PAD_ERR - 1

// This driver failed initialization

#define PSE_PAD_ERR_INIT			PSE_PAD_ERR - 2

// Warnings
// This warning might be returned as an undefined warning, but allowing driver to continue

#define PSE_PAD_WARN				80

// Net plugin v2
// Added by linuzappz

// Modes bits for NETsendData/NETrecvData

#define PSE_NET_BLOCKING	0x00000000
#define PSE_NET_NONBLOCKING	0x00000001

#endif

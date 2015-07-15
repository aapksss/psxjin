#ifndef __SYSTEM_H__
#define __SYSTEM_H__

int  SysInit();							// Initialize memory and plugins
void SysReset();						// Resets memory
void SysPrintf(char *fmt, ...);			// Printf used by BIOS syscalls
void SysMessage(char *fmt, ...);		// Message used to print message to users
void *SysLoadLibrary(char *lib);		// Loads library
void *SysLoadSym(void *lib, char *sym);	// Loads symbol from library
const char *SysLibError();				// Gets previous error loading symbols
void SysCloseLibrary(void *lib);		// Closes library
void SysUpdate();						// Called on VBlank (to update controllers, etc)
void SysRunGui();						// Returns to the GUI
void SysClose();						// Close memory and plugins

#endif /* __SYSTEM_H__ */

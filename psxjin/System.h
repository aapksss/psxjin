/*  PSXjin - Pc Psx Emulator
 *  Copyright (C) 1999-2003  PSXjin Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

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

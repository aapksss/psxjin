//#define  STRICT
#define  D3D_OVERLOADS
#define  DIRECT3D_VERSION 0x600
#define  CINTERFACE

#include <WINDOWS.H>
#include <WINDOWSX.H>
#include <TCHAR.H>
#include "resource.h"

// Pete: since my last OS + compiler reinstall, I needed to user newer
// defines/libs, therefore I've decided to use the mingw headers and
// the d3dx.lib (old libs: d3dim.lib dxguid.lib)

// We may need to update this ^^^

#include "mingwddraw.h"
#include "mingwd3dtypes.h"
#include "mingwd3d.h"

// stupid intel compiler warning on extern __inline funcs
//#pragma warning (disable:864)
// disable stupid MSVC2005 warnings as well...

// We may want to update this as well, seemingly ^^^
// because it may be either warning us of problems
// or because we all use newer compilers now

#pragma warning (disable:4996)
#pragma warning (disable:4244)

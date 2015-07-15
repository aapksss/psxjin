#include <stdexcept>
#include <string>
#include "cpudetect.h"

#ifndef __GNUC__
#error wrong platform - this source code file is for the GNU C compiler.
#endif

using namespace std;

#include <stdio.h>

// Processor instructions extension detection routines

// Flag variable indicating which ISA extensions are disabled (for debugging)

static uint _dwDisabledISA = 0x00;      // 0xffffffff; //<- use this to disable all extensions

// Disables given set of instruction extensions. See SUPPORT_... defines

void disableExtensions(uint dwDisableMask)
{
    _dwDisabledISA = dwDisableMask;
}

// Checks which instruction set extensions are supported by the CPU

uint detectCPUextensions(void)
{
#ifndef __i386__
    return 0; // Always disable extensions on non-x86 platforms
#else
    uint res = 0;

    if (_dwDisabledISA == 0xffffffff) return 0;

    asm volatile(
        "\n\txor     %%esi, %%esi"       // Clear %%esi = result register
        // Check if 'CPUID' instructions is available by toggling eflags bit 21

        "\n\tpushf"                      // Save eflags to stack
        "\n\tpop     %%eax"              // Load eax from stack (with eflags)
        "\n\tmovl    %%eax, %%ecx"       // Save the original eflags values to ecx
        "\n\txor     $0x00200000, %%eax" // Toggle bit 21
        "\n\tpush    %%eax"              // Store toggled eflags to stack
        "\n\tpopf"                       // Load eflags from stack
        "\n\tpushf"                      // Save updated eflags to stack
        "\n\tpop     %%eax"              // Load from stack
        "\n\txor     %%edx, %%edx"       // Clear edx for defaulting no MMX
        "\n\tcmp     %%ecx, %%eax"       // Compare to original eflags values
        "\n\tjz      end"                // Jumps to end if CPUID not present

        // CPUID instruction available, test for presence of MMX instructions

        "\n\tmovl    $1, %%eax"
        "\n\tcpuid"
//        movl       $0x00800000, %edx   // Force enable MMX
        "\n\ttest    $0x00800000, %%edx"
        "\n\tjz      end"                // Branch if MMX not available

        "\n\tor      $0x01, %%esi"       // Otherwise add MMX support bit

        "\n\ttest    $0x02000000, %%edx"
        "\n\tjz      test3DNow"          // Branch if SSE not available

        "\n\tor      $0x08, %%esi"       // Otherwise add SSE support bit

    "\n\ttest3DNow:"
        // Test for presence of AMD extensions
        "\n\tmov     $0x80000000, %%eax"
        "\n\tcpuid"
        "\n\tcmp     $0x80000000, %%eax"
        "\n\tjbe     end"                 // Branch if no AMD extensions detected

        // Test for presence of 3DNow! extension
		
        "\n\tmov     $0x80000001, %%eax"
        "\n\tcpuid"
        "\n\ttest    $0x80000000, %%edx"
        "\n\tjz      end"                  // Branch if 3DNow! not detected

        "\n\tor      $0x02, %%esi"         // Otherwise add 3DNow! support bit

    "\n\tend:"

        "\n\tmov     %%esi, %0"

      : "=r" (res)
      : /* No inputs */
      : "%edx", "%eax", "%ecx", "%esi" );
      
    return res & ~_dwDisabledISA;
	
#endif

}

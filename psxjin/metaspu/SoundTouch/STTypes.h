#ifndef STTypes_H
#define STTypes_H

//#define INTEGER_SAMPLES 1

typedef unsigned int    uint;
typedef unsigned long   ulong;

#ifdef __x86_64__
typedef unsigned long long   ulongptr;
#else
typedef unsigned long   ulongptr;
#endif


#ifdef __GNUC__
    // In GCC, include soundtouch_config.h made by configuration scripts
/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Use Integer as Sample type */
//#define INTEGER_SAMPLES 1

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

#endif

#ifndef _WINDEF_

    // If these aren't defined already by Windows headers, define now

    typedef int BOOL;

#ifndef FALSE
    #define FALSE   0
#endif

#ifndef TRUE
    #define TRUE    1
#endif

#endif  // _WINDEF_


namespace soundtouch
{
// Activate these undef's to overrule the possible sample type 
// setting inherited from some other header file:
//#undef INTEGER_SAMPLES
//#undef FLOAT_SAMPLES

#if !(INTEGER_SAMPLES || FLOAT_SAMPLES)
   
    // Choose either 32-bit floating point or 16-bit integer sample type
    // by choosing one of the following defines, unless this selection 
    // has already been done in some other file.

    // Notes:
    // - In Windows environment, choose the sample format with the
    //   following defines.
    // - In GNU environment, the floating point samples are used by 
    //   default, but integer samples can be chosen by giving the 
    //   following switch to the configure script:
    //       ./configure --enable-integer-samples
    //   However, if you still prefer to select the sample format here 
    //   also in GNU environment, then please #undef the INTEGER_SAMPLE
    //   and FLOAT_SAMPLE defines first as in comments above.
	
    //#define INTEGER_SAMPLES     1    // 16-bit integer samples
    #define FLOAT_SAMPLES       1    // 32-bit float samples
 
 #endif

    // Define this to allow CPU-specific assembler optimizations. Notice that 
    // having this enabled on non-x86 platforms doesn't matter; the compiler can 
    // drop unsupported extensions on different platforms automatically. 
    // However, if you're having difficulties getting the optimized routines 
    // compiled with your compiler (EG some GCC compiler versions may be picky),
    // you may wish to disable the optimizations to make the library compile
	
	#if !defined(_MSC_VER) || !defined(__x86_64__)
	#define ALLOW_OPTIMIZATIONS 1
	#define ALLOW_NONEXACT_SIMD_OPTIMIZATION    1
	#endif

    // If defined, allows the SIMD-optimized routines to take minor shortcuts 
    // for improved performance. Undefine to require faithfully similar SIMD 
    // calculations as in normal C implementation

    #ifdef INTEGER_SAMPLES
	
        // 16-bit integer sample type
		
        typedef short SAMPLETYPE;
		
        // Data type for sample accumulation: Use 32-bit integer to prevent overflows
		
        typedef long  LONG_SAMPLETYPE;

        #ifdef FLOAT_SAMPLES
		
            // Check that only one sample type is defined
			
            #error "conflicting sample types defined"
        #endif // FLOAT_SAMPLES

        #ifdef ALLOW_OPTIMIZATIONS
            #if (_WIN32 || __i386__ || __x86_64__)
                // Allow MMX optimizations
                #define ALLOW_MMX   1
            #endif
        #endif

    #else

        // Floating point samples
		
        typedef float  SAMPLETYPE;
		
        // Data type for sample accumulation: Use double to utilize full precision
		
        typedef double LONG_SAMPLETYPE;

        #ifdef ALLOW_OPTIMIZATIONS
                // Allow 3DNow! and SSE optimizations
            #if _WIN32
               // #define ALLOW_3DNOW     1
            #endif

            #if (_WIN32 || __i386__ || __x86_64__)
                #define ALLOW_SSE       1
            #endif
        #endif

    #endif  // INTEGER_SAMPLES
};

#endif

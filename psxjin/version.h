#define PCSXRR_VERSION 20

#define PCSXRR_NAME "PSXjin"

#define PCSXRR_FEATURE_STRING ""

#ifdef _DEBUG
#define PCSXRR_SUBVERSION_STRING " debug"
#else
#define PCSXRR_SUBVERSION_STRING ""
#endif

#if defined(_MSC_VER)
#define PCSXRR_COMPILER ""
#define PCSXRR_COMPILER_DETAIL " msvc " _Py_STRINGIZE(_MSC_VER)
#define _Py_STRINGIZE(X) _Py_STRINGIZE1((X))
#define _Py_STRINGIZE1(X) _Py_STRINGIZE2 ## X
#define _Py_STRINGIZE2(X) #X

// What the heck is that URL? I can't get it to load, nor can I get it to stay on that IP. I believe the domain has been purchased...
//re: Removed the weird URL. It pointed out to something on Python website, I couldn't tell you what it was about..
//test git VS
#else
// TODO: make for others compilers
#define PCSXRR_COMPILER ""
#define PCSXRR_COMPILER_DETAIL ""
#endif

#define PCSXRR_VERSION_STRING "v2.0.3pre" PCSXRR_SUBVERSION_STRING PCSXRR_COMPILER
#define PCSXRR_NAME_AND_VERSION PCSXRR_NAME " " PCSXRR_VERSION_STRING

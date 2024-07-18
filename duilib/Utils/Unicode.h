// Unicode define

#ifndef UTIL_UNICODE_H_
#define UTIL_UNICODE_H_

#include "duilib/duilib_defs.h"

// Type detection for wchar_t.
#if defined(DUILIB_BUILD_FOR_WIN)
#define WCHAR_T_IS_UTF16
#elif defined(DUILIB_BUILD_FOR_LINUX) && defined(COMPILER_GCC) && \
      defined(__WCHAR_MAX__) && \
      (__WCHAR_MAX__ == 0x7fffffff || __WCHAR_MAX__ == 0xffffffff)
#define WCHAR_T_IS_UTF32
#elif defined(DUILIB_BUILD_FOR_LINUX) && defined(COMPILER_GCC) && \
      defined(__WCHAR_MAX__) && \
      (__WCHAR_MAX__ == 0x7fff || __WCHAR_MAX__ == 0xffff)
// On Posix, we'll detect short wchar_t, but projects aren't guaranteed to
// compile in this mode (in particular, Chrome doesn't). This is intended for
// other projects using base who manage their own dependencies and make sure
// short wchar works for them.
#define WCHAR_T_IS_UTF16
#else
    #if defined (_WIN32) || defined (_WIN64)
        #define WCHAR_T_IS_UTF16
    #else
        #error Please add support for your compiler
    #endif
#endif

typedef char UTF8Char;
#if defined(WCHAR_T_IS_UTF16)
typedef wchar_t UTF16Char;
typedef int32_t UTF32Char;
#else
typedef int16_t UTF16Char;
typedef wchar_t UTF32Char;
#endif

typedef std::basic_string<UTF8Char> UTF8String;
typedef UTF8String U8String;
typedef std::basic_string<UTF16Char> UTF16String;
typedef UTF16String U16String;
typedef std::basic_string<UTF32Char> UTF32String;
typedef UTF32String U32String;

#endif // UTIL_UNICODE_H_

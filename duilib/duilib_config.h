#ifndef DUILIB_CONFIG_H_
#define DUILIB_CONFIG_H_

/** Windows or Linux 平台
*/
#if defined (_WIN32) || defined (_WIN64)
    #define DUILIB_PLATFORM_WIN     1
#elif defined(linux) || defined(__linux) || defined(__linux__)
    #define DUILIB_PLATFORM_LINUX   1
#else
    //不支持的系统
    #pragma message("Unknown Platform!")
#endif

/** 64位操作系统标识
*/
#if defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__) 
    #define DUILIB_BIT_64   1
#endif

/** Unicode or Ansi 版本(Ansi版本，文件的编码是UTF-8的，所以字符串编码也是UTF-8的)
*/
#if defined(UNICODE) || defined(_UNICODE)
    #define DUILIB_UNICODE 1
#endif

//未使用的变量宏，避免编译器报警报
#ifndef UNUSED_VARIABLE
    #define UNUSED_VARIABLE(x) ((void)(x))
#endif

#ifndef ASSERT_UNUSED_VARIABLE
    #ifdef _DEBUG
        #define ASSERT_UNUSED_VARIABLE(expr)  ASSERT(expr)
    #else
        #define ASSERT_UNUSED_VARIABLE(expr)  UNUSED_VARIABLE(expr)
    #endif
#endif

#if defined DUILIB_PLATFORM_WIN
    #include "duilib_config_windows.h"

    #ifndef ASSERT
        #define ASSERT(expr)  _ASSERTE(expr)
    #endif
#elif defined  DUILIB_PLATFORM_LINUX
    #include <cassert>

    #define UILIB_API
    #ifdef _DEBUG
        #define ASSERT(expr)  assert(expr)
    #else
        #define ASSERT(expr)  ((void)(0))
    #endif
#endif

//字符串类
#include <string>

/** Unicode版本的字符串宏定义
*/
#if !defined(_T)
    #if defined (DUILIB_UNICODE)
        #define _T(x)   L##x
    #else
        #define _T(x)   x
    #endif
#endif

//字符串类型定义（Window下用Unicode，Linux下用Ansi）

/** Unicode版本的String
*/
typedef std::wstring DStringW;

/** Ansi版本的String
*/
typedef std::string  DStringA;

/** String 类型宏定义
*/
#ifdef DUILIB_UNICODE
    typedef std::wstring  DString;
#else
    typedef std::string   DString;
#endif

#endif //DUILIB_CONFIG_H_

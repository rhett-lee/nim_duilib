#ifndef DUILIB_CONFIG_H_
#define DUILIB_CONFIG_H_

/** Windows or Linux 平台
*/
#if defined (_WIN32) || defined (_WIN64)
    #define DUILIB_BUILD_FOR_WIN     1
#elif defined(linux) || defined(__linux) || defined(__linux__)
    #define DUILIB_BUILD_FOR_LINUX   1
#else
    //不支持的系统
    #pragma message("Unknown Platform!")
#endif

/** SDL 是否使用SDL的窗口和鼠标键盘事件（目前只支持SDL3）
*/
//#define DUILIB_BUILD_FOR_SDL   1

/** 64位操作系统标识
*/
#if defined(_M_X64) || defined(_M_AMD64) || defined(_WIN64) || defined(__x86_64__) 
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

#if defined DUILIB_BUILD_FOR_WIN
    #include "duilib_config_windows.h"

    #ifndef ASSERT
        #define ASSERT(expr)  _ASSERTE(expr)
    #endif

#elif defined  DUILIB_BUILD_FOR_LINUX
    #include "duilib_config_linux.h"
    #include <cassert>

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

/** Unicode版本的字符串，数据为UTF16编码
*/
typedef std::wstring DStringW;

/** Ansi版本的String，数据为UTF8编码
*/
typedef std::string  DStringA;

/** String 类型宏定义
*/
#ifdef DUILIB_UNICODE
    //Unicode版本：数据为UTF16编码
    typedef std::wstring  DString;
#else
    //多字节编码：数据为UTF8编码
    typedef std::string   DString;
#endif

#endif //DUILIB_CONFIG_H_

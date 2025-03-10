#ifndef DUILIB_STRING_H_
#define DUILIB_STRING_H_

#ifndef DUILIB_CONFIG_H_
    #error "需要先包含duilib_config.h头文件"
#endif

//字符串类
#include <string>
#include <cstring>

/** Unicode版本的字符串宏定义
*/
#if !defined(_T)
    #if defined (DUILIB_UNICODE)
        #define _T(x)   L##x
    #else
        #define _T(x)   x
    #endif
#endif

//检测wchar_t的定义：Linux平台下，wchar_t是4个字节；Windows平台下，wchar_t是2个字节
#if defined(DUILIB_BUILD_FOR_WIN)
    //Windows平台
    #define WCHAR_T_IS_UTF16
#elif defined(DUILIB_BUILD_FOR_LINUX) && defined(__GNUC__) && \
      defined(__WCHAR_MAX__) && \
      (__WCHAR_MAX__ == 0x7fffffff || __WCHAR_MAX__ == 0xffffffff)
    #define WCHAR_T_IS_UTF32
#elif defined(DUILIB_BUILD_FOR_LINUX) && defined(__GNUC__) && \
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

typedef std::basic_string_view<UTF8Char> UTF8StringView;
typedef UTF8StringView U8StringView;

typedef std::basic_string<UTF16Char> UTF16String;
typedef UTF16String U16String;

typedef std::basic_string_view<UTF16Char> UTF16StringView;
typedef UTF16StringView U16StringView;

typedef std::basic_string<UTF32Char> UTF32String;
typedef UTF32String U32String;

typedef std::basic_string_view<UTF32Char> UTF32StringView;
typedef UTF32StringView U32StringView;

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
    //Unicode版本：数据为wchar_t类型（Windows平台为UTF16编码，Linux平台为UTF32编码）
    typedef std::wstring  DString;
#else
    //多字节编码：数据为UTF8编码
    typedef std::string   DString;
#endif

#endif //DUILIB_STRING_H_

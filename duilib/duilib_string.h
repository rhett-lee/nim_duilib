#ifndef DUILIB_STRING_H_
#define DUILIB_STRING_H_

#ifndef DUILIB_CONFIG_H_
    #error "需要先包含duilib_config.h头文件"
#endif

//字符串类
#include <string>
#include <cstring>
#include <cstdint>

/** Unicode版本的字符串宏定义
*/
#if !defined(_T)
    #if defined (DUILIB_UNICODE)
        #define _T(x)   L##x
    #else
        #define _T(x)   x
    #endif
#endif

//检测wchar_t的定义：各平台下的wchar_t大小
#if defined(DUILIB_BUILD_FOR_WIN)
    //Windows平台：wchar_t是2字节(UTF-16)
    #define WCHAR_T_IS_UTF16
#elif defined(DUILIB_BUILD_FOR_LINUX) || defined(DUILIB_BUILD_FOR_MACOS) || defined(DUILIB_BUILD_FOR_FREEBSD)
    #if defined(__GNUC__)
        #if defined(__WCHAR_MAX__)
            #if (__WCHAR_MAX__ == 0x7fffffff || __WCHAR_MAX__ == 0xffffffff)
                //Linux/macOS GCC: wchar_t通常是4字节(UTF-32)
                #define WCHAR_T_IS_UTF32
            #elif (__WCHAR_MAX__ == 0x7fff || __WCHAR_MAX__ == 0xffff)
                // On Posix, we'll detect short wchar_t, but projects aren't guaranteed to
                // compile in this mode (in particular, Chrome doesn't). This is intended for
                // other projects using base who manage their own dependencies and make sure
                // short wchar works for them.
                //某些特殊配置下可能是2字节
                #define WCHAR_T_IS_UTF16
            #endif
        #endif
    #endif
    
    // 如果未检测到，默认处理
    #if !defined(WCHAR_T_IS_UTF16) && !defined(WCHAR_T_IS_UTF32)
        // 默认情况下（Linux/FreeBSD/MacOS平台）wchar_t是4字节
        #define WCHAR_T_IS_UTF32
    #endif
#else
    #error "不支持的平台，请添加对您的编译器的支持"
#endif

typedef char DUTF8Char;
#if defined(WCHAR_T_IS_UTF16)
    typedef wchar_t  DUTF16Char;
    typedef char32_t DUTF32Char;
#else
    typedef char16_t DUTF16Char;
    typedef wchar_t  DUTF32Char;
#endif

typedef std::basic_string<DUTF8Char> UTF8String;
typedef UTF8String U8String;

typedef std::basic_string_view<DUTF8Char> UTF8StringView;
typedef UTF8StringView U8StringView;

typedef std::basic_string<DUTF16Char> UTF16String;
typedef UTF16String U16String;

typedef std::basic_string_view<DUTF16Char> UTF16StringView;
typedef UTF16StringView U16StringView;

typedef std::basic_string<DUTF32Char> UTF32String;
typedef UTF32String U32String;

typedef std::basic_string_view<DUTF32Char> UTF32StringView;
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
    //Unicode版本：数据为wchar_t类型
    //Windows平台为UTF16编码，Linux/macOS平台为UTF32编码
    typedef std::wstring  DString;
#else
    //多字节编码：数据为UTF8编码
    typedef std::string   DString;
#endif

// macOS特定字符串处理辅助函数
#if defined(DUILIB_BUILD_FOR_MACOS)
#include <CoreFoundation/CoreFoundation.h>

namespace duilib {
    // CFStringRef转换为UTF8字符串
    inline DStringA CFStringToUTF8(CFStringRef cfStr) {
        if (!cfStr) return DStringA();
        
        CFIndex length = CFStringGetLength(cfStr);
        CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
        DStringA result;
        result.resize(maxSize);
        
        if (CFStringGetCString(cfStr, &result[0], maxSize, kCFStringEncodingUTF8)) {
            result.resize(strlen(result.c_str()));
            return result;
        }
        return DStringA();
    }
    
    // UTF8字符串转换为CFStringRef
    inline CFStringRef UTF8ToCFString(const DStringA& utf8Str) {
        return CFStringCreateWithCString(kCFAllocatorDefault, 
                                       utf8Str.c_str(), 
                                       kCFStringEncodingUTF8);
    }
}
#endif // DUILIB_BUILD_FOR_MACOS

#endif //DUILIB_STRING_H_

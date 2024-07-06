#ifndef UI_UTILS_STRINGUTIL_H_
#define UI_UTILS_STRINGUTIL_H_

#include "duilib/duilib_defs.h"
#include "duilib/Utils/Unicode.h"
#include <string>
#include <list>

namespace ui
{
/** 字符串/路径操作的辅助类
*/
class UILIB_API StringUtil
{
public:
    // format a string
    static std::wstring Printf(const wchar_t *format, ...);
    static std::string Printf(const char* format, ...);

    // replace all 'find' with 'replace' in the string
    static size_t ReplaceAll(const std::wstring& find, const std::wstring& replace, std::wstring& output);
    static size_t ReplaceAll(const std::string& find, const std::string& replace, std::string& output);

    static void LowerString(std::string& str);
    static void LowerString(std::wstring& str);
    static void UpperString(std::string& str);
    static void UpperString(std::wstring& str);

    static std::wstring MakeLowerString(const std::wstring& str);
    static std::string MakeLowerString(const std::string& str);
    static std::wstring MakeUpperString(const std::wstring& str);
    static std::string MakeUpperString(const std::string& str);

    // the following functions are used to convert encodings in utf-8、utf-16 and utf-32
    static std::wstring UTF8ToUTF16(const UTF8Char* utf8, size_t length);
    static std::wstring UTF8ToUTF16(const std::string& utf8);
    static std::string UTF16ToUTF8(const UTF16Char* utf16, size_t length);
    static std::string UTF16ToUTF8(const std::wstring& utf16);

    static std::string TToUTF8(const std::wstring& str);
    static const std::string& TToUTF8(const std::string& str);

    static DString UTF8ToT(const std::string& utf8);
    static DString UTF8ToT(const UTF8Char* utf8, size_t length);

    static const std::wstring& TToUTF16(const std::wstring& str);
    static std::wstring TToUTF16(const std::string& str);
    static DString UTF16ToT(const std::wstring& utf16);

    static std::basic_string<UTF32Char> UTF8ToUTF32(const UTF8Char* utf8, size_t length);
    static std::basic_string<UTF32Char> UTF8ToUTF32(const std::string& utf8);
    static std::string UTF32ToUTF8(const UTF32Char* utf32, size_t length);
    static std::string UTF32ToUTF8(const std::basic_string<UTF32Char>& utf32);

    static std::basic_string<UTF32Char> UTF16ToUTF32(const UTF16Char* utf16, size_t length);
    static std::basic_string<UTF32Char> UTF16ToUTF32(const std::wstring& utf16);
    static std::wstring UTF32ToUTF16(const UTF32Char* utf32, size_t length);
    static std::wstring UTF32ToUTF16(const std::basic_string<UTF32Char>& utf32);

#ifdef DUILIB_PLATFORM_WIN
    //本地Ansi编码或者UTF8编码等转换成Unicode编码
    static std::wstring MBCSToUnicode(const std::string& input, int32_t code_page = CP_ACP);
    //Unicode编码转换成本地Ansi编码或者UTF8编码等
    static std::string UnicodeToMBCS(const std::wstring& input, int32_t code_page = CP_ACP);
    //本地编码字符串，转换为UTF8或者UTF16编码
    static DString MBCSToT(const std::string& input);
    //返回字符串编码：本地Ansi编码
    static std::string TToMBCS(const DString& input);
#endif

    //非Unicode时：
    //          input是UTF-8编码，Windows平台：返回本地Ansi编码，非Windows平台返回input
    //Unicode时：返回input
    static DString TToLocal(const DString& input);

    //非Unicode时：
    //          Windows平台：input是本地Ansi编码, 返回UTF-8编码
    //        非Windows平台：input是本地UTF-8编码, 返回UTF-8编码
    //Unicode时：返回input
    static DString LocalToT(const DString& input);

    // trimming, removing extra spaces
    static std::string TrimLeft(const char *input);
    static std::string TrimRight(const char* input);
    static std::string Trim(const char* input); /* both left and right */
    static std::string& TrimLeft(std::string& input);
    static std::string& TrimRight(std::string& input);
    static std::string& Trim(std::string& input); /* both left and right */
    static std::wstring TrimLeft(const wchar_t* input);
    static std::wstring TrimRight(const wchar_t* input);
    static std::wstring Trim(const wchar_t* input); /* both left and right */
    static std::wstring& TrimLeft(std::wstring& input);
    static std::wstring& TrimRight(std::wstring& input);
    static std::wstring& Trim(std::wstring& input); /* both left and right */

    // find all tokens splitted by one of the characters in 'delimitor'
    static std::list<std::string> Split(const std::string& input, const std::string& delimitor);
    static std::list<std::wstring> Split(const std::wstring& input, const std::wstring& delimitor);

    //比较字符串是否相等，比较时忽略大小写
    static bool IsEqualNoCase(const std::wstring& lhs, const std::wstring& rhs);
    static bool IsEqualNoCase(const wchar_t* lhs, const std::wstring& rhs);
    static bool IsEqualNoCase(const char* lhs, const std::string& rhs);
    static bool IsEqualNoCase(const std::wstring& lhs, const wchar_t* rhs);
    static bool IsEqualNoCase(const std::string& lhs, const std::string& rhs);
    static bool IsEqualNoCase(const std::string& lhs, const char* rhs);
    static bool IsEqualNoCase(const wchar_t* lhs, const wchar_t* rhs);
    static bool IsEqualNoCase(const char* lhs, const char* rhs);

    /** 比较字符串，区分大小写
    @param [in] lhs 左值字符串
    @param [in] rhs 右值字符串
    @return 返回值定义如下：
            0  : 相等
            1  : lhs > rhs
           -1  : lhs < rhs
    */
    static int32_t StringCompare(const std::wstring& lhs, const std::wstring& rhs);
    static int32_t StringCompare(const wchar_t* lhs, const wchar_t* rhs);
    static int32_t StringCompare(const std::string& lhs, const std::string& rhs);
    static int32_t StringCompare(const char* lhs, const char* rhs);

    /** 比较字符串，不区分大小写
    @param [in] lhs 左值字符串
    @param [in] rhs 右值字符串
    @return 返回值定义如下：
            0  : 相等
            1  : lhs > rhs
           -1  : lhs < rhs
    */
    static int32_t StringICompare(const std::wstring& lhs, const std::wstring& rhs);
    static int32_t StringICompare(const wchar_t* lhs, const wchar_t* rhs);
    static int32_t StringICompare(const std::string& lhs, const std::string& rhs);
    static int32_t StringICompare(const char* lhs, const char* rhs);

    //将整型转换为字符串
    static std::wstring UInt64ToStringW(uint64_t value);
    static std::wstring UInt32ToStringW(uint32_t value);
    static std::string UInt64ToStringA(uint64_t value);
    static std::string UInt32ToStringA(uint32_t value);

#ifdef DUILIB_UNICODE
    static std::wstring UInt64ToString(uint64_t value);
    static std::wstring UInt32ToString(uint32_t value);
#else
    static std::string UInt64ToString(uint64_t value);
    static std::string UInt32ToString(uint32_t value);
#endif

    /** 将字符串转换成整型(十进制)
    */
    static int32_t StringToInt32(const std::wstring& str);
    static int32_t StringToInt32(const std::wstring::value_type* str);
    static int32_t StringToInt32(const std::string& str);
    static int32_t StringToInt32(const std::string::value_type* str);

    /** 将字符串转换成整型(多进制)
    */
    static int32_t StringToInt32(const wchar_t* str, wchar_t** pEndPtr, int32_t nRadix);
    static int32_t StringToInt32(const char* str, char** pEndPtr, int32_t nRadix);
    static uint32_t StringToUInt32(const wchar_t* str, wchar_t** pEndPtr, int32_t nRadix);
    static uint32_t StringToUInt32(const char* str, char** pEndPtr, int32_t nRadix);

    static int64_t StringToInt64(const std::wstring& str);
    static int64_t StringToInt64(const std::wstring::value_type* str);
    static int64_t StringToInt64(const std::string& str);
    static int64_t StringToInt64(const std::string::value_type* str);

    /** 将字符串转换成浮点型(十进制)
    */
    static double StringToDouble(const std::wstring& str);
    static double StringToDouble(const std::wstring::value_type* str);
    static double StringToDouble(const std::string& str);
    static double StringToDouble(const std::string::value_type* str);

    /** 将字符串转换成浮点型(十进制)
    */
    static float StringToFloat(const wchar_t* str, wchar_t** pEndPtr);
    static float StringToFloat(const char* str, char** pEndPtr);

    /** 复制字符串
    */
    template <size_t _Size>
    static inline int32_t StringCopy(wchar_t(&dest)[_Size], const wchar_t* src)
    {
        return StringCopy(dest, _Size, src);
    }

    static int32_t StringCopy(wchar_t* dest, size_t destSize, const wchar_t* src);

    template <size_t _Size>
    static inline int32_t StringCopy(char(&dest)[_Size], const char* src)
    {
        return StringCopy(dest, _Size, src);
    }

    static int32_t StringCopy(char* dest, size_t destSize, const char* src);

    /** 计算字符串的长度
    */
    static size_t StringLen(const wchar_t* str);
    static size_t StringLen(const char* str);
};

}

#endif // UI_UTILS_STRINGUTIL_H_

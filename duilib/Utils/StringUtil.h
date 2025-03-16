#ifndef UI_UTILS_STRINGUTIL_H_
#define UI_UTILS_STRINGUTIL_H_

#include "duilib/duilib_defs.h"
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

    static std::wstring Int64ToStringW(int64_t value);
    static std::wstring Int32ToStringW(int32_t value);
    static std::string Int64ToStringA(int64_t value);
    static std::string Int32ToStringA(int32_t value);

#ifdef DUILIB_UNICODE
    static std::wstring UInt64ToString(uint64_t value);
    static std::wstring UInt32ToString(uint32_t value);
    static std::wstring Int64ToString(int64_t value);
    static std::wstring Int32ToString(int32_t value);
#else
    static std::string UInt64ToString(uint64_t value);
    static std::string UInt32ToString(uint32_t value);
    static std::string Int64ToString(int64_t value);
    static std::string Int32ToString(int32_t value);
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
    static int32_t StringNCopy(wchar_t* dest, size_t destSize, const wchar_t* src, size_t srcSize);

    template <size_t _Size>
    static inline int32_t StringCopy(char(&dest)[_Size], const char* src)
    {
        return StringCopy(dest, _Size, src);
    }

    static int32_t StringCopy(char* dest, size_t destSize, const char* src);
    static int32_t StringNCopy(char* dest, size_t destSize, const char* src, size_t srcSize);

    /** 计算字符串的长度
    */
    static size_t StringLen(const wchar_t* str);
    static size_t StringLen(const char* str);
};

}

#endif // UI_UTILS_STRINGUTIL_H_

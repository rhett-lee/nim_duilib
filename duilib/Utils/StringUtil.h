#ifndef UI_UTILS_STRINGUTIL_H_
#define UI_UTILS_STRINGUTIL_H_

#include "duilib/duilib_defs.h"
#include "duilib/Utils/Unicode.h"
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

    static std::wstring MakeLowerString(const std::wstring&str);
    static std::string MakeLowerString(const std::string& str);
    static std::wstring MakeUpperString(const std::wstring&str);
    static std::string MakeUpperString(const std::string& str);

    // the following functions are used to convert encodings in utf-8、utf-16 and utf-32
    static std::wstring UTF8ToUTF16(const UTF8Char* utf8, size_t length);
    static std::wstring UTF8ToUTF16(const std::string& utf8);
    static std::string UTF16ToUTF8(const UTF16Char* utf16, size_t length);
    static std::string UTF16ToUTF8(const std::wstring& utf16);

    static std::string TToUTF8(const DString& str);
    static DString UTF8ToT(const std::string& utf8);
    static DString UTF8ToT(const UTF8Char* utf8, size_t length);

    static std::basic_string<UTF32Char> UTF8ToUTF32(const UTF8Char* utf8, size_t length);
    static std::basic_string<UTF32Char> UTF8ToUTF32(const std::string& utf8);
    static std::string UTF32ToUTF8(const UTF32Char* utf32, size_t length);
    static std::string UTF32ToUTF8(const std::basic_string<UTF32Char>& utf32);

    static std::basic_string<UTF32Char> UTF16ToUTF32(const UTF16Char* utf16, size_t length);
    static std::basic_string<UTF32Char> UTF16ToUTF32(const DString& utf16);
    static std::wstring UTF32ToUTF16(const UTF32Char* utf32, size_t length);
    static std::wstring UTF32ToUTF16(const std::basic_string<UTF32Char>& utf32);

#ifdef DUILIB_PLATFORM_WIN
    static bool MBCSToUnicode(const std::string &input, DString& output, int code_page = CP_ACP);
    static bool UnicodeToMBCS(const DString& input, std::string &output, int code_page = CP_ACP);
#endif
    // trimming, removing extra spaces
    static std::string TrimLeft(const char *input);
    static std::string TrimRight(const char *input);
    static std::string Trim(const char *input); /* both left and right */
    static std::string& TrimLeft(std::string &input);
    static std::string& TrimRight(std::string &input);
    static std::string& Trim(std::string &input); /* both left and right */
    static std::wstring TrimLeft(const wchar_t *input);
    static std::wstring TrimRight(const wchar_t *input);
    static std::wstring Trim(const wchar_t *input); /* both left and right */
    static std::wstring& TrimLeft(std::wstring&input);
    static std::wstring& TrimRight(std::wstring&input);
    static std::wstring& Trim(std::wstring&input); /* both left and right */

    // find all tokens splitted by one of the characters in 'delimitor'
    static std::list<std::string> Split(const std::string& input, const std::string& delimitor);
    static std::list<DString> Split(const std::wstring& input, const std::wstring& delimitor);

    //比较字符串是否相等，比较时忽略大小写
    static bool IsEqualNoCase(const std::wstring& lhs, const std::wstring& rhs);
    static bool IsEqualNoCase(const wchar_t* lhs, const std::wstring& rhs);
    static bool IsEqualNoCase(const char* lhs, const std::string& rhs);
    static bool IsEqualNoCase(const std::wstring& lhs, const wchar_t* rhs);
    static bool IsEqualNoCase(const std::string& lhs, const char* rhs);
    static bool IsEqualNoCase(const wchar_t* lhs, const wchar_t* rhs);
    static bool IsEqualNoCase(const char* lhs, const char* rhs);

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

};

}

#endif // UI_UTILS_STRINGUTIL_H_

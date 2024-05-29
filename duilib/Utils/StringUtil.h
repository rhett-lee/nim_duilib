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
    static DString Printf(const wchar_t *format, ...);
    static std::string Printf(const char* format, ...);

    // replace all 'find' with 'replace' in the string
    static size_t ReplaceAll(const DString& find, const DString& replace, DString& output);
    static size_t ReplaceAll(const std::string& find, const std::string& replace, std::string& output);

    static void LowerString(std::string& str);
    static void LowerString(DString& str);
    static void UpperString(std::string& str);
    static void UpperString(DString& str);

    static DString MakeLowerString(const DString &str);
    static std::string MakeLowerString(const std::string& str);
    static DString MakeUpperString(const DString &str);
    static std::string MakeUpperString(const std::string& str);

    // the following functions are used to convert encodings in utf-8、utf-16 and utf-32
    static DString UTF8ToUTF16(const UTF8Char* utf8, size_t length);
    static std::string UTF16ToUTF8(const UTF16Char* utf16, size_t length);
    static std::basic_string<UTF32Char> UTF8ToUTF32(const UTF8Char* utf8, size_t length);
    static std::string UTF32ToUTF8(const UTF32Char* utf32, size_t length);
    static std::basic_string<UTF32Char> UTF16ToUTF32(const UTF16Char* utf16, size_t length);
    static DString UTF32ToUTF16(const UTF32Char* utf32, size_t length);
    static DString UTF8ToUTF16(const std::string& utf8);
    static std::string UTF16ToUTF8(const std::wstring& utf16);
    static std::basic_string<UTF32Char> UTF8ToUTF32(const std::string& utf8);
    static std::string UTF32ToUTF8(const std::basic_string<UTF32Char>& utf32);
    static std::basic_string<UTF32Char> UTF16ToUTF32(const DString& utf16);
    static DString UTF32ToUTF16(const std::basic_string<UTF32Char>& utf32);

    static std::string UToUTF8(const DString& str);

    static bool MBCSToUnicode(const char *input, DString& output, int code_page = CP_ACP);
    static bool MBCSToUnicode(const std::string &input, DString& output, int code_page = CP_ACP);
    static bool UnicodeToMBCS(const wchar_t *input, std::string &output, int code_page = CP_ACP);
    static bool UnicodeToMBCS(const DString& input, std::string &output, int code_page = CP_ACP);

    // trimming, removing extra spaces
    static std::string TrimLeft(const char *input);
    static std::string TrimRight(const char *input);
    static std::string Trim(const char *input); /* both left and right */
    static std::string& TrimLeft(std::string &input);
    static std::string& TrimRight(std::string &input);
    static std::string& Trim(std::string &input); /* both left and right */
    static DString TrimLeft(const wchar_t *input);
    static DString TrimRight(const wchar_t *input);
    static DString Trim(const wchar_t *input); /* both left and right */
    static DString& TrimLeft(DString &input);
    static DString& TrimRight(DString &input);
    static DString& Trim(DString &input); /* both left and right */

    // find all tokens splitted by one of the characters in 'delimitor'
    static std::list<std::string> Split(const std::string& input, const std::string& delimitor);
    static std::list<DString> Split(const DString& input, const DString& delimitor);

    //比较字符串是否相等，比较时忽略大小写
    static bool IsEqualNoCase(const DString& lhs, const DString& rhs);
    static bool IsEqualNoCase(const wchar_t* lhs, const DString& rhs);
    static bool IsEqualNoCase(const DString& lhs, const wchar_t* rhs);
    static bool IsEqualNoCase(const wchar_t* lhs, const wchar_t* rhs);

    //将整型转换为字符串
    static DString UInt64ToString(uint64_t value);
    static DString UInt32ToString(uint32_t value);
};

}

#endif // UI_UTILS_STRINGUTIL_H_

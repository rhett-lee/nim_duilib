#ifndef UI_UTILS_STRING_CONVERT_H_
#define UI_UTILS_STRING_CONVERT_H_

#include "duilib/duilib_defs.h"

namespace ui
{
/** 字符串编码转换辅助类
*/
class UILIB_API StringConvert
{
public:
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
#ifdef DUILIB_UNICODE
    static const DString& UTF16ToT(const std::wstring& utf16);
#else
    static DString UTF16ToT(const std::wstring& utf16);
#endif

    static std::basic_string<UTF32Char> UTF8ToUTF32(const UTF8Char* utf8, size_t length);
    static std::basic_string<UTF32Char> UTF8ToUTF32(const std::string& utf8);
    static std::string UTF32ToUTF8(const UTF32Char* utf32, size_t length);
    static std::string UTF32ToUTF8(const std::basic_string<UTF32Char>& utf32);

    static std::basic_string<UTF32Char> UTF16ToUTF32(const UTF16Char* utf16, size_t length);
    static std::basic_string<UTF32Char> UTF16ToUTF32(const std::wstring& utf16);
    static std::wstring UTF32ToUTF16(const UTF32Char* utf32, size_t length);
    static std::wstring UTF32ToUTF16(const std::basic_string<UTF32Char>& utf32);

#ifdef DUILIB_BUILD_FOR_WIN
    //本地Ansi编码或者UTF8编码等转换成Unicode编码
    static std::wstring MBCSToUnicode(const std::string& input, int32_t code_page = CP_ACP);
    static std::wstring MBCSToUnicode2(const char* input, size_t inputSize, int32_t code_page = CP_ACP);

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
};

} //namespace ui

#endif // UI_UTILS_STRING_CONVERT_H_

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
    //UTF8字符串转换为UTF16
    static std::basic_string<DUTF16Char> UTF8ToUTF16(const DUTF8Char* utf8, size_t length);

    //UTF8字符串转换为DStringW
    static DStringW UTF8ToWString(const std::string& utf8);

    //UTF16字符串转换为UTF8字符串
    static std::string UTF16ToUTF8(const DUTF16Char* utf16, size_t length);

    //DStringW字符串转换为UTF8字符串
    static std::string WStringToUTF8(const DStringW& wstr);

    //DString转换为UTF8字符串
    static std::string TToUTF8(const std::wstring& str);
    static const std::string& TToUTF8(const std::string& str);

    //UTF8字符串转换为DString
    static DString UTF8ToT(const std::string& utf8);
    static DString UTF8ToT(const DUTF8Char* utf8, size_t length);

    //DString转换为DStringW
    static const DStringW& TToWString(const std::wstring& str);
    static DStringW TToWString(const std::string& str);

    //DStringW转换为DString
#ifdef DUILIB_UNICODE
    static const DString& WStringToT(const std::wstring& wstr);
#else
    static DString WStringToT(const std::wstring& wstr);
#endif

    //UTF8转换为UTF32字符串
    static std::basic_string<DUTF32Char> UTF8ToUTF32(const DUTF8Char* utf8, size_t length);
    static std::basic_string<DUTF32Char> UTF8ToUTF32(const std::string& utf8);

    //UTF32转换为UTF8字符串
    static std::string UTF32ToUTF8(const DUTF32Char* utf32, size_t length);
    static std::string UTF32ToUTF8(const std::basic_string<DUTF32Char>& utf32);

    //UTF16转换为UTF32字符串
    static std::basic_string<DUTF32Char> UTF16ToUTF32(const DUTF16Char* utf16, size_t length);

    //DStringW字符串转换为UTF32字符串
    static std::basic_string<DUTF32Char> WStringToUTF32(const DStringW& wstr);

    //UTF32字符串转换为DStringW
    static DStringW UTF32ToWString(const DUTF32Char* utf32, size_t length);
    static DStringW UTF32ToWString(const std::basic_string<DUTF32Char>& utf32);

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
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_UNICODE)
    static DString TToLocal(const DString& input);
#else
    static const DString& TToLocal(const DString& input);
#endif

    //非Unicode时：
    //          Windows平台：input是本地Ansi编码, 返回UTF-8编码
    //          非Windows平台：input是本地UTF-8编码, 返回UTF-8编码
    //Unicode时：返回input
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_UNICODE)
    static DString LocalToT(const DString& input);
#else
    static const DString& LocalToT(const DString& input);
#endif
};

} //namespace ui

#endif // UI_UTILS_STRING_CONVERT_H_

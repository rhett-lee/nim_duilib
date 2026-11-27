#ifndef UI_UTILS_STRING_CONVERT_H_
#define UI_UTILS_STRING_CONVERT_H_

#include "duilib/duilib_defs.h"

namespace ui
{
/** 字符串编码转换辅助类，功能要点如下：
*  1. 支持 UTF8、UTF16、UTF32 编码之间的相互转换；
*  2. 支持 std::wstring（Windows 平台为 UTF16 编码，其他平台为 UTF32 编码）与 UTF8 的相互转换；
*  3. 支持 DString（内部字符串类型，由 DUILIB_UNICODE 决定）与 UTF8、宽字符的相互转换；
*  4. Windows 平台支持本地 Ansi 编码（或指定代码页）与 Unicode 的相互转换。
*  说明：输入为空指针或长度为0时返回空字符串；遇到非法的源字节时返回空字符串；
*        源字符串末尾存在未完成的多字节序列时，保留已转换的有效部分。
*/
class DUILIB_API StringConvert
{
public:
    /** UTF8字符串转换为UTF16字符串
    * @param [in] utf8 源字符串（UTF8 编码）
    * @param [in] length 源字符串的长度（UTF8 字节数，不含结尾的 '\0'）
    * @return 返回转换后的 UTF16 字符串，输入为空时返回空字符串
    */
    static std::basic_string<DUTF16Char> UTF8ToUTF16(const DUTF8Char* utf8, size_t length);

    /** UTF8字符串转换为std::wstring
    * @param [in] utf8 源字符串（UTF8 编码）
    * @return 返回转换后的宽字符串（Windows 平台为 UTF16 编码，其他平台为 UTF32 编码）
    */
    static DStringW UTF8ToWString(const std::string& utf8);

    /** UTF16字符串转换为UTF8字符串
    * @param [in] utf16 源字符串（UTF16 编码）
    * @param [in] length 源字符串的长度（UTF16 字符数，不含结尾的 '\0'）
    * @return 返回转换后的 UTF8 字符串，输入为空时返回空字符串
    */
    static std::string UTF16ToUTF8(const DUTF16Char* utf16, size_t length);

    /** DStringW字符串转换为UTF8字符串
    * @param [in] wstr 源字符串（Windows 平台为 UTF16 编码，其他平台为 UTF32 编码）
    * @return 返回转换后的 UTF8 字符串，输入为空时返回空字符串
    */
    static std::string WStringToUTF8(const DStringW& wstr);

    /** DString转换为UTF8字符串
    * @param [in] str 源字符串
    * @return 传入 std::wstring 时返回转换后的 UTF8 字符串；
    *         传入 std::string 时不做转换，直接返回其引用（调用方需保证 str 在使用期间有效）
    */
    static std::string TToUTF8(const std::wstring& str);
    static const std::string& TToUTF8(const std::string& str);

    /** UTF8字符串转换为DString
    * @param [in] utf8 源字符串（UTF8 编码），可以是 std::string 或字符指针加长度
    * @param [in] length 源字符串的长度（UTF8 字节数，不含结尾的 '\0'）
    * @return 返回转换后的 DString（Unicode 模式下转换为 UTF16 编码；非Unicode模式下返回 UTF8 编码的字符串，不做编码转换）
    */
    static DString UTF8ToT(const std::string& utf8);
    static DString UTF8ToT(const DUTF8Char* utf8, size_t length);

    /** DString转换为DStringW
    * @param [in] str 源字符串
    * @return 传入 std::wstring 时直接返回其引用（不做转换，调用方需保证 str 在使用期间有效）；
    *         传入 std::string 时返回转换后的宽字符串
    */
    static const DStringW& TToWString(const std::wstring& str);
    static DStringW TToWString(const std::string& str);

    /** DStringW转换为DString
    * @param [in] wstr 源宽字符串
    * @return Unicode 模式下直接返回其引用（不做转换）；
    *         非Unicode模式下返回转换为 UTF8 编码后的字符串
    */
#ifdef DUILIB_UNICODE
    static const DString& WStringToT(const std::wstring& wstr);
#else
    static DString WStringToT(const std::wstring& wstr);
#endif

    /** UTF8转换为UTF32字符串
    * @param [in] utf8 源字符串（UTF8 编码），可以是 std::string 或字符指针加长度
    * @param [in] length 源字符串的长度（UTF8 字节数，不含结尾的 '\0'）
    * @return 返回转换后的 UTF32 字符串，输入为空时返回空字符串
    */
    static std::basic_string<DUTF32Char> UTF8ToUTF32(const DUTF8Char* utf8, size_t length);
    static std::basic_string<DUTF32Char> UTF8ToUTF32(const std::string& utf8);

    /** UTF32转换为UTF8字符串
    * @param [in] utf32 源字符串（UTF32 编码），可以是字符指针加长度或 std::basic_string
    * @param [in] length 源字符串的长度（UTF32 字符数，不含结尾的 '\0'）
    * @return 返回转换后的 UTF8 字符串，输入为空时返回空字符串
    */
    static std::string UTF32ToUTF8(const DUTF32Char* utf32, size_t length);
    static std::string UTF32ToUTF8(const std::basic_string<DUTF32Char>& utf32);

    /** UTF16转换为UTF32字符串
    * @param [in] utf16 源字符串（UTF16 编码）
    * @param [in] length 源字符串的长度（UTF16 字符数，不含结尾的 '\0'）
    * @return 返回转换后的 UTF32 字符串，输入为空时返回空字符串
    */
    static std::basic_string<DUTF32Char> UTF16ToUTF32(const DUTF16Char* utf16, size_t length);

    /** DStringW字符串转换为UTF32字符串
    * @param [in] wstr 源宽字符串
    * @return 返回转换后的 UTF32 字符串，输入为空时返回空字符串
    */
    static std::basic_string<DUTF32Char> WStringToUTF32(const DStringW& wstr);

    /** UTF32字符串转换为DStringW
    * @param [in] utf32 源字符串（UTF32 编码），可以是字符指针加长度或 std::basic_string
    * @param [in] length 源字符串的长度（UTF32 字符数，不含结尾的 '\0'）
    * @return 返回转换后的宽字符串，输入为空时返回空字符串
    */
    static DStringW UTF32ToWString(const DUTF32Char* utf32, size_t length);
    static DStringW UTF32ToWString(const std::basic_string<DUTF32Char>& utf32);

#ifdef DUILIB_BUILD_FOR_WIN
    /** 本地Ansi编码或者UTF8编码等转换成Unicode编码
    * @param [in] input 源字符串（多字节编码）
    * @param [in] code_page 源字符串的代码页，默认 CP_ACP（本地Ansi编码）
    * @return 返回转换后的Unicode（宽字符）字符串，转换失败或输入为空时返回空字符串
    */
    static std::wstring MBCSToUnicode(const std::string& input, int32_t code_page = CP_ACP);

    /** 本地Ansi编码或者UTF8编码等转换成Unicode编码（字符指针加长度版本，函数名后缀2用于区分重载）
    * @param [in] input 源字符串（多字节编码）
    * @param [in] inputSize 源字符串的长度（字节数，不含结尾的 '\0'）
    * @param [in] code_page 源字符串的代码页，默认 CP_ACP（本地Ansi编码）
    * @return 返回转换后的Unicode（宽字符）字符串，转换失败或输入为空时返回空字符串
    */
    static std::wstring MBCSToUnicode2(const char* input, size_t inputSize, int32_t code_page = CP_ACP);

    /** Unicode编码转换成本地Ansi编码或者UTF8编码等
    * @param [in] input 源宽字符串
    * @param [in] code_page 目标代码页，默认 CP_ACP（本地Ansi编码）
    * @return 返回转换后的多字节字符串，转换失败或输入为空时返回空字符串
    */
    static std::string UnicodeToMBCS(const std::wstring& input, int32_t code_page = CP_ACP);

    /** 本地编码（Ansi）字符串转换为DString
    * @param [in] input 源字符串（本地Ansi编码）
    * @return 返回转换后的 DString（Unicode 模式下为 UTF16 编码，非Unicode模式下为 UTF8 编码）
    */
    static DString MBCSToT(const std::string& input);

    /** DString转换成本地Ansi编码字符串
    * @param [in] input 源字符串
    * @return 返回本地Ansi编码的字符串
    */
    static std::string TToMBCS(const DString& input);
#endif

    /** 将内部字符串编码转换为本地编码
    * 非Unicode时: input是UTF-8编码，Windows平台返回本地Ansi编码，非Windows平台返回input；
    * Unicode时: 返回input
    * @param [in] input 源字符串
    * @return 返回转换后的字符串（无需转换时返回其引用）
    */
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_UNICODE)
    static DString TToLocal(const DString& input);
#else
    static const DString& TToLocal(const DString& input);
#endif

    /** 将本地编码字符串转换为内部字符串编码
    * 非Unicode时: Windows平台 input 是本地Ansi编码，返回UTF-8编码；
    *              非Windows平台 input 是本地UTF-8编码，返回UTF-8编码；
    * Unicode时: 返回input
    * @param [in] input 源字符串
    * @return 返回转换后的字符串（无需转换时返回其引用）
    */
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_UNICODE)
    static DString LocalToT(const DString& input);
#else
    static const DString& LocalToT(const DString& input);
#endif
};

} //namespace ui

#endif // UI_UTILS_STRING_CONVERT_H_

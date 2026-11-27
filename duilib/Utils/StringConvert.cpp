#include "StringConvert.h"
#include "duilib/third_party/convert_utf/ConvertUTF.h"
#include <vector>
#include <cstring>
#include <climits>
#include <limits>

namespace ui
{
namespace
{
/** 判断 size * unitSize 是否会导致长度计算溢出
*/
bool IsSizeOverflow(size_t size, size_t unitSize)
{
    return size > (std::numeric_limits<size_t>::max() / unitSize);
}

/** 把UTF16编码单元复制为DUTF16Char字符串（避免类型双关）
*/
std::basic_string<DUTF16Char> MakeUTF16String(const std::vector<llvm::UTF16>& buffer, size_t count)
{
    std::basic_string<DUTF16Char> result;
    result.resize(count);
    if (count > 0) {
        static_assert(sizeof(DUTF16Char) == sizeof(llvm::UTF16), "DUTF16Char size mismatch");
        memcpy(&result[0], buffer.data(), count * sizeof(DUTF16Char));
    }
    return result;
}

/** 把UTF32编码单元复制为DUTF32Char字符串（避免类型双关）
*/
std::basic_string<DUTF32Char> MakeUTF32String(const std::vector<llvm::UTF32>& buffer, size_t count)
{
    std::basic_string<DUTF32Char> result;
    result.resize(count);
    if (count > 0) {
        static_assert(sizeof(DUTF32Char) == sizeof(llvm::UTF32), "DUTF32Char size mismatch");
        memcpy(&result[0], buffer.data(), count * sizeof(DUTF32Char));
    }
    return result;
}

/** 把UTF8编码单元复制为std::string
*/
std::string MakeUTF8String(const std::vector<llvm::UTF8>& buffer, size_t count)
{
    std::string result;
    result.resize(count);
    if (count > 0) {
        memcpy(&result[0], buffer.data(), count);
    }
    return result;
}

/** 把DUTF16Char输入复制为UTF16编码单元（避免类型双关）
*/
std::vector<llvm::UTF16> CopyToUTF16(const DUTF16Char* data, size_t length)
{
    std::vector<llvm::UTF16> units(length);
    if (length > 0) {
        static_assert(sizeof(DUTF16Char) == sizeof(llvm::UTF16), "DUTF16Char size mismatch");
        memcpy(units.data(), data, length * sizeof(llvm::UTF16));
    }
    return units;
}

/** 把DUTF32Char输入复制为UTF32编码单元（避免类型双关）
*/
std::vector<llvm::UTF32> CopyToUTF32(const DUTF32Char* data, size_t length)
{
    std::vector<llvm::UTF32> units(length);
    if (length > 0) {
        static_assert(sizeof(DUTF32Char) == sizeof(llvm::UTF32), "DUTF32Char size mismatch");
        memcpy(units.data(), data, length * sizeof(llvm::UTF32));
    }
    return units;
}

}//namespace

std::basic_string<DUTF16Char> StringConvert::UTF8ToUTF16(const DUTF8Char* utf8, size_t length)
{
    if ((utf8 == nullptr) || (length == 0)) {
        return std::basic_string<DUTF16Char>();
    }
    if (IsSizeOverflow(length, sizeof(llvm::UTF16))) {
        return std::basic_string<DUTF16Char>();
    }
    //UTF-8每个字节最多转换为一个UTF-16编码单元，输出缓冲以length为上界即可
    std::vector<llvm::UTF16> buffer(length);
    const llvm::UTF8* srcBegin = reinterpret_cast<const llvm::UTF8*>(utf8);
    const llvm::UTF8* srcEnd = srcBegin + length;
    llvm::UTF16* dstBegin = buffer.data();
    const llvm::ConversionResult result = llvm::ConvertUTF8toUTF16(&srcBegin,
                                                                   srcEnd,
                                                                   &dstBegin,
                                                                   buffer.data() + buffer.size(),
                                                                   llvm::strictConversion);
    if ((result == llvm::sourceIllegal) || (result == llvm::targetExhausted)) {
        return std::basic_string<DUTF16Char>();
    }
    //conversionOK 或 sourceExhausted（末尾不完整序列）时，保留已转换的有效部分
    return MakeUTF16String(buffer, static_cast<size_t>(dstBegin - buffer.data()));
}

std::basic_string<DUTF16Char> StringConvert::UTF8ToUTF16(const std::string& utf8)
{
    return UTF8ToUTF16(utf8.c_str(), utf8.length());
}

DStringW StringConvert::UTF8ToWString(const std::string& utf8)
{
    return UTF8ToWString(utf8.c_str(), utf8.length());
}

DStringW StringConvert::UTF8ToWString(const DUTF8Char* utf8, size_t length)
{
#if defined(WCHAR_T_IS_UTF16)
    return UTF8ToUTF16(utf8, length);
#else
    return UTF8ToUTF32(utf8, length);
#endif
}

std::string StringConvert::UTF16ToUTF8(const DUTF16Char* utf16, size_t length)
{
    if ((utf16 == nullptr) || (length == 0)) {
        return std::string();
    }
    //每个UTF-16编码单元最多转换为3字节UTF-8（代理对2个单元转4字节），输出缓冲以3 * length为上界
    if (IsSizeOverflow(length, 3)) {
        return std::string();
    }
    const std::vector<llvm::UTF16> src = CopyToUTF16(utf16, length);
    std::vector<llvm::UTF8> buffer(length * 3);
    const llvm::UTF16* srcBegin = src.data();
    const llvm::UTF16* srcEnd = srcBegin + src.size();
    llvm::UTF8* dstBegin = buffer.data();
    const llvm::ConversionResult result = llvm::ConvertUTF16toUTF8(&srcBegin,
                                                                   srcEnd,
                                                                   &dstBegin,
                                                                   buffer.data() + buffer.size(),
                                                                   llvm::strictConversion);
    if ((result == llvm::sourceIllegal) || (result == llvm::targetExhausted)) {
        return std::string();
    }
    return MakeUTF8String(buffer, static_cast<size_t>(dstBegin - buffer.data()));
}

std::string StringConvert::UTF16ToUTF8(const std::basic_string<DUTF16Char>& utf16)
{
    return UTF16ToUTF8(utf16.c_str(), utf16.length());
}

DStringW StringConvert::UTF16ToWString(const DUTF16Char* utf16, size_t length)
{
    if ((utf16 == nullptr) || (length == 0)) {
        return DStringW();
    }
#if defined(WCHAR_T_IS_UTF16)
    static_assert(sizeof(DUTF16Char) == sizeof(std::wstring::value_type), "DUTF16Char size mismatch");
    //孤立代理项不具备对应的Unicode码点，按非法输入处理
    if (!IsValidUTF16(utf16, length)) {
        return DStringW();
    }
    return DStringW(utf16, length);
#else
    //非Windows平台：wchar_t为32位，先转换为UTF32
    return UTF32ToWString(UTF16ToUTF32(utf16, length));
#endif
}

DStringW StringConvert::UTF16ToWString(const std::basic_string<DUTF16Char>& utf16)
{
    return UTF16ToWString(utf16.c_str(), utf16.length());
}

std::string StringConvert::WStringToUTF8(const DStringW& wstr)
{
#if defined(WCHAR_T_IS_UTF16)
    static_assert(sizeof(DUTF16Char) == sizeof(std::wstring::value_type), "DUTF16Char size mismatch");
    return UTF16ToUTF8(wstr.c_str(), wstr.length());
#else
    static_assert(sizeof(DUTF32Char) == sizeof(std::wstring::value_type), "DUTF32Char size mismatch");
    return UTF32ToUTF8(wstr.c_str(), wstr.length());
#endif
}

std::basic_string<DUTF16Char> StringConvert::WStringToUTF16(const DStringW& wstr)
{
#if defined(WCHAR_T_IS_UTF16)
    static_assert(sizeof(DUTF16Char) == sizeof(std::wstring::value_type), "DUTF16Char size mismatch");
    return std::basic_string<DUTF16Char>(wstr);
#else
    static_assert(sizeof(DUTF32Char) == sizeof(std::wstring::value_type), "DUTF32Char size mismatch");
    return UTF32ToUTF16(wstr.c_str(), wstr.length());
#endif
}

std::string StringConvert::TToUTF8(const std::wstring& str)
{
    return StringConvert::WStringToUTF8(str);
}

const std::string& StringConvert::TToUTF8(const std::string& str)
{
    return str;
}

DString StringConvert::UTF8ToT(const DUTF8Char* utf8, size_t length)
{
    if ((utf8 == nullptr) || (length == 0)) {
        return DString();
    }
#ifdef DUILIB_UNICODE
    return StringConvert::UTF8ToUTF16(utf8, length);
#else
    return std::string(utf8, length);
#endif
}

DString StringConvert::UTF8ToT(const std::string& utf8)
{
#ifdef DUILIB_UNICODE
    return StringConvert::UTF8ToWString(utf8);
#else
    return utf8;
#endif
}

const DStringW& StringConvert::TToWString(const std::wstring& str)
{
    return str;
}

DStringW StringConvert::TToWString(const std::string& str)
{
    return StringConvert::UTF8ToWString(str);
}

#ifdef DUILIB_UNICODE
const DString& StringConvert::WStringToT(const std::wstring& wstr)
{
    return wstr;
}
#else
DString StringConvert::WStringToT(const std::wstring& wstr)
{
    return StringConvert::WStringToUTF8(wstr);
}
#endif

std::basic_string<DUTF32Char> StringConvert::UTF8ToUTF32(const DUTF8Char* utf8, size_t length)
{
    if ((utf8 == nullptr) || (length == 0)) {
        return std::basic_string<DUTF32Char>();
    }
    //UTF-8每个字节最多转换为一个UTF-32编码单元，输出缓冲以length为上界即可
    if (IsSizeOverflow(length, sizeof(llvm::UTF32))) {
        return std::basic_string<DUTF32Char>();
    }
    std::vector<llvm::UTF32> buffer(length);
    const llvm::UTF8* srcBegin = reinterpret_cast<const llvm::UTF8*>(utf8);
    const llvm::UTF8* srcEnd = srcBegin + length;
    llvm::UTF32* dstBegin = buffer.data();
    const llvm::ConversionResult result = llvm::ConvertUTF8toUTF32(&srcBegin,
                                                                   srcEnd,
                                                                   &dstBegin,
                                                                   buffer.data() + buffer.size(),
                                                                   llvm::strictConversion);
    if ((result == llvm::sourceIllegal) || (result == llvm::targetExhausted)) {
        return std::basic_string<DUTF32Char>();
    }
    return MakeUTF32String(buffer, static_cast<size_t>(dstBegin - buffer.data()));
}

std::basic_string<DUTF32Char> StringConvert::UTF8ToUTF32(const std::string& utf8)
{
    return UTF8ToUTF32(utf8.c_str(), utf8.length());
}

std::string StringConvert::UTF32ToUTF8(const DUTF32Char* utf32, size_t length)
{
    if ((utf32 == nullptr) || (length == 0)) {
        return std::string();
    }
    //每个UTF-32编码单元最多转换为4字节UTF-8，输出缓冲以4 * length为上界
    if (IsSizeOverflow(length, 4)) {
        return std::string();
    }
    const std::vector<llvm::UTF32> src = CopyToUTF32(utf32, length);
    std::vector<llvm::UTF8> buffer(length * 4);
    const llvm::UTF32* srcBegin = src.data();
    const llvm::UTF32* srcEnd = srcBegin + src.size();
    llvm::UTF8* dstBegin = buffer.data();
    const llvm::ConversionResult result = llvm::ConvertUTF32toUTF8(&srcBegin,
                                                                   srcEnd,
                                                                   &dstBegin,
                                                                   buffer.data() + buffer.size(),
                                                                   llvm::strictConversion);
    if ((result == llvm::sourceIllegal) || (result == llvm::targetExhausted)) {
        return std::string();
    }
    return MakeUTF8String(buffer, static_cast<size_t>(dstBegin - buffer.data()));
}

std::string StringConvert::UTF32ToUTF8(const std::basic_string<DUTF32Char>& utf32)
{
    return UTF32ToUTF8(utf32.c_str(), utf32.length());
}

std::basic_string<DUTF32Char> StringConvert::UTF16ToUTF32(const DUTF16Char* utf16, size_t length)
{
    if ((utf16 == nullptr) || (length == 0)) {
        return std::basic_string<DUTF32Char>();
    }
    //每个UTF-16编码单元最多转换为一个UTF-32编码单元（代理对2个单元转1个），输出缓冲以length为上界
    if (IsSizeOverflow(length, sizeof(llvm::UTF32))) {
        return std::basic_string<DUTF32Char>();
    }
    const std::vector<llvm::UTF16> src = CopyToUTF16(utf16, length);
    std::vector<llvm::UTF32> buffer(length);
    const llvm::UTF16* srcBegin = src.data();
    const llvm::UTF16* srcEnd = srcBegin + src.size();
    llvm::UTF32* dstBegin = buffer.data();
    const llvm::ConversionResult result = llvm::ConvertUTF16toUTF32(&srcBegin,
                                                                   srcEnd,
                                                                   &dstBegin,
                                                                   buffer.data() + buffer.size(),
                                                                   llvm::strictConversion);
    if ((result == llvm::sourceIllegal) || (result == llvm::targetExhausted)) {
        return std::basic_string<DUTF32Char>();
    }
    return MakeUTF32String(buffer, static_cast<size_t>(dstBegin - buffer.data()));
}

std::basic_string<DUTF32Char> StringConvert::UTF16ToUTF32(const std::basic_string<DUTF16Char>& utf16)
{
    return UTF16ToUTF32(utf16.c_str(), utf16.length());
}

std::basic_string<DUTF16Char> StringConvert::UTF32ToUTF16(const DUTF32Char* utf32, size_t length)
{
    if ((utf32 == nullptr) || (length == 0)) {
        return std::basic_string<DUTF16Char>();
    }
    //每个UTF-32编码单元最多转换为2个UTF-16编码单元（代理对），输出缓冲以2 * length为上界
    if (IsSizeOverflow(length, 2 * sizeof(llvm::UTF16))) {
        return std::basic_string<DUTF16Char>();
    }
    const std::vector<llvm::UTF32> src = CopyToUTF32(utf32, length);
    std::vector<llvm::UTF16> buffer(length * 2);
    const llvm::UTF32* srcBegin = src.data();
    const llvm::UTF32* srcEnd = srcBegin + src.size();
    llvm::UTF16* dstBegin = buffer.data();
    const llvm::ConversionResult result = llvm::ConvertUTF32toUTF16(&srcBegin,
                                                                   srcEnd,
                                                                   &dstBegin,
                                                                   buffer.data() + buffer.size(),
                                                                   llvm::strictConversion);
    if ((result == llvm::sourceIllegal) || (result == llvm::targetExhausted)) {
        return std::basic_string<DUTF16Char>();
    }
    return MakeUTF16String(buffer, static_cast<size_t>(dstBegin - buffer.data()));
}

std::basic_string<DUTF16Char> StringConvert::UTF32ToUTF16(const std::basic_string<DUTF32Char>& utf32)
{
    return UTF32ToUTF16(utf32.c_str(), utf32.length());
}

std::basic_string<DUTF32Char> StringConvert::WStringToUTF32(const DStringW& wstr)
{
#if defined(WCHAR_T_IS_UTF16)
    static_assert(sizeof(DUTF16Char) == sizeof(std::wstring::value_type), "DUTF16Char size mismatch");
    return UTF16ToUTF32(wstr.c_str(), wstr.length());
#else
    static_assert(sizeof(DUTF32Char) == sizeof(std::wstring::value_type), "DUTF32Char size mismatch");
    return std::basic_string<DUTF32Char>(wstr);
#endif
}

DStringW StringConvert::UTF32ToWString(const DUTF32Char* utf32, size_t length)
{
    if ((utf32 == nullptr) || (length == 0)) {
        return DStringW();
    }
#if defined(WCHAR_T_IS_UTF16)
    //每个UTF-32编码单元最多转换为2个UTF-16编码单元（代理对），输出缓冲以2 * length为上界
    if (IsSizeOverflow(length, 2 * sizeof(llvm::UTF16))) {
        return DStringW();
    }
    const std::vector<llvm::UTF32> src = CopyToUTF32(utf32, length);
    std::vector<llvm::UTF16> buffer(length * 2);
    const llvm::UTF32* srcBegin = src.data();
    const llvm::UTF32* srcEnd = srcBegin + src.size();
    llvm::UTF16* dstBegin = buffer.data();
    const llvm::ConversionResult result = llvm::ConvertUTF32toUTF16(&srcBegin,
                                                                   srcEnd,
                                                                   &dstBegin,
                                                                   buffer.data() + buffer.size(),
                                                                   llvm::strictConversion);
    if ((result == llvm::sourceIllegal) || (result == llvm::targetExhausted)) {
        return DStringW();
    }
    static_assert(sizeof(DUTF16Char) == sizeof(std::wstring::value_type), "DUTF16Char size mismatch");
    DStringW utf16;
    const size_t count = static_cast<size_t>(dstBegin - buffer.data());
    utf16.resize(count);
    if (count > 0) {
        memcpy(&utf16[0], buffer.data(), count * sizeof(DUTF16Char));
    }
    return utf16;
#else
    //非Windows平台：wchar_t为32位UTF32编码，先校验码点合法性，再直接复制
    for (size_t i = 0; i < length; ++i) {
        const uint32_t codePoint = static_cast<uint32_t>(utf32[i]);
        if ((codePoint > 0x10FFFF) || ((codePoint >= 0xD800) && (codePoint <= 0xDFFF))) {
            return DStringW();
        }
    }
    static_assert(sizeof(DUTF32Char) == sizeof(std::wstring::value_type), "DUTF32Char size mismatch");
    return DStringW(utf32, length);
#endif
}

DStringW StringConvert::UTF32ToWString(const std::basic_string<DUTF32Char>& utf32)
{
    return UTF32ToWString(utf32.c_str(), utf32.length());
}

bool StringConvert::IsValidUTF8(const DUTF8Char* data, size_t length)
{
    if ((data == nullptr) || (length == 0)) {
        return false;
    }
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data);
    size_t i = 0;
    while (i < length) {
        const uint32_t lead = bytes[i];
        if ((lead & 0x80) == 0) {
            ++i;
            continue;
        }
        uint32_t codePoint = 0;
        size_t followCount = 0;
        if ((lead & 0xE0) == 0xC0) {
            codePoint = lead & 0x1F;
            followCount = 1;
        }
        else if ((lead & 0xF0) == 0xE0) {
            codePoint = lead & 0x0F;
            followCount = 2;
        }
        else if ((lead & 0xF8) == 0xF0) {
            codePoint = lead & 0x07;
            followCount = 3;
        }
        else {
            return false;
        }
        if ((i + followCount) >= length) {
            return false;
        }
        for (size_t j = 1; j <= followCount; ++j) {
            const uint32_t next = bytes[i + j];
            if ((next & 0xC0) != 0x80) {
                return false;
            }
            codePoint = (codePoint << 6) | (next & 0x3F);
        }
        if ((followCount == 1) && (codePoint < 0x80)) {
            return false;
        }
        if ((followCount == 2) && (codePoint < 0x800)) {
            return false;
        }
        if ((followCount == 3) && (codePoint < 0x10000)) {
            return false;
        }
        if ((codePoint >= 0xD800) && (codePoint <= 0xDFFF)) {
            return false;
        }
        if (codePoint > 0x10FFFF) {
            return false;
        }
        i += followCount + 1;
    }
    return true;
}

bool StringConvert::IsValidUTF16(const DUTF16Char* data, size_t length)
{
    if ((data == nullptr) || (length == 0)) {
        return false;
    }
    for (size_t i = 0; i < length; ++i) {
        const uint32_t value = static_cast<uint16_t>(data[i]);
        if ((value >= 0xD800) && (value <= 0xDBFF)) {
            //高代理项：必须跟随一个低代理项
            if ((i + 1) >= length) {
                return false;
            }
            const uint32_t next = static_cast<uint16_t>(data[i + 1]);
            if ((next < 0xDC00) || (next > 0xDFFF)) {
                return false;
            }
            ++i;
        }
        else if ((value >= 0xDC00) && (value <= 0xDFFF)) {
            //低代理项不能单独出现
            return false;
        }
    }
    return true;
}

bool StringConvert::IsValidUTF32(const DUTF32Char* data, size_t length)
{
    if ((data == nullptr) || (length == 0)) {
        return false;
    }
    for (size_t i = 0; i < length; ++i) {
        const uint32_t codePoint = static_cast<uint32_t>(data[i]);
        if ((codePoint > 0x10FFFF) || ((codePoint >= 0xD800) && (codePoint <= 0xDFFF))) {
            return false;
        }
    }
    return true;
}

#ifdef DUILIB_BUILD_FOR_WIN
std::wstring StringConvert::MBCSToUnicode(const std::string& input, int32_t code_page, bool bStrict)
{
    return MBCSToUnicode2(input.c_str(), input.size(), code_page, bStrict);
}

std::wstring StringConvert::MBCSToUnicode2(const char* input, size_t inputSize, int32_t code_page, bool bStrict)
{
    std::wstring output;
    if ((input == nullptr) || (inputSize == 0)) {
        return output;
    }
    //避免长度超过int的最大值导致截断
    if (inputSize > static_cast<size_t>(INT_MAX)) {
        return output;
    }
    const UINT flags = bStrict ? MB_ERR_INVALID_CHARS : 0;
    //MultiByteToWideChar失败时返回0(不会返回负数)
    const int length = ::MultiByteToWideChar(code_page, flags, input, static_cast<int>(inputSize), nullptr, 0);
    if (length <= 0) {
        return output;
    }
    output.resize(length);
    const int written = ::MultiByteToWideChar(code_page, flags, input, static_cast<int>(inputSize),
                                              &output[0], static_cast<int>(output.size()));
    if (written <= 0) {
        output.clear();
    }
    return output;
}

DString StringConvert::MBCSToT(const std::string& input, int32_t code_page)
{
    DString output;
#ifdef DUILIB_UNICODE
    output = MBCSToUnicode(input, code_page);
#else
    std::wstring temp = MBCSToUnicode(input, code_page);
    output = WStringToUTF8(temp);
#endif
    return output;
}

std::string StringConvert::UnicodeToMBCS(const std::wstring& input, int32_t code_page, bool bStrict)
{
    std::string output;
    if (input.empty()) {
        return output;
    }
    if (input.size() > static_cast<size_t>(INT_MAX)) {
        return output;
    }
    //WC_ERR_INVALID_CHARS 仅对 UTF-8(65001) 与 GB18030(54936) 有效，其他代码页需忽略该标志
    const bool useStrictFlag = bStrict && ((code_page == CP_UTF8) || (code_page == 54936));
    const UINT flags = useStrictFlag ? WC_ERR_INVALID_CHARS : 0;
    //WideCharToMultiByte失败时返回0(不会返回负数)
    const int length = ::WideCharToMultiByte(code_page, flags, input.c_str(), static_cast<int>(input.size()),
                                             nullptr, 0, nullptr, nullptr);
    if (length <= 0) {
        return output;
    }
    output.resize(length);
    const int written = ::WideCharToMultiByte(code_page, flags, input.c_str(), static_cast<int>(input.size()),
                                              &output[0], static_cast<int>(output.size()), nullptr, nullptr);
    if (written <= 0) {
        output.clear();
    }
    return output;
}

std::string StringConvert::TToMBCS(const DString& input, int32_t code_page)
{
    std::string output;
#ifdef DUILIB_UNICODE
    output = UnicodeToMBCS(input, code_page);
#else
    std::wstring temp = UTF8ToWString(input);
    output = UnicodeToMBCS(temp, code_page);
#endif
    return output;
}

#endif //DUILIB_BUILD_FOR_WIN

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_UNICODE)
DString StringConvert::TToLocal(const DString& input)
{
    DString output;
    std::wstring temp = UTF8ToWString(input);
    output = UnicodeToMBCS(temp);
    return output;
}
#else
const DString& StringConvert::TToLocal(const DString& input)
{
    return input;
}
#endif

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_UNICODE)
DString StringConvert::LocalToT(const DString& input)
{
    DStringW output = MBCSToUnicode(input);
    return WStringToUTF8(output);
}
#else
const DString& StringConvert::LocalToT(const DString& input)
{
    return input;
}
#endif

} //namespace ui

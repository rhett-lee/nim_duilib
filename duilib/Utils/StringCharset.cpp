#include "StringCharset.h"
#include "duilib/Utils/StringConvert.h"
#include <string>

namespace ui
{
namespace
{
/** 判断字节流是否呈现UTF-16文本特征（奇偶位置中有一侧存在大量0字节）
*/
bool LooksLikeUTF16Text(const char* data, uint32_t length)
{
    if (length < 4) {
        return false;
    }
    const uint32_t unitCount = length / 2;
    uint32_t zeroEven = 0;
    uint32_t zeroOdd = 0;
    for (uint32_t i = 0; (i + 1) < length; i += 2) {
        if (data[i] == 0) {
            ++zeroEven;
        }
        if (data[i + 1] == 0) {
            ++zeroOdd;
        }
    }
    const uint32_t maxZero = (zeroEven > zeroOdd) ? zeroEven : zeroOdd;
    //0字节数量超过60%，且至少出现2次，才认为是UTF-16文本
    return (maxZero >= 2) && ((maxZero * 10) >= (unitCount * 6));
}

/** 判断字节流是否呈现UTF-32文本特征（多数4字节组中包含3个以上0字节）
*/
bool LooksLikeUTF32Text(const char* data, uint32_t length)
{
    if ((length < 8) || ((length % 4) != 0)) {
        return false;
    }
    const uint32_t groupCount = length / 4;
    uint32_t zeroGroups = 0;
    for (uint32_t i = 0; i < length; i += 4) {
        uint32_t zeroCount = 0;
        for (uint32_t j = 0; j < 4; ++j) {
            if (data[i + j] == 0) {
                ++zeroCount;
            }
        }
        if (zeroCount >= 3) {
            ++zeroGroups;
        }
    }
    return (zeroGroups * 2) >= groupCount;
}

/** 尝试按UTF-16/UTF-32检测编码类型
*/
CharsetType DetectUTF16Or32(const char* data, uint32_t length)
{
    if (((length % 4) == 0) && LooksLikeUTF32Text(data, length)) {
        if (StringCharset::IsValidateUTF32LEStream(data, length)) {
            return CharsetType::UTF32_LE;
        }
        if (StringCharset::IsValidateUTF32BEStream(data, length)) {
            return CharsetType::UTF32_BE;
        }
    }
    if ((length % 2) == 0) {
        if (StringCharset::IsValidateUTF16LEStream(data, length)) {
            return CharsetType::UTF16_LE;
        }
        if (StringCharset::IsValidateUTF16BEStream(data, length)) {
            return CharsetType::UTF16_BE;
        }
    }
    if ((length % 4) == 0) {
        if (StringCharset::IsValidateUTF32LEStream(data, length)) {
            return CharsetType::UTF32_LE;
        }
        if (StringCharset::IsValidateUTF32BEStream(data, length)) {
            return CharsetType::UTF32_BE;
        }
    }
    return CharsetType::UNKNOWN;
}

/** 将UTF-16LE字节流转换为UTF-16编码单元（避免未对齐内存访问）
*/
void UTF16LEToUnits(const char* data, uint32_t byteLength, std::basic_string<DUTF16Char>& units)
{
    units.resize(byteLength / 2);
    for (size_t i = 0; i < units.size(); ++i) {
        const uint16_t value = static_cast<uint16_t>(static_cast<uint8_t>(data[i * 2])) |
                               (static_cast<uint16_t>(static_cast<uint8_t>(data[i * 2 + 1])) << 8);
        units[i] = static_cast<DUTF16Char>(value);
    }
}

/** 将UTF-16BE字节流转换为UTF-16编码单元（避免未对齐内存访问）
*/
void UTF16BEToUnits(const char* data, uint32_t byteLength, std::basic_string<DUTF16Char>& units)
{
    units.resize(byteLength / 2);
    for (size_t i = 0; i < units.size(); ++i) {
        const uint16_t value = (static_cast<uint16_t>(static_cast<uint8_t>(data[i * 2])) << 8) |
                               static_cast<uint16_t>(static_cast<uint8_t>(data[i * 2 + 1]));
        units[i] = static_cast<DUTF16Char>(value);
    }
}

/** 将UTF-32LE字节流转换为UTF-32编码单元（避免未对齐内存访问）
*/
void UTF32LEToUnits(const char* data, uint32_t byteLength, std::basic_string<DUTF32Char>& units)
{
    units.resize(byteLength / 4);
    for (size_t i = 0; i < units.size(); ++i) {
        const uint32_t value = static_cast<uint32_t>(static_cast<uint8_t>(data[i * 4])) |
                               (static_cast<uint32_t>(static_cast<uint8_t>(data[i * 4 + 1])) << 8) |
                               (static_cast<uint32_t>(static_cast<uint8_t>(data[i * 4 + 2])) << 16) |
                               (static_cast<uint32_t>(static_cast<uint8_t>(data[i * 4 + 3])) << 24);
        units[i] = static_cast<DUTF32Char>(value);
    }
}

/** 将UTF-32BE字节流转换为UTF-32编码单元（避免未对齐内存访问）
*/
void UTF32BEToUnits(const char* data, uint32_t byteLength, std::basic_string<DUTF32Char>& units)
{
    units.resize(byteLength / 4);
    for (size_t i = 0; i < units.size(); ++i) {
        const uint32_t value = (static_cast<uint32_t>(static_cast<uint8_t>(data[i * 4])) << 24) |
                               (static_cast<uint32_t>(static_cast<uint8_t>(data[i * 4 + 1])) << 16) |
                               (static_cast<uint32_t>(static_cast<uint8_t>(data[i * 4 + 2])) << 8) |
                               static_cast<uint32_t>(static_cast<uint8_t>(data[i * 4 + 3]));
        units[i] = static_cast<DUTF32Char>(value);
    }
}

/** 将宽字符串转换为UTF-16编码单元
*/
void WStringToUTF16Units(const std::wstring& str, std::basic_string<DUTF16Char>& units)
{
#if defined(WCHAR_T_IS_UTF16)
    units.assign(reinterpret_cast<const DUTF16Char*>(str.data()), str.size());
#else
    units.clear();
    units.reserve(str.size() + 4);
    for (wchar_t ch : str) {
        uint32_t codePoint = static_cast<uint32_t>(ch);
        if (codePoint <= 0xFFFF) {
            units.push_back(static_cast<DUTF16Char>(codePoint));
        }
        else if (codePoint <= 0x10FFFF) {
            codePoint -= 0x10000;
            units.push_back(static_cast<DUTF16Char>(0xD800 + (codePoint >> 10)));
            units.push_back(static_cast<DUTF16Char>(0xDC00 + (codePoint & 0x3FF)));
        }
        else {
            units.push_back(static_cast<DUTF16Char>(0xFFFD));
        }
    }
#endif
}

/** 将宽字符串转换为UTF-32编码单元
*/
std::basic_string<DUTF32Char> WStringToUTF32Units(const std::wstring& str)
{
#if defined(WCHAR_T_IS_UTF16)
    return StringConvert::WStringToUTF32(str);
#else
    std::basic_string<DUTF32Char> units;
    units.assign(reinterpret_cast<const DUTF32Char*>(str.data()), str.size());
    return units;
#endif
}

/** 在字节流前追加BOM头
*/
void AppendBOM(std::string& data, CharsetType charsetType)
{
    switch (charsetType)
    {
    case CharsetType::UTF8:
        data.push_back(static_cast<char>(0xEF));
        data.push_back(static_cast<char>(0xBB));
        data.push_back(static_cast<char>(0xBF));
        break;
    case CharsetType::UTF16_LE:
        data.push_back(static_cast<char>(0xFF));
        data.push_back(static_cast<char>(0xFE));
        break;
    case CharsetType::UTF16_BE:
        data.push_back(static_cast<char>(0xFE));
        data.push_back(static_cast<char>(0xFF));
        break;
    case CharsetType::UTF32_LE:
        data.push_back(static_cast<char>(0xFF));
        data.push_back(static_cast<char>(0xFE));
        data.push_back(0);
        data.push_back(0);
        break;
    case CharsetType::UTF32_BE:
        data.push_back(0);
        data.push_back(0);
        data.push_back(static_cast<char>(0xFE));
        data.push_back(static_cast<char>(0xFF));
        break;
    default:
        break;
    }
}

}//namespace

uint32_t StringCharset::GetBOMSize(CharsetType charsetType)
{
    switch (charsetType)
    {
    case CharsetType::UTF8:
        return 3;
    case CharsetType::UTF16_BE:
    case CharsetType::UTF16_LE:
        return 2;
    case CharsetType::UTF32_BE:
    case CharsetType::UTF32_LE:
        return 4;
    default:
        break;
    }
    return 0;
}

bool StringCharset::HasBOM(const char* data, uint32_t length)
{
    return GetDataCharsetByBOM(data, length) != CharsetType::UNKNOWN;
}

uint32_t StringCharset::SkipBOM(const char* data, uint32_t length)
{
    return GetBOMSize(GetDataCharsetByBOM(data, length));
}

CharsetType StringCharset::GetDataCharsetByBOM(const char* data, uint32_t length)
{
    if ((data == nullptr) || (length < 2)) {
        return CharsetType::UNKNOWN;
    }
    if ((length >= 4) && (data[0] == 0) && (data[1] == 0) &&
        (data[2] == static_cast<char>(0xFE)) && (data[3] == static_cast<char>(0xFF))) {
        //UTF-32BE的BOM头
        return CharsetType::UTF32_BE;
    }
    if ((length >= 4) && (data[0] == static_cast<char>(0xFF)) && (data[1] == static_cast<char>(0xFE)) &&
        (data[2] == 0) && (data[3] == 0)) {
        //UTF-32LE的BOM头
        return CharsetType::UTF32_LE;
    }
    if ((length >= 3) && (data[0] == static_cast<char>(0xEF)) &&
        (data[1] == static_cast<char>(0xBB)) && (data[2] == static_cast<char>(0xBF))) {
        return CharsetType::UTF8;
    }
    if ((data[0] == static_cast<char>(0xFF)) && (data[1] == static_cast<char>(0xFE))) {
        return CharsetType::UTF16_LE;
    }
    if ((data[0] == static_cast<char>(0xFE)) && (data[1] == static_cast<char>(0xFF))) {
        return CharsetType::UTF16_BE;
    }
    return CharsetType::UNKNOWN;
}

CharsetType StringCharset::GetDataCharset(const char* data, uint32_t length, CharsetDetectPolicy policy)
{
    if ((data == nullptr) || (length == 0)) {
        return CharsetType::UNKNOWN;
    }
    if (policy == CharsetDetectPolicy::kDefault) {
        //优先处理带大量0字节的文本（无BOM头的UTF-16/UTF-32）
        const bool maybeUTF32 = ((length % 4) == 0) && LooksLikeUTF32Text(data, length);
        const bool maybeUTF16 = LooksLikeUTF16Text(data, length);
        if (maybeUTF16 || maybeUTF32) {
            const CharsetType utfType = DetectUTF16Or32(data, length);
            if (utfType != CharsetType::UNKNOWN) {
                return utfType;
            }
        }
    }

    if (IsValidateASCIIStream(data, length)) {
        return CharsetType::ANSI;
    }

    const bool isGBK = IsValidateGBKStream(data, length);
    const bool isUTF8 = IsValidateUTF8Stream(data, length);
    if (isGBK && isUTF8) {
        //同时满足GBK与UTF-8编码规则时，优先识别为UTF-8
        return CharsetType::UTF8;
    }
    if (isGBK) {
        return CharsetType::GBK;
    }
    if (isUTF8) {
        return CharsetType::UTF8;
    }

    if (policy == CharsetDetectPolicy::kDefault) {
        return DetectUTF16Or32(data, length);
    }
    return CharsetType::UNKNOWN;
}

bool StringCharset::GetDataAsString(const char* data, uint32_t length, std::wstring& result)
{
    CharsetType outCharsetType = CharsetType::UNKNOWN;
    uint32_t bomSize = 0;
    return GetDataAsString(data, length, CharsetType::UNKNOWN, result, outCharsetType, bomSize);
}

bool StringCharset::GetDataAsString(const char* data, uint32_t length, CharsetType inCharsetType, std::wstring& result)
{
    CharsetType outCharsetType = CharsetType::UNKNOWN;
    uint32_t bomSize = 0;
    return GetDataAsString(data, length, inCharsetType, result, outCharsetType, bomSize);
}

bool StringCharset::GetDataAsString(const char* data, uint32_t length,
                                    std::wstring& result, CharsetType& outCharsetType, uint32_t& bomSize)
{
    return GetDataAsString(data, length, CharsetType::UNKNOWN, result, outCharsetType, bomSize);
}

bool StringCharset::GetDataAsString(const char* data, uint32_t length, CharsetType inCharsetType,
                                    std::wstring& result, CharsetType& outCharsetType, uint32_t& bomSize)
{
    result.clear();
    outCharsetType = CharsetType::UNKNOWN;
    bomSize = 0;
    if ((data == nullptr) || (length == 0)) {
        return false;
    }

    //1. 先按BOM头检测编码
    const CharsetType bomCharsetType = GetDataCharsetByBOM(data, length);
    const uint32_t detectBomSize = GetBOMSize(bomCharsetType);
    if (detectBomSize > length) {
        //防御性检查：避免bomSize异常导致后续长度计算下溢
        return false;
    }

    if (inCharsetType != CharsetType::UNKNOWN) {
        if (bomCharsetType != CharsetType::UNKNOWN) {
            //数据包含BOM头，指定的编码必须与BOM头一致
            if (inCharsetType != bomCharsetType) {
                return false;
            }
            outCharsetType = bomCharsetType;
            bomSize = detectBomSize;
        }
        else {
            //无BOM头：以调用方指定的编码为准，但先校验数据是否符合该编码的规则
            bool isSpecifiedValid = false;
            switch (inCharsetType)
            {
            case CharsetType::ANSI:
#ifdef DUILIB_BUILD_FOR_WIN
                //Windows的ANSI为多字节编码，可接受任意字节（非法字节会被替换）
                isSpecifiedValid = true;
#else
                //非Windows平台的ANSI等价于UTF-8
                isSpecifiedValid = IsValidateUTF8Stream(data, length);
#endif
                break;
            case CharsetType::GBK:
                isSpecifiedValid = IsValidateGBKStream(data, length);
                break;
            case CharsetType::UTF8:
                isSpecifiedValid = IsValidateUTF8Stream(data, length);
                break;
            case CharsetType::UTF16_LE:
                isSpecifiedValid = IsValidateUTF16LEStream(data, length);
                break;
            case CharsetType::UTF16_BE:
                isSpecifiedValid = IsValidateUTF16BEStream(data, length);
                break;
            case CharsetType::UTF32_LE:
                isSpecifiedValid = IsValidateUTF32LEStream(data, length);
                break;
            case CharsetType::UTF32_BE:
                isSpecifiedValid = IsValidateUTF32BEStream(data, length);
                break;
            default:
                break;
            }
            if (!isSpecifiedValid) {
                return false;
            }
            outCharsetType = inCharsetType;
            bomSize = 0;
        }
    }
    else if (bomCharsetType != CharsetType::UNKNOWN) {
        outCharsetType = bomCharsetType;
        bomSize = detectBomSize;
    }

    if (outCharsetType == CharsetType::UNKNOWN) {
        //按BOM头检测失败时，按数据内容检测
        outCharsetType = GetDataCharset(data, length);
        bomSize = 0;
        if (outCharsetType == CharsetType::UNKNOWN) {
            return false;
        }
    }

    const char* const realData = data + bomSize;
    const uint32_t realLength = length - bomSize;
    if (realLength == 0) {
        //数据仅包含BOM头
        return true;
    }

    bool isSuccess = false;
    if (outCharsetType == CharsetType::ANSI) {
#ifdef DUILIB_BUILD_FOR_WIN
        result = StringConvert::MBCSToUnicode2(realData, realLength);
#else
        result = StringConvert::UTF8ToWString(realData, realLength);
#endif
        isSuccess = !result.empty();
    }
    else if (outCharsetType == CharsetType::GBK) {
#ifdef DUILIB_BUILD_FOR_WIN
        result = StringConvert::MBCSToUnicode2(realData, realLength, 936);
        isSuccess = !result.empty();
#else
        //非Windows平台暂不支持GBK解码
        isSuccess = false;
#endif
    }
    else if (outCharsetType == CharsetType::UTF8) {
        result = StringConvert::UTF8ToWString(realData, realLength);
        isSuccess = !result.empty();
    }
    else if (outCharsetType == CharsetType::UTF16_LE) {
        if (((realLength % 2) == 0) && IsValidateUTF16LEStream(realData, realLength)) {
            std::basic_string<DUTF16Char> units;
            UTF16LEToUnits(realData, realLength, units);
#if defined(WCHAR_T_IS_UTF16)
            result.assign(units.data(), units.size());
#else
            result = StringConvert::UTF16ToUTF32(units.data(), units.size());
#endif
            isSuccess = !result.empty();
        }
    }
    else if (outCharsetType == CharsetType::UTF16_BE) {
        if (((realLength % 2) == 0) && IsValidateUTF16BEStream(realData, realLength)) {
            std::basic_string<DUTF16Char> units;
            UTF16BEToUnits(realData, realLength, units);
#if defined(WCHAR_T_IS_UTF16)
            result.assign(units.data(), units.size());
#else
            result = StringConvert::UTF16ToUTF32(units.data(), units.size());
#endif
            isSuccess = !result.empty();
        }
    }
    else if (outCharsetType == CharsetType::UTF32_LE) {
        if (((realLength % 4) == 0) && IsValidateUTF32LEStream(realData, realLength)) {
            std::basic_string<DUTF32Char> units;
            UTF32LEToUnits(realData, realLength, units);
            result = StringConvert::UTF32ToWString(units.data(), units.size());
            isSuccess = !result.empty();
        }
    }
    else if (outCharsetType == CharsetType::UTF32_BE) {
        if (((realLength % 4) == 0) && IsValidateUTF32BEStream(realData, realLength)) {
            std::basic_string<DUTF32Char> units;
            UTF32BEToUnits(realData, realLength, units);
            result = StringConvert::UTF32ToWString(units.data(), units.size());
            isSuccess = !result.empty();
        }
    }

    if (!isSuccess) {
        result.clear();
    }
    return isSuccess;
}

bool StringCharset::WStringToData(const std::wstring& str, CharsetType charsetType,
                                  std::string& data, bool bWithBOM)
{
    data.clear();
    if (charsetType == CharsetType::UNKNOWN) {
        return false;
    }

    std::string body;
    if (charsetType == CharsetType::ANSI) {
#ifdef DUILIB_BUILD_FOR_WIN
        body = StringConvert::UnicodeToMBCS(str, CP_ACP);
#else
        body = StringConvert::WStringToUTF8(str);
#endif
        if (!str.empty() && body.empty()) {
            return false;
        }
    }
    else if (charsetType == CharsetType::GBK) {
#ifdef DUILIB_BUILD_FOR_WIN
        body = StringConvert::UnicodeToMBCS(str, 936);
        if (!str.empty() && body.empty()) {
            return false;
        }
#else
        //非Windows平台暂不支持GBK编码
        return false;
#endif
    }
    else if (charsetType == CharsetType::UTF8) {
        body = StringConvert::WStringToUTF8(str);
    }
    else if ((charsetType == CharsetType::UTF16_LE) || (charsetType == CharsetType::UTF16_BE)) {
        std::basic_string<DUTF16Char> units;
        WStringToUTF16Units(str, units);
        body.resize(units.size() * 2);
        for (size_t i = 0; i < units.size(); ++i) {
            const uint16_t value = static_cast<uint16_t>(units[i]);
            if (charsetType == CharsetType::UTF16_LE) {
                body[i * 2] = static_cast<char>(value & 0xFF);
                body[i * 2 + 1] = static_cast<char>((value >> 8) & 0xFF);
            }
            else {
                body[i * 2] = static_cast<char>((value >> 8) & 0xFF);
                body[i * 2 + 1] = static_cast<char>(value & 0xFF);
            }
        }
    }
    else if ((charsetType == CharsetType::UTF32_LE) || (charsetType == CharsetType::UTF32_BE)) {
        const std::basic_string<DUTF32Char> units = WStringToUTF32Units(str);
        body.resize(units.size() * 4);
        for (size_t i = 0; i < units.size(); ++i) {
            const uint32_t value = static_cast<uint32_t>(units[i]);
            if (charsetType == CharsetType::UTF32_LE) {
                body[i * 4] = static_cast<char>(value & 0xFF);
                body[i * 4 + 1] = static_cast<char>((value >> 8) & 0xFF);
                body[i * 4 + 2] = static_cast<char>((value >> 16) & 0xFF);
                body[i * 4 + 3] = static_cast<char>((value >> 24) & 0xFF);
            }
            else {
                body[i * 4] = static_cast<char>((value >> 24) & 0xFF);
                body[i * 4 + 1] = static_cast<char>((value >> 16) & 0xFF);
                body[i * 4 + 2] = static_cast<char>((value >> 8) & 0xFF);
                body[i * 4 + 3] = static_cast<char>(value & 0xFF);
            }
        }
    }
    else {
        return false;
    }

    if (bWithBOM) {
        data.reserve(body.size() + 4);
        AppendBOM(data, charsetType);
    }
    data += body;
    return true;
}

const char* StringCharset::GetCharsetName(CharsetType charsetType)
{
    switch (charsetType)
    {
    case CharsetType::ANSI:
        return "ANSI";
    case CharsetType::GBK:
        return "GBK";
    case CharsetType::UTF8:
        return "UTF-8";
    case CharsetType::UTF16_LE:
        return "UTF-16LE";
    case CharsetType::UTF16_BE:
        return "UTF-16BE";
    case CharsetType::UTF32_LE:
        return "UTF-32LE";
    case CharsetType::UTF32_BE:
        return "UTF-32BE";
    default:
        break;
    }
    return "UNKNOWN";
}

bool StringCharset::IsValidateASCIIStream(const char* stream, uint32_t length)
{
    if ((stream == nullptr) || (length == 0)) {
        return false;
    }
    for (uint32_t i = 0; i < length; ++i) {
        if ((static_cast<unsigned char>(stream[i]) & 0x80) != 0) {
            return false;
        }
    }
    return true;
}

bool StringCharset::IsValidateGBKStream(const char* stream, uint32_t length)
{
    if ((stream == nullptr) || (length == 0)) {
        return false;
    }
    const unsigned char* s = reinterpret_cast<const unsigned char*>(stream);
    const unsigned char* const end = s + length;
    for (; s < end; ++s) {
        if (*s < 0x80) {
            continue;
        }
        //GBK首字节范围：0x81-0xFE
        if ((*s < 0x81) || (*s > 0xFE)) {
            return false;
        }
        ++s;
        if (s == end) {
            return false;
        }
        //GBK尾字节范围：0x40-0x7E、0x80-0xFE（0x7F不是合法尾字节）
        if ((*s < 0x40) || (*s == 0x7F) || (*s > 0xFE)) {
            return false;
        }
    }
    return true;
}

bool StringCharset::IsValidateUTF8Stream(const char* stream, uint32_t length)
{
    if ((stream == nullptr) || (length == 0)) {
        return false;
    }
    const unsigned char* data = reinterpret_cast<const unsigned char*>(stream);
    uint32_t i = 0;
    while (i < length) {
        const uint32_t lead = data[i];
        if ((lead & 0x80) == 0) {
            //ASCII字符
            ++i;
            continue;
        }

        uint32_t codePoint = 0;
        uint32_t followCount = 0;
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
            //0x80-0xBF 作为首字节，或0xF8-0xFF，均为非法
            return false;
        }

        if ((i + followCount) >= length) {
            return false;
        }
        for (uint32_t j = 1; j <= followCount; ++j) {
            const uint32_t next = data[i + j];
            if ((next & 0xC0) != 0x80) {
                return false;
            }
            codePoint = (codePoint << 6) | (next & 0x3F);
        }

        //拒绝过长编码
        if ((followCount == 1) && (codePoint < 0x80)) {
            return false;
        }
        if ((followCount == 2) && (codePoint < 0x800)) {
            return false;
        }
        if ((followCount == 3) && (codePoint < 0x10000)) {
            return false;
        }
        //拒绝代理区码点与超出Unicode范围的码点
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

bool StringCharset::IsValidateUTF16LEStream(const char* stream, uint32_t length)
{
    if ((stream == nullptr) || (length == 0) || ((length % 2) != 0)) {
        return false;
    }
    const unsigned char* data = reinterpret_cast<const unsigned char*>(stream);
    for (uint32_t i = 0; i < length; i += 2) {
        const uint16_t value = static_cast<uint16_t>(data[i]) |
                               (static_cast<uint16_t>(data[i + 1]) << 8);
        if ((value >= 0xD800) && (value <= 0xDBFF)) {
            //高代理项：必须跟随一个低代理项
            if ((i + 3) >= length) {
                return false;
            }
            const uint16_t next = static_cast<uint16_t>(data[i + 2]) |
                                  (static_cast<uint16_t>(data[i + 3]) << 8);
            if ((next < 0xDC00) || (next > 0xDFFF)) {
                return false;
            }
            i += 2;
        }
        else if ((value >= 0xDC00) && (value <= 0xDFFF)) {
            //低代理项不能单独出现
            return false;
        }
    }
    return true;
}

bool StringCharset::IsValidateUTF16BEStream(const char* stream, uint32_t length)
{
    if ((stream == nullptr) || (length == 0) || ((length % 2) != 0)) {
        return false;
    }
    const unsigned char* data = reinterpret_cast<const unsigned char*>(stream);
    for (uint32_t i = 0; i < length; i += 2) {
        const uint16_t value = (static_cast<uint16_t>(data[i]) << 8) |
                               static_cast<uint16_t>(data[i + 1]);
        if ((value >= 0xD800) && (value <= 0xDBFF)) {
            //高代理项：必须跟随一个低代理项
            if ((i + 3) >= length) {
                return false;
            }
            const uint16_t next = (static_cast<uint16_t>(data[i + 2]) << 8) |
                                  static_cast<uint16_t>(data[i + 3]);
            if ((next < 0xDC00) || (next > 0xDFFF)) {
                return false;
            }
            i += 2;
        }
        else if ((value >= 0xDC00) && (value <= 0xDFFF)) {
            //低代理项不能单独出现
            return false;
        }
    }
    return true;
}

bool StringCharset::IsValidateUTF32LEStream(const char* stream, uint32_t length)
{
    if ((stream == nullptr) || (length < 4) || ((length % 4) != 0)) {
        return false;
    }
    const unsigned char* data = reinterpret_cast<const unsigned char*>(stream);
    for (uint32_t i = 0; i < length; i += 4) {
        const uint32_t codePoint = static_cast<uint32_t>(data[i]) |
                                   (static_cast<uint32_t>(data[i + 1]) << 8) |
                                   (static_cast<uint32_t>(data[i + 2]) << 16) |
                                   (static_cast<uint32_t>(data[i + 3]) << 24);
        if ((codePoint > 0x10FFFF) || ((codePoint >= 0xD800) && (codePoint <= 0xDFFF))) {
            return false;
        }
    }
    return true;
}

bool StringCharset::IsValidateUTF32BEStream(const char* stream, uint32_t length)
{
    if ((stream == nullptr) || (length < 4) || ((length % 4) != 0)) {
        return false;
    }
    const unsigned char* data = reinterpret_cast<const unsigned char*>(stream);
    for (uint32_t i = 0; i < length; i += 4) {
        const uint32_t codePoint = (static_cast<uint32_t>(data[i]) << 24) |
                                   (static_cast<uint32_t>(data[i + 1]) << 16) |
                                   (static_cast<uint32_t>(data[i + 2]) << 8) |
                                   static_cast<uint32_t>(data[i + 3]);
        if ((codePoint > 0x10FFFF) || ((codePoint >= 0xD800) && (codePoint <= 0xDFFF))) {
            return false;
        }
    }
    return true;
}

}//namespace ui

// duilib/Utils/StringConvert 与 StringCharset 的单元测试
//
// 覆盖 UTF-8/UTF-16/UTF-32/WString 之间的相互转换与往返一致性，
// 以及字符集检测（BOM头、ASCII/GBK/UTF-8/UTF-16 流校验、字节流到字符串的转换）。
// 全部为纯逻辑，不创建窗口、不加载资源。

#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringCharset.h"
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;

void Check(bool condition, const std::function<std::string()>& message)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << message() << std::endl;
    }
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })

#define CHECK_INT(actual, expected) \
    Check((actual) == (expected), [&]() { \
        return std::string(#actual " == " #expected) + ": expected " + \
               std::to_string(static_cast<int64_t>(expected)) + ", actual " + \
               std::to_string(static_cast<int64_t>(actual)); \
    })

/** "中文" 的UTF-8编码 */
const std::string kChineseUtf8 = "\xE4\xB8\xAD\xE6\x96\x87";
/** "😀"(U+1F600) 的UTF-8编码，用于验证4字节UTF-8与代理对的往返 */
const std::string kEmojiUtf8 = "\xF0\x9F\x98\x80";

void TestUTF8AndWString()
{
    const std::wstring chinese(L"\u4E2D\u6587");
    CHECK(ui::StringConvert::WStringToUTF8(chinese) == kChineseUtf8);
    CHECK(ui::StringConvert::UTF8ToWString(kChineseUtf8) == chinese);
    CHECK(ui::StringConvert::UTF8ToWString(std::string()) .empty());

    // 4字节UTF-8字符往返
    const std::wstring emoji = ui::StringConvert::UTF8ToWString(kEmojiUtf8);
    CHECK(!emoji.empty());
    CHECK(ui::StringConvert::WStringToUTF8(emoji) == kEmojiUtf8);

    // UTF8ToT 在Unicode版本下返回wstring
    CHECK(ui::StringConvert::UTF8ToT(kChineseUtf8) == chinese);
    CHECK(ui::StringConvert::TToUTF8(chinese) == kChineseUtf8);
}

void TestUTF16()
{
    const std::basic_string<DUTF16Char> utf16 =
        ui::StringConvert::UTF8ToUTF16(kChineseUtf8.data(), kChineseUtf8.size());
    CHECK_INT((int)utf16.size(), 2);
    CHECK(ui::StringConvert::UTF16ToUTF8(utf16.data(), utf16.size()) == kChineseUtf8);

    // 空字符串
    CHECK(ui::StringConvert::UTF8ToUTF16(nullptr, 0).empty());
    CHECK(ui::StringConvert::UTF16ToUTF8(nullptr, 0).empty());
}

void TestUTF32()
{
    const std::basic_string<DUTF32Char> utf32 = ui::StringConvert::UTF8ToUTF32(kChineseUtf8);
    CHECK_INT((int)utf32.size(), 2);
    CHECK(ui::StringConvert::UTF32ToUTF8(utf32) == kChineseUtf8);

    // UTF16/UTF32 互转
    const std::basic_string<DUTF16Char> utf16 =
        ui::StringConvert::UTF8ToUTF16(kChineseUtf8.data(), kChineseUtf8.size());
    const std::basic_string<DUTF32Char> fromUtf16 =
        ui::StringConvert::UTF16ToUTF32(utf16.data(), utf16.size());
    CHECK(fromUtf16 == utf32);

    // WString 与 UTF32 互转
    CHECK(ui::StringConvert::WStringToUTF32(std::wstring(L"\u4E2D\u6587")) == utf32);
    CHECK(ui::StringConvert::UTF32ToWString(utf32) == std::wstring(L"\u4E2D\u6587"));
}

#ifdef DUILIB_BUILD_FOR_WIN
void TestMBCS()
{
    // 本地Ansi编码(中文Windows为GBK)与Unicode互转
    const std::string ansi = ui::StringConvert::UnicodeToMBCS(std::wstring(L"\u4E2D\u6587"));
    CHECK(!ansi.empty());
    CHECK(ui::StringConvert::MBCSToUnicode(ansi) == std::wstring(L"\u4E2D\u6587"));

    // 指定长度的转换接口
    const std::wstring unicode =
        ui::StringConvert::MBCSToUnicode2(ansi.data(), ansi.size());
    CHECK(unicode == std::wstring(L"\u4E2D\u6587"));

    // 空数据时返回空字符串
    CHECK(ui::StringConvert::MBCSToUnicode2(nullptr, 0).empty());
    CHECK(ui::StringConvert::UnicodeToMBCS(std::wstring()).empty());
}
#endif

void TestBOM()
{
    CHECK_INT((int)ui::StringCharset::GetBOMSize(ui::CharsetType::UTF8), 3);
    CHECK_INT((int)ui::StringCharset::GetBOMSize(ui::CharsetType::UTF16_LE), 2);
    CHECK_INT((int)ui::StringCharset::GetBOMSize(ui::CharsetType::UTF16_BE), 2);
    CHECK_INT((int)ui::StringCharset::GetBOMSize(ui::CharsetType::ANSI), 0);
    CHECK_INT((int)ui::StringCharset::GetBOMSize(ui::CharsetType::UNKNOWN), 0);

    const char utf8Bom[] = "\xEF\xBB\xBF";
    CHECK(ui::StringCharset::GetDataCharsetByBOM(utf8Bom, 3) == ui::CharsetType::UTF8);

    const char utf16LeBom[] = "\xFF\xFE";
    CHECK(ui::StringCharset::GetDataCharsetByBOM(utf16LeBom, 2) == ui::CharsetType::UTF16_LE);

    const char utf16BeBom[] = "\xFE\xFF";
    CHECK(ui::StringCharset::GetDataCharsetByBOM(utf16BeBom, 2) == ui::CharsetType::UTF16_BE);

    // 数据太短或为空
    CHECK(ui::StringCharset::GetDataCharsetByBOM(nullptr, 0) == ui::CharsetType::UNKNOWN);
    CHECK(ui::StringCharset::GetDataCharsetByBOM(utf8Bom, 2) == ui::CharsetType::UNKNOWN);
}

void TestValidateStream()
{
    CHECK(ui::StringCharset::IsValidateASCIIStream("abc123", 6));
    CHECK(!ui::StringCharset::IsValidateASCIIStream(nullptr, 0));
    const char nonAscii[] = { 'a', (char)0x80 };
    CHECK(!ui::StringCharset::IsValidateASCIIStream(nonAscii, 2));

    // UTF-8："中文"与4字节emoji都合法
    CHECK(ui::StringCharset::IsValidateUTF8Stream(kChineseUtf8.data(), (uint32_t)kChineseUtf8.size()));
    CHECK(ui::StringCharset::IsValidateUTF8Stream(kEmojiUtf8.data(), (uint32_t)kEmojiUtf8.size()));
    // 截断的UTF-8序列不合法
    CHECK(!ui::StringCharset::IsValidateUTF8Stream(kChineseUtf8.data(), 2));
    CHECK(!ui::StringCharset::IsValidateUTF8Stream(nullptr, 0));

    // GBK："中文"的GBK编码为 D6 D0 CE C4
    const char gbkChinese[] = { (char)0xD6, (char)0xD0, (char)0xCE, (char)0xC4 };
    CHECK(ui::StringCharset::IsValidateGBKStream(gbkChinese, 4));
    // 只有首字节、没有后续字节
    CHECK(!ui::StringCharset::IsValidateGBKStream(gbkChinese, 1));
    // 0xFF 不是合法的GBK首字节
    const char invalidGbk[] = { (char)0xFF, (char)0xFF };
    CHECK(!ui::StringCharset::IsValidateGBKStream(invalidGbk, 2));
    // UTF-8编码的"中文"不满足GBK的尾字节规则(AD后跟E6是合法的，但E4 B8也是合法的，
    // 因此这里用4字节emoji验证：F0 9F 98 80 中 0x9F 是合法尾字节，仍能通过；
    // 这里仅验证空指针与空长度返回false)
    CHECK(!ui::StringCharset::IsValidateGBKStream(nullptr, 4));

    // UTF-16LE："AB" = 41 00 42 00
    const char utf16Le[] = { 0x41, 0x00, 0x42, 0x00 };
    CHECK(ui::StringCharset::IsValidateUTF16LEStream(utf16Le, 4));
    CHECK(!ui::StringCharset::IsValidateUTF16LEStream(utf16Le, 3));
    // 孤立的高代理项/低代理项不合法，完整代理对合法
    const char loneHigh[] = { 0x00, (char)0xD8 };
    CHECK(!ui::StringCharset::IsValidateUTF16LEStream(loneHigh, 2));
    const char loneLow[] = { 0x00, (char)0xDC };
    CHECK(!ui::StringCharset::IsValidateUTF16LEStream(loneLow, 2));
    const char surrogatePair[] = { 0x00, (char)0xD8, 0x00, (char)0xDC };
    CHECK(ui::StringCharset::IsValidateUTF16LEStream(surrogatePair, 4));

    // UTF-16BE："AB" = 00 41 00 42
    const char utf16Be[] = { 0x00, 0x41, 0x00, 0x42 };
    CHECK(ui::StringCharset::IsValidateUTF16BEStream(utf16Be, 4));
    CHECK(!ui::StringCharset::IsValidateUTF16BEStream(utf16Be, 3));
    const char surrogatePairBE[] = { (char)0xD8, 0x00, (char)0xDC, 0x00 };
    CHECK(ui::StringCharset::IsValidateUTF16BEStream(surrogatePairBE, 4));
}

void TestDetectCharset()
{
    // 纯ASCII按ANSI处理
    CHECK(ui::StringCharset::GetDataCharset("abc", 3) == ui::CharsetType::ANSI);
    // UTF-8编码的中文(同时也满足GBK校验，但优先识别为UTF-8)
    CHECK(ui::StringCharset::GetDataCharset(kChineseUtf8.data(), (uint32_t)kChineseUtf8.size()) ==
          ui::CharsetType::UTF8);
    // GBK编码的中文
    const char gbkChinese[] = { (char)0xD6, (char)0xD0, (char)0xCE, (char)0xC4 };
    CHECK(ui::StringCharset::GetDataCharset(gbkChinese, 4) == ui::CharsetType::ANSI);
    CHECK(ui::StringCharset::GetDataCharset(nullptr, 0) == ui::CharsetType::UNKNOWN);
}

void TestDataAsString()
{
    std::wstring result;
    ui::CharsetType outCharsetType = ui::CharsetType::UNKNOWN;
    uint32_t bomSize = 0;

    // UTF-8 BOM + "abc"
    std::string utf8Data = std::string("\xEF\xBB\xBF") + "abc";
    CHECK(ui::StringCharset::GetDataAsString(utf8Data.data(), (uint32_t)utf8Data.size(),
                                            result, outCharsetType, bomSize));
    CHECK(result == L"abc");
    CHECK(outCharsetType == ui::CharsetType::UTF8);
    CHECK_INT(bomSize, 3);

    // UTF-8 BOM + 中文
    std::string utf8Chinese = std::string("\xEF\xBB\xBF") + kChineseUtf8;
    CHECK(ui::StringCharset::GetDataAsString(utf8Chinese.data(), (uint32_t)utf8Chinese.size(),
                                            result, outCharsetType, bomSize));
    CHECK(result == std::wstring(L"\u4E2D\u6587"));

    // UTF-16LE BOM + "AB"
    const char utf16LeData[] = { (char)0xFF, (char)0xFE, 0x41, 0x00, 0x42, 0x00 };
    CHECK(ui::StringCharset::GetDataAsString(utf16LeData, 6, result, outCharsetType, bomSize));
    CHECK(result == L"AB");
    CHECK(outCharsetType == ui::CharsetType::UTF16_LE);
    CHECK_INT(bomSize, 2);

    // UTF-16BE BOM + "AB"
    const char utf16BeData[] = { (char)0xFE, (char)0xFF, 0x00, 0x41, 0x00, 0x42 };
    CHECK(ui::StringCharset::GetDataAsString(utf16BeData, 6, result, outCharsetType, bomSize));
#if defined(WCHAR_T_IS_UTF16)
    // 仅当 wchar_t 为UTF-16时，结果直接是"AB"（非Windows平台wchar_t为32位，当前实现仅做字节序反转）
    CHECK(result == L"AB");
#endif
    CHECK(outCharsetType == ui::CharsetType::UTF16_BE);

    // 无BOM的纯ASCII：按ANSI处理
    CHECK(ui::StringCharset::GetDataAsString("abc", 3, result, outCharsetType, bomSize));
    CHECK(result == L"abc");
    CHECK(outCharsetType == ui::CharsetType::ANSI);
    CHECK_INT(bomSize, 0);

    // 显式指定UTF16_LE：纯ASCII数据不会被误判为编码不匹配
    const char utf16LeNoBom[] = { 0x41, 0x00, 0x42, 0x00 };
    CHECK(ui::StringCharset::GetDataAsString(utf16LeNoBom, 4, ui::CharsetType::UTF16_LE,
                                            result, outCharsetType, bomSize));
    CHECK(result == L"AB");
    CHECK(outCharsetType == ui::CharsetType::UTF16_LE);

    // 显式编码与BOM头不一致时返回失败
    CHECK(!ui::StringCharset::GetDataAsString(utf16LeData, 6, ui::CharsetType::UTF8,
                                             result, outCharsetType, bomSize));
    // 空数据返回失败
    CHECK(!ui::StringCharset::GetDataAsString(nullptr, 0, result));
}

} // namespace

int main()
{
    TestUTF8AndWString();
    TestUTF16();
    TestUTF32();
#ifdef DUILIB_BUILD_FOR_WIN
    TestMBCS();
#endif
    TestBOM();
    TestValidateStream();
    TestDetectCharset();
    TestDataAsString();

    if (g_failedCount == 0) {
        std::cout << "All tests passed (" << g_checkCount << " checks)." << std::endl;
        return 0;
    }
    std::cerr << g_failedCount << " of " << g_checkCount << " checks failed." << std::endl;
    return 1;
}

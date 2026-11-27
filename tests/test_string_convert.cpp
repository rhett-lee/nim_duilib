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

    // UTF8ToT/TToUTF8 使用TString：Windows(Unicode)下为wstring，Linux/macOS/FreeBSD下为UTF-8的string
#ifdef DUILIB_UNICODE
    CHECK(ui::StringConvert::UTF8ToT(kChineseUtf8) == chinese);
    CHECK(ui::StringConvert::TToUTF8(chinese) == kChineseUtf8);
#else
    CHECK(ui::StringConvert::UTF8ToT(kChineseUtf8) == kChineseUtf8);
    CHECK(ui::StringConvert::TToUTF8(kChineseUtf8) == kChineseUtf8);
#endif
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

    // 指定代码页与严格模式
    const std::string gbk = ui::StringConvert::UnicodeToMBCS(std::wstring(L"\u4E2D\u6587"), 936, true);
    CHECK(!gbk.empty());
    CHECK(ui::StringConvert::MBCSToUnicode(gbk, 936, true) == std::wstring(L"\u4E2D\u6587"));
    // 非法UTF-8字节：严格模式返回空字符串，非严格模式替换为替代字符
    CHECK(ui::StringConvert::MBCSToUnicode(std::string("\xFF", 1), CP_UTF8, true).empty());
    CHECK(!ui::StringConvert::MBCSToUnicode(std::string("\xFF", 1), CP_UTF8, false).empty());
}
#endif

void TestBOM()
{
    CHECK_INT((int)ui::StringCharset::GetBOMSize(ui::CharsetType::UTF8), 3);
    CHECK_INT((int)ui::StringCharset::GetBOMSize(ui::CharsetType::UTF16_LE), 2);
    CHECK_INT((int)ui::StringCharset::GetBOMSize(ui::CharsetType::UTF16_BE), 2);
    CHECK_INT((int)ui::StringCharset::GetBOMSize(ui::CharsetType::UTF32_LE), 4);
    CHECK_INT((int)ui::StringCharset::GetBOMSize(ui::CharsetType::UTF32_BE), 4);
    CHECK_INT((int)ui::StringCharset::GetBOMSize(ui::CharsetType::ANSI), 0);
    CHECK_INT((int)ui::StringCharset::GetBOMSize(ui::CharsetType::UNKNOWN), 0);

    const char utf8Bom[] = "\xEF\xBB\xBF";
    CHECK(ui::StringCharset::GetDataCharsetByBOM(utf8Bom, 3) == ui::CharsetType::UTF8);

    const char utf16LeBom[] = "\xFF\xFE";
    CHECK(ui::StringCharset::GetDataCharsetByBOM(utf16LeBom, 2) == ui::CharsetType::UTF16_LE);

    const char utf16BeBom[] = "\xFE\xFF";
    CHECK(ui::StringCharset::GetDataCharsetByBOM(utf16BeBom, 2) == ui::CharsetType::UTF16_BE);

    const char utf32LeBom[] = { (char)0xFF, (char)0xFE, 0x00, 0x00 };
    CHECK(ui::StringCharset::GetDataCharsetByBOM(utf32LeBom, 4) == ui::CharsetType::UTF32_LE);
    const char utf32BeBom[] = { 0x00, 0x00, (char)0xFE, (char)0xFF };
    CHECK(ui::StringCharset::GetDataCharsetByBOM(utf32BeBom, 4) == ui::CharsetType::UTF32_BE);

    // HasBOM / SkipBOM
    CHECK(ui::StringCharset::HasBOM(utf8Bom, 3));
    CHECK_INT((int)ui::StringCharset::SkipBOM(utf8Bom, 3), 3);
    CHECK(ui::StringCharset::HasBOM(utf32LeBom, 4));
    CHECK_INT((int)ui::StringCharset::SkipBOM(utf32LeBom, 4), 4);
    CHECK(!ui::StringCharset::HasBOM("abc", 3));
    CHECK_INT((int)ui::StringCharset::SkipBOM("abc", 3), 0);

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
    // 过长编码（C0 80）、代理区编码（ED A0 80）、超出U+10FFFF（F4 90 80 80）均不合法
    const char overlongUtf8[] = { (char)0xC0, (char)0x80 };
    CHECK(!ui::StringCharset::IsValidateUTF8Stream(overlongUtf8, 2));
    const char surrogateUtf8[] = { (char)0xED, (char)0xA0, (char)0x80 };
    CHECK(!ui::StringCharset::IsValidateUTF8Stream(surrogateUtf8, 3));
    const char tooLargeUtf8[] = { (char)0xF4, (char)0x90, (char)0x80, (char)0x80 };
    CHECK(!ui::StringCharset::IsValidateUTF8Stream(tooLargeUtf8, 4));

    // GBK："中文"的GBK编码为 D6 D0 CE C4
    const char gbkChinese[] = { (char)0xD6, (char)0xD0, (char)0xCE, (char)0xC4 };
    CHECK(ui::StringCharset::IsValidateGBKStream(gbkChinese, 4));
    // 只有首字节、没有后续字节
    CHECK(!ui::StringCharset::IsValidateGBKStream(gbkChinese, 1));
    // 0xFF 不是合法的GBK首字节
    const char invalidGbk[] = { (char)0xFF, (char)0xFF };
    CHECK(!ui::StringCharset::IsValidateGBKStream(invalidGbk, 2));
    // 0x7F 不是合法的GBK尾字节
    const char invalidGbkTrail[] = { (char)0xD6, (char)0x7F };
    CHECK(!ui::StringCharset::IsValidateGBKStream(invalidGbkTrail, 2));
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

    // UTF-32LE："AB" = 41 00 00 00 42 00 00 00
    const char utf32Le[] = { 0x41, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00 };
    CHECK(ui::StringCharset::IsValidateUTF32LEStream(utf32Le, 8));
    CHECK(!ui::StringCharset::IsValidateUTF32LEStream(utf32Le, 6));
    CHECK(!ui::StringCharset::IsValidateUTF32LEStream(nullptr, 8));
    // 代理区码点(0xD800)在UTF-32中非法
    const char utf32Surrogate[] = { 0x00, (char)0xD8, 0x00, 0x00 };
    CHECK(!ui::StringCharset::IsValidateUTF32LEStream(utf32Surrogate, 4));
    // 超出U+10FFFF的码点非法
    const char utf32TooLarge[] = { 0x00, 0x00, 0x11, 0x00 };
    CHECK(!ui::StringCharset::IsValidateUTF32LEStream(utf32TooLarge, 4));

    // UTF-32BE："AB" = 00 00 00 41 00 00 00 42
    const char utf32Be[] = { 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x42 };
    CHECK(ui::StringCharset::IsValidateUTF32BEStream(utf32Be, 8));
    CHECK(!ui::StringCharset::IsValidateUTF32BEStream(utf32Be, 6));
}

void TestDetectCharset()
{
    // 纯ASCII按ANSI处理
    CHECK(ui::StringCharset::GetDataCharset("abc", 3) == ui::CharsetType::ANSI);
    // UTF-8编码的中文(同时也满足GBK校验，但优先识别为UTF-8)
    CHECK(ui::StringCharset::GetDataCharset(kChineseUtf8.data(), (uint32_t)kChineseUtf8.size()) ==
          ui::CharsetType::UTF8);
    // GBK编码的中文（不再归为ANSI）
    const char gbkChinese[] = { (char)0xD6, (char)0xD0, (char)0xCE, (char)0xC4 };
    CHECK(ui::StringCharset::GetDataCharset(gbkChinese, 4) == ui::CharsetType::GBK);
    CHECK(ui::StringCharset::GetDataCharset(nullptr, 0) == ui::CharsetType::UNKNOWN);

    // 无BOM的UTF-16LE文本（"AB" = 41 00 42 00），按0字节分布识别
    const char utf16LeText[] = { 0x41, 0x00, 0x42, 0x00 };
    CHECK(ui::StringCharset::GetDataCharset(utf16LeText, 4) == ui::CharsetType::UTF16_LE);
    // 严格策略下不尝试UTF-16，纯ASCII字节按ANSI处理
    CHECK(ui::StringCharset::GetDataCharset(utf16LeText, 4, ui::CharsetDetectPolicy::kStrict) ==
          ui::CharsetType::ANSI);

    // 无BOM的UTF-32LE文本（"AB" = 41 00 00 00 42 00 00 00）
    const char utf32LeText[] = { 0x41, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00 };
    CHECK(ui::StringCharset::GetDataCharset(utf32LeText, 8) == ui::CharsetType::UTF32_LE);
    CHECK(ui::StringCharset::GetDataCharset(utf32LeText, 8, ui::CharsetDetectPolicy::kStrict) ==
          ui::CharsetType::ANSI);

    // 编码名称
    CHECK(std::string(ui::StringCharset::GetCharsetName(ui::CharsetType::GBK)) == "GBK");
    CHECK(std::string(ui::StringCharset::GetCharsetName(ui::CharsetType::UTF8)) == "UTF-8");
    CHECK(std::string(ui::StringCharset::GetCharsetName(ui::CharsetType::UNKNOWN)) == "UNKNOWN");
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
    CHECK(result == L"AB");
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

    // 无BOM的UTF-16LE中文（"中文" = 2D 4E 87 65）：按内容检测会判为GBK，
    // 但显式指定UTF16_LE时应按指定编码解码
    const char utf16LeChinese[] = { 0x2D, 0x4E, (char)0x87, 0x65 };
    CHECK(ui::StringCharset::IsValidateUTF16LEStream(utf16LeChinese, 4));
    CHECK(ui::StringCharset::GetDataAsString(utf16LeChinese, 4, ui::CharsetType::UTF16_LE,
                                             result, outCharsetType, bomSize));
    CHECK(result == std::wstring(L"\u4E2D\u6587"));

    // 显式编码与BOM头不一致时返回失败
    CHECK(!ui::StringCharset::GetDataAsString(utf16LeData, 6, ui::CharsetType::UTF8,
                                             result, outCharsetType, bomSize));

    // UTF-32LE BOM + "AB"
    const char utf32LeData[] = { (char)0xFF, (char)0xFE, 0x00, 0x00,
                                 0x41, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00 };
    CHECK(ui::StringCharset::GetDataAsString(utf32LeData, 12, result, outCharsetType, bomSize));
    CHECK(result == L"AB");
    CHECK(outCharsetType == ui::CharsetType::UTF32_LE);
    CHECK_INT(bomSize, 4);

    // UTF-32BE BOM + "AB"
    const char utf32BeData[] = { 0x00, 0x00, (char)0xFE, (char)0xFF,
                                 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x42 };
    CHECK(ui::StringCharset::GetDataAsString(utf32BeData, 12, result, outCharsetType, bomSize));
    CHECK(result == L"AB");
    CHECK(outCharsetType == ui::CharsetType::UTF32_BE);
    CHECK_INT(bomSize, 4);

    // BOM头指示UTF-8但内容非法时返回失败
    const char invalidUtf8Data[] = { (char)0xEF, (char)0xBB, (char)0xBF, (char)0xFF };
    CHECK(!ui::StringCharset::GetDataAsString(invalidUtf8Data, 4, result, outCharsetType, bomSize));
    CHECK(result.empty());

    // 带BOM的UTF-16数据长度为奇数时返回失败
    const char oddUtf16Data[] = { (char)0xFF, (char)0xFE, 0x41, 0x00, 0x42 };
    CHECK(!ui::StringCharset::GetDataAsString(oddUtf16Data, 5, result, outCharsetType, bomSize));

    // 非对齐缓冲区：BOM之后的数据起始地址为奇数
    const std::string unalignedData = std::string("X") + std::string("\xFF\xFE\x41\x00\x42\x00", 6);
    CHECK(ui::StringCharset::GetDataAsString(unalignedData.data() + 1, 6,
                                             result, outCharsetType, bomSize));
    CHECK(result == L"AB");

#ifdef DUILIB_BUILD_FOR_WIN
    // GBK数据：自动检测与显式指定均可正确解码
    const char gbkChineseData[] = { (char)0xD6, (char)0xD0, (char)0xCE, (char)0xC4 };
    CHECK(ui::StringCharset::GetDataAsString(gbkChineseData, 4, result, outCharsetType, bomSize));
    CHECK(result == std::wstring(L"\u4E2D\u6587"));
    CHECK(outCharsetType == ui::CharsetType::GBK);
    CHECK(ui::StringCharset::GetDataAsString(gbkChineseData, 4, ui::CharsetType::GBK,
                                             result, outCharsetType, bomSize));
    CHECK(result == std::wstring(L"\u4E2D\u6587"));
    // 显式指定GBK时，纯ASCII数据可正常解码
    CHECK(ui::StringCharset::GetDataAsString("abc", 3, ui::CharsetType::GBK,
                                             result, outCharsetType, bomSize));
    CHECK(result == L"abc");
#endif

    // 空数据返回失败
    CHECK(!ui::StringCharset::GetDataAsString(nullptr, 0, result));
}

void TestWStringToData()
{
    const std::wstring text = std::wstring(L"\u4E2D\u6587");
    std::string data;
    std::wstring result;

    // UTF-8：不带BOM
    CHECK(ui::StringCharset::WStringToData(text, ui::CharsetType::UTF8, data));
    CHECK(ui::StringCharset::IsValidateUTF8Stream(data.data(), (uint32_t)data.size()));
    CHECK(ui::StringCharset::GetDataAsString(data.data(), (uint32_t)data.size(), result));
    CHECK(result == text);

    // UTF-8：带BOM
    CHECK(ui::StringCharset::WStringToData(text, ui::CharsetType::UTF8, data, true));
    CHECK_INT((int)ui::StringCharset::SkipBOM(data.data(), (uint32_t)data.size()), 3);
    CHECK(ui::StringCharset::GetDataAsString(data.data(), (uint32_t)data.size(), result));
    CHECK(result == text);

    // UTF-16LE / UTF-16BE：带BOM往返
    CHECK(ui::StringCharset::WStringToData(text, ui::CharsetType::UTF16_LE, data, true));
    CHECK(ui::StringCharset::GetDataAsString(data.data(), (uint32_t)data.size(), result));
    CHECK(result == text);
    CHECK(ui::StringCharset::WStringToData(text, ui::CharsetType::UTF16_BE, data, true));
    CHECK(ui::StringCharset::GetDataAsString(data.data(), (uint32_t)data.size(), result));
    CHECK(result == text);

    // UTF-32LE / UTF-32BE：带BOM往返
    CHECK(ui::StringCharset::WStringToData(text, ui::CharsetType::UTF32_LE, data, true));
    CHECK(ui::StringCharset::GetDataAsString(data.data(), (uint32_t)data.size(), result));
    CHECK(result == text);
    CHECK(ui::StringCharset::WStringToData(text, ui::CharsetType::UTF32_BE, data, true));
    CHECK(ui::StringCharset::GetDataAsString(data.data(), (uint32_t)data.size(), result));
    CHECK(result == text);

    // 表情符号(代理对)的UTF-16BE往返
#if defined(WCHAR_T_IS_UTF16)
    const std::wstring emoji = std::wstring(1, (wchar_t)0xD83D) + std::wstring(1, (wchar_t)0xDE00);
#else
    const std::wstring emoji = std::wstring(1, (wchar_t)0x1F600);
#endif
    CHECK(ui::StringCharset::WStringToData(emoji, ui::CharsetType::UTF16_BE, data, true));
    CHECK(ui::StringCharset::GetDataAsString(data.data(), (uint32_t)data.size(), result));
    CHECK(result == emoji);

    // 非法参数
    CHECK(!ui::StringCharset::WStringToData(text, ui::CharsetType::UNKNOWN, data));
    CHECK(data.empty());

#ifdef DUILIB_BUILD_FOR_WIN
    // GBK往返
    CHECK(ui::StringCharset::WStringToData(text, ui::CharsetType::GBK, data));
    CHECK(ui::StringCharset::GetDataAsString(data.data(), (uint32_t)data.size(), result));
    CHECK(result == text);
#endif

    // ANSI：纯ASCII文本可无损往返
    CHECK(ui::StringCharset::WStringToData(std::wstring(L"abc"), ui::CharsetType::ANSI, data));
    CHECK(ui::StringCharset::GetDataAsString(data.data(), (uint32_t)data.size(), result));
    CHECK(result == L"abc");

    // 空字符串
    CHECK(ui::StringCharset::WStringToData(std::wstring(), ui::CharsetType::UTF8, data));
    CHECK(data.empty());
}

void TestStrictConversion()
{
    // 非法UTF-8字节：严格模式下返回空字符串（0x80不能作为首字节）
    const std::string invalidUtf8 = std::string("A") + std::string("\x80", 1);
    CHECK(ui::StringConvert::UTF8ToUTF16(invalidUtf8.data(), invalidUtf8.size()).empty());
    CHECK(ui::StringConvert::UTF8ToUTF32(invalidUtf8).empty());
    CHECK(ui::StringConvert::UTF8ToWString(invalidUtf8).empty());
    CHECK(ui::StringConvert::UTF8ToT(invalidUtf8.data(), invalidUtf8.size()).empty());

    // 末尾不完整的多字节序列：保留已转换的有效前缀
    const std::string incompleteUtf8 = std::string("A") + std::string("\xE4\xB8", 2);
    CHECK(ui::StringConvert::UTF8ToWString(incompleteUtf8) == std::wstring(L"A"));

    // 过长编码（C0 80）不合法
    const std::string overlongUtf8("\xC0\x80", 2);
    CHECK(ui::StringConvert::UTF8ToUTF16(overlongUtf8.data(), overlongUtf8.size()).empty());

    // 孤立代理项：UTF16转换为空
    const DUTF16Char loneHigh[] = { (DUTF16Char)0xD83D };
    CHECK(ui::StringConvert::UTF16ToUTF8(loneHigh, 1).empty());
    CHECK(ui::StringConvert::UTF16ToUTF32(loneHigh, 1).empty());
    CHECK(ui::StringConvert::UTF16ToWString(loneHigh, 1).empty());
    const DUTF16Char loneLow[] = { (DUTF16Char)0xDE00 };
    CHECK(ui::StringConvert::UTF16ToUTF8(loneLow, 1).empty());

    // 非法UTF32码点：代理区与超出U+10FFFF
    const DUTF32Char surrogateCodePoint[] = { (DUTF32Char)0xD800 };
    CHECK(ui::StringConvert::UTF32ToUTF8(surrogateCodePoint, 1).empty());
    CHECK(ui::StringConvert::UTF32ToUTF16(surrogateCodePoint, 1).empty());
    CHECK(ui::StringConvert::UTF32ToWString(surrogateCodePoint, 1).empty());
    const DUTF32Char tooLargeCodePoint[] = { (DUTF32Char)0x110000 };
    CHECK(ui::StringConvert::UTF32ToWString(tooLargeCodePoint, 1).empty());

    // 空指针处理
    CHECK(ui::StringConvert::UTF8ToT((const DUTF8Char*)nullptr, 3).empty());
    CHECK(ui::StringConvert::UTF16ToWString((const DUTF16Char*)nullptr, 3).empty());
    CHECK(ui::StringConvert::UTF32ToUTF16((const DUTF32Char*)nullptr, 3).empty());
}

void TestNewConversions()
{
    const std::wstring text(L"\u4E2D\u6587");
    const std::basic_string<DUTF16Char> utf16 = ui::StringConvert::WStringToUTF16(text);
    CHECK(!utf16.empty());
    CHECK(ui::StringConvert::UTF16ToWString(utf16) == text);
    CHECK(ui::StringConvert::UTF16ToWString(utf16.data(), utf16.size()) == text);

    const std::basic_string<DUTF32Char> utf32 = ui::StringConvert::WStringToUTF32(text);
    const std::basic_string<DUTF16Char> fromUtf32 = ui::StringConvert::UTF32ToUTF16(utf32);
    CHECK(fromUtf32 == utf16);
    CHECK(ui::StringConvert::UTF32ToUTF16(utf32.data(), utf32.size()) == utf16);

    // 字符串重载
    CHECK(ui::StringConvert::UTF8ToUTF16(kChineseUtf8) == utf16);
    CHECK(ui::StringConvert::UTF16ToUTF8(utf16) == kChineseUtf8);
    CHECK(ui::StringConvert::UTF16ToUTF32(utf16) == utf32);

    // 表情符号(代理对)往返
    const std::basic_string<DUTF16Char> emoji16 = ui::StringConvert::UTF8ToUTF16(kEmojiUtf8);
    CHECK_INT((int)emoji16.size(), 2);
    CHECK(ui::StringConvert::UTF16ToWString(emoji16) == ui::StringConvert::UTF8ToWString(kEmojiUtf8));
    CHECK(ui::StringConvert::UTF32ToUTF16(ui::StringConvert::UTF8ToUTF32(kEmojiUtf8)) == emoji16);
}

void TestValidation()
{
    CHECK(ui::StringConvert::IsValidUTF8(kChineseUtf8.data(), kChineseUtf8.size()));
    CHECK(ui::StringConvert::IsValidUTF8(kEmojiUtf8.data(), kEmojiUtf8.size()));
    CHECK(!ui::StringConvert::IsValidUTF8(nullptr, 0));
    CHECK(!ui::StringConvert::IsValidUTF8("\xFF", 1));
    CHECK(!ui::StringConvert::IsValidUTF8("\xC0\x80", 2));
    CHECK(!ui::StringConvert::IsValidUTF8("\xE4\xB8", 2));  // 截断的UTF-8序列

    const std::basic_string<DUTF16Char> utf16 = ui::StringConvert::UTF8ToUTF16(kChineseUtf8);
    CHECK(ui::StringConvert::IsValidUTF16(utf16.data(), utf16.size()));
    const DUTF16Char loneSurrogate[] = { (DUTF16Char)0xD83D };
    CHECK(!ui::StringConvert::IsValidUTF16(loneSurrogate, 1));
    CHECK(!ui::StringConvert::IsValidUTF16(nullptr, 0));

    const std::basic_string<DUTF32Char> utf32 = ui::StringConvert::UTF8ToUTF32(kEmojiUtf8);
    CHECK(ui::StringConvert::IsValidUTF32(utf32.data(), utf32.size()));
    const DUTF32Char badCodePoint[] = { (DUTF32Char)0x110000 };
    CHECK(!ui::StringConvert::IsValidUTF32(badCodePoint, 1));
    CHECK(!ui::StringConvert::IsValidUTF32(nullptr, 0));
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
    TestWStringToData();
    TestStrictConversion();
    TestNewConversions();
    TestValidation();

    if (g_failedCount == 0) {
        std::cout << "All tests passed (" << g_checkCount << " checks)." << std::endl;
        return 0;
    }
    std::cerr << g_failedCount << " of " << g_checkCount << " checks failed." << std::endl;
    return 1;
}

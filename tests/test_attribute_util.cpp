// duilib/Utils 属性解析与进程单例通信协议的单元测试
//
// 覆盖 AttributeUtil 的 Size/Point/Rect/Padding/Margin 解析、百分比解析、
// 属性列表解析（单引号/双引号/花括号），DiskUtils 的盘符提取，
// 以及 ProcessSingletonData 的序列化/反序列化（含异常场景）。
// 全部为纯逻辑，不创建窗口、不加载资源。

#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/ProcessSingletonData.h"
#ifdef DUILIB_BUILD_FOR_WIN
#include "duilib/Utils/DiskUtils_Windows.h"
#endif

#include <cstdint>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
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

#define CHECK_WSTR(actual, expected) \
    Check((actual) == (expected), [&]() { \
        return std::string(#actual " == " #expected); \
    })

void TestParseSizeAndPoint()
{
    ui::UiSize size;
    ui::AttributeUtil::ParseSizeValue(L"800,500", size);
    CHECK_INT(size.cx, 800);
    CHECK_INT(size.cy, 500);

    // 空格作为分隔符
    ui::AttributeUtil::ParseSizeValue(L"120 80", size);
    CHECK_INT(size.cx, 120);
    CHECK_INT(size.cy, 80);

    // 只有一个值时，第二个值解析为0
    ui::AttributeUtil::ParseSizeValue(L"64", size);
    CHECK_INT(size.cx, 64);
    CHECK_INT(size.cy, 0);

    // 空输入时清零
    size.SetWidthHeight(1, 2);
    ui::AttributeUtil::ParseSizeValue(L"", size);
    CHECK_INT(size.cx, 0);
    CHECK_INT(size.cy, 0);

    ui::UiPoint point;
    ui::AttributeUtil::ParsePointValue(L"-10,20", point);
    CHECK_INT(point.x, -10);
    CHECK_INT(point.y, 20);
}

void TestParseRectPaddingMargin()
{
    ui::UiRect rect;
    ui::AttributeUtil::ParseRectValue(L"10,20,110,220", rect);
    CHECK_INT(rect.left, 10);
    CHECK_INT(rect.top, 20);
    CHECK_INT(rect.right, 110);
    CHECK_INT(rect.bottom, 220);
    CHECK_INT(rect.Width(), 100);
    CHECK_INT(rect.Height(), 200);

    // 空输入时清零
    ui::AttributeUtil::ParseRectValue(L"", rect);
    CHECK(rect.IsEmpty());

    ui::UiPadding padding;
    ui::AttributeUtil::ParsePaddingValue(L"1,2,3,4", padding);
    CHECK_INT(padding.left, 1);
    CHECK_INT(padding.top, 2);
    CHECK_INT(padding.right, 3);
    CHECK_INT(padding.bottom, 4);

    ui::UiMargin margin;
    ui::AttributeUtil::ParseMarginValue(L"5,6,7,8", margin);
    CHECK_INT(margin.left, 5);
    CHECK_INT(margin.top, 6);
    CHECK_INT(margin.right, 7);
    CHECK_INT(margin.bottom, 8);
}

void TestParseString()
{
    wchar_t text[] = L"50%,20";
    wchar_t* pEnd = nullptr;
    std::tuple<int32_t, float> value = ui::AttributeUtil::ParseString(text, &pEnd);
    CHECK_INT(std::get<0>(value), 0);
    CHECK(std::get<1>(value) > 49.9f && std::get<1>(value) < 50.1f);
    CHECK(pEnd == text + 3); // 停在逗号前

    value = ui::AttributeUtil::ParseString(pEnd + 1, &pEnd);
    CHECK_INT(std::get<0>(value), 20);
    CHECK(std::get<1>(value) < 0.0001f);

    // 解析完成后应继续跳过分隔符
    ui::AttributeUtil::SkipSepChar(pEnd);

    // 空字符串返回0，且结束指针为nullptr
    pEnd = reinterpret_cast<wchar_t*>(1);
    value = ui::AttributeUtil::ParseString(L"", &pEnd);
    CHECK_INT(std::get<0>(value), 0);
    CHECK(pEnd == nullptr);
}

void TestParseAttributeList()
{
    std::vector<std::pair<DString, DString>> attributes;

    // 双引号、单引号混合
    CHECK(ui::AttributeUtil::ParseAttributeList(
        DString(L"normal_text_color=\"white\" bkcolor='red' offset=\"1,1\""), attributes));
    CHECK_INT((int)attributes.size(), 3);
    if (attributes.size() == 3) {
        CHECK_WSTR(attributes[0].first, DString(L"normal_text_color"));
        CHECK_WSTR(attributes[0].second, DString(L"white"));
        CHECK_WSTR(attributes[1].first, DString(L"bkcolor"));
        CHECK_WSTR(attributes[1].second, DString(L"red"));
        CHECK_WSTR(attributes[2].first, DString(L"offset"));
        CHECK_WSTR(attributes[2].second, DString(L"1,1"));
    }

    // 花括号：内部可继续使用引号
    CHECK(ui::AttributeUtil::ParseAttributeList(
        DString(L"normal_image={file='test/arrow.svg' valign='center'}"), attributes));
    CHECK_INT((int)attributes.size(), 1);
    if (attributes.size() == 1) {
        CHECK_WSTR(attributes[0].first, DString(L"normal_image"));
        CHECK_WSTR(attributes[0].second, DString(L"file='test/arrow.svg' valign='center'"));
    }

    // 属性名两侧空白会被去除
    CHECK(ui::AttributeUtil::ParseAttributeList(DString(L"  width =\"24\"   "), attributes));
    CHECK_INT((int)attributes.size(), 1);
    if (attributes.size() == 1) {
        CHECK_WSTR(attributes[0].first, DString(L"width"));
        CHECK_WSTR(attributes[0].second, DString(L"24"));
    }

    // 空输入、空分隔符（长度不匹配）返回false，并清空输出
    CHECK(!ui::AttributeUtil::ParseAttributeList(DString(), attributes));
    CHECK(attributes.empty());
}

#ifdef DUILIB_BUILD_FOR_WIN
void TestGetDriveFromDirectoryPath()
{
    using ui::DiskUtils;
    CHECK_WSTR(DiskUtils::GetDriveFromDirectoryPath(DString(L"C:\\abc\\def")), DString(L"C:\\"));
    CHECK_WSTR(DiskUtils::GetDriveFromDirectoryPath(DString(L"D:/abc")), DString(L"D:/"));
    // "C:" 与盘符相对路径 "C:abc" 都应补全为 "C:\\"
    CHECK_WSTR(DiskUtils::GetDriveFromDirectoryPath(DString(L"C:")), DString(L"C:\\"));
    CHECK_WSTR(DiskUtils::GetDriveFromDirectoryPath(DString(L"C:abc")), DString(L"C:\\"));
    // 非盘符路径
    CHECK(DiskUtils::GetDriveFromDirectoryPath(DString(L"abc\\def")).empty());
    CHECK(DiskUtils::GetDriveFromDirectoryPath(DString(L"")).empty());
    CHECK(DiskUtils::GetDriveFromDirectoryPath(DString(L"C")).empty());
}
#endif

void TestProcessSingletonData()
{
    // 正常往返
    const std::vector<std::string> args = { "hello", "world", "1" };
    const std::string package = ui::ProcessSingletonData::SerializeArguments(args);
    CHECK_INT((int)package.size(),
              (int)(sizeof(ui::ProcessSingletonData::ProtocolHeader) +
                    std::string("hello\x1E" "world\x1E" "1\x1E").size()));
    const std::vector<std::string> decoded = ui::ProcessSingletonData::DeserializeData(package);
    CHECK(decoded == args);

    // 空参数列表
    const std::vector<std::string> emptyArgs;
    const std::string emptyPackage = ui::ProcessSingletonData::SerializeArguments(emptyArgs);
    CHECK_INT((int)emptyPackage.size(), (int)sizeof(ui::ProcessSingletonData::ProtocolHeader));
    CHECK(ui::ProcessSingletonData::DeserializeData(emptyPackage).empty());

    // 空字符串参数可以往返（空参数会被保留）
    const std::vector<std::string> emptyItem = { "" };
    const std::vector<std::string> emptyItemDecoded =
        ui::ProcessSingletonData::DeserializeData(ui::ProcessSingletonData::SerializeArguments(emptyItem));
    CHECK_INT((int)emptyItemDecoded.size(), 1);
    if (emptyItemDecoded.size() == 1) {
        CHECK(emptyItemDecoded[0].empty());
    }

    // 协议头损坏：长度不足
    bool caught = false;
    try {
        ui::ProcessSingletonData::DeserializeData(std::string("short"));
    }
    catch (const std::exception&) {
        caught = true;
    }
    CHECK(caught);

    // 协议头损坏：Magic不匹配
    std::string badMagic = package;
    badMagic[0] = static_cast<char>(badMagic[0] + 1);
    caught = false;
    try {
        ui::ProcessSingletonData::DeserializeData(badMagic);
    }
    catch (const std::exception&) {
        caught = true;
    }
    CHECK(caught);

    // 参数超长
    caught = false;
    try {
        const std::vector<std::string> tooLongArgs = { std::string(1025, 'a') };
        ui::ProcessSingletonData::SerializeArguments(tooLongArgs);
    }
    catch (const std::invalid_argument&) {
        caught = true;
    }
    CHECK(caught);
}

} // namespace

int main()
{
    TestParseSizeAndPoint();
    TestParseRectPaddingMargin();
    TestParseString();
    TestParseAttributeList();
#ifdef DUILIB_BUILD_FOR_WIN
    TestGetDriveFromDirectoryPath();
#endif
    TestProcessSingletonData();

    if (g_failedCount == 0) {
        std::cout << "All tests passed (" << g_checkCount << " checks)." << std::endl;
        return 0;
    }
    std::cerr << g_failedCount << " of " << g_checkCount << " checks failed." << std::endl;
    return 1;
}

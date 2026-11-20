// RenderSkia 目录的单元测试
//
// 测试对象：duilib/RenderSkia 下与平台无关的基础设施代码：
//  1. SkUTF：UTF-8/UTF-16/UTF-32 的字符计数、解码、编码与非法输入判断；
//  2. FastVector：容量管理、扩容/缩容、迭代器、swap 与移动语义；
//  3. SkiaTextData：三种编码下的字符枚举、单字符字节数、UTF-32 转换；
//  4. TextGlyphRunList：游标方式构建 run、空 run 丢弃、positions 模式的数据保存、
//     advances 模式的总宽度统计；
//  5. Render_Skia 的离屏渲染：位图渲染（CreateBitmapRender）、ReadPixels 的越界判断、
//     MakeImageSnapshot 的区域裁剪，用于覆盖 Render_Skia_Bitmap 的画布包装逻辑；
//  6. MeasureString：多行文本的行高是否与 fSpacingMul / fSpacingAdd 一致
//     （与 DrawString 的 SkTextBox 行距规则保持一致）。
//
// 说明：
//  - Clear/ClearRect/裁剪/BitBlt/AlphaBlend 等绘制语义已由 test_render_api.cpp 覆盖，
//    本文件不重复测试；
//  - 不创建窗口，离屏渲染对象由 IRenderFactory::CreateOffscreenRender 提供。

#if defined(_WIN32)
#include <crtdbg.h>
#endif

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Core/UiColor.h"
#include "duilib/Core/UiFont.h"
#include "duilib/Core/UiRect.h"
#include "duilib/Core/UiString.h"

#include "duilib/Render/IRender.h"
#include "duilib/RenderSkia/FastVector.h"
#include "duilib/RenderSkia/SkUTF.h"
#include "duilib/RenderSkia/SkiaTextData.h"
#include "duilib/RenderSkia/TextGlyphRun.h"

#include "duilib/Utils/FilePath.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;

void Check(bool condition, const char* expression)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << expression << std::endl;
    }
}

#define CHECK(condition) Check((condition), #condition)

void CheckInt(int64_t actual, int64_t expected, const char* expression)
{
    ++g_checkCount;
    if (actual != expected) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << expression << ": actual=" << actual
                  << ", expected=" << expected << std::endl;
    }
}

#define CHECK_INT(actual, expected) CheckInt((int64_t)(actual), (int64_t)(expected), #actual " == " #expected)

void CheckFloat(float actual, float expected, float tolerance, const char* expression)
{
    ++g_checkCount;
    if (std::fabs(actual - expected) > tolerance) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << expression << ": actual=" << actual
                  << ", expected=" << expected << std::endl;
    }
}

#define CHECK_FLOAT(actual, expected, tolerance) \
    CheckFloat((float)(actual), (float)(expected), (float)(tolerance), #actual " == " #expected)

/** 输出当前测试小节的标题，便于定位失败用例 */
void Section(const char* name)
{
    std::cerr << "--- " << name << std::endl;
}

/** 读取离屏渲染中的单个像素（ARGB 格式） */
bool ReadPixel(ui::IRender* pRender, int32_t x, int32_t y, uint32_t& color)
{
    color = 0;
    if (pRender == nullptr) {
        return false;
    }
    return pRender->ReadPixels(ui::UiRect(x, y, x + 1, y + 1), &color, sizeof(color));
}

/** 判断两个颜色是否接近（允许抗锯齿带来的轻微误差） */
bool ColorNear(uint32_t color, uint32_t expected, int32_t tolerance = 4)
{
    for (int32_t i = 0; i < 32; i += 8) {
        const int32_t a = (int32_t)((color >> i) & 0xFF);
        const int32_t b = (int32_t)((expected >> i) & 0xFF);
        if (std::abs(a - b) > tolerance) {
            char buf[128] = { 0 };
            std::snprintf(buf, sizeof(buf), "[COLOR] actual=0x%08X, expected=0x%08X", color, expected);
            std::cerr << buf << std::endl;
            return false;
        }
    }
    return true;
}

/** 创建离屏渲染对象 */
std::unique_ptr<ui::IRender> CreateRender(ui::IRenderFactory* pRenderFactory, uint32_t w, uint32_t h)
{
    if (pRenderFactory == nullptr) {
        return nullptr;
    }
    return pRenderFactory->CreateOffscreenRender(w, h);
}

/** 创建一个指定字号的字体对象 */
std::unique_ptr<ui::IFont> CreateFont(ui::IRenderFactory* pRenderFactory, const DString& fontName, int32_t fontSize)
{
    if (pRenderFactory == nullptr) {
        return nullptr;
    }
    std::unique_ptr<ui::IFont> pFont(pRenderFactory->CreateIFont());
    if (pFont == nullptr) {
        return nullptr;
    }
    ui::UiFont uiFont;
    uiFont.m_fontName = fontName;
    uiFont.m_fontSize = fontSize;
    if (!pFont->InitFont(uiFont)) {
        return nullptr;
    }
    return pFont;
}

// ---------------------------------------------------------------------------
// 1. SkUTF（SK-01 ~ SK-40）
// ---------------------------------------------------------------------------
void TestSkUTF()
{
    Section("TestSkUTF");

    //UTF-8 文本："A" + U+00A3 + U+20AC + U+1F600
    const char kUtf8[] = "A\xC2\xA3\xE2\x82\xAC\xF0\x9F\x98\x80";
    const size_t nUtf8Len = sizeof(kUtf8) - 1;
    CHECK_INT(nUtf8Len, 10);

    //SK-01 字符计数
    CHECK_INT(ui::SkUTF::CountUTF8(kUtf8, nUtf8Len), 4);
    CHECK_INT(ui::SkUTF::CountUTF8(kUtf8, 0), 0);
    CHECK_INT(ui::SkUTF::CountUTF8(kUtf8, 1), 1);
    CHECK_INT(ui::SkUTF::CountUTF8(kUtf8, 3), 2);
    CHECK_INT(ui::SkUTF::CountUTF8(nullptr, 0), 0);

    //SK-02 非法 UTF-8：孤立续字节、被截断的序列、非法首字节
    CHECK_INT(ui::SkUTF::CountUTF8("\x80", 1), -1);
    CHECK_INT(ui::SkUTF::CountUTF8("\xC2", 1), -1);
    CHECK_INT(ui::SkUTF::CountUTF8("\xE2\x82", 2), -1);
    CHECK_INT(ui::SkUTF::CountUTF8("\xF0\x9F\x98", 3), -1);
    CHECK_INT(ui::SkUTF::CountUTF8("\xFF", 1), -1);
    CHECK_INT(ui::SkUTF::CountUTF8("\xC2\x41", 2), -1);

    //SK-03 逐个解码：值与指针前进的字节数
    {
        const char* p = kUtf8;
        const char* pEnd = kUtf8 + nUtf8Len;
        CHECK_INT(ui::SkUTF::NextUTF8(&p, pEnd), 0x41);
        CHECK(p == kUtf8 + 1);
        CHECK_INT(ui::SkUTF::NextUTF8(&p, pEnd), 0xA3);
        CHECK(p == kUtf8 + 3);
        CHECK_INT(ui::SkUTF::NextUTF8(&p, pEnd), 0x20AC);
        CHECK(p == kUtf8 + 6);
        CHECK_INT(ui::SkUTF::NextUTF8(&p, pEnd), 0x1F600);
        CHECK(p == pEnd);

        //到达末尾后继续解码：返回 -1，指针保持在末尾
        CHECK_INT(ui::SkUTF::NextUTF8(&p, pEnd), -1);
        CHECK(p == pEnd);
    }

    //SK-04 非法输入：指针被移动到末尾
    {
        const char bad[] = "\x80";
        const char* p = bad;
        const char* pEnd = bad + 1;
        CHECK_INT(ui::SkUTF::NextUTF8(&p, pEnd), -1);
        CHECK(p == pEnd);

        p = bad;
        CHECK_INT(ui::SkUTF::NextUTF8WithReplacement(&p, pEnd), 0xFFFD);
        CHECK(p == pEnd);
    }

    //SK-05 UTF-8 编码（含长度查询）
    {
        char buf[4] = { 0 };
        CHECK_INT(ui::SkUTF::ToUTF8('A', buf), 1);
        CHECK(buf[0] == 'A');

        CHECK_INT(ui::SkUTF::ToUTF8(0xA3, buf), 2);
        CHECK((uint8_t)buf[0] == 0xC2);
        CHECK((uint8_t)buf[1] == 0xA3);

        CHECK_INT(ui::SkUTF::ToUTF8(0x20AC, buf), 3);
        CHECK((uint8_t)buf[0] == 0xE2);
        CHECK((uint8_t)buf[1] == 0x82);
        CHECK((uint8_t)buf[2] == 0xAC);

        CHECK_INT(ui::SkUTF::ToUTF8(0x1F600, buf), 4);
        CHECK((uint8_t)buf[0] == 0xF0);
        CHECK((uint8_t)buf[1] == 0x9F);
        CHECK((uint8_t)buf[2] == 0x98);
        CHECK((uint8_t)buf[3] == 0x80);

        //超出 Unicode 范围或为负数：无法编码
        CHECK_INT(ui::SkUTF::ToUTF8(-1, buf), 0);
        CHECK_INT(ui::SkUTF::ToUTF8(0x110000, buf), 0);
        CHECK_INT(ui::SkUTF::ToUTF8(0x10FFFF, buf), 4);

        //utf8 为 nullptr 时只返回所需字节数
        CHECK_INT(ui::SkUTF::ToUTF8('A'), 1);
        CHECK_INT(ui::SkUTF::ToUTF8(0x1F600), 4);
    }

    //SK-06 UTF-16 编码
    {
        uint16_t buf[2] = { 0 };
        CHECK_INT(ui::SkUTF::ToUTF16('A', buf), 1);
        CHECK_INT(buf[0], 0x41);

        CHECK_INT(ui::SkUTF::ToUTF16(0x20AC, buf), 1);
        CHECK_INT(buf[0], 0x20AC);

        CHECK_INT(ui::SkUTF::ToUTF16(0x1F600, buf), 2);
        CHECK_INT(buf[0], 0xD83D);
        CHECK_INT(buf[1], 0xDE00);

        CHECK_INT(ui::SkUTF::ToUTF16(-1, buf), 0);
        CHECK_INT(ui::SkUTF::ToUTF16(0x110000, buf), 0);

        CHECK_INT(ui::SkUTF::ToUTF16('A'), 1);
        CHECK_INT(ui::SkUTF::ToUTF16(0x1F600), 2);
    }

    //SK-07 UTF-16 字符计数与逐个解码
    {
        const uint16_t kUtf16[] = { 0x41, 0xD83D, 0xDE00 };
        CHECK_INT(ui::SkUTF::CountUTF16(kUtf16, 6), 2);
        CHECK_INT(ui::SkUTF::CountUTF16(kUtf16, 2), 1);
        //代理对不完整：非法
        CHECK_INT(ui::SkUTF::CountUTF16(kUtf16, 4), -1);
        //孤立低代理：非法
        const uint16_t kLowOnly[] = { 0xDE00 };
        CHECK_INT(ui::SkUTF::CountUTF16(kLowOnly, 2), -1);
        //字节数不是 2 的整数倍：非法
        CHECK_INT(ui::SkUTF::CountUTF16(kUtf16, 3), -1);

        const uint16_t* p = kUtf16;
        const uint16_t* pEnd = kUtf16 + 3;
        CHECK_INT(ui::SkUTF::NextUTF16(&p, pEnd), 0x41);
        CHECK_INT(ui::SkUTF::NextUTF16(&p, pEnd), 0x1F600);
        CHECK(p == pEnd);
        CHECK_INT(ui::SkUTF::NextUTF16(&p, pEnd), -1);
    }

    //SK-08 UTF-32 字符计数与逐个解码
    {
        const int32_t kUtf32[] = { 0x41, 0x20AC, 0x1F600 };
        CHECK_INT(ui::SkUTF::CountUTF32(kUtf32, 12), 3);
        CHECK_INT(ui::SkUTF::CountUTF32(kUtf32, 0), 0);
        //高位字节非 0：非法（Unicode 只使用低 24 位）
        const int32_t kInvalid1[] = { 0x01000000 };
        CHECK_INT(ui::SkUTF::CountUTF32(kInvalid1, 4), -1);
        const int32_t kInvalid2[] = { -1 };
        CHECK_INT(ui::SkUTF::CountUTF32(kInvalid2, 4), -1);
        //字节数不是 4 的整数倍：非法
        CHECK_INT(ui::SkUTF::CountUTF32(kUtf32, 6), -1);

        const int32_t* p = kUtf32;
        const int32_t* pEnd = kUtf32 + 3;
        CHECK_INT(ui::SkUTF::NextUTF32(&p, pEnd), 0x41);
        CHECK_INT(ui::SkUTF::NextUTF32(&p, pEnd), 0x20AC);
        CHECK_INT(ui::SkUTF::NextUTF32(&p, pEnd), 0x1F600);
        CHECK(p == pEnd);
        CHECK_INT(ui::SkUTF::NextUTF32(&p, pEnd), -1);
    }

    //SK-09 UTF-8 -> UTF-16
    {
        uint16_t dst[8] = { 0 };
        CHECK_INT(ui::SkUTF::UTF8ToUTF16(dst, 8, kUtf8, nUtf8Len), 5);
        CHECK_INT(dst[0], 0x41);
        CHECK_INT(dst[1], 0xA3);
        CHECK_INT(dst[2], 0x20AC);
        CHECK_INT(dst[3], 0xD83D);
        CHECK_INT(dst[4], 0xDE00);

        //容量不足：只写入能容纳的部分，返回值仍为完整长度
        uint16_t small[2] = { 0 };
        CHECK_INT(ui::SkUTF::UTF8ToUTF16(small, 2, kUtf8, nUtf8Len), 5);
        CHECK_INT(small[0], 0x41);
        CHECK_INT(small[1], 0xA3);

        //dst 为 nullptr：只查询需要的长度
        CHECK_INT(ui::SkUTF::UTF8ToUTF16(nullptr, 0, kUtf8, nUtf8Len), 5);

        //非法 UTF-8：返回 -1
        CHECK_INT(ui::SkUTF::UTF8ToUTF16(dst, 8, "\x80", 1), -1);
    }

    //SK-10 UTF-16 -> UTF-8（与 SK-09 互为逆过程）
    {
        const uint16_t kUtf16[] = { 0x41, 0xA3, 0x20AC, 0xD83D, 0xDE00 };
        char dst[16] = { 0 };
        CHECK_INT(ui::SkUTF::UTF16ToUTF8(dst, 16, kUtf16, 5), (int)nUtf8Len);
        CHECK(std::memcmp(dst, kUtf8, nUtf8Len) == 0);

        CHECK_INT(ui::SkUTF::UTF16ToUTF8(nullptr, 0, kUtf16, 5), (int)nUtf8Len);

        //容量不足：只写入可容纳的字节
        char small[4] = { 0 };
        CHECK_INT(ui::SkUTF::UTF16ToUTF8(small, 4, kUtf16, 5), (int)nUtf8Len);
        CHECK(std::memcmp(small, kUtf8, 4) == 0);

        //孤立代理：非法
        const uint16_t kBad[] = { 0xD83D };
        CHECK_INT(ui::SkUTF::UTF16ToUTF8(dst, 16, kBad, 1), -1);
    }

    //SK-11 UTF-8 字节类型与代理判断
    {
        CHECK_INT(ui::SkUTF::SkUTF8_ByteType(0x41), 1);
        CHECK_INT(ui::SkUTF::SkUTF8_ByteType(0xC2), 2);
        CHECK_INT(ui::SkUTF::SkUTF8_ByteType(0xE2), 3);
        CHECK_INT(ui::SkUTF::SkUTF8_ByteType(0xF0), 4);
        CHECK_INT(ui::SkUTF::SkUTF8_ByteType(0x80), 0);
        CHECK_INT(ui::SkUTF::SkUTF8_ByteType(0xFF), -1);
        CHECK_INT(ui::SkUTF::SkUTF8_ByteType(0xC0), -1);
        CHECK_INT(ui::SkUTF::SkUTF8_ByteType(0xF5), -1);

        CHECK(ui::SkUTF::IsLeadingSurrogateUTF16(0xD83D));
        CHECK(!ui::SkUTF::IsLeadingSurrogateUTF16(0xDE00));
        CHECK(!ui::SkUTF::IsLeadingSurrogateUTF16(0x41));
        CHECK(ui::SkUTF::IsTrailingSurrogateUTF16(0xDE00));
        CHECK(!ui::SkUTF::IsTrailingSurrogateUTF16(0xD83D));
        CHECK(!ui::SkUTF::IsTrailingSurrogateUTF16(0x41));
    }
}

// ---------------------------------------------------------------------------
// 2. FastVector（SK-41 ~ SK-60）
// ---------------------------------------------------------------------------
void TestFastVector()
{
    Section("TestFastVector");
    ui::FastVector<int32_t> vec;
    CHECK(vec.empty());
    CHECK_INT(vec.size(), 0);
    CHECK_INT(vec.capacity(), 0);
    CHECK(vec.data() == nullptr);

    //SK-41 首次 push_back 分配容量，元素按顺序保存
    vec.push_back(1);
    CHECK(!vec.empty());
    CHECK_INT(vec.size(), 1);
    CHECK(vec.capacity() >= 1);
    CHECK(vec.data() != nullptr);
    CHECK_INT(vec[0], 1);

    //SK-42 连续增长不丢数据
    for (int32_t i = 1; i < 100; ++i) {
        vec.push_back(i);
    }
    CHECK_INT(vec.size(), 100);
    bool bAllMatch = true;
    for (int32_t i = 1; i < 100; ++i) {
        if (vec[(size_t)i] != i) {
            std::cerr << "[FAIL] FastVector push_back: index=" << i
                      << ", actual=" << vec[(size_t)i] << std::endl;
            bAllMatch = false;
            break;
        }
    }
    CHECK(bAllMatch);

    //SK-43 clear 只清空数据，保留已分配的容量
    const size_t nCapacity = vec.capacity();
    vec.clear();
    CHECK(vec.empty());
    CHECK_INT(vec.size(), 0);
    CHECK_INT(vec.capacity(), (int64_t)nCapacity);

    //SK-44 reserve：小于现有容量时不收缩；大于时扩容
    vec.reserve(4);
    CHECK_INT(vec.capacity(), (int64_t)nCapacity);
    vec.reserve(200);
    CHECK_INT(vec.capacity(), 200);
    CHECK(vec.empty());

    //SK-45 resize：增长时写入默认值，缩小时保留前缀，容量不变
    vec.resize(3, 7);
    CHECK_INT(vec.size(), 3);
    CHECK_INT(vec[0], 7);
    CHECK_INT(vec[1], 7);
    CHECK_INT(vec[2], 7);
    vec.resize(1);
    CHECK_INT(vec.size(), 1);
    CHECK_INT(vec[0], 7);
    CHECK_INT(vec.capacity(), 200);

    //SK-46 resize 到相同大小：不影响已有数据
    vec.resize(1, 9);
    CHECK_INT(vec.size(), 1);
    CHECK_INT(vec[0], 7);

    //SK-47 begin/end 迭代器
    vec.clear();
    for (int32_t i = 0; i < 5; ++i) {
        vec.push_back(i * 10);
    }
    int32_t nSum = 0;
    for (int32_t value : vec) {
        nSum += value;
    }
    CHECK_INT(nSum, 100);
    CHECK(vec.end() - vec.begin() == 5);

    //SK-48 swap：交换数据与容量
    ui::FastVector<int32_t> vecOther;
    vecOther.push_back(1000);
    vecOther.push_back(2000);
    vec.swap(vecOther);
    CHECK_INT(vec.size(), 2);
    CHECK_INT(vec[0], 1000);
    CHECK_INT(vec[1], 2000);
    CHECK_INT(vecOther.size(), 5);
    CHECK_INT(vecOther[0], 0);

    //SK-49 移动构造：接管数据，被移动对象被置空
    ui::FastVector<int32_t> vecMoved(std::move(vec));
    CHECK_INT(vecMoved.size(), 2);
    CHECK_INT(vecMoved[1], 2000);
    CHECK(vec.empty());
    CHECK(vec.data() == nullptr);
    CHECK_INT(vec.capacity(), 0);

    //SK-50 移动赋值
    ui::FastVector<int32_t> vecAssigned;
    vecAssigned.push_back(42);
    vecAssigned = std::move(vecMoved);
    CHECK_INT(vecAssigned.size(), 2);
    CHECK_INT(vecAssigned[0], 1000);
    CHECK(vecMoved.empty());
    CHECK(vecMoved.data() == nullptr);

    //SK-51 空容器上的操作是安全的
    ui::FastVector<int32_t> vecEmpty;
    vecEmpty.clear();
    vecEmpty.reserve(0);
    CHECK(vecEmpty.empty());
    CHECK(vecEmpty.begin() == vecEmpty.end());
}

// ---------------------------------------------------------------------------
// 3. SkiaTextData（SK-61 ~ SK-80）
// ---------------------------------------------------------------------------
void TestSkiaTextData()
{
    Section("TestSkiaTextData");

    //SK-61 默认构造：空文本
    {
        ui::SkiaTextData textData;
        CHECK(textData.IsEmpty());
    }

    //SK-62 UTF-8：字符枚举（码点 + 字节数）
    const char kUtf8[] = "A\xC2\xA3\xE2\x82\xAC\xF0\x9F\x98\x80";
    {
        ui::SkiaTextData textData(kUtf8, sizeof(kUtf8) - 1, SkTextEncoding::kUTF8);
        CHECK(!textData.IsEmpty());

        std::vector<SkUnichar> charList;
        std::vector<size_t> byteLenList;
        const bool bRet = textData.EnumChars([&](SkUnichar unicodeChar, size_t charByteLen) {
            charList.push_back(unicodeChar);
            byteLenList.push_back(charByteLen);
            return true;
        });
        CHECK(bRet);
        CHECK_INT(charList.size(), 4);
        CHECK_INT(byteLenList.size(), 4);
        if (charList.size() == 4) {
            CHECK_INT(charList[0], 0x41);
            CHECK_INT(charList[1], 0xA3);
            CHECK_INT(charList[2], 0x20AC);
            CHECK_INT(charList[3], 0x1F600);
            CHECK_INT(byteLenList[0], 1);
            CHECK_INT(byteLenList[1], 2);
            CHECK_INT(byteLenList[2], 3);
            CHECK_INT(byteLenList[3], 4);
        }

        //SK-63 回调返回 false：提前停止，整体返回 true
        size_t nVisitCount = 0;
        const bool bStopRet = textData.EnumChars([&](SkUnichar, size_t) {
            ++nVisitCount;
            return false;
        });
        CHECK(bStopRet);
        CHECK_INT(nVisitCount, 1);

        //SK-64 转换为 UTF-32 字符串
        auto utf32List = textData.ToUTF32String();
        CHECK_INT(utf32List.size(), 4);
        if (utf32List.size() == 4) {
            CHECK_INT(utf32List[0], 0x41);
            CHECK_INT(utf32List[1], 0xA3);
            CHECK_INT(utf32List[2], 0x20AC);
            CHECK_INT(utf32List[3], 0x1F600);
        }
    }

    //SK-65 UTF-8 非法输入：枚举失败
    {
        const char kInvalid[] = "\x80";
        ui::SkiaTextData textData(kInvalid, 1, SkTextEncoding::kUTF8);
        const bool bRet = textData.EnumChars([](SkUnichar, size_t) { return true; });
        CHECK(!bRet);
    }

    //SK-66 UTF-16：代理对作为一个字符，字节数为 4
    {
        const uint16_t kUtf16[] = { 0x41, 0xD83D, 0xDE00 };
        ui::SkiaTextData textData(kUtf16, sizeof(kUtf16), SkTextEncoding::kUTF16);
        std::vector<SkUnichar> charList;
        std::vector<size_t> byteLenList;
        const bool bRet = textData.EnumChars([&](SkUnichar unicodeChar, size_t charByteLen) {
            charList.push_back(unicodeChar);
            byteLenList.push_back(charByteLen);
            return true;
        });
        CHECK(bRet);
        CHECK_INT(charList.size(), 2);
        if (charList.size() == 2) {
            CHECK_INT(charList[0], 0x41);
            CHECK_INT(charList[1], 0x1F600);
            CHECK_INT(byteLenList[0], 2);
            CHECK_INT(byteLenList[1], 4);
        }
        CHECK_INT(textData.ToUTF32String().size(), 2);
    }

    //SK-67 UTF-32：每个字符 4 字节
    {
        const int32_t kUtf32[] = { 0x41, 0x20AC, 0x1F600 };
        ui::SkiaTextData textData(kUtf32, sizeof(kUtf32), SkTextEncoding::kUTF32);
        std::vector<SkUnichar> charList;
        std::vector<size_t> byteLenList;
        const bool bRet = textData.EnumChars([&](SkUnichar unicodeChar, size_t charByteLen) {
            charList.push_back(unicodeChar);
            byteLenList.push_back(charByteLen);
            return true;
        });
        CHECK(bRet);
        CHECK_INT(charList.size(), 3);
        CHECK_INT(byteLenList.size(), 3);
        for (size_t i = 0; i < byteLenList.size(); ++i) {
            CHECK_INT(byteLenList[i], 4);
        }
        if (charList.size() == 3) {
            CHECK_INT(charList[0], 0x41);
            CHECK_INT(charList[1], 0x20AC);
            CHECK_INT(charList[2], 0x1F600);
        }
    }

    //SK-68 单个字符的字节数（静态接口）
    {
        const char kEuro[] = "\xE2\x82\xAC";
        CHECK_INT(ui::SkiaTextData::GetCharByteLength(kEuro, SkTextEncoding::kUTF8), 3);
        CHECK_INT(ui::SkiaTextData::GetCharByteLength(kUtf8, SkTextEncoding::kUTF8), 1);
        const char kAscii[] = "A";
        CHECK_INT(ui::SkiaTextData::GetCharByteLength(kAscii, SkTextEncoding::kUTF8), 1);

        const uint16_t kUtf16[] = { 0xD83D, 0xDE00 };
        CHECK_INT(ui::SkiaTextData::GetCharByteLength(kUtf16, SkTextEncoding::kUTF16), 4);
        CHECK_INT(ui::SkiaTextData::GetCharByteLength(&kUtf16[1], SkTextEncoding::kUTF16), 2);

        const int32_t kUtf32[] = { 0x20AC };
        CHECK_INT(ui::SkiaTextData::GetCharByteLength(kUtf32, SkTextEncoding::kUTF32), 4);
    }
}

// ---------------------------------------------------------------------------
// 4. TextGlyphRunList（SK-81 ~ SK-100）
// ---------------------------------------------------------------------------
void TestTextGlyphRunList()
{
    Section("TestTextGlyphRunList");
    ui::TextGlyphRunList runList;
    CHECK_INT(runList.GetRunCount(), 0);
    CHECK(runList.IsEmpty());
    CHECK(!runList.IsInRun());
    CHECK_FLOAT(runList.GetTotalWidth(), 0.0f, 0.001f);

    //SK-81 游标方式构建一个 run
    runList.BeginRun(nullptr, nullptr, 0.0f, 0.0f);
    CHECK(runList.IsInRun());
    runList.AddGlyph(1, 10.0f);
    runList.AddGlyph(2, 20.0f);
    runList.AddGlyph(3, 30.0f);
    runList.EndRun();
    CHECK(!runList.IsInRun());

    CHECK_INT(runList.GetRunCount(), 1);
    CHECK_FLOAT(runList.GetTotalWidth(), 60.0f, 0.001f);
    {
        const ui::TextGlyphRun& run = runList.GetRun(0);
        CHECK_INT(run.m_nCount, 3);
        CHECK(run.m_pGlyphs != nullptr);
        CHECK(run.m_pAdvances != nullptr);
        CHECK(run.m_pPositions == nullptr);
        CHECK(!run.m_bRotate90);
        if ((run.m_pGlyphs != nullptr) && (run.m_pAdvances != nullptr)) {
            CHECK_INT(run.m_pGlyphs[0], 1);
            CHECK_INT(run.m_pGlyphs[2], 3);
            CHECK_FLOAT(run.m_pAdvances[0], 10.0f, 0.001f);
            CHECK_FLOAT(run.m_pAdvances[2], 30.0f, 0.001f);
        }
    }

    //SK-82 空 run 在 EndRun 时被丢弃
    runList.BeginRun(nullptr, nullptr, 0.0f, 0.0f);
    runList.EndRun();
    CHECK_INT(runList.GetRunCount(), 1);

    //SK-83 未调用 EndRun 就 BeginRun：自动结束上一个 run
    runList.BeginRun(nullptr, nullptr, 0.0f, 0.0f);
    runList.AddGlyph(5, 5.0f);
    runList.BeginRun(nullptr, nullptr, 0.0f, 0.0f);
    runList.AddGlyph(6, 6.0f);
    runList.EndRun();
    CHECK_INT(runList.GetRunCount(), 3);
    CHECK_FLOAT(runList.GetTotalWidth(), 71.0f, 0.001f);

    //SK-84 positions 模式：保存字形位置，不产生 advances（总宽度统计为 0）
    {
        ui::TextGlyphRunList posRunList;
        posRunList.BeginRun(nullptr, nullptr, 0.0f, 0.0f, false, true);
        posRunList.AddGlyph(10, 0.0f, SkPoint::Make(1.5f, 2.5f));
        posRunList.AddGlyph(11, 0.0f, SkPoint::Make(11.5f, 2.5f));
        posRunList.EndRun();
        CHECK_INT(posRunList.GetRunCount(), 1);
        const ui::TextGlyphRun& run = posRunList.GetRun(0);
        CHECK_INT(run.m_nCount, 2);
        CHECK(run.m_pPositions != nullptr);
        CHECK(run.m_pAdvances == nullptr);
        if (run.m_pPositions != nullptr) {
            CHECK_FLOAT(run.m_pPositions[0].fX, 1.5f, 0.001f);
            CHECK_FLOAT(run.m_pPositions[1].fX, 11.5f, 0.001f);
            CHECK_FLOAT(run.m_pPositions[1].fY, 2.5f, 0.001f);
        }
        CHECK_FLOAT(posRunList.GetTotalWidth(), 0.0f, 0.001f);
    }

    //SK-85 rotate 标记与 run 起点/基线坐标
    {
        ui::TextGlyphRunList rotateRunList;
        rotateRunList.BeginRun(nullptr, nullptr, 3.0f, 4.0f, true);
        rotateRunList.AddGlyph(20, 10.0f);
        rotateRunList.EndRun();
        CHECK_INT(rotateRunList.GetRunCount(), 1);
        const ui::TextGlyphRun& run = rotateRunList.GetRun(0);
        CHECK(run.m_bRotate90);
        CHECK_FLOAT(run.m_x, 3.0f, 0.001f);
        CHECK_FLOAT(run.m_y, 4.0f, 0.001f);
    }

    //SK-86 AddRun：批量添加（advances 为空时按 0 处理）
    {
        ui::TextGlyphRunList batchRunList;
        const SkGlyphID glyphs[] = { 30, 31, 32 };
        const SkScalar advances[] = { 1.0f, 2.0f, 3.0f };
        batchRunList.AddRun(nullptr, nullptr, glyphs, advances, 3, 0.0f, 0.0f);
        CHECK_INT(batchRunList.GetRunCount(), 1);
        CHECK_FLOAT(batchRunList.GetTotalWidth(), 6.0f, 0.001f);
        const ui::TextGlyphRun& run = batchRunList.GetRun(0);
        CHECK_INT(run.m_nCount, 3);
        if (run.m_pGlyphs != nullptr) {
            CHECK_INT(run.m_pGlyphs[1], 31);
        }

        //advances 为 nullptr
        ui::TextGlyphRunList batchRunList2;
        batchRunList2.AddRun(nullptr, nullptr, glyphs, nullptr, 2, 0.0f, 0.0f);
        CHECK_INT(batchRunList2.GetRunCount(), 1);
        CHECK_FLOAT(batchRunList2.GetTotalWidth(), 0.0f, 0.001f);

        //字形指针为空或数量为 0：不产生 run
        batchRunList2.AddRun(nullptr, nullptr, nullptr, advances, 3, 0.0f, 0.0f);
        batchRunList2.AddRun(nullptr, nullptr, glyphs, advances, 0, 0.0f, 0.0f);
        CHECK_INT(batchRunList2.GetRunCount(), 1);
    }

    //SK-87 Clear：清空 run 数量，同时退出 run 构建状态
    runList.BeginRun(nullptr, nullptr, 0.0f, 0.0f);
    runList.Clear();
    CHECK_INT(runList.GetRunCount(), 0);
    CHECK(!runList.IsInRun());
    CHECK_FLOAT(runList.GetTotalWidth(), 0.0f, 0.001f);
}

// ---------------------------------------------------------------------------
// 5. Render_Skia 的位图渲染与快照（SK-101 ~ SK-120）
// ---------------------------------------------------------------------------
void TestRenderSkiaBitmap(ui::IRenderFactory* pRenderFactory)
{
    Section("TestRenderSkiaBitmap");
    CHECK(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }

    //SK-101 位图渲染：绘制内容直接写入位图
    std::unique_ptr<ui::IBitmap> pBitmap(pRenderFactory->CreateBitmap(16, 8));
    CHECK(pBitmap != nullptr);
    if (pBitmap == nullptr) {
        return;
    }
    CHECK(pBitmap->IsValid());
    CHECK_INT(pBitmap->GetWidth(), 16);
    CHECK_INT(pBitmap->GetHeight(), 8);

    std::unique_ptr<ui::IRender> pBitmapRender = pRenderFactory->CreateBitmapRender(pBitmap.get());
    CHECK(pBitmapRender != nullptr);
    if (pBitmapRender == nullptr) {
        return;
    }
    CHECK_INT(pBitmapRender->GetWidth(), 16);
    CHECK_INT(pBitmapRender->GetHeight(), 8);

    const uint32_t kGreen = 0xFF00FF00u;
    pBitmapRender->Clear(ui::UiColor(0xFF000000u));
    pBitmapRender->FillRect(ui::UiRectF(0.0f, 0.0f, 16.0f, 8.0f), ui::UiColor(kGreen));

    uint32_t color = 0;
    CHECK(ReadPixel(pBitmapRender.get(), 0, 0, color));
    CHECK(ColorNear(color, kGreen));
    CHECK(ReadPixel(pBitmapRender.get(), 15, 7, color));
    CHECK(ColorNear(color, kGreen));

    //SK-102 位图数据与渲染结果一致（零拷贝）
    {
        const void* pPixelBits = pBitmap->GetPixelBits();
        CHECK(pPixelBits != nullptr);
        if (pPixelBits != nullptr) {
            const uint32_t* pPixels = (const uint32_t*)pPixelBits;
            CHECK(ColorNear(pPixels[0], kGreen));
            CHECK(ColorNear(pPixels[16 * 7], kGreen));
        }
    }

    //SK-103 位图渲染的 ReadPixels：越界矩形返回 false；完整位于画布内的 1x1 矩形成功
    CHECK(!pBitmapRender->ReadPixels(ui::UiRect(-1, 0, 4, 4), &color, sizeof(color)));
    CHECK(!pBitmapRender->ReadPixels(ui::UiRect(12, 0, 20, 4), &color, sizeof(color)));
    CHECK(pBitmapRender->ReadPixels(ui::UiRect(12, 0, 13, 1), &color, sizeof(color)));

    //SK-104 位图渲染的快照：整图与局部区域
    {
        std::shared_ptr<ui::IBitmap> spSnapshot(pBitmapRender->MakeImageSnapshot());
        CHECK(spSnapshot != nullptr);
        if (spSnapshot != nullptr) {
            CHECK_INT(spSnapshot->GetWidth(), 16);
            CHECK_INT(spSnapshot->GetHeight(), 8);
        }

        std::shared_ptr<ui::IBitmap> spPart(pBitmapRender->MakeImageSnapshot(ui::UiRect(4, 2, 12, 6)));
        CHECK(spPart != nullptr);
        if (spPart != nullptr) {
            CHECK_INT(spPart->GetWidth(), 8);
            CHECK_INT(spPart->GetHeight(), 4);
            const void* pPartBits = spPart->GetPixelBits();
            CHECK(pPartBits != nullptr);
            if (pPartBits != nullptr) {
                CHECK(ColorNear(((const uint32_t*)pPartBits)[0], kGreen));
            }
        }
    }

    //SK-105 位图渲染也支持克隆
    std::unique_ptr<ui::IBitmap> pClone(pBitmap->Clone());
    CHECK(pClone != nullptr);
    if (pClone != nullptr) {
        CHECK_INT(pClone->GetWidth(), 16);
        CHECK_INT(pClone->GetHeight(), 8);
        const void* pCloneBits = pClone->GetPixelBits();
        CHECK(pCloneBits != nullptr);
        if (pCloneBits != nullptr) {
            CHECK(ColorNear(((const uint32_t*)pCloneBits)[0], kGreen));
        }
    }
}

// ---------------------------------------------------------------------------
// 6. MeasureString 的行间距（SK-121 ~ SK-130）
// ---------------------------------------------------------------------------
void TestMeasureStringSpacing(ui::IRenderFactory* pRenderFactory)
{
    Section("TestMeasureStringSpacing");
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 64, 64);
    CHECK(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    std::unique_ptr<ui::IFont> pFont = CreateFont(pRenderFactory, _T("Arial"), 16);
    CHECK(pFont != nullptr);
    if (pFont == nullptr) {
        return;
    }

    const DString strText = _T("a\nb\nc");

    ui::MeasureStringParam param;
    param.pFont = pFont.get();
    param.limitWidth = 200;

    //SK-121 默认行间距（1.0 倍 + 0）：三行文本
    const ui::UiRect rcDefault = pRender->MeasureString(strText, param);
    CHECK(rcDefault.Width() > 0);
    CHECK(rcDefault.Height() > 0);

    //SK-122 行间距倍数与测量高度一致：行距 = 字体高度 × 倍数 + 附加量
    //     默认倍数 1.0 时，总高 = 字体高度 × 3；倍数 2.0 时，总高 = 字体高度 × 5
    param.fSpacingMul = 2.0f;
    const ui::UiRect rcMul = pRender->MeasureString(strText, param);
    if (rcDefault.Height() > 0) {
        const float fFontHeight = (float)rcDefault.Height() / 3.0f;
        CHECK_FLOAT((float)rcMul.Height(), fFontHeight * 5.0f, 2.0f);
    }

    //SK-123 附加量按“每行间距”累加：2 个行间距 × 10 像素
    param.fSpacingMul = 1.0f;
    param.fSpacingAdd = 10.0f;
    const ui::UiRect rcAdd = pRender->MeasureString(strText, param);
    CHECK_FLOAT((float)(rcAdd.Height() - rcDefault.Height()), 20.0f, 2.0f);

    //SK-124 单行模式不受行间距影响
    param.fSpacingMul = 2.0f;
    param.fSpacingAdd = 10.0f;
    param.uFormat = ui::DrawStringFormat::TEXT_SINGLELINE;
    const ui::UiRect rcSingleLine = pRender->MeasureString(strText, param);
    CHECK(rcSingleLine.Height() < rcDefault.Height());
}

} // namespace

int main()
{
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif

    ui::GlobalManager::Instance().SetAnimationEnabled(false);
    const bool bStartup = ui::GlobalManager::Instance().Startup(
        ui::LocalFilesResParam(ui::FilePath(DString(_T(DUILIB_TEST_RESOURCE_DIR)))));
    CHECK(bStartup);

    ui::IRenderFactory* pRenderFactory = ui::GlobalManager::Instance().GetRenderFactory();
    CHECK(pRenderFactory != nullptr);

    TestSkUTF();
    TestFastVector();
    TestSkiaTextData();
    TestTextGlyphRunList();
    if (pRenderFactory != nullptr) {
        TestRenderSkiaBitmap(pRenderFactory);
        TestMeasureStringSpacing(pRenderFactory);
    }

    ui::GlobalManager::Instance().Shutdown();

    if (g_failedCount == 0) {
        std::cerr << "All checks passed: " << g_checkCount << std::endl;
        return 0;
    }
    std::cerr << "Failed checks: " << g_failedCount << " / " << g_checkCount << std::endl;
    return 1;
}

// CEFControl 目录的单元测试
//
// 测试对象（不启动真实的 CEF 浏览器进程、不创建窗口）：
//  1. CefMemoryBlock：初始化参数校验、整图复制、脏区域增量复制、
//     尺寸变化时重建缓冲区、绘制到 Render 时的目标区域与裁剪语义、
//     以及 MakeImageSnapshot 的尺寸与参数校验；
//  2. CefUnregistedCallbackList / CefAutoUnregister：回调的注册、调用、
//     单独注销、批量清空、析构自动注销，以及列表销毁后注销回调的安全性。
//
// 说明：CefMemoryBlock 的绘制结果通过离屏渲染（IRenderFactory::CreateOffscreenRender）
//       写回像素后再读取校验，因此需要初始化 GlobalManager 以获取渲染工厂。

#if defined(_WIN32)
#include <crtdbg.h>
#endif

#include "duilib/CEFControl/internal/CefAutoUnregister.h"
#include "duilib/CEFControl/internal/CefMemoryBlock.h"

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Core/UiColor.h"
#include "duilib/Core/UiRect.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/FilePath.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
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

/** 校验指定位置的像素颜色，mismatch 时输出明细 */
void CheckPixel(ui::IRender* pRender, int32_t x, int32_t y, uint32_t expected, const char* tag)
{
    ++g_checkCount;
    uint32_t color = 0;
    if (!ReadPixel(pRender, x, y, color)) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << tag << ": ReadPixel failed at (" << x << "," << y << ")" << std::endl;
        return;
    }
    if (color != expected) {
        ++g_failedCount;
        char buf[160] = { 0 };
        std::snprintf(buf, sizeof(buf), "[FAIL] %s: (%d,%d) actual=0x%08X, expected=0x%08X",
                      tag, x, y, color, expected);
        std::cerr << buf << std::endl;
    }
}

std::unique_ptr<ui::IRender> CreateRender(ui::IRenderFactory* pRenderFactory, uint32_t w, uint32_t h)
{
    if (pRenderFactory == nullptr) {
        return nullptr;
    }
    return pRenderFactory->CreateOffscreenRender(w, h);
}

/** CefMemoryBlock 的初始状态与参数校验 */
void TestMemoryBlockBasics()
{
    Section("CefMemoryBlock basics");

    ui::CefMemoryBlock block;
    CHECK(!block.IsValid());
    CHECK_INT(block.GetWidth(), 0);
    CHECK_INT(block.GetHeight(), 0);

    //未初始化时快照应失败（不访问空指针）
    CHECK(!block.MakeImageSnapshot(nullptr));

    //有效初始化
    uint32_t src[4] = { 0xFF010203u, 0xFF111213u, 0xFF212223u, 0xFF313233u };
    CHECK(block.Init(src, {}, 2, 2));
    CHECK(block.IsValid());
    CHECK_INT(block.GetWidth(), 2);
    CHECK_INT(block.GetHeight(), 2);

    //重复初始化相同尺寸：仍然有效
    CHECK(block.Init(src, {}, 2, 2));
    CHECK(block.IsValid());
    CHECK_INT(block.GetWidth(), 2);
    CHECK_INT(block.GetHeight(), 2);

    //尺寸变化时重建缓冲区
    std::vector<uint32_t> large(16, 0xFF404040u);
    CHECK(block.Init(large.data(), {}, 4, 4));
    CHECK_INT(block.GetWidth(), 4);
    CHECK_INT(block.GetHeight(), 4);
}

/** 整图复制与脏区域增量复制 */
void TestMemoryBlockCopy(ui::IRenderFactory* pRenderFactory)
{
    Section("CefMemoryBlock copy");

    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 2, 2);
    CHECK(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }

    uint32_t src[4] = { 0xFF010203u, 0xFF111213u, 0xFF212223u, 0xFF313233u };
    ui::CefMemoryBlock block;
    CHECK(block.Init(src, {}, 2, 2));
    CHECK(block.MakeImageSnapshot(pRender.get()));
    CheckPixel(pRender.get(), 0, 0, src[0], "full copy");
    CheckPixel(pRender.get(), 1, 0, src[1], "full copy");
    CheckPixel(pRender.get(), 0, 1, src[2], "full copy");
    CheckPixel(pRender.get(), 1, 1, src[3], "full copy");

    //脏区域为 (1,1)-(2,2)：只更新右下角像素
    src[0] = 0xFFA0A0A0u;
    src[3] = 0xFF00FF00u;
    CHECK(block.Init(src, { ui::UiRect(1, 1, 2, 2) }, 2, 2));
    CHECK(block.MakeImageSnapshot(pRender.get()));
    CheckPixel(pRender.get(), 0, 0, 0xFF010203u, "dirty rect keep");
    CheckPixel(pRender.get(), 1, 0, 0xFF111213u, "dirty rect keep");
    CheckPixel(pRender.get(), 0, 1, 0xFF212223u, "dirty rect keep");
    CheckPixel(pRender.get(), 1, 1, 0xFF00FF00u, "dirty rect update");

    //整屏脏区域：等价于整图复制
    src[0] = 0xFF0000FFu;
    src[1] = 0xFF0000AAu;
    src[2] = 0xFF000055u;
    CHECK(block.Init(src, { ui::UiRect(0, 0, 2, 2) }, 2, 2));
    CHECK(block.MakeImageSnapshot(pRender.get()));
    CheckPixel(pRender.get(), 0, 0, 0xFF0000FFu, "full dirty rect");
    CheckPixel(pRender.get(), 1, 0, 0xFF0000AAu, "full dirty rect");
    CheckPixel(pRender.get(), 0, 1, 0xFF000055u, "full dirty rect");
    CheckPixel(pRender.get(), 1, 1, 0xFF00FF00u, "full dirty rect");

    //越界的脏区域视为无效，退化为整图复制
    src[0] = 0xFF123456u;
    src[1] = 0xFF234567u;
    src[2] = 0xFF345678u;
    src[3] = 0xFF456789u;
    CHECK(block.Init(src, { ui::UiRect(0, 0, 3, 1) }, 2, 2));
    CHECK(block.MakeImageSnapshot(pRender.get()));
    CheckPixel(pRender.get(), 0, 0, 0xFF123456u, "invalid dirty rect");
    CheckPixel(pRender.get(), 1, 0, 0xFF234567u, "invalid dirty rect");
    CheckPixel(pRender.get(), 0, 1, 0xFF345678u, "invalid dirty rect");
    CheckPixel(pRender.get(), 1, 1, 0xFF456789u, "invalid dirty rect");

    //空脏区域列表：整图复制
    src[0] = 0xFF0A0A0Au;
    CHECK(block.Init(src, {}, 2, 2));
    CHECK(block.MakeImageSnapshot(pRender.get()));
    CheckPixel(pRender.get(), 0, 0, 0xFF0A0A0Au, "empty dirty list");
}

/** 多个脏区域的增量复制 */
void TestMemoryBlockMultiDirty(ui::IRenderFactory* pRenderFactory)
{
    Section("CefMemoryBlock multi dirty rects");

    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 4, 4);
    CHECK(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }

    std::vector<uint32_t> src(16, 0xFF101010u);
    ui::CefMemoryBlock block;
    CHECK(block.Init(src.data(), {}, 4, 4));
    CHECK(block.MakeImageSnapshot(pRender.get()));
    CheckPixel(pRender.get(), 0, 0, 0xFF101010u, "init 4x4");

    //修改两个互不相邻的像素，只提交这两个脏区域
    src[0 * 4 + 0] = 0xFF0000FFu;
    src[3 * 4 + 3] = 0xFF00FF00u;
    CHECK(block.Init(src.data(), { ui::UiRect(0, 0, 1, 1), ui::UiRect(3, 3, 4, 4) }, 4, 4));
    CHECK(block.MakeImageSnapshot(pRender.get()));
    CheckPixel(pRender.get(), 0, 0, 0xFF0000FFu, "multi dirty");
    CheckPixel(pRender.get(), 3, 3, 0xFF00FF00u, "multi dirty");
    CheckPixel(pRender.get(), 1, 0, 0xFF101010u, "multi dirty keep");
    CheckPixel(pRender.get(), 0, 1, 0xFF101010u, "multi dirty keep");
}

/** PaintData 的目标区域与裁剪语义 */
void TestMemoryBlockPaintData(ui::IRenderFactory* pRenderFactory)
{
    Section("CefMemoryBlock PaintData");

    const uint32_t kBackground = 0xFF202020u;
    uint32_t src[4] = { 0xFF010101u, 0xFF020202u, 0xFF030303u, 0xFF040404u };
    ui::CefMemoryBlock block;
    CHECK(block.Init(src, {}, 2, 2));

    //目标区域与内存大小一致：整块写入
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 4, 4);
    CHECK(pRender != nullptr);
    if (pRender != nullptr) {
        pRender->Clear(ui::UiColor(kBackground));
        block.PaintData(pRender.get(), ui::UiRect(1, 1, 3, 3));
        CheckPixel(pRender.get(), 1, 1, src[0], "PaintData full");
        CheckPixel(pRender.get(), 2, 1, src[1], "PaintData full");
        CheckPixel(pRender.get(), 1, 2, src[2], "PaintData full");
        CheckPixel(pRender.get(), 2, 2, src[3], "PaintData full");
        CheckPixel(pRender.get(), 0, 0, kBackground, "PaintData outside");
        CheckPixel(pRender.get(), 3, 3, kBackground, "PaintData outside");

        //空目标区域：不绘制
        block.PaintData(pRender.get(), ui::UiRect(0, 0, 0, 0));
        CheckPixel(pRender.get(), 1, 1, src[0], "PaintData empty");
    }

    //目标区域小于内存大小：仅写入裁剪区域（1x1）
    std::unique_ptr<ui::IRender> pClipRender = CreateRender(pRenderFactory, 4, 4);
    CHECK(pClipRender != nullptr);
    if (pClipRender != nullptr) {
        pClipRender->Clear(ui::UiColor(kBackground));
        block.PaintData(pClipRender.get(), ui::UiRect(1, 1, 2, 2));
        CheckPixel(pClipRender.get(), 1, 1, src[0], "PaintData clip");
        CheckPixel(pClipRender.get(), 2, 1, kBackground, "PaintData clip");
        CheckPixel(pClipRender.get(), 1, 2, kBackground, "PaintData clip");
        CheckPixel(pClipRender.get(), 2, 2, kBackground, "PaintData clip");
    }
}

/** MakeImageSnapshot 的尺寸调整与参数校验 */
void TestMemoryBlockSnapshot(ui::IRenderFactory* pRenderFactory)
{
    Section("CefMemoryBlock MakeImageSnapshot");

    uint32_t src[6] = { 0xFF010101u, 0xFF020202u, 0xFF030303u,
                        0xFF040404u, 0xFF050505u, 0xFF060606u };
    ui::CefMemoryBlock block;
    CHECK(block.Init(src, {}, 3, 2));

    //渲染对象初始尺寸与内存不同，快照后应调整为内存尺寸
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 1, 1);
    CHECK(pRender != nullptr);
    if (pRender != nullptr) {
        CHECK(block.MakeImageSnapshot(pRender.get()));
        CHECK_INT(pRender->GetWidth(), 3);
        CHECK_INT(pRender->GetHeight(), 2);
        CheckPixel(pRender.get(), 0, 0, src[0], "snapshot");
        CheckPixel(pRender.get(), 2, 0, src[2], "snapshot");
        CheckPixel(pRender.get(), 0, 1, src[3], "snapshot");
        CheckPixel(pRender.get(), 2, 1, src[5], "snapshot");
    }
}

/** CefUnregistedCallbackList 的回调注册、注销与批量清空 */
void TestUnregisterCallbackList()
{
    Section("CefUnregistedCallbackList");

    typedef std::function<void(int)> TestCallback;
    ui::CefUnregistedCallbackList<TestCallback> callbackList;

    int32_t nSum = 0;
    ui::CefUnregisterCallback cb1 = callbackList.AddCallback(
        TestCallback([&nSum](int32_t value) { nSum += value; }));
    ui::CefUnregisterCallback cb2 = callbackList.AddCallback(
        TestCallback([&nSum](int32_t value) { nSum += value * 10; }));

    callbackList(1);
    CHECK_INT(nSum, 11);

    //单独注销第一个回调
    CHECK(cb1 != nullptr);
    if (cb1 != nullptr) {
        cb1();
    }
    callbackList(1);
    CHECK_INT(nSum, 21);

    //重复注销同一个回调不应影响其它回调
    if (cb1 != nullptr) {
        cb1();
    }
    callbackList(1);
    CHECK_INT(nSum, 31);

    //清空后不再调用任何回调
    callbackList.Clear();
    callbackList(1);
    CHECK_INT(nSum, 31);

    //列表销毁后，之前返回的注销回调必须安全（弱引用保护）
    ui::CefUnregisterCallback cbAfterDestroy;
    {
        ui::CefUnregistedCallbackList<TestCallback> tempList;
        cbAfterDestroy = tempList.AddCallback(TestCallback([&nSum](int32_t value) { nSum += value; }));
        tempList(5);
        CHECK_INT(nSum, 36);
    }
    CHECK(cbAfterDestroy != nullptr);
    if (cbAfterDestroy != nullptr) {
        //此时列表已销毁，调用注销回调不应崩溃，也不应改变状态
        cbAfterDestroy();
    }
    CHECK_INT(nSum, 36);
}

/** CefAutoUnregister 的析构自动注销 */
void TestAutoUnregister()
{
    Section("CefAutoUnregister");

    int32_t nCount = 0;
    {
        ui::CefAutoUnregister autoUnregister;
        autoUnregister.Add(ui::CefUnregisterCallback([&nCount]() { ++nCount; }));
        autoUnregister.Add(ui::CefUnregisterCallback([&nCount]() { nCount += 2; }));
        CHECK_INT(nCount, 0);
    }
    CHECK_INT(nCount, 3);

    //空的自动注销对象析构不应产生副作用
    {
        ui::CefAutoUnregister emptyUnregister;
    }
    CHECK_INT(nCount, 3);
}

} //namespace

int main()
{
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif

    //关闭动画，避免启动资源时的额外线程/定时器影响测试
    ui::GlobalManager::Instance().SetAnimationEnabled(false);
    const bool bStartup = ui::GlobalManager::Instance().Startup(
        ui::LocalFilesResParam(ui::FilePath(DString(_T(DUILIB_TEST_RESOURCE_DIR)))));
    CHECK(bStartup);

    ui::IRenderFactory* pRenderFactory = ui::GlobalManager::Instance().GetRenderFactory();
    CHECK(pRenderFactory != nullptr);

    TestMemoryBlockBasics();
    if (pRenderFactory != nullptr) {
        TestMemoryBlockCopy(pRenderFactory);
        TestMemoryBlockMultiDirty(pRenderFactory);
        TestMemoryBlockPaintData(pRenderFactory);
        TestMemoryBlockSnapshot(pRenderFactory);
    }
    TestUnregisterCallbackList();
    TestAutoUnregister();

    ui::GlobalManager::Instance().Shutdown();

    if (g_failedCount == 0) {
        std::cerr << "All checks passed: " << g_checkCount << std::endl;
        return 0;
    }
    std::cerr << "Failed checks: " << g_failedCount << " / " << g_checkCount << std::endl;
    return (g_failedCount > 0) ? 1 : 0;
}

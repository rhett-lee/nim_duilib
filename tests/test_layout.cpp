// Layout 目录的单元测试
//
// 测试对象：duilib/Layout 下各布局管理器的纯布局计算逻辑，不创建窗口、不启动消息循环。
// 通过给 Box 容器设置固定大小的子控件后调用 SetPos，直接检查子控件最终的位置和大小。
//
// 覆盖内容：
//  1. Layout 基类：child_margin/child_align 等属性解析、取值钳制、DPI 变化（同比例）无副作用、
//     布局类型与 LayoutByActualAreaSize 判断；
//  2. HLayout：固定+拉伸控件分配、child_margin 间距、水平/垂直对齐、auto 尺寸估算，
//     以及"宽度为 0 的固定控件不占边距"（规则 9）的回归用例；
//  3. VLayout：固定+拉伸控件分配、child_margin 间距、垂直对齐、auto 尺寸估算，
//     以及"高度为 0 的固定控件不占边距"的回归用例；
//  4. GridLayout：固定行列时的等分单元格与子控件位置；
//  5. HTileLayout / VTileLayout：按容器高度/宽度自动计算行列数后的排列位置；
//  6. HFlowLayout / VFlowLayout：自动换行/换列，以及 justify_content 的空间分布。

#if defined(_WIN32)
#include <crtdbg.h>
#endif

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/UiFixedInt.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"
#include "duilib/Box/GridBox.h"
#include "duilib/Box/TileBox.h"
#include "duilib/Layout/HLayout.h"
#include "duilib/Layout/VLayout.h"
#include "duilib/Layout/GridLayout.h"
#include "duilib/Layout/HTileLayout.h"
#include "duilib/Layout/VTileLayout.h"
#include "duilib/Layout/HFlowLayout.h"
#include "duilib/Layout/VFlowLayout.h"

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

void Check(bool condition, const char* message)
{
    Check(condition, [message]() { return std::string(message); });
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })

/** 创建指定大小、指定外边距的普通子控件（固定宽高） */
ui::Control* MakeChild(int32_t cx, int32_t cy, int32_t nMargin = 0)
{
    ui::HBox* pChild = new ui::HBox(nullptr);
    pChild->SetFixedWidth(ui::UiFixedInt::MakeInt(cx), false, false);
    pChild->SetFixedHeight(ui::UiFixedInt::MakeInt(cy), false, false);
    if (nMargin != 0) {
        pChild->SetMargin(ui::UiMargin(nMargin, nMargin, nMargin, nMargin), false);
    }
    return pChild;
}

/** 创建宽度 100% 拉伸、固定高度的子控件 */
ui::Control* MakeStretchWidthChild(int32_t cy, int32_t nMargin = 0)
{
    ui::HBox* pChild = new ui::HBox(nullptr);
    pChild->SetFixedWidth(ui::UiFixedInt::MakeStretch(100), false, false);
    pChild->SetFixedHeight(ui::UiFixedInt::MakeInt(cy), false, false);
    if (nMargin != 0) {
        pChild->SetMargin(ui::UiMargin(nMargin, nMargin, nMargin, nMargin), false);
    }
    return pChild;
}

/** 创建高度 100% 拉伸、固定宽度的子控件 */
ui::Control* MakeStretchHeightChild(int32_t cx, int32_t nMargin = 0)
{
    ui::HBox* pChild = new ui::HBox(nullptr);
    pChild->SetFixedWidth(ui::UiFixedInt::MakeInt(cx), false, false);
    pChild->SetFixedHeight(ui::UiFixedInt::MakeStretch(100), false, false);
    if (nMargin != 0) {
        pChild->SetMargin(ui::UiMargin(nMargin, nMargin, nMargin, nMargin), false);
    }
    return pChild;
}

bool RectEquals(const ui::UiRect& rc, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
    const bool bEquals = (rc.left == left) && (rc.top == top) && (rc.right == right) && (rc.bottom == bottom);
    if (!bEquals) {
        std::cerr << "[rect] expected(" << left << "," << top << "," << right << "," << bottom
                  << ") actual(" << rc.left << "," << rc.top << "," << rc.right << "," << rc.bottom << ")"
                  << std::endl;
    }
    return bEquals;
}

std::vector<ui::Control*> GetItems(const ui::Box& box)
{
    std::vector<ui::Control*> items;
    const size_t itemCount = box.GetItemCount();
    for (size_t i = 0; i < itemCount; ++i) {
        items.push_back(box.GetItemAt(i));
    }
    return items;
}

/** Layout 基类：属性解析、取值钳制、布局类型判断 */
void TestLayoutAttributes()
{
    ui::HBox box(nullptr);
    ui::Layout* pLayout = box.GetLayout();
    CHECK(pLayout != nullptr);
    CHECK(pLayout->GetLayoutType() == ui::LayoutType::HLayout);
    CHECK(pLayout->LayoutByActualAreaSize());

    const ui::DpiManager& dpi = ui::GlobalManager::Instance().Dpi();

    // 负值钳制为 0
    pLayout->SetChildMargin(-5);
    CHECK(pLayout->GetChildMarginX() == 0);
    CHECK(pLayout->GetChildMarginY() == 0);

    pLayout->SetChildMargin(10);
    CHECK(pLayout->GetChildMarginX() == 10);
    CHECK(pLayout->GetChildMarginY() == 10);

    pLayout->SetChildMarginX(6);
    pLayout->SetChildMarginY(8);
    CHECK(pLayout->GetChildMarginX() == 6);
    CHECK(pLayout->GetChildMarginY() == 8);

    // 属性解析：child_margin / child_halign / child_valign / child_align
    CHECK(pLayout->SetAttribute(_T("child_margin"), _T("12"), dpi));
    const int32_t nScaledMargin = dpi.GetScaleInt(12);
    CHECK(pLayout->GetChildMarginX() == nScaledMargin);
    CHECK(pLayout->GetChildMarginY() == nScaledMargin);

    CHECK(pLayout->SetAttribute(_T("child_halign"), _T("center"), dpi));
    CHECK(pLayout->GetChildHAlignType() == ui::HorAlignType::kAlignCenter);
    CHECK(pLayout->SetAttribute(_T("child_valign"), _T("bottom"), dpi));
    CHECK(pLayout->GetChildVAlignType() == ui::VerAlignType::kAlignBottom);

    CHECK(pLayout->SetAttribute(_T("child_align"), _T("hcenter,vcenter"), dpi));
    CHECK(pLayout->GetChildHAlignType() == ui::HorAlignType::kAlignCenter);
    CHECK(pLayout->GetChildVAlignType() == ui::VerAlignType::kAlignCenter);

    // 不存在的属性返回 false
    CHECK(!pLayout->SetAttribute(_T("no_such_attribute"), _T("1"), dpi));

    // DPI 比例不变时，边距保持不变
    const uint32_t nDpiScale = dpi.GetDisplayScaleFactor();
    pLayout->ChangeDpiScale(dpi, nDpiScale);
    CHECK(pLayout->GetChildMarginX() == nScaledMargin);
    CHECK(pLayout->GetChildMarginY() == nScaledMargin);
}

/** HLayout：固定/拉伸分配、间距、对齐、auto 尺寸估算、零宽控件回归 */
void TestHLayout()
{
    {
        ui::HBox box(nullptr);
        ui::Control* pFixed = MakeChild(100, 50);
        ui::Control* pStretch = MakeStretchWidthChild(50);
        CHECK(box.AddItem(pFixed));
        CHECK(box.AddItem(pStretch));

        box.SetPos(ui::UiRect(0, 0, 300, 100));
        CHECK(RectEquals(pFixed->GetRect(), 0, 0, 100, 50));
        CHECK(RectEquals(pStretch->GetRect(), 100, 0, 300, 50));

        // auto 尺寸估算：拉伸控件不计入宽度
        ui::UiSize64 est = box.GetLayout()->EstimateLayoutSize(GetItems(box), ui::UiSize(300, 100));
        CHECK(est.cx == 100);
        CHECK(est.cy == 50);
    }

    {
        // child_margin 作为子控件之间的间距
        ui::HBox box(nullptr);
        box.GetLayout()->SetChildMarginX(10);
        ui::Control* pFirst = MakeChild(50, 20);
        ui::Control* pSecond = MakeChild(50, 20);
        box.AddItem(pFirst);
        box.AddItem(pSecond);
        box.SetPos(ui::UiRect(0, 0, 300, 100));
        CHECK(RectEquals(pFirst->GetRect(), 0, 0, 50, 20));
        CHECK(RectEquals(pSecond->GetRect(), 60, 0, 110, 20));
    }

    {
        // 水平居中：HLayout 使用布局自身的 child_halign 决定子控件整体的水平位置；
        // 垂直居中：HLayout 使用子控件自身的 ver_align，因此需要设置到子控件上
        ui::HBox box(nullptr);
        box.GetLayout()->SetChildHAlignType(ui::HorAlignType::kAlignCenter);
        ui::Control* pChild = MakeChild(50, 20);
        pChild->SetVerAlignType(ui::VerAlignType::kAlignCenter);
        box.AddItem(pChild);
        box.SetPos(ui::UiRect(0, 0, 300, 100));
        CHECK(RectEquals(pChild->GetRect(), 125, 40, 175, 60));
    }

    {
        // 回归：宽度为 0 的固定控件被忽略，其外边距不能挤占拉伸控件的空间
        ui::HBox box(nullptr);
        ui::Control* pZero = MakeChild(0, 50, 10);
        ui::Control* pStretch = MakeStretchWidthChild(50);
        box.AddItem(pZero);
        box.AddItem(pStretch);
        box.SetPos(ui::UiRect(0, 0, 300, 100));
        CHECK(pZero->GetRect().Width() == 0);
        CHECK(RectEquals(pStretch->GetRect(), 0, 0, 300, 50));
    }
}

/** VLayout：固定/拉伸分配、间距、对齐、auto 尺寸估算、零高控件回归 */
void TestVLayout()
{
    {
        ui::VBox box(nullptr);
        ui::Control* pFixed = MakeChild(50, 100);
        ui::Control* pStretch = MakeStretchHeightChild(50);
        CHECK(box.AddItem(pFixed));
        CHECK(box.AddItem(pStretch));

        box.SetPos(ui::UiRect(0, 0, 100, 300));
        CHECK(RectEquals(pFixed->GetRect(), 0, 0, 50, 100));
        CHECK(RectEquals(pStretch->GetRect(), 0, 100, 50, 300));

        ui::UiSize64 est = box.GetLayout()->EstimateLayoutSize(GetItems(box), ui::UiSize(100, 300));
        CHECK(est.cx == 50);
        CHECK(est.cy == 100);
    }

    {
        ui::VBox box(nullptr);
        box.GetLayout()->SetChildMarginY(10);
        ui::Control* pFirst = MakeChild(20, 50);
        ui::Control* pSecond = MakeChild(20, 50);
        box.AddItem(pFirst);
        box.AddItem(pSecond);
        box.SetPos(ui::UiRect(0, 0, 100, 300));
        CHECK(RectEquals(pFirst->GetRect(), 0, 0, 20, 50));
        CHECK(RectEquals(pSecond->GetRect(), 0, 60, 20, 110));
    }

    {
        // 垂直居中：VLayout 使用布局自身的 child_valign 决定子控件整体的垂直位置；
        // 水平居中：VLayout 使用子控件自身的 hor_align，因此需要设置到子控件上
        ui::VBox box(nullptr);
        box.GetLayout()->SetChildVAlignType(ui::VerAlignType::kAlignCenter);
        ui::Control* pChild = MakeChild(20, 50);
        pChild->SetHorAlignType(ui::HorAlignType::kAlignCenter);
        box.AddItem(pChild);
        box.SetPos(ui::UiRect(0, 0, 100, 300));
        CHECK(RectEquals(pChild->GetRect(), 40, 125, 60, 175));
    }

    {
        // 回归：高度为 0 的固定控件被忽略，其外边距不能挤占拉伸控件的空间
        ui::VBox box(nullptr);
        ui::Control* pZero = MakeChild(50, 0, 10);
        ui::Control* pStretch = MakeStretchHeightChild(50);
        box.AddItem(pZero);
        box.AddItem(pStretch);
        box.SetPos(ui::UiRect(0, 0, 100, 300));
        CHECK(pZero->GetRect().Height() == 0);
        CHECK(RectEquals(pStretch->GetRect(), 0, 0, 50, 300));
    }
}

/** GridLayout：固定行列时的等分单元格与子控件位置 */
void TestGridLayout()
{
    ui::GridBox box(nullptr);
    ui::GridLayout* pLayout = dynamic_cast<ui::GridLayout*>(box.GetLayout());
    CHECK(pLayout != nullptr);
    CHECK(pLayout->GetLayoutType() == ui::LayoutType::GridLayout);
    CHECK(pLayout->LayoutByActualAreaSize());

    pLayout->SetRows(2);
    pLayout->SetColumns(2);
    CHECK(pLayout->GetRows() == 2);
    CHECK(pLayout->GetColumns() == 2);

    ui::Control* p0 = MakeChild(50, 50);
    ui::Control* p1 = MakeChild(50, 50);
    ui::Control* p2 = MakeChild(50, 50);
    ui::Control* p3 = MakeChild(50, 50);
    box.AddItem(p0);
    box.AddItem(p1);
    box.AddItem(p2);
    box.AddItem(p3);

    box.SetPos(ui::UiRect(0, 0, 200, 200));
    // 每个单元格 100x100：默认 scale_down=false 时子控件填满单元格
    CHECK(RectEquals(p0->GetRect(), 0, 0, 100, 100));
    CHECK(RectEquals(p1->GetRect(), 100, 0, 200, 100));
    CHECK(RectEquals(p2->GetRect(), 0, 100, 100, 200));
    CHECK(RectEquals(p3->GetRect(), 100, 100, 200, 200));
}

/** HTileLayout / VTileLayout：按容器尺寸自动计算行列数 */
void TestTileLayout()
{
    {
        // 横向瓦片：容器高度 100、子项高度 20 => 每列 5 行，4 个子项排在第 1 列；
        // 默认垂直居中对齐会产生偏移，这里固定为靠上对齐，便于断言位置
        ui::HTileBox box(nullptr);
        box.GetLayout()->SetChildVAlignType(ui::VerAlignType::kAlignTop);
        ui::Control* p0 = MakeChild(20, 20);
        ui::Control* p1 = MakeChild(20, 20);
        ui::Control* p2 = MakeChild(20, 20);
        ui::Control* p3 = MakeChild(20, 20);
        box.AddItem(p0);
        box.AddItem(p1);
        box.AddItem(p2);
        box.AddItem(p3);
        box.SetPos(ui::UiRect(0, 0, 100, 100));
        CHECK(RectEquals(p0->GetRect(), 0, 0, 20, 20));
        CHECK(RectEquals(p1->GetRect(), 0, 20, 20, 40));
        CHECK(RectEquals(p2->GetRect(), 0, 40, 20, 60));
        CHECK(RectEquals(p3->GetRect(), 0, 60, 20, 80));
    }

    {
        // 纵向瓦片：容器宽度 100、子项宽度 20 => 每行 5 列，4 个子项排在第 1 行；
        // 默认水平居中对齐会产生偏移，这里固定为靠左对齐，便于断言位置
        ui::VTileBox box(nullptr);
        box.GetLayout()->SetChildHAlignType(ui::HorAlignType::kAlignLeft);
        ui::Control* p0 = MakeChild(20, 20);
        ui::Control* p1 = MakeChild(20, 20);
        ui::Control* p2 = MakeChild(20, 20);
        ui::Control* p3 = MakeChild(20, 20);
        box.AddItem(p0);
        box.AddItem(p1);
        box.AddItem(p2);
        box.AddItem(p3);
        box.SetPos(ui::UiRect(0, 0, 100, 100));
        CHECK(RectEquals(p0->GetRect(), 0, 0, 20, 20));
        CHECK(RectEquals(p1->GetRect(), 20, 0, 40, 20));
        CHECK(RectEquals(p2->GetRect(), 40, 0, 60, 20));
        CHECK(RectEquals(p3->GetRect(), 60, 0, 80, 20));
    }
}

/** HFlowLayout / VFlowLayout：自动换行/换列与 justify_content 分布 */
void TestFlowLayout()
{
    {
        // 横向流式：每个子项宽 60，容器宽 100 => 每行只放得下一个
        ui::HFlowBox box(nullptr);
        ui::HFlowLayout* pLayout = dynamic_cast<ui::HFlowLayout*>(box.GetLayout());
        CHECK(pLayout != nullptr);
        CHECK(pLayout->GetLayoutType() == ui::LayoutType::HFlowLayout);
        CHECK(pLayout->LayoutByActualAreaSize());

        ui::Control* p0 = MakeChild(60, 20);
        ui::Control* p1 = MakeChild(60, 20);
        ui::Control* p2 = MakeChild(60, 20);
        box.AddItem(p0);
        box.AddItem(p1);
        box.AddItem(p2);
        box.SetPos(ui::UiRect(0, 0, 100, 100));
        CHECK(RectEquals(p0->GetRect(), 0, 0, 60, 20));
        CHECK(RectEquals(p1->GetRect(), 0, 20, 60, 40));
        CHECK(RectEquals(p2->GetRect(), 0, 40, 60, 60));
    }

    {
        // 主轴 distribution：space_between 把剩余空间平均分配到相邻子项之间
        ui::HFlowBox box(nullptr);
        ui::HFlowLayout* pLayout = dynamic_cast<ui::HFlowLayout*>(box.GetLayout());
        pLayout->SetJustifyContent(ui::HFlowLayout::JustifyContent::kSpaceBetween);
        CHECK(pLayout->GetJustifyContent() == ui::HFlowLayout::JustifyContent::kSpaceBetween);

        ui::Control* p0 = MakeChild(20, 20);
        ui::Control* p1 = MakeChild(20, 20);
        box.AddItem(p0);
        box.AddItem(p1);
        box.SetPos(ui::UiRect(0, 0, 100, 100));
        CHECK(RectEquals(p0->GetRect(), 0, 0, 20, 20));
        CHECK(RectEquals(p1->GetRect(), 80, 0, 100, 20));
    }

    {
        // 纵向流式：每个子项高 60，容器高 100 => 每列只放得下一个
        ui::VFlowBox box(nullptr);
        ui::VFlowLayout* pLayout = dynamic_cast<ui::VFlowLayout*>(box.GetLayout());
        CHECK(pLayout != nullptr);
        CHECK(pLayout->GetLayoutType() == ui::LayoutType::VFlowLayout);
        CHECK(pLayout->LayoutByActualAreaSize());

        ui::Control* p0 = MakeChild(20, 60);
        ui::Control* p1 = MakeChild(20, 60);
        ui::Control* p2 = MakeChild(20, 60);
        box.AddItem(p0);
        box.AddItem(p1);
        box.AddItem(p2);
        box.SetPos(ui::UiRect(0, 0, 100, 100));
        CHECK(RectEquals(p0->GetRect(), 0, 0, 20, 60));
        CHECK(RectEquals(p1->GetRect(), 20, 0, 40, 60));
        CHECK(RectEquals(p2->GetRect(), 40, 0, 60, 60));
    }

    {
        ui::VFlowBox box(nullptr);
        ui::VFlowLayout* pLayout = dynamic_cast<ui::VFlowLayout*>(box.GetLayout());
        pLayout->SetJustifyContent(ui::VFlowLayout::JustifyContent::kSpaceBetween);
        CHECK(pLayout->GetJustifyContent() == ui::VFlowLayout::JustifyContent::kSpaceBetween);

        ui::Control* p0 = MakeChild(20, 20);
        ui::Control* p1 = MakeChild(20, 20);
        box.AddItem(p0);
        box.AddItem(p1);
        box.SetPos(ui::UiRect(0, 0, 100, 100));
        CHECK(RectEquals(p0->GetRect(), 0, 0, 20, 20));
        CHECK(RectEquals(p1->GetRect(), 0, 80, 20, 100));
    }
}

}// namespace

int main()
{
#if defined(_WIN32)
    // Debug 下控件内部断言不应弹框阻塞命令行测试
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif
    // 测试不创建窗口，关闭全局动画避免控件创建/布局时启动定时器
    ui::GlobalManager::Instance().SetAnimationEnabled(false);

    TestLayoutAttributes();
    TestHLayout();
    TestVLayout();
    TestGridLayout();
    TestTileLayout();
    TestFlowLayout();

    if (g_failedCount == 0) {
        std::cout << "[PASS] layout: " << g_checkCount << " checks" << std::endl;
        return 0;
    }
    std::cerr << "[FAILED] layout: " << g_failedCount << " of " << g_checkCount
              << " checks failed" << std::endl;
    return 1;
}

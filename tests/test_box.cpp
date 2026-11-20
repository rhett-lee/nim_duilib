// Box 目录的单元测试
//
// 测试对象：duilib/Box 下的容器与列表逻辑（Box 基类、HBox/VBox/GridBox/TileBox、
// ListBox/ListBoxItem、TabBox、Panel），不创建窗口、不启动消息循环，因此可以在命令行
// （含 CI）直接运行。
//
// 覆盖内容：
//  1. Box 基类：子项增加/插入/删除/清空、索引查询、SetItemIndex 重排、边界返回值；
//  2. FindSelectable：跳过不可见/不可用/不可选择的子项，前向与后向查找；
//  3. 布局容器：HBox/VBox/GridBox/TileBox 的布局类型与 GridBox 行列属性；
//  4. ListBox：选择状态、插入/删除时的当前选择索引与 Shift 锚点维护、
//     自动选择下一项、多选切换单选、子项索引同步、框选配置项；
//  5. TabBox：页签选择、插入/删除时的选择索引维护，以及"无选择项时删除子项"
//     不会把 InvalidIndex 下溢成有效索引（回归用例）；
//  6. Panel：标题/可折叠状态、折叠布局对 min_height 的临时放宽与恢复、属性读写。

#if defined(_WIN32)
#include <crtdbg.h>
#endif

#include "duilib/Core/GlobalManager.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"
#include "duilib/Box/GridBox.h"
#include "duilib/Box/TileBox.h"
#include "duilib/Box/ListBox.h"
#include "duilib/Box/ListBoxItem.h"
#include "duilib/Box/TabBox.h"
#include "duilib/Box/Panel.h"
#include "duilib/Layout/HLayout.h"
#include "duilib/Layout/VLayout.h"
#include "duilib/Layout/GridLayout.h"
#include "duilib/Layout/HTileLayout.h"
#include "duilib/Layout/VTileLayout.h"

#include <functional>
#include <iostream>
#include <string>

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

/** 直接使用常量字符串作为失败信息 */
void Check(bool condition, const char* message)
{
    Check(condition, [message]() { return std::string(message); });
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })

/** 暴露 ListBox 的 protected 查询接口，便于验证内部状态 */
class TestableListBox : public ui::VListBox
{
public:
    explicit TestableListBox(ui::Window* pWindow) : ui::VListBox(pWindow) {}

    size_t LastNoShiftItem() const { return GetLastNoShiftItem(); }
    bool ItemSelected(size_t iIndex) const { return IsItemSelected(iIndex); }
};

/** Box 基类：子项的增加/插入/删除/清空、索引查询与重排 */
void TestBoxContainerBasics()
{
    ui::HBox box(nullptr);
    CHECK(box.GetItemCount() == 0);
    CHECK(box.GetItemAt(0) == nullptr);
    CHECK(box.GetItemIndex(nullptr) == ui::Box::InvalidIndex);
    CHECK(box.GetType() == DUI_CTR_HBOX);
    CHECK(box.FindSelectable(0, true) == ui::Box::InvalidIndex);

    ui::ListBoxItem* pA = new ui::ListBoxItem(nullptr);
    ui::ListBoxItem* pB = new ui::ListBoxItem(nullptr);
    ui::ListBoxItem* pC = new ui::ListBoxItem(nullptr);

    CHECK(box.AddItem(pA));
    CHECK(box.AddItem(pB));
    CHECK(box.GetItemCount() == 2);
    CHECK(box.GetItemAt(0) == pA);
    CHECK(box.GetItemAt(1) == pB);
    CHECK(box.GetItemIndex(pA) == 0);
    CHECK(box.GetItemIndex(pB) == 1);

    // 在中间插入
    CHECK(box.AddItemAt(pC, 1));
    CHECK(box.GetItemCount() == 3);
    CHECK(box.GetItemAt(0) == pA);
    CHECK(box.GetItemAt(1) == pC);
    CHECK(box.GetItemAt(2) == pB);

    // 重排：把 pC 移到最前面
    CHECK(box.SetItemIndex(pC, 0));
    CHECK(box.GetItemAt(0) == pC);
    CHECK(box.GetItemIndex(pA) == 1);
    CHECK(box.GetItemIndex(pB) == 2);
    // 索引未变化时返回 true，越界返回 false
    CHECK(box.SetItemIndex(pC, 0));
    CHECK(!box.SetItemIndex(pC, 5));

    // 按指针删除（m_bAutoDestroyChild 默认为 true，删除后由容器销毁）
    CHECK(box.RemoveItem(pA));
    CHECK(box.GetItemCount() == 2);
    CHECK(box.GetItemAt(0) == pC);
    CHECK(box.GetItemAt(1) == pB);

    // 按索引删除与边界
    CHECK(box.RemoveItemAt(0));
    CHECK(box.GetItemCount() == 1);
    CHECK(box.GetItemAt(0) == pB);
    CHECK(!box.RemoveItemAt(5));

    box.RemoveAllItems();
    CHECK(box.GetItemCount() == 0);
    CHECK(box.GetItemAt(0) == nullptr);
}

/** FindSelectable：跳过不可见、不可用、不可选择的子项 */
void TestFindSelectable()
{
    ui::VBox box(nullptr);
    ui::ListBoxItem* p0 = new ui::ListBoxItem(nullptr);
    ui::ListBoxItem* p1 = new ui::ListBoxItem(nullptr);
    ui::ListBoxItem* p2 = new ui::ListBoxItem(nullptr);
    box.AddItem(p0);
    box.AddItem(p1);
    box.AddItem(p2);

    // 全部可选
    CHECK(box.FindSelectable(0, true) == 0);
    CHECK(box.FindSelectable(ui::Box::InvalidIndex, true) == 0);

    // 只保留中间项可选
    p0->SetVisible(false);
    p2->SetVisible(false);
    CHECK(box.FindSelectable(0, true) == 1);
    CHECK(box.FindSelectable(1, true) == 1);
    CHECK(box.FindSelectable(2, true) == ui::Box::InvalidIndex);
    // 后向查找：从 2 开始跳过不可见的 p2，命中 p1
    CHECK(box.FindSelectable(2, false) == 1);

    // 中间项也不可用时，前向与后向都找不到
    p1->SetEnabled(false);
    CHECK(box.FindSelectable(0, true) == ui::Box::InvalidIndex);
    CHECK(box.FindSelectable(2, false) == ui::Box::InvalidIndex);
}

/** 布局容器：HBox/VBox/GridBox/TileBox 的布局类型与 GridBox 行列属性 */
void TestLayoutBoxes()
{
    ui::HBox hbox(nullptr);
    ui::VBox vbox(nullptr);
    ui::GridBox gridBox(nullptr);
    ui::VTileBox vTileBox(nullptr);
    ui::HTileBox hTileBox(nullptr);

    CHECK(hbox.GetType() == DUI_CTR_HBOX);
    CHECK(vbox.GetType() == DUI_CTR_VBOX);
    CHECK(gridBox.GetType() == DUI_CTR_GRIDBOX);
    CHECK(vTileBox.GetType() == DUI_CTR_VTILE_BOX);
    CHECK(hTileBox.GetType() == DUI_CTR_HTILE_BOX);

    CHECK(dynamic_cast<ui::HLayout*>(hbox.GetLayout()) != nullptr);
    CHECK(dynamic_cast<ui::VLayout*>(vbox.GetLayout()) != nullptr);
    CHECK(dynamic_cast<ui::GridLayout*>(gridBox.GetLayout()) != nullptr);
    CHECK(dynamic_cast<ui::VTileLayout*>(vTileBox.GetLayout()) != nullptr);
    CHECK(dynamic_cast<ui::HTileLayout*>(hTileBox.GetLayout()) != nullptr);

    // GridBox 默认行列数均为 0（由布局自动计算）
    CHECK(gridBox.GetRows() == 0);
    CHECK(gridBox.GetColumns() == 0);
    gridBox.SetRows(2);
    gridBox.SetColumns(3);
    CHECK(gridBox.GetRows() == 2);
    CHECK(gridBox.GetColumns() == 3);
    gridBox.SetRows(0);
    gridBox.SetColumns(0);
    CHECK(gridBox.GetRows() == 0);
    CHECK(gridBox.GetColumns() == 0);
}

/** ListBox：选择状态与插入/删除时的索引维护 */
void TestListBoxSelection()
{
    TestableListBox list(nullptr);
    CHECK(list.GetItemCount() == 0);
    CHECK(list.GetCurSel() == ui::Box::InvalidIndex);
    CHECK(!list.IsMultiSelect());

    ui::ListBoxItem* p0 = new ui::ListBoxItem(nullptr);
    ui::ListBoxItem* p1 = new ui::ListBoxItem(nullptr);
    ui::ListBoxItem* p2 = new ui::ListBoxItem(nullptr);
    CHECK(list.AddItem(p0));
    CHECK(list.AddItem(p1));
    CHECK(list.AddItem(p2));
    CHECK(list.GetItemCount() == 3);
    CHECK(p0->GetListBoxIndex() == 0);
    CHECK(p2->GetListBoxIndex() == 2);
    CHECK(p0->GetOwner() == &list);

    // 单选：只保留一个选择项
    CHECK(list.SelectItem(1, false, false));
    CHECK(list.GetCurSel() == 1);
    CHECK(p1->IsSelected());
    CHECK(!p0->IsSelected());

    // 在最前面插入：原选择项索引后移，Shift 锚点同步后移
    ui::ListBoxItem* pNew = new ui::ListBoxItem(nullptr);
    CHECK(list.AddItemAt(pNew, 0));
    CHECK(list.GetItemCount() == 4);
    CHECK(pNew->GetListBoxIndex() == 0);
    CHECK(p0->GetListBoxIndex() == 1);
    CHECK(p2->GetListBoxIndex() == 3);
    CHECK(list.GetCurSel() == 2);
    CHECK(p1->IsSelected());
    CHECK(list.LastNoShiftItem() == 1);
    // 删除最前面的一项：选择项索引前移，Shift 锚点回到 0
    CHECK(list.RemoveItemAt(0));
    CHECK(list.GetItemCount() == 3);
    CHECK(list.GetCurSel() == 1);
    CHECK(list.LastNoShiftItem() == 0);
    CHECK(p1->IsSelected());

    // 删除当前选择项（未开启自动选择下一项）：当前选择变为无效
    CHECK(list.RemoveItemAt(1));
    CHECK(list.GetItemCount() == 2);
    CHECK(list.GetCurSel() == ui::Box::InvalidIndex);
    CHECK(list.GetItemAt(0) == p0);
    CHECK(list.GetItemAt(1) == p2);

    // SetItemIndex：重排后子项索引同步更新
    CHECK(list.SetItemIndex(p2, 0));
    CHECK(list.GetItemAt(0) == p2);
    CHECK(p2->GetListBoxIndex() == 0);
    CHECK(p0->GetListBoxIndex() == 1);
    CHECK(!list.SetItemIndex(p2, 5));

    // 边界：无效索引删除返回 false，越界访问返回 nullptr
    CHECK(!list.RemoveItemAt(ui::Box::InvalidIndex));
    CHECK(!list.RemoveItemAt(10));
    CHECK(list.GetItemAt(10) == nullptr);

    list.RemoveAllItems();
    CHECK(list.GetItemCount() == 0);
    CHECK(list.GetCurSel() == ui::Box::InvalidIndex);
    CHECK(list.LastNoShiftItem() == 0);
}

/** ListBox：删除当前选择项时自动选择同位置的下一个可选项 */
void TestListBoxSelectNextWhenRemoved()
{
    ui::VListBox list(nullptr);
    list.SetSelectNextWhenActiveRemoved(true);
    CHECK(list.IsSelectNextWhenActiveRemoved());

    ui::ListBoxItem* p0 = new ui::ListBoxItem(nullptr);
    ui::ListBoxItem* p1 = new ui::ListBoxItem(nullptr);
    ui::ListBoxItem* p2 = new ui::ListBoxItem(nullptr);
    list.AddItem(p0);
    list.AddItem(p1);
    list.AddItem(p2);

    CHECK(list.SelectItem(1, false, false));
    CHECK(list.GetCurSel() == 1);
    CHECK(list.RemoveItemAt(1));
    CHECK(list.GetItemCount() == 2);
    // 删除后原位置变为 p2，自动选中它
    CHECK(list.GetCurSel() == 1);
    CHECK(list.GetItemAt(1) == p2);
    CHECK(p2->IsSelected());
    CHECK(!p0->IsSelected());
}

/** ListBox：多选与切换回单选的同步逻辑 */
void TestListBoxMultiSelect()
{
    TestableListBox list(nullptr);
    list.SetMultiSelect(true);
    CHECK(list.IsMultiSelect());

    ui::ListBoxItem* p0 = new ui::ListBoxItem(nullptr);
    ui::ListBoxItem* p1 = new ui::ListBoxItem(nullptr);
    ui::ListBoxItem* p2 = new ui::ListBoxItem(nullptr);
    list.AddItem(p0);
    list.AddItem(p1);
    list.AddItem(p2);

    CHECK(list.SelectItem(0, false, false));
    CHECK(list.SelectItem(2, false, false));
    CHECK(list.GetCurSel() == 2);
    CHECK(p0->IsSelected());
    CHECK(!p1->IsSelected());
    CHECK(p2->IsSelected());
    CHECK(list.ItemSelected(0));
    CHECK(!list.ItemSelected(1));
    CHECK(list.ItemSelected(2));

    // 再次选择同一项：取消选择
    CHECK(list.SelectItem(2, false, false));
    CHECK(!p2->IsSelected());
    CHECK(list.GetCurSel() == ui::Box::InvalidIndex);

    // 切换回单选：只保留第一个已选择项
    list.SetMultiSelect(false);
    CHECK(!list.IsMultiSelect());
    CHECK(list.GetCurSel() == 0);
    CHECK(p0->IsSelected());
    CHECK(!p1->IsSelected());
    CHECK(!p2->IsSelected());
}

/** ListBox：框选相关配置项与取值范围 */
void TestListBoxHelperConfig()
{
    ui::VListBox list(nullptr);
    CHECK(!list.IsEnableFrameSelection());
    list.SetEnableFrameSelection(true);
    CHECK(list.IsEnableFrameSelection());

    list.SetFrameSelectionColor(_T("#11223344"));
    CHECK(list.GetFrameSelectionColor() == _T("#11223344"));
    list.SetFrameSelectionBorderColor(_T("#FF000000"));
    CHECK(list.GetFrameSelectionBorderColor() == _T("#FF000000"));

    list.SetFrameSelectionAlpha(0);
    CHECK(list.GetFrameSelectionAlpha() == 0);
    list.SetFrameSelectionAlpha(255);
    CHECK(list.GetFrameSelectionAlpha() == 255);

    // 边框大小限制在 [0, 255]
    list.SetFrameSelectionBorderSize(-5);
    CHECK(list.GetFrameSelectionBorderSize() == 0);
    list.SetFrameSelectionBorderSize(300);
    CHECK(list.GetFrameSelectionBorderSize() == 255);
    list.SetFrameSelectionBorderSize(3);
    CHECK(list.GetFrameSelectionBorderSize() == 3);

    list.SetNormalItemTop(-1);
    CHECK(list.GetNormalItemTop() == -1);
    list.SetNormalItemTop(24);
    CHECK(list.GetNormalItemTop() == 24);
}

/** TabBox：页签选择与插入/删除时的选择索引维护 */
void TestTabBox()
{
    ui::TabBox tab(nullptr);
    tab.SetFadeSwitch(false);
    CHECK(tab.GetItemCount() == 0);
    CHECK(tab.GetCurSel() == ui::Box::InvalidIndex);

    ui::VBox* pPage0 = new ui::VBox(nullptr);
    ui::VBox* pPage1 = new ui::VBox(nullptr);
    ui::VBox* pPage2 = new ui::VBox(nullptr);

    // 第一项加入后自动选中，其他页面保持隐藏
    CHECK(tab.AddItem(pPage0));
    CHECK(tab.GetCurSel() == 0);
    CHECK(pPage0->IsVisible());
    CHECK(tab.AddItem(pPage1));
    CHECK(tab.AddItem(pPage2));
    CHECK(tab.GetCurSel() == 0);
    CHECK(!pPage1->IsVisible());
    CHECK(!pPage2->IsVisible());

    CHECK(tab.SelectItem(2));
    CHECK(tab.GetCurSel() == 2);
    CHECK(pPage2->IsVisible());
    CHECK(!pPage0->IsVisible());

    // 在当前选择页之前插入：选择索引后移
    ui::VBox* pPageNew = new ui::VBox(nullptr);
    CHECK(tab.AddItemAt(pPageNew, 0));
    CHECK(tab.GetItemCount() == 4);
    CHECK(tab.GetCurSel() == 3);
    CHECK(pPage2->IsVisible());
    CHECK(!pPageNew->IsVisible());

    // 删除选择页之前的页面：选择索引前移
    CHECK(tab.RemoveItemAt(0));
    CHECK(tab.GetItemCount() == 3);
    CHECK(tab.GetCurSel() == 2);

    // 删除当前选择页：改为选择前一个页面
    CHECK(tab.RemoveItemAt(2));
    CHECK(tab.GetItemCount() == 2);
    CHECK(tab.GetCurSel() == 1);
    CHECK(pPage1->IsVisible());
    CHECK(!pPage0->IsVisible());

    // 无效索引不能被选择
    CHECK(!tab.SelectItem(ui::Box::InvalidIndex));

    tab.RemoveAllItems();
    CHECK(tab.GetItemCount() == 0);
    CHECK(tab.GetCurSel() == ui::Box::InvalidIndex);
}

/**
 * TabBox 回归用例：所有页面均不可见时当前选择为 InvalidIndex，
 * 此时删除子项不能把 m_nCurSel 减成 size_t(-2)（会被 IsValidItemIndex 误判为有效索引）
 */
void TestTabBoxRemoveItemWithInvalidCurSel()
{
    ui::TabBox tab(nullptr);
    tab.SetFadeSwitch(false);

    ui::VBox* pHidden0 = new ui::VBox(nullptr);
    pHidden0->SetVisible(false);
    CHECK(tab.AddItem(pHidden0));
    CHECK(tab.GetCurSel() == ui::Box::InvalidIndex);

    ui::VBox* pHidden1 = new ui::VBox(nullptr);
    pHidden1->SetVisible(false);
    CHECK(tab.AddItem(pHidden1));
    CHECK(tab.GetCurSel() == ui::Box::InvalidIndex);

    // 删除非当前选择项时，选择索引必须保持 InvalidIndex
    CHECK(tab.RemoveItemAt(0));
    CHECK(tab.GetCurSel() == ui::Box::InvalidIndex);
    CHECK(!ui::Box::IsValidItemIndex(tab.GetCurSel()));
    CHECK(tab.GetItemCount() == 1);

    tab.RemoveAllItems();
    CHECK(tab.GetItemCount() == 0);
    CHECK(tab.GetCurSel() == ui::Box::InvalidIndex);
}

/** Panel：标题、可折叠状态与折叠布局对 min_height 的临时放宽/恢复 */
void TestPanel()
{
    ui::Panel panel(nullptr);
    CHECK(panel.GetType() == DUI_CTR_PANEL);
    CHECK(!panel.IsCollapsible());
    CHECK(!panel.IsCollapsed());
    CHECK(panel.GetTitle().empty());
    CHECK(panel.GetTitleBarHeight() > 0);

    panel.SetTitle(_T("设置"));
    CHECK(panel.GetTitle() == _T("设置"));

    panel.SetCollapsible(true);
    CHECK(panel.IsCollapsible());

    // 折叠时临时把 min_height 放宽到标题栏高度，展开后恢复
    panel.SetMinHeight(120, false);
    CHECK(panel.GetMinHeight() == 120);
    panel.SetCollapsed(true, false, false);
    CHECK(panel.IsCollapsed());
    CHECK(panel.GetMinHeight() == panel.GetTitleBarHeight());
    panel.SetCollapsed(false, false, false);
    CHECK(!panel.IsCollapsed());
    CHECK(panel.GetMinHeight() == 120);

    // 未启用可折叠时只记录状态，启用后直接呈现折叠态
    ui::Panel panel2(nullptr);
    panel2.SetCollapsed(true, false, false);
    CHECK(panel2.IsCollapsed());
    panel2.SetCollapsible(true);
    CHECK(panel2.IsCollapsed());
    CHECK(panel2.GetMinHeight() <= panel2.GetTitleBarHeight());

    // 属性读写
    panel.SetGroup(_T("group1"));
    CHECK(panel.GetGroup() == _T("group1"));
    panel.SetArrowAlign(ui::Panel::ArrowAlign::kLeft);
    CHECK(panel.GetArrowAlign() == ui::Panel::ArrowAlign::kLeft);
    panel.SetCollapseTrigger(ui::Panel::CollapseTrigger::kArrow);
    CHECK(panel.GetCollapseTrigger() == ui::Panel::CollapseTrigger::kArrow);
    panel.SetCollapseAnimMillSeconds(0);
    CHECK(panel.GetCollapseAnimMillSeconds() == 0);
    panel.SetCollapsePhysicsEnabled(true);
    CHECK(panel.IsCollapsePhysicsEnabled());
}

}// namespace

int main()
{
#if defined(_WIN32)
    // Debug 下控件内部断言不应弹框阻塞命令行测试
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif
    // 测试不创建窗口，关闭全局动画避免控件创建/显示时启动定时器
    ui::GlobalManager::Instance().SetAnimationEnabled(false);

    TestBoxContainerBasics();
    TestFindSelectable();
    TestLayoutBoxes();
    TestListBoxSelection();
    TestListBoxSelectNextWhenRemoved();
    TestListBoxMultiSelect();
    TestListBoxHelperConfig();
    TestTabBox();
    TestTabBoxRemoveItemWithInvalidCurSel();
    TestPanel();

    if (g_failedCount == 0) {
        std::cout << "[PASS] box: " << g_checkCount << " checks" << std::endl;
        return 0;
    }
    std::cerr << "[FAILED] box: " << g_failedCount << " of " << g_checkCount
              << " checks failed" << std::endl;
    return 1;
}

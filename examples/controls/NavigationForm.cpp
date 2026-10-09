#include "NavigationForm.h"
#include <functional>

using namespace ui;

NavigationForm::NavigationForm()
{
}

NavigationForm::~NavigationForm()
{
}

DString NavigationForm::GetSkinFolder()
{
    return _T("controls");
}

DString NavigationForm::GetSkinFile()
{
    return _T("navigation.xml");
}

void NavigationForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    ui::NavigationView* pNav = dynamic_cast<ui::NavigationView*>(FindControl(_T("nav")));
    ui::Label* pStatusValue = dynamic_cast<ui::Label*>(FindControl(_T("nav_status_value")));
    if (pNav == nullptr) {
        return;
    }

    //状态栏回调：显示当前选中项序号与关联页
    std::function<void()> updateStatus = [pStatusValue, pNav]() {
        if (pStatusValue == nullptr) {
            return;
        }
        size_t nSel = pNav->GetSelectedIndex();
        DString text;
        if (nSel == ui::Box::InvalidIndex) {
            text = _T("(none)");
        }
        else {
            text = ui::StringUtil::Printf(_T("index=%zu, page=%s"),
                                          nSel, pNav->GetSelectedPage().c_str());
        }
        pStatusValue->SetText(text);
        };
    updateStatus();

    //AttachNavSelect 监听导航选中变化
    pNav->AttachNavSelect([updateStatus](const ui::EventArgs& /*args*/) {
        updateStatus();
        return true;
        });

    //首页按钮以编程方式切换到"工具"页
    ui::Button* pJumpBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_jump_tools")));
    if (pJumpBtn != nullptr) {
        pJumpBtn->AttachClick([pNav](const ui::EventArgs& /*args*/) {
            pNav->SelectItem(_T("page_tools"));
            return true;
            });
    }

    //演示动态添加导航项：用 C++ API AddNavItem 新增一项（复用首页内容容器）
    ui::Button* pAddBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_add_nav")));
    if (pAddBtn != nullptr) {
        pAddBtn->AttachClick([pNav, updateStatus](const ui::EventArgs& /*args*/) {
            static int32_t nCount = 0;
            DString pageName = ui::StringUtil::Printf(_T("page_dynamic_%d"), nCount);
            DString itemText = ui::StringUtil::Printf(_T("Item %d"), nCount);
            ui::NavigationViewItem* pItem = pNav->AddNavItem(
                DString(),
                _T("file='nav/star.svg' width='16' height='16' svg_replace_colors='#333333|border_svg_image'"),
                pageName);
            if (pItem != nullptr) {
                pItem->SetItemText(itemText);
            }
            ++nCount;
            updateStatus();
            return true;
            });
    }

    //演示程序化切换展开/紧凑状态
    ui::Button* pToggleBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_toggle_nav")));
    if (pToggleBtn != nullptr) {
        pToggleBtn->AttachClick([pNav](const ui::EventArgs& /*args*/) {
            pNav->SetCollapsed(!pNav->IsCollapsed());
            return true;
            });
    }
}

#include "NavigationForm.h"
#include <functional>
#include <memory>
#include <vector>

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

    //AttachNavSelectionChanged 监听导航选中变化（wParam 新序号，lParam 旧序号）
    pNav->AttachNavSelectionChanged([updateStatus](const ui::EventArgs& /*args*/) {
        updateStatus();
        return true;
        });

    //AttachNavItemClick：重复点击同一项也会触发（适合刷新/回顶场景）
    pNav->AttachNavItemClick([pStatusValue](const ui::EventArgs& args) {
        if (pStatusValue != nullptr) {
            pStatusValue->SetText(ui::StringUtil::Printf(_T("item clicked: %zu"), (size_t)args.wParam));
        }
        return true;
        });

    //AttachNavPaneToggled：窗格收起/展开完成后更新状态（wParam: 1=收起 0=展开）
    pNav->AttachNavPaneToggled([pStatusValue](const ui::EventArgs& args) {
        if (pStatusValue != nullptr) {
            pStatusValue->SetText(args.wParam == 1 ? _T("pane collapsed") : _T("pane expanded"));
        }
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

    //演示动态添加导航项：AddNavItem 一次完成"建内容页 + 加导航项 + 关联"
    //用 shared_ptr 记录动态创建项，供"移除末项"演示使用
    auto spDynamicItems = std::make_shared<std::vector<ui::NavigationViewItem*>>();
    ui::Button* pAddBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_add_nav")));
    if (pAddBtn != nullptr) {
        pAddBtn->AttachClick([pNav, updateStatus, spDynamicItems](const ui::EventArgs& /*args*/) {
            static int32_t nCount = 0;
            DString pageName = ui::StringUtil::Printf(_T("page_dynamic_%d"), nCount);
            DString itemText = ui::StringUtil::Printf(_T("Item %d"), nCount);

            //动态创建一个内容页（VBox 承载一段说明文字）
            ui::VBox* pPage = new ui::VBox(pNav->GetWindow());
            pPage->SetBkColor(_T("bg_window_main"));
            pPage->SetAttribute(_T("padding"), _T("24,24,24,24"));
            ui::Label* pLabel = new ui::Label(pNav->GetWindow());
            pLabel->SetText(ui::StringUtil::Printf(_T("动态页面：%s"), pageName.c_str()));
            pLabel->SetAttribute(_T("text_color"), _T("text_default"));
            pLabel->SetFontId(_T("system_regular_16"));
            pPage->AddItem(pLabel);

            //第 4 参传入页面控件：自动挂到内容区 TabBox，并与导航项 page 联动
            ui::NavigationViewItem* pItem = pNav->AddNavItem(
                DString(),
                _T("file='nav/star.svg' width='16' height='16' svg_replace_colors='#333333|border_svg_image'"),
                pageName,
                pPage);
            if (pItem != nullptr) {
                pItem->SetItemText(itemText);
                spDynamicItems->push_back(pItem);
                //立即选中新建项，直观看到右侧内容页
                pNav->SelectItem(pItem);
            }
            ++nCount;
            updateStatus();
            return true;
            });
    }

    //演示动态移除导航项：RemoveNavItem(pItem, true) 一并移除关联内容页
    ui::Button* pRemoveBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_remove_nav")));
    if (pRemoveBtn != nullptr) {
        pRemoveBtn->AttachClick([pNav, updateStatus, spDynamicItems](const ui::EventArgs& /*args*/) {
            if (spDynamicItems->empty()) {
                return true;
            }
            ui::NavigationViewItem* pItem = spDynamicItems->back();
            spDynamicItems->pop_back();
            //第 2 参 true：同时移除其关联的内容页，避免右侧残留
            pNav->RemoveNavItem(pItem, true);
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

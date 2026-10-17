#include "ToolBarForm.h"

ToolBarForm::ToolBarForm()
{
}

ToolBarForm::~ToolBarForm()
{
}

DString ToolBarForm::GetSkinFolder()
{
    return _T("controls");
}

DString ToolBarForm::GetSkinFile()
{
    return _T("toolbar.xml");
}

void ToolBarForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //绑定工具条溢出项激活的回调：用户点击"更多"浮层中的按钮时触发
    ui::ToolBar* pToolBar = dynamic_cast<ui::ToolBar*>(FindControl(_T("main_toolbar")));
    if (pToolBar != nullptr) {
        pToolBar->AttachOverflowItemActivated([this](const DString& itemName) {
            OnOverflowItemActivated(itemName);
            });
    }
}

void ToolBarForm::OnOverflowItemActivated(const DString& itemName)
{
    //被收纳的按钮已被隐藏，但仍可通过名称找到它，并取得其显示文本
    DString strText;
    ui::Label* pItem = dynamic_cast<ui::Label*>(FindControl(itemName));
    if (pItem != nullptr) {
        strText = pItem->GetText();
    }
    if (strText.empty()) {
        strText = itemName;
    }

    //更新窗口内的事件提示
    ui::Label* pEvent = dynamic_cast<ui::Label*>(FindControl(_T("tb_event")));
    if (pEvent != nullptr) {
        DString strPrefix = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TOOLBAR_EVENT_ITEM"));
        pEvent->SetText(strPrefix + strText);
    }

    //同时弹出 Toast 提示
    ui::ToastWnd::Show(this, strText, ui::ToastWnd::kTypeInfo, 2000, ui::ToastWnd::kPosTop, false);
}

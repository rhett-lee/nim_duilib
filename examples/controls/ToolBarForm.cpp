#include "ToolBarForm.h"

ToolBarForm::ToolBarForm() :
    m_pDynamicToolBar(nullptr)
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

    //动态工具条：演示工具项管理 API 与点击/勾选回调
    m_pDynamicToolBar = dynamic_cast<ui::ToolBar*>(FindControl(_T("dynamic_toolbar")));
    if (m_pDynamicToolBar == nullptr) {
        return;
    }
    m_pDynamicToolBar->AttachToolItemClick([this](const DString& itemName) {
        OnToolItemClick(itemName);
        });
    m_pDynamicToolBar->AttachToolItemCheckedChanged([this](const DString& itemName, bool bChecked) {
        OnToolItemCheckedChanged(itemName, bChecked);
        });

    m_pDynamicToolBar->AddToolButton(_T("dyn_bold"), _T("STRID_TOOLBAR_DYN_BOLD"), DString(), true);
    m_pDynamicToolBar->AddToolCheckBox(_T("dyn_italic"), _T("STRID_TOOLBAR_DYN_ITALIC"), false, DString(), true);
    m_pDynamicToolBar->AddToolCheckBox(_T("dyn_underline"), _T("STRID_TOOLBAR_DYN_UNDERLINE"), false, DString(), true);
    m_pDynamicToolBar->AddToolSeparator();
    m_pDynamicToolBar->AddToolButton(_T("dyn_align_left"), _T("STRID_TOOLBAR_DYN_ALIGN_LEFT"), DString(), true);
    m_pDynamicToolBar->AddToolCheckBox(_T("dyn_align_center"), _T("STRID_TOOLBAR_DYN_ALIGN_CENTER"), true, DString(), true);
    m_pDynamicToolBar->AddToolButton(_T("dyn_more"), _T("STRID_TOOLBAR_DYN_MORE"), DString(), true);
    m_pDynamicToolBar->AddToolSeparator();
    m_pDynamicToolBar->AddToolButton(_T("dyn_remove"), _T("STRID_TOOLBAR_DYN_REMOVE"), DString(), true);
}

void ToolBarForm::OnToolItemClick(const DString& itemName)
{
    //演示 RemoveTool：点击"移除一项"后删除"更多动态"工具项
    if ((m_pDynamicToolBar != nullptr) && (itemName == _T("dyn_remove"))) {
        m_pDynamicToolBar->RemoveTool(_T("dyn_more"));
    }

    DString strText = itemName;
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(itemName));
    if (pLabel != nullptr) {
        strText = pLabel->GetText();
    }
    const DString strPrefix = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TOOLBAR_EVENT_TOOL"));
    SetEventText(strPrefix + strText);
}

void ToolBarForm::OnToolItemCheckedChanged(const DString& itemName, bool bChecked)
{
    DString strText = itemName;
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(itemName));
    if (pLabel != nullptr) {
        strText = pLabel->GetText();
    }
    const DString strPrefix = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TOOLBAR_EVENT_CHECKED"));
    const DString strState = ui::GlobalManager::Instance().Lang().GetStringByID(
        bChecked ? _T("STRID_TOOLBAR_EVENT_CHECKED_ON") : _T("STRID_TOOLBAR_EVENT_CHECKED_OFF"));
    SetEventText(strPrefix + strText + _T(" ") + strState);
}

void ToolBarForm::SetEventText(const DString& strText)
{
    ui::Label* pEvent = dynamic_cast<ui::Label*>(FindControl(_T("tb_event")));
    if (pEvent != nullptr) {
        pEvent->SetText(strText);
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
    DString strPrefix = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_TOOLBAR_EVENT_ITEM"));
    SetEventText(strPrefix + strText);

    //同时弹出 Toast 提示
    ui::ToastWnd::Show(this, strText, ui::ToastWnd::kTypeInfo, 2000, ui::ToastWnd::kPosTop, false);
}

#include "BrowserTabItem.h"

using namespace ui;

BrowserTabItem::BrowserTabItem(ui::Window* pWindow):
    ui::ListBoxItem(pWindow),
    m_pIcon(nullptr),
    m_pName(nullptr),
    m_pButtonClose(nullptr)
{
}

void BrowserTabItem::InitControl(const std::string& browser_id)
{
    this->AttachContextMenu(UiBind(&BrowserTabItem::OnItemMenu, this, std::placeholders::_1));
    this->AttachMouseEnter(UiBind(&BrowserTabItem::OnMouseEnter, this, std::placeholders::_1));
    this->AttachMouseLeave(UiBind(&BrowserTabItem::OnMouseLeave, this, std::placeholders::_1));

    m_pIcon = dynamic_cast<Button*>(this->FindSubControl(_T("tab_item_icon")));
    m_pName = dynamic_cast<Label*>(this->FindSubControl(_T("tab_item_name")));
    m_pButtonClose = dynamic_cast<Button*>(this->FindSubControl(_T("tab_item_close")));

    this->SetUTF8Name(browser_id);
}

void BrowserTabItem::SetTitle(const DString& name)
{
    this->SetToolTipText(name);
    m_pName->SetText(name);
}

void BrowserTabItem::SetIcon(const DString& icon)
{
    m_pIcon->SetBkImage(icon);
}

bool BrowserTabItem::OnMouseEnter(const ui::EventArgs& msg)
{
    m_pButtonClose->SetFadeVisible(true);

    return true;
}

bool BrowserTabItem::OnMouseLeave(const ui::EventArgs& msg)
{
    if (!GetRect().ContainsPt(msg.ptMouse))
    {
        m_pButtonClose->SetFadeVisible(false);
    }
    
    return false;
}

bool BrowserTabItem::OnItemMenu(const ui::EventArgs& /*arg*/)
{
    POINT point;
    ::GetCursorPos(&point);
    PopupItemMenu(point);
    return true;
}

void BrowserTabItem::PopupItemMenu(POINT point)
{

}

bool BrowserTabItem::MergeItemMenuItemClick(const ui::EventArgs& /*param*/)
{
    return true;
}

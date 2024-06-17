#include "browser_tab_item.h"

using namespace ui;

BrowserTabItem::BrowserTabItem(ui::Window* pWindow):
    ui::ListBoxItem(pWindow),
    icon_(nullptr),
    name_(nullptr),
    button_close_(nullptr)
{
}

void BrowserTabItem::InitControl(const std::string& browser_id)
{
    this->AttachContextMenu(UiBind(&BrowserTabItem::OnItemMenu, this, std::placeholders::_1));
    this->AttachMouseEnter(UiBind(&BrowserTabItem::OnMouseEnter, this, std::placeholders::_1));
    this->AttachMouseLeave(UiBind(&BrowserTabItem::OnMouseLeave, this, std::placeholders::_1));

    icon_ = dynamic_cast<Button*>(this->FindSubControl(_T("tab_item_icon")));
    name_ = dynamic_cast<Label*>(this->FindSubControl(_T("tab_item_name")));
    button_close_ = dynamic_cast<Button*>(this->FindSubControl(_T("tab_item_close")));

    this->SetUTF8Name(browser_id);
}

void BrowserTabItem::SetTitle(const DString& name)
{
    this->SetToolTipText(name);
    name_->SetText(name);
}

void BrowserTabItem::SetIcon(const DString& icon)
{
    icon_->SetBkImage(icon);
}

bool BrowserTabItem::OnMouseEnter(const ui::EventArgs& msg)
{
    button_close_->SetFadeVisible(true);

    return true;
}

bool BrowserTabItem::OnMouseLeave(const ui::EventArgs& msg)
{
    if (!GetRect().ContainsPt(msg.ptMouse))
    {
        button_close_->SetFadeVisible(false);
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

#include "stdafx.h"
#include "item.h"


Item::Item(ui::Window* pWindow):
    ui::ListBoxItem(pWindow)
{
}

Item::~Item()
{
}

void Item::InitSubControls(const std::wstring& img, const std::wstring& title)
{
    // 查找 Item 下的控件
    control_img_    = dynamic_cast<ui::Control*>(FindSubControl(L"control_img"));
    label_title_    = dynamic_cast<ui::Label*>(FindSubControl(L"label_title"));
    progress_        = dynamic_cast<ui::Progress*>(FindSubControl(L"progress"));
    btn_del_        = dynamic_cast<ui::Button*>(FindSubControl(L"btn_del"));

    // 模拟进度条进度
    int32_t nProgress = std::time(nullptr) % 100;
    progress_->SetValue((double)(nProgress));

    // 设置图标和任务名称
    control_img_->SetBkImage(img);
    label_title_->SetText(ui::StringUtil::Printf(L"%s %d%%", title.c_str(), nProgress));

    // 绑定删除任务处理函数
    btn_del_->AttachClick(UiBind(&Item::OnRemove, this, std::placeholders::_1));
}

bool Item::OnRemove(const ui::EventArgs& args)
{
    ui::ListBox* parent = dynamic_cast<ui::ListBox*>(this->GetParent());
    return parent->RemoveItem(this);
}

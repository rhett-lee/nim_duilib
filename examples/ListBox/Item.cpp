#include "Item.h"

Item::Item(ui::Window* pWindow):
    ui::ControlDragableT<ui::ListBoxItem>(pWindow),
    m_pListBox(nullptr),
    m_pImageControl(nullptr),
    m_pTitleLabel(nullptr),
    m_pProgressControl(nullptr),
    m_pDelBtn(nullptr)
{
}

Item::~Item()
{
}

void Item::InitSubControls(const DString& img, const DString& title)
{
    // 查找 Item 下的控件
    m_pImageControl = dynamic_cast<ui::Control*>(FindSubControl(_T("control_img")));
    m_pTitleLabel = dynamic_cast<ui::Label*>(FindSubControl(_T("label_title")));
    m_pProgressControl = dynamic_cast<ui::Progress*>(FindSubControl(_T("progress")));
    m_pDelBtn = dynamic_cast<ui::Button*>(FindSubControl(_T("btn_del")));

    // 模拟进度条进度
    int32_t nProgress = std::time(nullptr) % 100;
    m_pProgressControl->SetValue((double)(nProgress));

    // 设置图标和任务名称
    m_pImageControl->SetBkImage(img);
    m_pTitleLabel->SetText(ui::StringUtil::Printf(_T("%s %d%%"), title.c_str(), nProgress));

    // 绑定删除任务处理函数
    m_pDelBtn->AttachClick(UiBind(&Item::OnRemove, this, std::placeholders::_1));
}

bool Item::OnRemove(const ui::EventArgs& args)
{
    ui::ListBox* parent = dynamic_cast<ui::ListBox*>(this->GetParent());
    if (parent != nullptr) {
        parent->RemoveItem(this);
    }
    return true;
}

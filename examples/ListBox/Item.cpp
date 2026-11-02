#include "Item.h"

Item::Item(ui::Window* pWindow):
    ui::ControlDragableT<ui::ListBoxItem>(pWindow),
    m_pListBox(nullptr),
    m_pImageControl(nullptr),
    m_pTitleLabel(nullptr),
    m_pPctLabel(nullptr),
    m_pProgressControl(nullptr),
    m_pDelBtn(nullptr)
{
}

Item::~Item()
{
}

void Item::InitSubControls(const DString& img, const DString& title, int32_t nProgress)
{
    // 查找 Item 下的控件
    m_pImageControl = dynamic_cast<ui::Control*>(FindSubControl(_T("control_img")));
    m_pTitleLabel = dynamic_cast<ui::Label*>(FindSubControl(_T("label_title")));
    m_pPctLabel = dynamic_cast<ui::Label*>(FindSubControl(_T("label_pct")));
    m_pProgressControl = dynamic_cast<ui::Progress*>(FindSubControl(_T("progress")));
    m_pDelBtn = dynamic_cast<ui::Button*>(FindSubControl(_T("btn_del")));

    if (nProgress < 0) {
        nProgress = 0;
    }
    if (nProgress > 100) {
        nProgress = 100;
    }

    // 设置图标、任务名称和进度
    if (m_pImageControl != nullptr) {
        m_pImageControl->SetBkImage(img);
    }
    if (m_pTitleLabel != nullptr) {
        m_pTitleLabel->SetText(title);
    }
    if (m_pPctLabel != nullptr) {
        m_pPctLabel->SetText(ui::StringUtil::Printf(_T("%d%%"), nProgress));
    }
    if (m_pProgressControl != nullptr) {
        m_pProgressControl->SetValue((double)nProgress);
    }

    // 绑定删除任务处理函数
    if (m_pDelBtn != nullptr) {
        m_pDelBtn->AttachClick(UiBind(&Item::OnRemove, this, std::placeholders::_1));
    }
}

void Item::SetOnRemoved(std::function<void()> onRemoved)
{
    m_onRemoved = std::move(onRemoved);
}

bool Item::OnRemove(const ui::EventArgs& args)
{
    //先把回调拷贝到局部变量：RemoveItem 之后 this 可能已经被销毁，不能再访问成员变量
    std::function<void()> onRemoved = m_onRemoved;
    ui::ListBox* parent = dynamic_cast<ui::ListBox*>(this->GetParent());
    if (parent != nullptr) {
        parent->RemoveItem(this);
    }
    //本项已经从列表中移除，通知列表窗口刷新项数等统计信息
    if (onRemoved) {
        onRemoved();
    }
    return true;
}

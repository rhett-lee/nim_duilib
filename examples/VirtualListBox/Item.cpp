#include "Item.h"
#include "DataProvider.h"
#include <chrono>

Item::Item(ui::Window* pWindow):
    ui::ListBoxItem(pWindow),
    m_pListBox(nullptr),
    m_pImageControl(nullptr),
    m_pTitleLabel(nullptr),
    m_pProgressControl(nullptr),
    m_pDelBtn(nullptr),
    t_time(0),
    m_nDataIndex(0)
{
}


Item::~Item()
= default;

void Item::InitSubControls(const DString& img, const DString& title, size_t nDataIndex)
{
    // 查找 Item 下的控件
    if (m_pImageControl == nullptr) {
        m_pImageControl = dynamic_cast<ui::Control*>(FindSubControl(_T("control_img")));
        m_pTitleLabel = dynamic_cast<ui::Label*>(FindSubControl(_T("label_title")));
        m_pProgressControl = dynamic_cast<ui::Progress*>(FindSubControl(_T("progress")));
        m_pDelBtn = dynamic_cast<ui::Button*>(FindSubControl(_T("btn_del")));
        // 模拟进度条进度
        t_time = std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
        m_pProgressControl->SetValue((double)(t_time % 100));
        // 设置图标和任务名称
        m_pImageControl->SetBkImage(img);
        // 绑定删除任务处理函数
        m_pDelBtn->AttachClick(UiBind(&Item::OnRemove, this, std::placeholders::_1));
    }
    m_pTitleLabel->SetText(ui::StringUtil::Printf(_T("%s %d%%"), title.c_str(), t_time % 100));
    m_nDataIndex = nDataIndex;
}


bool Item::OnRemove(const ui::EventArgs& args)
{
    // 删除时，只需删除数据就可以了，不要删除界面上的元素
    ui::VirtualListBox* pTileBox = dynamic_cast<ui::VirtualListBox*>(GetOwner());
    ASSERT(pTileBox != nullptr);
    if (pTileBox != nullptr) {
        DataProvider* pProvider = dynamic_cast<DataProvider*>(pTileBox->GetDataProvider());
        ASSERT(pProvider != nullptr);
        if (pProvider != nullptr) {
            pProvider->RemoveTask(m_nDataIndex);
            return true;
        }
    }
    return false;    
}

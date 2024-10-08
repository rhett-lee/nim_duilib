#include "item.h"
#include "provider.h"
#include <chrono>

Item::Item(ui::Window* pWindow):
    ui::ListBoxItem(pWindow),
    list_box_(nullptr),
    control_img_(nullptr),
    label_title_(nullptr),
    progress_(nullptr),
    btn_del_(nullptr),
    t_time(0),
    m_nDataIndex(0)
{
}


Item::~Item()
= default;

void Item::InitSubControls(const DString& img, const DString& title, size_t nDataIndex)
{
    // 查找 Item 下的控件
    if (control_img_ == nullptr)
    {
        control_img_ = dynamic_cast<ui::Control*>(FindSubControl(_T("control_img")));
        label_title_ = dynamic_cast<ui::Label*>(FindSubControl(_T("label_title")));
        progress_ = dynamic_cast<ui::Progress*>(FindSubControl(_T("progress")));
        btn_del_ = dynamic_cast<ui::Button*>(FindSubControl(_T("btn_del")));
        // 模拟进度条进度
        t_time = std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
        progress_->SetValue((double)(t_time % 100));
        // 设置图标和任务名称
        control_img_->SetBkImage(img);
        // 绑定删除任务处理函数
        btn_del_->AttachClick(UiBind(&Item::OnRemove, this, std::placeholders::_1));
    }


    label_title_->SetText(ui::StringUtil::Printf(_T("%s %d%%"), title.c_str(), t_time % 100));
    m_nDataIndex = nDataIndex;

}


bool Item::OnRemove(const ui::EventArgs& args)
{
    // 删除时，只需删除数据就可以了，不要删除界面上的元素
    ui::VirtualListBox* pTileBox = dynamic_cast<ui::VirtualListBox*>(GetOwner());
    ASSERT(pTileBox != nullptr);
    if (pTileBox != nullptr) {
        Provider* pProvider = dynamic_cast<Provider*>(pTileBox->GetDataProvider());
        ASSERT(pProvider != nullptr);
        if (pProvider != nullptr) {
            pProvider->RemoveTask(m_nDataIndex);
            return true;
        }
    }
    return false;    
}

#include "Item.h"
#include "DataProvider.h"

Item::Item(ui::Window* pWindow):
    ui::ListBoxItem(pWindow),
    m_pImageControl(nullptr),
    m_pTitleLabel(nullptr),
    m_pProgressControl(nullptr),
    m_pPercentLabel(nullptr),
    m_pDelBtn(nullptr),
    m_nDataIndex(0),
    m_bSubControlsInited(false)
{
}


Item::~Item()
= default;

void Item::InitSubControls(const DString& img, const DString& title, size_t nDataIndex, int32_t nProgress)
{
    // 查找 Item 下的控件（只在第一次显示时查找）
    if (!m_bSubControlsInited) {
        m_bSubControlsInited = true;
        m_pImageControl = dynamic_cast<ui::Control*>(FindSubControl(_T("control_img")));
        m_pTitleLabel = dynamic_cast<ui::Label*>(FindSubControl(_T("label_title")));
        m_pProgressControl = dynamic_cast<ui::Progress*>(FindSubControl(_T("progress")));
        m_pPercentLabel = dynamic_cast<ui::Label*>(FindSubControl(_T("label_percent")));
        m_pDelBtn = dynamic_cast<ui::Button*>(FindSubControl(_T("btn_del")));
        if (m_pDelBtn != nullptr) {
            // 绑定删除任务处理函数
            m_pDelBtn->AttachClick(UiBind(&Item::OnRemove, this, std::placeholders::_1));
        }
    }

    if (m_pImageControl != nullptr) {
        m_pImageControl->SetBkImage(img);
    }
    if (m_pTitleLabel != nullptr) {
        m_pTitleLabel->SetText(title);
    }
    if (m_pProgressControl != nullptr) {
        m_pProgressControl->SetValue(nProgress);
    }
    if (m_pPercentLabel != nullptr) {
        m_pPercentLabel->SetText(ui::StringUtil::Printf(_T("%d%%"), nProgress));
    }
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

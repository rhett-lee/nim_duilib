#include "AppItemUi.h"
#include "AppDb.h"

AppItemUi* AppItemUi::Create(const AppItem& item, Box* p)
{
    ASSERT(p != nullptr);
    if (p == nullptr) {
        return nullptr;
    }
    ASSERT(p->GetWindow() != nullptr);
    AppItemUi* uiItem = new AppItemUi(p->GetWindow());
    uiItem->SetAppData(item, false);
    ui::GlobalManager::Instance().FillBoxWithCache(uiItem, ui::FilePath(_T("move_control/app_item.xml")));
    return uiItem;
}

AppItemUi::AppItemUi(ui::Window* pWindow) :
    ui::ControlDragableT<ui::VBox>(pWindow)
{
}

void AppItemUi::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();
    m_app_icon = static_cast<ui::Control*>(FindSubControl(_T("app_icon")));
    if (m_app_icon) {
        m_app_icon->SetBkImage(m_app_data.m_icon);
    }
    m_app_name = static_cast<ui::Label*>(FindSubControl(_T("app_name")));
    if (m_app_name) {
        m_app_name->SetTextId(m_app_data.m_name);
    }

    //绑定事件
}

void AppItemUi::SetAppData(const AppItem& item, bool bRefresh)
{
    m_app_data = item;
    if (bRefresh) {
        if (m_app_icon) {
            m_app_icon->SetBkImage(m_app_data.m_icon);
        }
        if (m_app_name) {
            m_app_name->SetTextId(m_app_data.m_name);
        }
    }
}

void AppItemUi::OnItemBoxChanged(Box* /*pOldBox*/, size_t /*nOldItemIndex*/,
                                 Box* pNewBox, size_t /*nNewItemIndex*/)
{
    if (pNewBox == m_pFrequentBox) {
        //常用应用
        m_app_data.m_isFrequent = true;
    }
    else {
        //我的应用
        m_app_data.m_isFrequent = false;
    }
    AppDb::GetInstance().SaveToDb(m_app_data);
}

void AppItemUi::SetFrequentBox(ui::Box* pFrequentBox)
{
    m_pFrequentBox = pFrequentBox;
}

ui::Control* AppItemUi::CreateDestControl(Box* pTargetBox)
{
    ui::Control* pDestControl = BaseClass::CreateDestControl(pTargetBox);
    if (pDestControl == nullptr) {
        return nullptr;
    }
    //占位控件只作为"插入位置的示意"，不应比图标本身更抢眼：
    //1. 填充色改为与背景一致，只保留描边；
    //2. 描边改为虚线，并使用与图标一致的圆角（12）；
    //3. 占位控件的大小仍由基类按格子尺寸设置，不能改动，插入位置的计算依赖它。
    pDestControl->SetAttribute(_T("bkcolor"), _T("bg_container"));
    pDestControl->SetAttribute(_T("border_size"), _T("1"));
    pDestControl->SetAttribute(_T("border_color"), _T("color_blue"));
    pDestControl->SetAttribute(_T("border_round"), _T("12,12"));
    pDestControl->SetAttribute(_T("border_dash_style"), _T("dash"));
    pDestControl->SetAlpha(192);
    return pDestControl;
}

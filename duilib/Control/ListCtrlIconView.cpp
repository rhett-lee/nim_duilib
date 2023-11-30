#include "ListCtrlIconView.h" 
#include "duilib/Control/ListCtrl.h"
#include "duilib/Control/ListCtrlData.h"

namespace ui
{
ListCtrlIconView::ListCtrlIconView(bool bListMode):
    ListCtrlView(new VirtualVTileLayout),
    m_pListCtrl(nullptr),
    m_pData(nullptr),
    m_bListMode(bListMode)
{
    if (bListMode) {
        Layout* pLayout = GetLayout();
        VirtualHTileLayout* pNewLayout = new VirtualHTileLayout;
        ReSetLayout(pNewLayout);
        SetVirtualLayout(pNewLayout);
        if (pLayout) {
            delete pLayout;
            pLayout = nullptr;
        }        
    }
    else {
        VirtualVTileLayout* pDataLayout = dynamic_cast<VirtualVTileLayout*>(GetLayout());
        ASSERT(pDataLayout != nullptr);
        VirtualLayout* pVirtualLayout = pDataLayout;
        SetVirtualLayout(pVirtualLayout);
    }    
}

ListCtrlIconView::~ListCtrlIconView()
{
}

void ListCtrlIconView::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"horizontal_layout") {
        SetHorizontalLayout(strValue == L"true");
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void ListCtrlIconViewItem::HandleEvent(const EventArgs& msg)
{
    __super::HandleEvent(msg);
    if (m_pListCtrl != nullptr) {
        if ((msg.Type > kEventKeyBegin) && (msg.Type < kEventKeyEnd)) {
            m_pListCtrl->OnViewKeyboardEvents(msg);
        }
        else if ((msg.Type > kEventMouseBegin) && (msg.Type < kEventMouseEnd)) {
            m_pListCtrl->OnViewMouseEvents(msg);
        }
    }
}

void ListCtrlListViewItem::HandleEvent(const EventArgs& msg)
{
    __super::HandleEvent(msg);
    if (m_pListCtrl != nullptr) {
        if ((msg.Type > kEventKeyBegin) && (msg.Type < kEventKeyEnd)) {
            m_pListCtrl->OnViewKeyboardEvents(msg);
        }
        else if ((msg.Type > kEventMouseBegin) && (msg.Type < kEventMouseEnd)) {
            m_pListCtrl->OnViewMouseEvents(msg);
        }
    }
}

void ListCtrlIconView::HandleEvent(const EventArgs& msg)
{
    __super::HandleEvent(msg);
    if (m_pListCtrl != nullptr) {
        if ((msg.Type > kEventKeyBegin) && (msg.Type < kEventKeyEnd)) {
            m_pListCtrl->OnViewKeyboardEvents(msg);
        }
        else if ((msg.Type > kEventMouseBegin) && (msg.Type < kEventMouseEnd)) {
            m_pListCtrl->OnViewMouseEvents(msg);
        }
    }
}

void ListCtrlIconView::SetHorizontalLayout(bool bHorizontal)
{
    if (bHorizontal) {
        //横向布局        
        if (dynamic_cast<VirtualHTileLayout*>(GetLayout()) == nullptr) {
            Layout* pLayout = GetLayout();
            VirtualVTileLayout* pOldLayout = dynamic_cast<VirtualVTileLayout*>(pLayout);
            VirtualHTileLayout* pNewLayout = new VirtualHTileLayout;
            ReSetLayout(pNewLayout);
            SetVirtualLayout(pNewLayout);
            if (pOldLayout != nullptr) {
                //同步属性
                pNewLayout->SetItemSize(pOldLayout->GetItemSize());
                pNewLayout->SetRows(pOldLayout->GetColumns());
                pNewLayout->SetAutoCalcRows(pOldLayout->IsAutoCalcColumns());
                pNewLayout->SetChildMarginX(pOldLayout->GetChildMarginX());
                pNewLayout->SetChildMarginY(pOldLayout->GetChildMarginY());
                pNewLayout->SetScaleDown(pOldLayout->IsScaleDown());
            }
            if (pLayout) {
                delete pLayout;
                pLayout = nullptr;
            }
        }
    }
    else {
        //纵向布局
        if (dynamic_cast<VirtualVTileLayout*>(GetLayout()) == nullptr) {
            Layout* pLayout = GetLayout();
            VirtualHTileLayout* pOldLayout = dynamic_cast<VirtualHTileLayout*>(pLayout);
            VirtualVTileLayout* pNewLayout = new VirtualVTileLayout;
            ReSetLayout(pNewLayout);
            SetVirtualLayout(pNewLayout);
            if (pOldLayout != nullptr) {
                //同步属性
                pNewLayout->SetItemSize(pOldLayout->GetItemSize());
                pNewLayout->SetColumns(pOldLayout->GetRows());
                pNewLayout->SetAutoCalcColumns(pOldLayout->IsAutoCalcRows());
                pNewLayout->SetChildMarginX(pOldLayout->GetChildMarginX());
                pNewLayout->SetChildMarginY(pOldLayout->GetChildMarginY());
                pNewLayout->SetScaleDown(pOldLayout->IsScaleDown());
            }
            if (pLayout) {
                delete pLayout;
                pLayout = nullptr;
            }
        }
    }
}

bool ListCtrlIconView::IsHorizontalLayout() const
{
    VirtualHTileLayout* pDataLayout = dynamic_cast<VirtualHTileLayout*>(GetLayout());
    return pDataLayout != nullptr;
}

void ListCtrlIconView::SetListCtrl(ListCtrl* pListCtrl)
{
    m_pListCtrl = pListCtrl;
}

void ListCtrlIconView::SetDataProvider(VirtualListBoxElement* pProvider)
{
    __super::SetDataProvider(pProvider);
    m_pData = dynamic_cast<ListCtrlData*>(GetDataProvider());
}

Control* ListCtrlIconView::CreateDataItem()
{
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return nullptr;
    }
    Control* pControl = nullptr;
    if (m_bListMode) {
        //列表视图: 水平布局
        ListCtrlListViewItem* pItem = new ListCtrlListViewItem;
        pItem->SetListCtrl(m_pListCtrl);
        pControl = pItem;
        pControl->SetClass(m_pListCtrl->GetListViewItemClass());
        Control* pItemImage = new Control;
        ListCtrlLabel* pItemLabel = new ListCtrlLabel;
        pItem->AddItem(pItemImage);
        pItem->AddItem(pItemLabel);
    }
    else {
        //图标视图：垂直布局
        ListCtrlIconViewItem* pItem = new ListCtrlIconViewItem;
        pItem->SetListCtrl(m_pListCtrl);
        pControl = pItem;
        pControl->SetClass(m_pListCtrl->GetIconViewItemClass());
        Control* pItemImage = new Control;
        ListCtrlLabel* pItemLabel = new ListCtrlLabel;
        pItem->AddItem(pItemImage);
        pItem->AddItem(pItemLabel);
    }
    AttachMouseEvents(pControl);
    return pControl;
}

bool ListCtrlIconView::FillDataItem(Control* pControl,
                                    size_t nElementIndex,
                                    const ListCtrlItemData& itemData,
                                    const std::vector<ListCtrlSubItemData2Pair>& subItemList)
{
    ASSERT((pControl != nullptr) && (m_pListCtrl != nullptr));
    if ((pControl == nullptr) || (m_pListCtrl == nullptr)) {
        return false;
    }
    ListCtrlSubItemData2Ptr pSubItemData;
    int32_t nImageId = -1;
    size_t nColumnId = m_pListCtrl->GetColumnId(0); //取第一列的ID
    for (const ListCtrlSubItemData2Pair& pair : subItemList) {
        if (pair.nColumnId == nColumnId) {
            if (pair.pSubItemData != nullptr) {
                nImageId = pair.pSubItemData->nImageId;
                pSubItemData = pair.pSubItemData;
            }
            break;
        }
    }
    if (nImageId < 0) {
        //如果列没有设置图标，则取行的
        nImageId = itemData.nImageId;
    }
    Box* pItemBox = dynamic_cast<Box*>(pControl);
    ASSERT(pItemBox != nullptr);
    if (pItemBox == nullptr) {
        return false;
    }

    Control* pItemImage = pItemBox->GetItemAt(0);
    ListCtrlLabel* pItemLabel = dynamic_cast<ListCtrlLabel*>(pItemBox->GetItemAt(1));
    ASSERT((pItemImage != nullptr) && (pItemLabel != nullptr));
    if ((pItemImage == nullptr) || (pItemLabel == nullptr)) {
        return false;
    }

    if (m_bListMode) {//列表视图
        //图标的属性
        pItemImage->SetClass(m_pListCtrl->GetListViewItemImageClass());
        //文本的属性
        pItemLabel->SetClass(m_pListCtrl->GetListViewItemLabelClass());
    }
    else {//图标视图
        //图标的属性
        pItemImage->SetClass(m_pListCtrl->GetIconViewItemImageClass());
        //文本的属性
        pItemLabel->SetClass(m_pListCtrl->GetIconViewItemLabelClass());
    }

    std::wstring imageString;
    UiSize imageSize;
    if (nImageId >= 0) {
        ImageList* pImageList = nullptr;
        if (m_bListMode) {
            pImageList = m_pListCtrl->GetImageList(ListCtrlType::List);
        }
        else {
            pImageList = m_pListCtrl->GetImageList(ListCtrlType::Icon);
        }
        if (pImageList == nullptr) {
            pImageList = m_pListCtrl->GetImageList(ListCtrlType::Report);
        }
        if (pImageList != nullptr) {
            imageString = pImageList->GetImageString(nImageId);
            imageSize = pImageList->GetImageSize();
        }
    }
    pItemImage->SetBkImage(imageString);
    if (imageSize.cx > 0) {
        pItemImage->SetFixedWidth(UiFixedInt(imageSize.cx), false, false);
    }
    if (imageSize.cy > 0) {
        pItemImage->SetFixedHeight(UiFixedInt(imageSize.cy), false, false);
    }
    if (pSubItemData != nullptr) {
        pItemLabel->SetText(pSubItemData->text.c_str());
    }
    else {
        pItemLabel->SetText(L"");
    }

    //设置不获取焦点等属性
    pItemImage->SetNoFocus();
    pItemLabel->SetNoFocus();

    pItemImage->SetMouseEnabled(false);

    //设置可编辑属性
    bool bEditable = (pSubItemData != nullptr) ? pSubItemData->bEditable : false;
    if (bEditable && m_pListCtrl->IsEnableItemEdit()) {
        IListBoxItem* pItem = dynamic_cast<IListBoxItem*>(pControl);
        ListCtrlLabel* pSubItem = pItemLabel;
        ASSERT(pItem != nullptr);
        pItemLabel->SetListBoxItem(pControl);
        pItemLabel->SetMouseEnabled(true);
        pItemLabel->DetachEvent(kEventEnterEdit);
        pItemLabel->AttachEvent(kEventEnterEdit, [this, nElementIndex, nColumnId, pItem, pSubItem](const EventArgs& /*args*/) {
            if (m_pListCtrl != nullptr) {
                m_pListCtrl->OnItemEnterEditMode(nElementIndex, nColumnId, pItem, pSubItem);
            }
            return true;
            });
    }
    else {
        pItemLabel->SetListBoxItem(nullptr);
        pItemLabel->DetachEvent(kEventEnterEdit);
        pItemLabel->SetMouseEnabled(false);
    }
    return true;
}

int32_t ListCtrlIconView::GetMaxDataItemWidth(const std::vector<ListCtrlSubItemData2Ptr>& /*subItemList*/)
{
    //不需要实现
    return -1;
}

}//namespace ui


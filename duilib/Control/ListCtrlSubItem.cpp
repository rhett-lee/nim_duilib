#include "ListCtrlSubItem.h"
#include "duilib/Control/ListCtrl.h"

namespace ui
{
ListCtrlSubItem::ListCtrlSubItem():
    m_pItem(nullptr)
{
}

void ListCtrlSubItem::SetListCtrlItem(ListCtrlItem* pItem)
{
    m_pItem = pItem;
}

ListCtrlItem* ListCtrlSubItem::GetListCtrlItem() const
{
    ASSERT(m_pItem != nullptr);
    return m_pItem;
}

bool ListCtrlSubItem::SetCheckBoxVisible(bool bVisible)
{
    bool bRet = false;
    if (bVisible) {
        ListCtrlItem* pItem = GetListCtrlItem();
        if (pItem == nullptr) {
            return false;
        }
        ListCtrlCheckBox* pCheckBox = nullptr;
        if (GetItemCount() > 0) {
            pCheckBox = dynamic_cast<ListCtrlCheckBox*>(GetItemAt(0));
        }
        if (pCheckBox == nullptr) {
            pCheckBox = new ListCtrlCheckBox;
            AddItem(pCheckBox);
            ListCtrl* pListCtrl = pItem->GetListCtrl();            
            std::wstring checkBoxClass;
            if (pListCtrl != nullptr) {
                checkBoxClass = pListCtrl->GetCheckBoxClass();
            }
            ASSERT(!checkBoxClass.empty());
            pCheckBox->SetClass(checkBoxClass);
        }
        //设置内边距，避免与文字重叠
        UiPadding textPadding = GetTextPadding();
        int32_t nCheckBoxWidth = pCheckBox->GetCheckBoxWidth();
        if ((nCheckBoxWidth > 0) && (textPadding.left < nCheckBoxWidth)) {
            textPadding.left = nCheckBoxWidth;
            SetTextPadding(textPadding, false);
        }
        pCheckBox->SetVisible(true);
    }
    else if (GetItemCount() > 0) {
        ListCtrlCheckBox* pCheckBox = dynamic_cast<ListCtrlCheckBox*>(GetItemAt(0));
        if (pCheckBox != nullptr) {
            UiPadding textPadding = GetTextPadding();
            int32_t nCheckBoxWidth = pCheckBox->GetCheckBoxWidth();
            if ((nCheckBoxWidth > 0) && (textPadding.left >= nCheckBoxWidth)) {
                textPadding.left -= nCheckBoxWidth;
                SetTextPadding(textPadding, false);
            }
            RemoveItemAt(0);
            bRet = true;
        }
    }
    return bRet;
}

bool ListCtrlSubItem::IsCheckBoxVisible() const
{
    if (GetItemCount() > 0) {
        CheckBox* pCheckBox = dynamic_cast<CheckBox*>(GetItemAt(0));
        if (pCheckBox != nullptr) {
            return pCheckBox->IsVisible();
        }
    }
    return false;
}

bool ListCtrlSubItem::SetCheckBoxSelect(bool bSelected, bool bPartSelect)
{
    if (GetItemCount() > 0) {
        CheckBox* pCheckBox = dynamic_cast<CheckBox*>(GetItemAt(0));
        if (pCheckBox != nullptr) {
            bool bChanged = pCheckBox->IsSelected() != bSelected;
            pCheckBox->SetSelected(bSelected);
            if (bSelected) {
                if (pCheckBox->IsPartSelected() != bPartSelect) {
                    pCheckBox->SetPartSelected(bPartSelect);
                    bChanged = true;
                }
            }
            if (bChanged) {
                pCheckBox->Invalidate();
            }
            return true;
        }
    }
    return false;
}

bool ListCtrlSubItem::GetCheckBoxSelect(bool& bSelected, bool& bPartSelect) const
{
    bSelected = false;
    bPartSelect = false;
    if (GetItemCount() > 0) {
        CheckBox* pCheckBox = dynamic_cast<CheckBox*>(GetItemAt(0));
        if (pCheckBox != nullptr) {
            bSelected = pCheckBox->IsSelected();
            if (bSelected) {
                bPartSelect = pCheckBox->IsPartSelected();
            }
            return true;
        }
    }
    return false;
}

ListCtrlCheckBox* ListCtrlSubItem::GetCheckBox() const
{
    ListCtrlCheckBox* pCheckBox = nullptr;
    if (GetItemCount() > 0) {
        pCheckBox = dynamic_cast<ListCtrlCheckBox*>(GetItemAt(0));
    }
    return pCheckBox;
}

}//namespace ui


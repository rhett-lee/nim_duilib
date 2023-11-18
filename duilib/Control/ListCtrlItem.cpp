#include "ListCtrlItem.h"
#include "duilib/Control/ListCtrlSubItem.h"
#include "duilib/Control/ListCtrl.h"

namespace ui
{
ListCtrlItem::ListCtrlItem():
    m_bSelectable(true),
    m_pListCtrl(nullptr),
    m_imageId(-1),
    m_nCheckBoxWidth(0)
{
    m_nIconSpacing = GlobalManager::Instance().Dpi().GetScaleInt(2);
}

ListCtrlItem::~ListCtrlItem()
{
}

std::wstring ListCtrlItem::GetType() const { return L"ListCtrlItem"; }

void ListCtrlItem::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"icon_spacing") {
        SetIconSpacing(_wtoi(strValue.c_str()), true);
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

bool ListCtrlItem::IsSelectableType() const
{
    return m_bSelectable;
}

void ListCtrlItem::SetSelectableType(bool bSelectable)
{
    m_bSelectable = bSelectable;
}

void ListCtrlItem::SetListCtrl(ListCtrl* pListCtrl)
{
    m_pListCtrl = pListCtrl;
}

ListCtrl* ListCtrlItem::GetListCtrl() const
{
    ASSERT(m_pListCtrl != nullptr);
    return m_pListCtrl;
}

size_t ListCtrlItem::GetSubItemCount() const
{
    return GetItemCount();
}

ListCtrlSubItem* ListCtrlItem::GetSubItem(size_t columnIndex) const
{
    ASSERT(columnIndex < GetSubItemCount());
    ListCtrlSubItem* pSubItem = dynamic_cast<ListCtrlSubItem*>(GetItemAt(columnIndex));
    ASSERT(pSubItem != nullptr);
    return pSubItem;
}

void ListCtrlItem::Selected(bool bSelect, bool bTriggerEvent)
{
    if (__super::IsSelected() != bSelect) {
        __super::Selected(bSelect, bTriggerEvent);
    }
}

void ListCtrlItem::Activate()
{
    //重写基类的实现逻辑，这里只发出一个Click事件
    if (IsActivatable()) {
        SendEvent(kEventClick);
    }
}

bool ListCtrlItem::ButtonDown(const EventArgs& msg)
{
    if (IsEnabled() && IsActivatable() && IsPointInWithScrollOffset(msg.ptMouse)) {
        uint64_t vkFlag = kVkLButton;
#ifdef UILIB_IMPL_WINSDK
        if (msg.wParam & MK_CONTROL) {
            vkFlag |= kVkControl;
        }
        if (msg.wParam & MK_SHIFT) {
            vkFlag |= kVkShift;
        }
#endif
        //左键按下的时候，选择
        SelectItem(vkFlag);
    }
    return __super::ButtonDown(msg);
}

bool ListCtrlItem::ButtonUp(const EventArgs& msg)
{
    if ((m_pListCtrl != nullptr) && m_pListCtrl->IsAutoCheckSelect()) {
        //跳过CheckBox的处理函数，避免功能冲突
        return HBox::ButtonUp(msg);
    }
    else {
        return __super::ButtonUp(msg);
    }
}

bool ListCtrlItem::ButtonDoubleClick(const EventArgs& msg)
{
    return __super::ButtonDoubleClick(msg);
}

bool ListCtrlItem::RButtonDown(const EventArgs& msg)
{
    uint64_t vkFlag = kVkRButton;
#ifdef UILIB_IMPL_WINSDK
    if (msg.wParam & MK_CONTROL) {
        vkFlag |= kVkControl;
    }
    if (msg.wParam & MK_SHIFT) {
        vkFlag |= kVkShift;
    }
#endif
    //左键按下的时候，选择
    SelectItem(vkFlag);
    return __super::RButtonDown(msg);
}

bool ListCtrlItem::RButtonUp(const EventArgs& msg)
{
    return __super::RButtonUp(msg);
}

bool ListCtrlItem::RButtonDoubleClick(const EventArgs& msg)
{
    return __super::RButtonDoubleClick(msg);
}

void ListCtrlItem::SelectItem(uint64_t vkFlag)
{
    IListBoxOwner* pOwner = GetOwner();
    ASSERT(pOwner != nullptr);
    if (pOwner != nullptr) {
        size_t nListBoxIndex = GetListBoxIndex();
        pOwner->SelectItem(nListBoxIndex, true, true, vkFlag);
    }
}

bool ListCtrlItem::SupportCheckedMode() const
{
    return true;
}

bool ListCtrlItem::SetShowCheckBox(bool bShow)
{
    bool bRet = false;
    if (bShow) {
        if (IsShowCheckBox()) {
            return true;
        }
        ListCtrl* pListCtrl = GetListCtrl();
        if (pListCtrl != nullptr) {
            std::wstring checkBoxClass = pListCtrl->GetCheckBoxClass();
            if (!checkBoxClass.empty()) {
                SetClass(checkBoxClass);
                bRet = IsShowCheckBox();
            }
        }
    }
    else {
        //清除CheckBox图片资源，就不显示了
        ClearStateImages();
        ASSERT(!IsShowCheckBox());
        bRet = true;
    }
    return bRet;
}

bool ListCtrlItem::IsShowCheckBox() const
{
    //如果有CheckBox图片资源，则认为显示了CheckBox
    return !GetStateImage(kControlStateNormal).empty() && !GetSelectedStateImage(kControlStateNormal).empty();
}

int32_t ListCtrlItem::GetCheckBoxImageWidth()
{
    if (GetWindow() == nullptr) {
        return 0;
    }
    UiSize sz = GetStateImageSize(kStateImageBk, kControlStateNormal);
    return sz.cx;
}

void ListCtrlItem::SetImageId(int32_t imageId)
{
    if (m_imageId != imageId) {
        m_imageId = imageId;
        Invalidate();
    }
}

int32_t ListCtrlItem::GetImageId() const
{
    return m_imageId;
}

void ListCtrlItem::Paint(IRender* pRender, const UiRect& rcPaint)
{
    __super::Paint(pRender, rcPaint);
    if (pRender == nullptr) {
        return;
    }
    ImagePtr pItemImage = LoadItemImage();
    if (pItemImage != nullptr) {
        UiRect rc = GetRect();
        rc.Deflate(GetControlPadding());
        int32_t nPaddingLeft = 0;
        if (IsShowCheckBox()) {
            nPaddingLeft += GetCheckBoxImageWidth();
            nPaddingLeft += m_nIconSpacing;
        }
        rc.left += nPaddingLeft;
        rc.Validate();
        PaintImage(pRender, pItemImage.get(), L"", -1, nullptr, &rc, nullptr);
    }
}

ImagePtr ListCtrlItem::LoadItemImage() const
{
    ImagePtr pItemImage;
    if (m_imageId >= 0) {
        ListCtrl* pListCtrl = GetListCtrl();
        if (pListCtrl != nullptr) {
            pItemImage = pListCtrl->GetImageList().GetImageData(m_imageId);
            ASSERT(pItemImage != nullptr);
        }
    }
    if ((pItemImage != nullptr) && (pItemImage->GetImageCache() == nullptr)) {
        LoadImageData(*pItemImage);
        if (pItemImage->GetImageCache() == nullptr) {
            pItemImage = nullptr;
        }
        else {
            if ((pItemImage->GetImageCache()->GetWidth() <= 0) ||
                (pItemImage->GetImageCache()->GetHeight() <= 0)) {
                pItemImage = nullptr;
            }
        }
    }
    return pItemImage;
}

void ListCtrlItem::SetIconSpacing(int32_t nIconSpacing, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nIconSpacing);
    }
    if (m_nIconSpacing != nIconSpacing) {
        m_nIconSpacing = nIconSpacing;
        if (m_nIconSpacing < 0) {
            m_nIconSpacing = 0;
        }
        Invalidate();
    }
}

int32_t ListCtrlItem::GetIconSpacing() const
{
    return m_nIconSpacing;
}

int32_t ListCtrlItem::GetItemPaddingLeft()
{
    int32_t nPaddingLeft = 0;
    if (IsShowCheckBox()) {
        nPaddingLeft += GetCheckBoxImageWidth();
        nPaddingLeft += m_nIconSpacing;
    }
    ImagePtr pItemImage = LoadItemImage();
    if (pItemImage != nullptr) {
        nPaddingLeft += pItemImage->GetImageCache()->GetWidth();
        nPaddingLeft += m_nIconSpacing;
    }
    return nPaddingLeft;
}

}//namespace ui


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

void ListCtrlItem::HandleEvent(const EventArgs& msg)
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

ListCtrlSubItem* ListCtrlItem::GetSubItem(const UiPoint& ptMouse) const
{
    UiPoint pt(ptMouse);
    pt.Offset(GetScrollOffsetInScrollBox());
    ListCtrlSubItem* pFoundSubItem = nullptr;
    size_t nItemCount = GetItemCount();
    for (size_t index = 0; index < nItemCount; ++index) {
        ListCtrlSubItem* pSubItem = dynamic_cast<ListCtrlSubItem*>(GetItemAt(index));
        if (pSubItem != nullptr) {
            if (pSubItem->GetRect().ContainsPt(pt)) {
                pFoundSubItem = pSubItem;
                break;
            }
        }
    }
    return pFoundSubItem;
}

size_t ListCtrlItem::GetSubItemIndex(const UiPoint& ptMouse) const
{
    UiPoint pt(ptMouse);
    pt.Offset(GetScrollOffsetInScrollBox());
    size_t nSubItemIndex = Box::InvalidIndex;
    size_t nItemCount = GetItemCount();
    for (size_t index = 0; index < nItemCount; ++index) {
        ListCtrlSubItem* pSubItem = dynamic_cast<ListCtrlSubItem*>(GetItemAt(index));
        if (pSubItem != nullptr) {
            if (pSubItem->GetRect().ContainsPt(pt)) {
                nSubItemIndex = index;
                break;
            }
        }
    }
    return nSubItemIndex;
}

size_t ListCtrlItem::GetSubItemIndex(ListCtrlSubItem* pSubItem) const
{
    size_t nSubItemIndex = Box::InvalidIndex;
    size_t nItemCount = GetItemCount();
    for (size_t index = 0; index < nItemCount; ++index) {
        if (pSubItem == dynamic_cast<ListCtrlSubItem*>(GetItemAt(index))) {
            nSubItemIndex = index;
            break;
        }
    }
    return nSubItemIndex;
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
        UiSize imageSize;
        ListCtrl* pListCtrl = GetListCtrl();
        if (pListCtrl != nullptr) {
            ImageList* pImageList = pListCtrl->GetImageList(ListCtrlType::Report);
            if (pImageList != nullptr) {
                imageSize = pImageList->GetImageSize();
            }
        }

        if (imageSize.cx <= 0) {
            imageSize.cx = pItemImage->GetImageCache()->GetWidth();
        }
        if (imageSize.cy <= 0) {
            imageSize.cy = pItemImage->GetImageCache()->GetHeight();
        }

        UiRect rc = GetRect();
        rc.Deflate(GetControlPadding());
        int32_t nPaddingLeft = 0;
        if (IsShowCheckBox()) {
            nPaddingLeft += GetCheckBoxImageWidth();
            nPaddingLeft += m_nIconSpacing;
        }
        rc.left += nPaddingLeft;
        rc.Validate();
        rc.right = rc.left + imageSize.cx;
        VAlignRect(rc, TEXT_VCENTER, imageSize.cy);
        PaintImage(pRender, pItemImage.get(), L"", -1, nullptr, &rc, nullptr);
    }
}

ImagePtr ListCtrlItem::LoadItemImage() const
{
    ImagePtr pItemImage;
    if (m_imageId >= 0) {
        ListCtrl* pListCtrl = GetListCtrl();
        if (pListCtrl != nullptr) {
            ImageList* pImageList = pListCtrl->GetImageList(ListCtrlType::Report);
            if (pImageList != nullptr) {
                pItemImage = pImageList->GetImageData(m_imageId);
                ASSERT(pItemImage != nullptr);
            }
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

void ListCtrlItem::VAlignRect(UiRect& rc, uint32_t textStyle, int32_t nImageHeight)
{
    if ((nImageHeight <= 0) || (nImageHeight >= rc.Height())) {
        return;
    }
    if (textStyle & TEXT_VCENTER) {
        //居中对齐
        rc.top = rc.CenterY() - nImageHeight / 2;
        rc.bottom = rc.top + nImageHeight;
    }
    else if (textStyle & TEXT_BOTTOM) {
        //底部对齐
        rc.top = rc.bottom - nImageHeight;
    }
    else {
        //顶部对齐
        rc.bottom = rc.top + nImageHeight;
    }
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


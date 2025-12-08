#include "ListCtrlSubItem.h"
#include "duilib/Control/ListCtrl.h"

namespace ui
{
ListCtrlSubItem::ListCtrlSubItem(Window* pWindow):
    ListCtrlLabel(pWindow),
    m_pItem(nullptr),
    m_imageId(-1),
    m_nIconSpacing(0)
{
    SetIconSpacing(2, true);
}

DString ListCtrlSubItem::GetType() const { return _T("ListCtrlSubItem"); }

void ListCtrlSubItem::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("icon_spacing")) {
        SetIconSpacing(StringUtil::StringToInt32(strValue), true);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void ListCtrlSubItem::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    int32_t iValue = GetIconSpacing();
    iValue = Dpi().GetScaleInt(iValue, nOldDpiScale);
    SetIconSpacing(iValue, false);
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

bool ListCtrlSubItem::SupportCheckMode() const
{
    return true;
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

bool ListCtrlSubItem::SetShowCheckBox(bool bShow)
{
    ListCtrlItem* pItem = GetListCtrlItem();
    if (pItem == nullptr) {
        return false;
    }
    bool bRet = false;
    if (bShow) {
        if (IsShowCheckBox()) {
            return true;
        }
        ListCtrl* pListCtrl = pItem->GetListCtrl();
        if (pListCtrl != nullptr) {
            DString checkBoxClass = pListCtrl->GetCheckBoxClass();
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

bool ListCtrlSubItem::IsShowCheckBox() const
{
    //如果有CheckBox图片资源，则认为显示了CheckBox
    return !GetStateImage(kControlStateNormal).empty() && !GetSelectedStateImage(kControlStateNormal).empty();
}

bool ListCtrlSubItem::IsCheckBoxChecked() const
{
    return IsShowCheckBox() && IsChecked();
}

int32_t ListCtrlSubItem::GetCheckBoxImageWidth()
{
    if (GetWindow() == nullptr) {
        return 0;
    }
    UiSize sz = GetStateImageSize(kStateImageBk, kControlStateNormal);
    return sz.cx;
}

void ListCtrlSubItem::SetImageId(int32_t imageId)
{
    if (m_imageId != imageId) {
        m_imageId = imageId;
        Invalidate();
    }
}

int32_t ListCtrlSubItem::GetImageId() const
{
    return m_imageId;
}

void ListCtrlSubItem::SetIconSpacing(int32_t nIconSpacing, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        Dpi().ScaleInt(nIconSpacing);
    }
    if (m_nIconSpacing != nIconSpacing) {
        m_nIconSpacing = nIconSpacing;
        if (m_nIconSpacing < 0) {
            m_nIconSpacing = 0;
        }
        Invalidate();
    }
}

int32_t ListCtrlSubItem::GetIconSpacing() const
{
    return m_nIconSpacing;
}

ImagePtr ListCtrlSubItem::LoadItemImage() const
{
    ImagePtr pItemImage;
    if ((m_imageId >= 0) && (m_pItem != nullptr)) {
        ListCtrl* pListCtrl = m_pItem->GetListCtrl();
        if (pListCtrl != nullptr) {
            ImageListPtr pImageList = pListCtrl->GetImageList(ListCtrlType::Report);
            if (pImageList != nullptr) {
                pItemImage = pImageList->GetImageData(m_imageId);
                ASSERT(pItemImage != nullptr);
            }
        }
    }
    if (pItemImage != nullptr) {
        LoadImageInfo(*pItemImage);
        std::shared_ptr<ImageInfo> pItemImageCache = pItemImage->GetImageInfo();
        if (pItemImageCache == nullptr) {
            pItemImage = nullptr;
        }
        else {
            if ((pItemImageCache->GetWidth() <= 0) ||
                (pItemImageCache->GetHeight() <= 0)) {
                pItemImage = nullptr;
            }
        }
    }
    return pItemImage;
}

void ListCtrlSubItem::PaintText(IRender* pRender)
{
    //需要绘制的内容包括：图标、文字
    SetTextRect(UiRect());
    if (pRender == nullptr) {
        return;
    }
    //文本前的图标
    ImagePtr pItemImage = LoadItemImage();
    if (pItemImage == nullptr) {
        BaseClass::PaintText(pRender);
        return;
    }

    int32_t nIconTextSpacing = GetIconSpacing();

    //CheckBox的宽度，需要留出来
    int32_t nCheckBoxWidth = 0;
    if (IsShowCheckBox()) {
        nCheckBoxWidth += GetCheckBoxImageWidth();
        nCheckBoxWidth += nIconTextSpacing;
    }

    UiSize imageSize;
    ListCtrl* pListCtrl = nullptr;
    if (m_pItem != nullptr) {
        pListCtrl = m_pItem->GetListCtrl();
    }
    if (pListCtrl != nullptr) {
        ImageListPtr pImageList = pListCtrl->GetImageList(ListCtrlType::Report);
        if (pImageList != nullptr) {
            imageSize = pImageList->GetImageSize();
        }
    }

    if (imageSize.cx <= 0) {
        imageSize.cx = pItemImage->GetImageInfo()->GetWidth();
    }
    if (imageSize.cy <= 0) {
        imageSize.cy = pItemImage->GetImageInfo()->GetHeight();
    }

    //图标靠左侧，文字按原来的方式绘制
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    rc.left += nCheckBoxWidth;
    rc.Validate();
    rc.right = rc.left + imageSize.cx;

    UiRect imageRect = rc;
    VAlignRect(imageRect, GetTextStyle(), imageSize.cy);
    PaintImage(pRender, pItemImage.get(), _T(""), -1, nullptr, &imageRect, nullptr);
    rc.left += imageSize.cx;
    rc.left += nIconTextSpacing;

    UiRect textRect = GetRect();
    textRect.Deflate(GetControlPadding());
    textRect.Deflate(GetTextPadding());
    textRect.left = std::max(textRect.left, rc.left);

    DoPaintText(textRect, pRender);
    SetTextRect(textRect);
}

void ListCtrlSubItem::VAlignRect(UiRect& rc, uint32_t textStyle, int32_t nImageHeight)
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

UiSize ListCtrlSubItem::EstimateText(UiSize szAvailable)
{
    UiSize sz = BaseClass::EstimateText(szAvailable);

    int32_t nIconTextSpacing = GetIconSpacing();

    //CheckBox的宽度，需要留出来
    int32_t nCheckBoxWidth = 0;
    if (IsShowCheckBox()) {
        nCheckBoxWidth += GetCheckBoxImageWidth();
        nCheckBoxWidth += nIconTextSpacing;
    }

    const UiPadding rcPadding = GetControlPadding();
    const UiPadding rcTextPadding = GetTextPadding();

    //图标靠左侧，文字按原来的方式绘制
    UiRect rc = GetRect();
    rc.Deflate(rcPadding);
    rc.left += nCheckBoxWidth;
    rc.Validate();

    ImagePtr pItemImage = LoadItemImage();
    if (pItemImage != nullptr) {
        rc.left += pItemImage->GetImageInfo()->GetWidth();
        rc.left += nIconTextSpacing;
    }   

    UiRect textRect = GetRect();
    textRect.Deflate(rcPadding);
    textRect.Deflate(rcTextPadding);
    textRect.left = std::max(textRect.left, rc.left);

    int32_t nPaddingLeft = rcPadding.left + rcTextPadding.left;
    if (textRect.left > nPaddingLeft) {
        sz.cx -= nPaddingLeft;
        sz.cx += textRect.left;
    }    
    return sz;
}

}//namespace ui


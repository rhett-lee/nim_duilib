#include "ListCtrlSubItem.h"
#include "duilib/Control/ListCtrl.h"

namespace ui
{
ListCtrlSubItem::ListCtrlSubItem(Window* pWindow):
    ListCtrlLabel(pWindow),
    m_pItem(nullptr),
    m_imageId(-1),
    m_nIconSpacing(0),
    m_nColumnId(Box::InvalidIndex),
    m_nProgress(-1),
    m_sProgressColor(_T("bg_progress")),
    m_sProgressBkColor(_T("bg_progress_track")),
    m_sProgressTextPos(_T("center"))
{
    SetIconSpacing(2, true);
}

DString ListCtrlSubItem::GetType() const { return _T("ListCtrlSubItem"); }

void ListCtrlSubItem::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("icon_spacing")) {
        SetIconSpacing(StringUtil::StringToInt32(strValue), true);
    }
    else if ((strName == _T("progress_color")) || (strName == _T("progresscolor"))) {
        if (m_sProgressColor != strValue) {
            m_sProgressColor = strValue;
            Invalidate();
        }
    }
    else if ((strName == _T("progress_bkcolor")) || (strName == _T("progress_bk_color")) || (strName == _T("progressbkcolor"))) {
        if (m_sProgressBkColor != strValue) {
            m_sProgressBkColor = strValue;
            Invalidate();
        }
    }
    else if ((strName == _T("progress_text_pos")) || (strName == _T("progresstextpos")) || (strName == _T("progress_text_position"))) {
        SetProgressTextPos(strValue);
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

size_t ListCtrlSubItem::GetDataItemIndex() const
{
    if (m_pItem != nullptr) {
        return m_pItem->GetDataItemIndex();
    }
    return Box::InvalidIndex;
}

size_t ListCtrlSubItem::GetDataColumnIndex() const
{
    if (m_pItem != nullptr) {
        ListCtrl* pListCtrl = m_pItem->GetListCtrl();
        if (pListCtrl != nullptr) {
            return pListCtrl->GetColumnIndex(m_nColumnId);
        }
    }
    return Box::InvalidIndex;
}

size_t ListCtrlSubItem::GetDataColumnId() const
{
    return m_nColumnId;
}

void ListCtrlSubItem::SetDataColumnId(size_t nColumnId)
{
    m_nColumnId = nColumnId;
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

void ListCtrlSubItem::SetProgress(int32_t nProgress)
{
    if (nProgress > 100) {
        nProgress = 100;
    }
    if (nProgress < -1) {
        nProgress = -1;
    }
    if (m_nProgress != nProgress) {
        m_nProgress = nProgress;
        //进度条与文本共用绘制区域，进度值变化时更新文本区域
        SetTextRect(UiRect());
        Invalidate();
    }
}

int32_t ListCtrlSubItem::GetProgress() const
{
    return m_nProgress;
}

const DString& ListCtrlSubItem::GetProgressColor() const
{
    return m_sProgressColor;
}

const DString& ListCtrlSubItem::GetProgressBkColor() const
{
    return m_sProgressBkColor;
}

void ListCtrlSubItem::SetProgressTextPos(const DString& textPos)
{
    if (m_sProgressTextPos != textPos) {
        m_sProgressTextPos = textPos;
        Invalidate();
    }
}

const DString& ListCtrlSubItem::GetProgressTextPos() const
{
    return m_sProgressTextPos;
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
    if (m_nProgress >= 0) {
        //按进度条方式显示（不绘制图标/普通文本）
        PaintProgress(pRender);
        return;
    }
    //文本前的图标
    ImagePtr pItemImage = LoadItemImage();
    if (pItemImage == nullptr) {
        if (IsShowCheckBox()) {
            //显示CheckBox时，文字需要向右偏移CheckBox的宽度，避免文字与CheckBox重叠
            UiRect textRect = GetRect();
            textRect.Deflate(GetControlPadding());
            textRect.Deflate(GetTextPadding());
            textRect.left += GetCheckBoxImageWidth();
            textRect.left += GetIconSpacing();
            textRect.Validate();
            DoPaintText(textRect, pRender);
            SetTextRect(textRect);
        }
        else {
            BaseClass::PaintText(pRender);
        }
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

void ListCtrlSubItem::PaintProgress(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    //CheckBox的宽度，需要留出来
    int32_t nCheckBoxWidth = 0;
    if (IsShowCheckBox()) {
        nCheckBoxWidth += GetCheckBoxImageWidth();
        nCheckBoxWidth += GetIconSpacing();
    }

    //图标：绘制在进度条的左侧，并为图标预留宽度，避免被进度条覆盖（参考图标分支的处理方式）
    int32_t nIconWidth = 0;
    ImagePtr pItemImage = LoadItemImage();
    if (pItemImage != nullptr) {
        const int32_t nIconTextSpacing = GetIconSpacing();
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

        UiRect rcImage = GetRect();
        rcImage.Deflate(GetControlPadding());
        rcImage.left += nCheckBoxWidth;
        rcImage.Validate();
        rcImage.right = rcImage.left + imageSize.cx;
        VAlignRect(rcImage, GetTextStyle(), imageSize.cy);
        PaintImage(pRender, pItemImage.get(), _T(""), -1, nullptr, &rcImage, nullptr);

        nIconWidth = imageSize.cx + nIconTextSpacing;
    }

    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    rc.Deflate(GetTextPadding());
    rc.left += nCheckBoxWidth;
    rc.left += nIconWidth;
    rc.Validate();
    if (rc.IsEmpty()) {
        return;
    }

    //文本位置模式："center"(默认，在单元格内居中)/"right"(在单元格内右对齐)/"outside"(显示在进度条右侧外部)
    const bool bTextOutside = (m_sProgressTextPos == _T("outside")) && !GetText().empty();
    const int32_t nTextGap = Dpi().GetScaleInt(4);

    //文本区域：默认与单元格内容区域相同；"outside"模式下，文本占用右侧空间，进度条宽度相应缩短
    UiRect rcText = rc;
    if (bTextOutside) {
        const UiSize szText = EstimateText(UiSize(rc.Width(), rc.Height()));
        if ((szText.cx > 0) && (szText.cx + nTextGap < rc.Width())) {
            rcText.left = rc.right - szText.cx;
        }
        else {
            rcText.left = rc.right;
        }
    }

    //进度条高度：保证能容纳文本（默认字体的行高再加少量内边距），垂直居中，上下留白，
    //避免多个单元格的进度条上下相连形成整块色条
    int32_t nBarHeight = Dpi().GetScaleInt(22);
    if (nBarHeight > rc.Height()) {
        nBarHeight = rc.Height();
    }
    if (nBarHeight < Dpi().GetScaleInt(6)) {
        nBarHeight = Dpi().GetScaleInt(6);
    }

    UiRect rcBar = rc;
    if (bTextOutside) {
        rcBar.right = rcText.left - nTextGap;
        if (rcBar.right < rcBar.left) {
            rcBar.right = rcBar.left;
        }
    }
    rcBar.top = rc.CenterY() - nBarHeight / 2;
    rcBar.bottom = rcBar.top + nBarHeight;

    //圆角半径取进度条高度的一半，形成胶囊形
    const float fRoundRadius = static_cast<float>(nBarHeight) / 2.0f;

    //背景轨道
    UiColor trackColor = GetUiColor(m_sProgressBkColor.c_str());
    if ((trackColor.GetARGB() != 0) && (rcBar.Width() > 0)) {
        pRender->FillRoundRect(UiRectF::MakeFromRect(rcBar), fRoundRadius, fRoundRadius, trackColor, GetAlpha());
    }

    //前景进度（按百分比填充）
    UiColor foreColor = GetUiColor(m_sProgressColor.c_str());
    if ((foreColor.GetARGB() != 0) && (rcBar.Width() > 0)) {
        int32_t nProgress = m_nProgress;
        if (nProgress > 100) {
            nProgress = 100;
        }
        else if (nProgress < 0) {
            nProgress = 0;
        }
        int32_t nForeWidth = rcBar.Width() * nProgress / 100;
        if (nForeWidth > 0) {
            float fForeRadius = fRoundRadius;
            if (nForeWidth < nBarHeight) {
                //进度很小时，至少显示为一个圆点，避免出现细长条
                fForeRadius = static_cast<float>(nForeWidth) / 2.0f;
            }
            UiRect rcFore = rcBar;
            rcFore.right = rcFore.left + nForeWidth;
            pRender->FillRoundRect(UiRectF::MakeFromRect(rcFore), fForeRadius, fForeRadius, foreColor, GetAlpha());
        }
    }

    //如果有文本，叠加显示（例如显示百分比文字）；文字强制垂直居中，水平对齐按位置模式设置
    if (!GetText().empty()) {
        const uint32_t nOldTextStyle = GetTextStyle();
        uint32_t nNewTextStyle = nOldTextStyle;
        nNewTextStyle &= ~(TEXT_TOP | TEXT_BOTTOM | TEXT_VJUSTIFY | TEXT_LEFT | TEXT_RIGHT | TEXT_HJUSTIFY | TEXT_HCENTER);
        nNewTextStyle |= TEXT_VCENTER;
        if (bTextOutside || (m_sProgressTextPos == _T("right"))) {
            nNewTextStyle |= TEXT_RIGHT;
        }
        else if (m_sProgressTextPos == _T("left")) {
            nNewTextStyle |= TEXT_LEFT;
        }
        else {
            nNewTextStyle |= TEXT_HCENTER;
        }
        SetTextStyle(nNewTextStyle, false);
        DoPaintText(rcText, pRender);
        SetTextStyle(nOldTextStyle, false);
        SetTextRect(rcText);
    }
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
    if (sz.cx <= 0) {
        //文本为空，或者无法估算时，不需要调整
        return sz;
    }

    //文本左侧需要额外预留的宽度：CheckBox + 图标 + 间距（与PaintText中的绘制逻辑保持一致）
    const int32_t nIconTextSpacing = GetIconSpacing();
    int32_t nExtraLeftWidth = 0;
    if (IsShowCheckBox()) {
        nExtraLeftWidth += GetCheckBoxImageWidth();
        nExtraLeftWidth += nIconTextSpacing;
    }
    ImagePtr pItemImage = LoadItemImage();
    if (pItemImage != nullptr) {
        nExtraLeftWidth += pItemImage->GetImageInfo()->GetWidth();
        nExtraLeftWidth += nIconTextSpacing;
    }

    //估算的宽度 = 文本宽度 + 内边距 + 左侧额外占位宽度
    sz.cx += nExtraLeftWidth;
    return sz;
}

}//namespace ui


#include "ListCtrlHeaderItem.h"
#include "ListCtrl.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

namespace ui
{
ListCtrlHeaderItem::ListCtrlHeaderItem(Window* pWindow) :
    ControlDragableT<ButtonBox>(pWindow),
    m_pSortedDownImage(nullptr),
    m_pSortedUpImage(nullptr),
    m_sortMode(SortMode::kDown),
    m_pSplitBox(nullptr),
    m_bColumnResizeable(true),
    m_nColumnWidth(0),
    m_bShowIconAtTop(true),
    m_bColumnVisible(true),
    m_imageId(-1),
    m_pHeaderCtrl(nullptr),
    m_nIconSpacing(0),
    m_bShowSortImage(false)
{
    SetIconSpacing(6, true);
}

ListCtrlHeaderItem::~ListCtrlHeaderItem()
{
    if (m_pSortedDownImage != nullptr) {
        delete m_pSortedDownImage;
        m_pSortedDownImage = nullptr;
    }
    if (m_pSortedUpImage != nullptr) {
        delete m_pSortedUpImage;
        m_pSortedUpImage = nullptr;
    }
}
 
DString ListCtrlHeaderItem::GetType() const { return _T("ListCtrlHeaderItem"); }

void ListCtrlHeaderItem::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("sorted_up_image")) {
        SetSortedUpImage(strValue);
    }
    else if (strName == _T("sorted_down_image")) {
        SetSortedDownImage(strValue);
    }
    else if (strName == _T("icon_spacing")) {
        SetIconSpacing(StringUtil::StringToInt32(strValue), true);
    }
    else if (strName == _T("show_icon_at_top")) {
        SetShowIconAtTop(strValue == _T("true"));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void ListCtrlHeaderItem::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    int32_t iValue = GetIconSpacing();
    iValue = Dpi().GetScaleInt(iValue, nOldDpiScale);
    SetIconSpacing(iValue, false);

    int32_t nColumnWidth = GetColumnWidth();
    if (nColumnWidth > 0) {
        nColumnWidth = Dpi().GetScaleInt(nColumnWidth, nOldDpiScale);
        SetColumnWidth(nColumnWidth, false);
    }
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void ListCtrlHeaderItem::PaintText(IRender* pRender)
{
    //需要绘制的内容包括：图标、文字、排序图标
    if (pRender == nullptr) {
        return;
    }
    //文本前的图标
    ImagePtr pItemImage;
    UiSize itemImageSize;
    if ((m_imageId >= 0) && (m_pHeaderCtrl != nullptr)) {
        ListCtrl* pListCtrl = m_pHeaderCtrl->GetListCtrl();
        if (pListCtrl != nullptr) {
            ImageListPtr pImageList = pListCtrl->GetImageList(ListCtrlType::Report);
            if (pImageList != nullptr) {
                itemImageSize = pImageList->GetImageSize();
                pItemImage = pImageList->GetImageData(m_imageId);
                ASSERT(pItemImage != nullptr);
            }
        }
    }
    std::shared_ptr<ImageInfo> pItemImageCache;
    if (pItemImage != nullptr) {
        LoadImageInfo(*pItemImage);
        pItemImageCache = pItemImage->GetImageInfo();
        if (pItemImageCache == nullptr) {
            pItemImage = nullptr;
            pItemImageCache.reset();
        }
        else {
            if ((pItemImageCache->GetWidth() <= 0) ||
                (pItemImageCache->GetHeight() <= 0)) {
                pItemImage = nullptr;
                pItemImageCache.reset();
            }
        }
    }

    //排序图标
    Image* pSortImage = nullptr;
    if (IsShowSortImage()) {
        if (m_sortMode == SortMode::kUp) {
            //升序
            pSortImage = m_pSortedUpImage;
        }
        else if (m_sortMode == SortMode::kDown) {
            //降序
            pSortImage = m_pSortedDownImage;
        }
    }

    std::shared_ptr<ImageInfo> pSortImageCache;
    if (pSortImage != nullptr) {
        LoadImageInfo(*pSortImage);
        pSortImageCache = pSortImage->GetImageInfo();
        if (pSortImageCache == nullptr) {
            pSortImage = nullptr;
            pSortImageCache.reset();
        }
        else {
            if ((pSortImageCache->GetWidth() <= 0) ||
                (pSortImageCache->GetHeight() <= 0)) {
                pSortImage = nullptr;
                pSortImageCache.reset();
            }
        }
    }

    if ((pSortImageCache != nullptr) && IsShowIconAtTop()) {
        //图标显示在文字的上方，居中显示
        UiRect rc = GetRect();
        rc.Deflate(GetControlPadding());
        int32_t nImageWidth = pSortImageCache->GetWidth();
        int32_t nImageHeight = pSortImageCache->GetHeight();
        rc.left = rc.CenterX() - nImageWidth / 2;
        rc.right = rc.left + nImageWidth;
        if (!(GetTextStyle() & TEXT_VCENTER) && !(GetTextStyle() & TEXT_BOTTOM)) {
            rc.top = rc.bottom - nImageHeight;
        }
        else {
            rc.bottom = rc.top + nImageHeight;
        }

        //绘制排序图标
        PaintImage(pRender, pSortImage, _T(""), -1, nullptr, &rc, nullptr);
        pSortImage = nullptr;
    }

    if ((pSortImage == nullptr) && (pItemImage == nullptr)) {
        BaseClass::PaintText(pRender);
        return;
    }

    if (pItemImageCache != nullptr) {
        if (itemImageSize.cx <= 0) {
            itemImageSize.cx = pItemImageCache->GetWidth();
        }
        if (itemImageSize.cy <= 0) {
            itemImageSize.cy = pItemImageCache->GetHeight();
        }
    }

    UiSize sortImageSize;
    if (pSortImageCache != nullptr) {
        sortImageSize.cx = pSortImageCache->GetWidth();
        sortImageSize.cy = pSortImageCache->GetHeight();
    }

    int32_t nIconTextSpacing = GetIconSpacing();
    //CheckBox的宽度，需要留出来
    int32_t nCheckBoxWidth = 0;
    ListCtrlCheckBox* pCheckBox = nullptr;
    if (GetItemCount() > 0) {
        pCheckBox = dynamic_cast<ListCtrlCheckBox*>(GetItemAt(0));
    }
    if ((pCheckBox != nullptr) && pCheckBox->IsVisible()) {
        UiSize sz = pCheckBox->GetStateImageSize(kStateImageBk, kControlStateNormal);
        nCheckBoxWidth += sz.cx;
        nCheckBoxWidth += nIconTextSpacing;
    }

    uint32_t textStyle = GetTextStyle();
    UiRect measureRect = pRender->MeasureString(GetText(), GetIFontById(GetFontId()), textStyle);
    UiRect rcItemRect = GetRect();
    rcItemRect.Deflate(GetControlPadding());
    if (nCheckBoxWidth > 0) {
        rcItemRect.left += nCheckBoxWidth;
        rcItemRect.Validate();
    }
    
    if ((sortImageSize.cx + itemImageSize.cx + measureRect.Width()) > rcItemRect.Width()) {
        //横向的空间不足，按左对齐绘制
        nIconTextSpacing = 0;
        textStyle = TEXT_LEFT;
    }
    
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    if (nCheckBoxWidth > 0) {
        rc.left += nCheckBoxWidth;
        rc.Validate();
    }
    if (textStyle & TEXT_CENTER) {
        //居中对齐
        UiRect textRect = GetRect();
        textRect.Deflate(GetControlPadding());
        textRect.Deflate(GetTextPadding());
        if (pItemImage != nullptr) {
            UiRect itemRect = rc;
            itemRect.left = textRect.CenterX() - measureRect.Width() / 2;
            itemRect.left -= nIconTextSpacing;
            itemRect.left -= itemImageSize.cx;
            itemRect.left = std::max(itemRect.left, rc.left);
            itemRect.Validate();
            itemRect.right = itemRect.left + itemImageSize.cx;
            VAlignRect(itemRect, GetTextStyle(), itemImageSize.cy);
            PaintImage(pRender, pItemImage.get(), _T(""), -1, nullptr, &itemRect, nullptr);
        }
        if (pSortImage != nullptr) {
            UiRect sortRect = rc;
            sortRect.left = textRect.CenterX() + measureRect.Width() / 2;
            sortRect.left += nIconTextSpacing;
            sortRect.right = sortRect.left + sortImageSize.cx;
            sortRect.Validate();
            VAlignRect(sortRect, GetTextStyle(), sortImageSize.cy);
            PaintImage(pRender, pSortImage, _T(""), -1, nullptr, &sortRect, nullptr);
        }

        DoPaintText(textRect, pRender);
    }
    else if (textStyle & TEXT_RIGHT) {
        //靠右对齐
        if (pSortImage != nullptr) {
            UiRect sortRect = rc;
            sortRect.left = sortRect.right - sortImageSize.cx;
            sortRect.Validate();
            VAlignRect(sortRect, GetTextStyle(), sortImageSize.cy);
            PaintImage(pRender, pSortImage, _T(""), -1, nullptr, &sortRect, nullptr);
            rc.right = sortRect.left;
            rc.right -= nIconTextSpacing;
            rc.Validate();
        }

        UiRect textRect = GetRect();
        textRect.Deflate(GetControlPadding());
        textRect.Deflate(GetTextPadding());
        textRect.right = std::min(rc.right, textRect.right);
        if (textRect.Width() > measureRect.Width()) {
            textRect.left = textRect.right - measureRect.Width();
        }
        DoPaintText(textRect, pRender);

        if (pItemImage != nullptr) {
            rc.right = textRect.left;
            rc.right -= nIconTextSpacing;
            rc.Validate();

            if (rc.Width() > itemImageSize.cx) {
                rc.left = rc.right - itemImageSize.cx;
                rc.Validate();
            }
            UiRect itemRect = rc;
            VAlignRect(itemRect, GetTextStyle(), itemImageSize.cy);
            PaintImage(pRender, pItemImage.get(), _T(""), -1, nullptr, &itemRect, nullptr);
        }
    }
    else {
        //靠左对齐：图标、文字、排序图标依次绘制
        if (pItemImage != nullptr) {
            UiRect itemRect = rc;
            itemRect.right = itemRect.left + itemImageSize.cx;
            VAlignRect(itemRect, GetTextStyle(), itemImageSize.cy);
            PaintImage(pRender, pItemImage.get(), _T(""), -1, nullptr, &itemRect, nullptr);
            rc.left += itemImageSize.cx;
            rc.left += nIconTextSpacing;
        }

        UiRect textRect = GetRect();
        textRect.Deflate(GetControlPadding());
        textRect.Deflate(GetTextPadding());
        if (pItemImage != nullptr) {
            textRect.left = std::max(textRect.left, rc.left);
        }
        DoPaintText(textRect, pRender);

        rc.left = textRect.left;
        rc.left += measureRect.Width();
        rc.left += nIconTextSpacing;

        if (pSortImage != nullptr) {
            UiRect sortRect = rc;
            VAlignRect(sortRect, GetTextStyle(), sortImageSize.cy);
            PaintImage(pRender, pSortImage, _T(""), -1, nullptr, &sortRect, nullptr);
        }
    }
}

void ListCtrlHeaderItem::VAlignRect(UiRect& rc, uint32_t textStyle, int32_t nImageHeight)
{
    if ((nImageHeight <= 0) || (nImageHeight >= rc.Height())){
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

void ListCtrlHeaderItem::Activate(const EventArgs* pMsg)
{
    if (IsInDraggingOrder() || IsInDraggingOut()) {
        //处于拖动改变列顺序的状态
        return;
    }
    if (!this->IsActivatable()) {
        return;
    }
    bool bSortChanged = false;
    if (m_sortMode == SortMode::kUp) {
        m_sortMode = SortMode::kDown;
        bSortChanged = true;
        Invalidate();
    }
    else if (m_sortMode == SortMode::kDown) {
        m_sortMode = SortMode::kUp;
        bSortChanged = true;
        Invalidate();        
    }

    if (bSortChanged) {
        ListCtrlHeader* pHeader = GetHeaderCtrl();
        if (pHeader != nullptr) {
            pHeader->OnHeaderColumnSorted(this);
        }
    }
    BaseClass::Activate(pMsg);
}

void ListCtrlHeaderItem::SetFadeVisible(bool bVisible)
{
    BaseClass::SetFadeVisible(bVisible);
}

void ListCtrlHeaderItem::SetVisible(bool bVisible)
{
    BaseClass::SetVisible(bVisible);
}

void ListCtrlHeaderItem::SetSortMode(SortMode sortMode, bool bTriggerEvent)
{
    if (m_sortMode != sortMode) {
        m_sortMode = sortMode;
        Invalidate();
        if (bTriggerEvent && (m_sortMode != SortMode::kNone)) {
            ListCtrlHeader* pHeader = GetHeaderCtrl();
            if (pHeader != nullptr) {
                pHeader->OnHeaderColumnSorted(this);
            }
        } 
    }
}

ListCtrlHeaderItem::SortMode ListCtrlHeaderItem::GetSortMode() const
{
    return m_sortMode;
}

void ListCtrlHeaderItem::SetSortedDownImage(const DString& sImageString)
{
    if (m_pSortedDownImage == nullptr) {
        m_pSortedDownImage = new Image;
    }
    m_pSortedDownImage->SetImageString(sImageString, Dpi());
    Invalidate();
}

void ListCtrlHeaderItem::SetSortedUpImage(const DString& sImageString)
{
    if (m_pSortedUpImage == nullptr) {
        m_pSortedUpImage = new Image;
    }
    m_pSortedUpImage->SetImageString(sImageString, Dpi());
    Invalidate();
}

void ListCtrlHeaderItem::SetShowSortImage(bool bShowSortImage)
{
    if (m_bShowSortImage != bShowSortImage) {
        m_bShowSortImage = bShowSortImage;
        Invalidate();
    }
}

bool ListCtrlHeaderItem::IsShowSortImage() const
{
    return m_bShowSortImage;
}

size_t ListCtrlHeaderItem::GetColumnId() const
{
    return (size_t)this;
}

void ListCtrlHeaderItem::SetSplitBox(SplitBox* pSplitBox)
{
    m_pSplitBox = pSplitBox;
    if (pSplitBox != nullptr) {
        ASSERT(pSplitBox->GetFixedWidth().IsInt32());
        pSplitBox->SetEnabled(IsColumnResizeable() ? true : false);
    }
    if (GetColumnWidth() > 0) {
        CheckColumnWidth();
    }
}

SplitBox* ListCtrlHeaderItem::GetSplitBox() const
{
    return m_pSplitBox;
}

void ListCtrlHeaderItem::SetColumnResizeable(bool bResizeable)
{
    m_bColumnResizeable = bResizeable;
    if (m_pSplitBox != nullptr) {
        m_pSplitBox->SetEnabled(IsColumnResizeable() ? true : false);
    }
}

bool ListCtrlHeaderItem::IsColumnResizeable() const
{
    return m_bColumnResizeable;
}

void ListCtrlHeaderItem::SetColumnWidth(int32_t nWidth, bool bNeedDpiScale)
{
    if (nWidth < 0) {
        nWidth = 0;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(nWidth);
    }
    m_nColumnWidth = nWidth;
    CheckColumnWidth();
}

int32_t ListCtrlHeaderItem::GetColumnWidth() const
{
    return m_nColumnWidth;
}

void ListCtrlHeaderItem::CheckColumnWidth()
{
    int32_t nSplitWidth = 0;
    if (m_pSplitBox != nullptr) {
        ASSERT(m_pSplitBox->GetFixedWidth().IsInt32());
        nSplitWidth = m_pSplitBox->GetFixedWidth().GetInt32();
    }
    int32_t nWidth = GetFixedWidth().GetInt32();
    if ((nWidth + nSplitWidth) != GetColumnWidth()) {
        nWidth = GetColumnWidth() - nSplitWidth;
        if (nWidth < 0) {
            nWidth = 0;
        }
        SetFixedWidth(UiFixedInt(nWidth), true, false);
    }
}

void ListCtrlHeaderItem::SetIconSpacing(int32_t nIconSpacing, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        Dpi().ScaleInt(nIconSpacing);
    }
    if (m_nIconSpacing != nIconSpacing) {
        m_nIconSpacing = ui::TruncateToInt16(nIconSpacing);
        if (m_nIconSpacing < 0) {
            m_nIconSpacing = 0;
        }
        Invalidate();
    }    
}

int32_t ListCtrlHeaderItem::GetIconSpacing() const
{
    return m_nIconSpacing;
}

void ListCtrlHeaderItem::SetShowIconAtTop(bool bShowIconAtTop)
{
    if (m_bShowIconAtTop != bShowIconAtTop) {
        m_bShowIconAtTop = bShowIconAtTop;
        Invalidate();
    }    
}

bool ListCtrlHeaderItem::IsShowIconAtTop() const
{
    return m_bShowIconAtTop;
}

void ListCtrlHeaderItem::SetTextHorAlign(HorAlignType alignType)
{
    uint32_t textStyle = GetTextStyle();
    if (alignType == HorAlignType::kHorAlignCenter) {
        //文本：居中对齐
        textStyle &= ~(TEXT_LEFT | TEXT_RIGHT);
        textStyle |= TEXT_CENTER;
    }
    else if (alignType == HorAlignType::kHorAlignRight) {
        //文本：右对齐
        textStyle &= ~(TEXT_LEFT | TEXT_CENTER);
        textStyle |= TEXT_RIGHT;
    }
    else {
        //文本：左对齐
        textStyle &= ~(TEXT_CENTER | TEXT_RIGHT);
        textStyle |= TEXT_LEFT;
    }
    SetTextStyle(textStyle, true);
}

HorAlignType ListCtrlHeaderItem::GetTextHorAlign() const
{
    HorAlignType alignType = HorAlignType::kHorAlignLeft;//文本：左对齐
    uint32_t textStyle = GetTextStyle();
    if (textStyle & TEXT_CENTER) {
        //文本：居中对齐
        alignType = HorAlignType::kHorAlignCenter;
    }
    else if (textStyle & TEXT_RIGHT) {
        //文本：右对齐
        alignType = HorAlignType::kHorAlignRight;
    }
    return alignType;
}

void ListCtrlHeaderItem::SetImageId(int32_t imageId)
{
    if (m_imageId != imageId) {
        m_imageId = imageId;
        Invalidate();
    }    
}

int32_t ListCtrlHeaderItem::GetImageId() const
{
    return m_imageId;
}

bool ListCtrlHeaderItem::IsEnableDragOrder() const
{
    ListCtrlHeader* pHeader = GetHeaderCtrl();
    if (pHeader != nullptr) {
        if (!pHeader->IsEnableHeaderDragOrder()) {
            //不支持拖动调整顺序
            return false;
        }
    }
    return BaseClass::IsEnableDragOrder();
}

bool ListCtrlHeaderItem::SetCheckBoxVisible(bool bVisible)
{
    bool bRet = false;
    if (bVisible) {
        ListCtrlHeader* pHeader = GetHeaderCtrl();
        if (pHeader == nullptr) {
            return false;
        }
        ListCtrl* pListCtrl = pHeader->GetListCtrl();
        ListCtrlCheckBox* pCheckBox = nullptr;
        if (GetItemCount() > 0) {
            pCheckBox = dynamic_cast<ListCtrlCheckBox*>(GetItemAt(0));
        }
        if (pCheckBox == nullptr) {
            pCheckBox = new ListCtrlCheckBox(GetWindow());
            AddItem(pCheckBox);
            DString checkBoxClass;
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
            textPadding.left += nCheckBoxWidth;
            SetTextPadding(textPadding, false);
        }
        pCheckBox->SetVisible(true);

        //挂载CheckBox的事件处理
        pCheckBox->DetachEvent(kEventSelect);
        pCheckBox->DetachEvent(kEventUnSelect);
        //同步数据
        if (pListCtrl != nullptr) {
            pListCtrl->UpdateHeaderColumnCheckBox(GetColumnId());
        }
        pCheckBox = dynamic_cast<ListCtrlCheckBox*>(GetItemAt(0));
        if (pCheckBox != nullptr) {
            pCheckBox->AttachSelect([this, pHeader](const EventArgs& /*args*/) {
                pHeader->OnHeaderColumnCheckStateChanged(this, true);
                return true;
                });
            pCheckBox->AttachUnSelect([this, pHeader](const EventArgs& /*args*/) {
                pHeader->OnHeaderColumnCheckStateChanged(this, false);
                return true;
                });
        }        
        bRet = true;
    }
    else if(GetItemCount() > 0) {
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

bool ListCtrlHeaderItem::IsCheckBoxVisible() const
{
    if (GetItemCount() > 0) {
        CheckBox* pCheckBox = dynamic_cast<CheckBox*>(GetItemAt(0));
        if (pCheckBox != nullptr) {
            return pCheckBox->IsVisible();
        }
    }
    return false;
}

bool ListCtrlHeaderItem::SetCheckBoxCheck(bool bChecked, bool bPartChecked)
{
    if (GetItemCount() > 0) {
        CheckBox* pCheckBox = dynamic_cast<CheckBox*>(GetItemAt(0));
        if (pCheckBox != nullptr) {
            bool bChanged = pCheckBox->IsSelected() != bChecked;
            pCheckBox->SetSelected(bChecked);
            if (bChecked) {
                if (pCheckBox->IsPartSelected() != bPartChecked) {
                    pCheckBox->SetPartSelected(bPartChecked);
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

bool ListCtrlHeaderItem::GetCheckBoxCheck(bool& bChecked, bool& bPartChecked) const
{
    bChecked = false;
    bPartChecked = false;
    if (GetItemCount() > 0) {
        CheckBox* pCheckBox = dynamic_cast<CheckBox*>(GetItemAt(0));
        if (pCheckBox != nullptr) {
            bChecked = pCheckBox->IsSelected();
            if (bChecked) {
                bPartChecked = pCheckBox->IsPartSelected();
            }
            return true;
        }
    }
    return false;
}

void ListCtrlHeaderItem::SetColumnVisible(bool bColumnVisible)
{
    m_bColumnVisible = bColumnVisible;
    SetVisible(bColumnVisible);
    if (m_pSplitBox != nullptr) {
        m_pSplitBox->SetVisible(bColumnVisible);
    }
    ListCtrlHeader* pHeader = GetHeaderCtrl();
    if (pHeader != nullptr) {
        pHeader->OnHeaderColumnVisibleChanged();
    }
}

bool ListCtrlHeaderItem::IsColumnVisible() const
{
    //有别与IsVisible(), 当表头隐藏的时候，IsVisible()也会返回false
    return m_bColumnVisible;
}

void ListCtrlHeaderItem::SetHeaderCtrl(ListCtrlHeader* pHeaderCtrl)
{
    m_pHeaderCtrl = pHeaderCtrl;
}

ListCtrlHeader* ListCtrlHeaderItem::GetHeaderCtrl() const
{
    ASSERT(m_pHeaderCtrl != nullptr);
    return m_pHeaderCtrl;
}

void ListCtrlHeaderItem::AdjustItemPos(const UiPoint& pt, const UiPoint& ptMouseDown,
                                       const std::vector<ItemStatus>& rcItemList) const
{
    Control* pMouseItem = nullptr;
    size_t nMouseItemIndex = Box::InvalidIndex;
    size_t nMouseDownItemIndex = Box::InvalidIndex;
    for (const ItemStatus& itemStatus : rcItemList) {
        if (itemStatus.m_rcPos.ContainsPt(pt)) {
            pMouseItem = itemStatus.m_pItem;
            nMouseItemIndex = itemStatus.m_index;
        }
        if (itemStatus.m_pItem == this) {
            nMouseDownItemIndex = itemStatus.m_index;
        }
    }
    if ((pMouseItem == nullptr) ||
        (nMouseItemIndex == Box::InvalidIndex) ||
        (nMouseDownItemIndex == Box::InvalidIndex)) {
        return;
    }
    ListCtrlHeaderItem* pHeaderItem = dynamic_cast<ListCtrlHeaderItem*>(pMouseItem);
    if (pHeaderItem == nullptr) {
        //鼠标不在表头控件上
        return;
    }
    if (!pHeaderItem->IsEnableDragOrder()) {
        //当前列为固定列，不允许调整顺序
        return;
    }

    const size_t itemCount = rcItemList.size();
    int32_t xOffset = pt.x - ptMouseDown.x;
    if (pMouseItem == this) {
        //当前鼠标位置：在自身的位置，恢复各个控件的实际位置
        for (const ItemStatus& item : rcItemList) {
            if (item.m_pItem == this) {
                continue;
            }
            if (item.m_pItem->GetRect() != item.m_rcPos) {
                item.m_pItem->SetPos(item.m_rcPos);
            }
        }
    }
    else if (xOffset < 0) {
        //当前鼠标位置：在按下点的左侧，向右侧移动控件
        for (size_t index = 0; index < itemCount; ++index) {
            const ItemStatus& item = rcItemList[index];
            if (item.m_pItem == this) {
                //恢复关联的Split控件位置
                if ((index + 1) < itemCount) {
                    const ItemStatus& nextItem = rcItemList[index + 1];
                    if ((nextItem.m_pItem->GetRect() != nextItem.m_rcPos)) {
                        nextItem.m_pItem->SetPos(nextItem.m_rcPos);
                    }
                }
                continue;
            }
            else if ((item.m_index >= nMouseItemIndex) && (item.m_index < nMouseDownItemIndex)) {
                //向右侧移动
                if ((index + 2) < itemCount) {
                    const ItemStatus& nextItem = rcItemList[index + 2];
                    item.m_pItem->SetPos(nextItem.m_rcPos);
                }
                else {
                    if (item.m_pItem->GetRect() != item.m_rcPos) {
                        item.m_pItem->SetPos(item.m_rcPos);
                    }
                }
            }
            else {
                //恢复原位置
                if (item.m_pItem->GetRect() != item.m_rcPos) {
                    item.m_pItem->SetPos(item.m_rcPos);
                }
            }
        }
    }
    else {
        //当前鼠标位置：在按下点的右侧，向左侧移动控件
        for (size_t index = 0; index < itemCount; ++index) {
            const ItemStatus& item = rcItemList[index];
            if (item.m_pItem == this) {
                //恢复关联的Split控件位置
                if ((index + 1) < itemCount) {
                    const ItemStatus& nextItem = rcItemList[index + 1];
                    if ((nextItem.m_pItem->GetRect() != nextItem.m_rcPos)) {
                        nextItem.m_pItem->SetPos(nextItem.m_rcPos);
                    }
                }
                continue;
            }
            else if ((item.m_index > nMouseDownItemIndex) && (item.m_index <= nMouseItemIndex)) {
                //向左侧移动
                if ((index - 2) < itemCount) {
                    const ItemStatus& nextItem = rcItemList[index - 2];
                    item.m_pItem->SetPos(nextItem.m_rcPos);
                }
                else {
                    if (item.m_pItem->GetRect() != item.m_rcPos) {
                        item.m_pItem->SetPos(item.m_rcPos);
                    }
                }
            }
            else {
                //恢复原位置
                if (item.m_pItem->GetRect() != item.m_rcPos) {
                    item.m_pItem->SetPos(item.m_rcPos);
                }
            }
        }
    }
}

bool ListCtrlHeaderItem::AdjustItemOrders(const UiPoint& pt,
                                          const std::vector<ItemStatus>& rcItemList,
                                          size_t& nOldItemIndex,
                                          size_t& nNewItemIndex)
{
    nOldItemIndex = Box::InvalidIndex;
    nNewItemIndex = Box::InvalidIndex;
    Box* pHeader = GetParent();
    if (pHeader == nullptr) {
        return false;
    }
    bool bOrderChanged = false;
    const size_t itemCount = pHeader->GetItemCount();
    size_t nMouseItemIndex = Box::InvalidIndex;
    size_t nCurrentItemIndex = Box::InvalidIndex;
    for (const ItemStatus& itemStatus : rcItemList) {
        if (itemStatus.m_rcPos.ContainsPt(pt)) {
            nMouseItemIndex = itemStatus.m_index;
            ListCtrlHeaderItem* pHeaderItem = dynamic_cast<ListCtrlHeaderItem*>(itemStatus.m_pItem);
            if ((pHeaderItem != nullptr) && !pHeaderItem->IsEnableDragOrder()) {
                //当前列为固定列，不允许调整顺序
                nMouseItemIndex = Box::InvalidIndex;
            }
        }
        if ((itemStatus.m_pItem == this) && !itemStatus.m_rcPos.ContainsPt(pt)) {
            nCurrentItemIndex = itemStatus.m_index;
        }
    }
    if ((nMouseItemIndex != Box::InvalidIndex) &&
        (nCurrentItemIndex != Box::InvalidIndex) &&
        (nMouseItemIndex < itemCount) &&
        (nCurrentItemIndex < itemCount)) {
        //交换控件的位置
        if (nMouseItemIndex < nCurrentItemIndex) {
            //向左侧交换
            pHeader->SetItemIndex(this, nMouseItemIndex);
            if (m_pSplitBox != nullptr) {
                size_t nNewIndex = pHeader->GetItemIndex(this);
                ASSERT(nNewIndex < itemCount);
                ASSERT((nNewIndex + 1) < itemCount);
                if ((nNewIndex + 1) < itemCount) {
                    pHeader->SetItemIndex(m_pSplitBox, nNewIndex + 1);
                }
            }
        }
        else {
            //向右侧交换
            nMouseItemIndex += 1;
            if (nMouseItemIndex >= itemCount) {
                //已经到最后一列，将其放在最后
                nMouseItemIndex = itemCount - 1;
            }
            ASSERT(nMouseItemIndex < itemCount);
            if (nMouseItemIndex < itemCount) {
                pHeader->SetItemIndex(this, nMouseItemIndex);
                if (m_pSplitBox != nullptr) {
                    size_t nNewIndex = pHeader->GetItemIndex(this);
                    ASSERT(nNewIndex < itemCount);
                    pHeader->SetItemIndex(m_pSplitBox, nNewIndex);
                }
            }
        }
        nOldItemIndex = nCurrentItemIndex;
        nNewItemIndex = nMouseItemIndex;
        bOrderChanged = true;
        ASSERT(pHeader->GetItemIndex(this) == (pHeader->GetItemIndex(m_pSplitBox) - 1));

        //交换后，对所有的项进行校验
        for (size_t index = 0; index < itemCount; index += 2) {
            ASSERT(dynamic_cast<ListCtrlHeaderItem*>(pHeader->GetItemAt(index)) != nullptr);
            ASSERT((index + 1) < itemCount);
            if ((index + 1) >= itemCount) {
                break;
            }
            ASSERT(dynamic_cast<SplitBox*>(pHeader->GetItemAt(index + 1)) != nullptr);
            ASSERT(dynamic_cast<SplitBox*>(pHeader->GetItemAt(index + 1)) ==
                   dynamic_cast<ListCtrlHeaderItem*>(pHeader->GetItemAt(index))->m_pSplitBox);
        }
    }

    return bOrderChanged;
}

void ListCtrlHeaderItem::OnItemOrdersChanged(size_t /*nOldItemIndex*/, size_t /*nNewItemIndex*/)
{
    ListCtrlHeader* pHeader = GetHeaderCtrl();
    if (pHeader != nullptr) {
        pHeader->OnHeaderColumnOrderChanged();
    }
}

}//namespace ui


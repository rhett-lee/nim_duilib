#include "ListCtrlItem.h"
#include "duilib/Control/ListCtrlSubItem.h"
#include "duilib/Control/ListCtrl.h"

namespace ui
{
ListCtrlItem::ListCtrlItem(Window* pWindow):
    ListCtrlItemBaseH(pWindow),
    m_bSelectable(true),
    m_pListCtrl(nullptr),
    m_imageId(-1),
    m_nIconSpacing(0)
{
    SetIconSpacing(2, true);
}

ListCtrlItem::~ListCtrlItem()
{
}

DString ListCtrlItem::GetType() const { return _T("ListCtrlItem"); }

void ListCtrlItem::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("icon_spacing")) {
        SetIconSpacing(StringUtil::StringToInt32(strValue), true);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void ListCtrlItem::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    int32_t iValue = GetIconSpacing();
    iValue = Dpi().GetScaleInt(iValue, nOldDpiScale);
    SetIconSpacing(iValue, false);
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void ListCtrlItem::HandleEvent(const EventArgs& msg)
{
    BaseClass::HandleEvent(msg);
    if (m_pListCtrl != nullptr) {
        if ((msg.eventType > kEventKeyBegin) && (msg.eventType < kEventKeyEnd)) {
            m_pListCtrl->OnViewKeyboardEvents(msg);
        }
        else if ((msg.eventType > kEventMouseBegin) && (msg.eventType < kEventMouseEnd)) {
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

size_t ListCtrlItem::GetDataItemIndex() const
{
    return GetElementIndex();
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
            if (pSubItem->IsVisible() && pSubItem->GetRect().ContainsPt(pt)) {
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
        return BaseClass::ButtonUp(msg);
    }
}

UiPoint ListCtrlItem::GetCheckBoxMousePos(const UiPoint& ptMouse) const
{
    UiPoint pt(ptMouse);
    pt.Offset(GetScrollOffsetInScrollBox());
    //行首的CheckBox和图标固定显示在行的最左侧，位于冻结列区域内（不随横向滚动移动）：
    //存在冻结列时，横向的滚动量按0计，使点击判定与冻结列的显示位置保持一致
    if ((m_pListCtrl != nullptr) && (m_pListCtrl->GetFrozenColumnCount() > 0) && IsShowCheckBox()) {
        pt.x = ptMouse.x;
    }
    return pt;
}

bool ListCtrlItem::ButtonDown(const EventArgs& msg)
{
    if ((m_pListCtrl != nullptr) &&
        (m_pListCtrl->GetSelectionMode() == ListCtrlSelectionMode::kCell)) {
        //单元格选择模式下，跳过行选择逻辑（SelectItem），避免与单元格选择冲突
        return HBox::ButtonDown(msg);
    }
    else {
        return BaseClass::ButtonDown(msg);
    }
}

bool ListCtrlItem::MouseEnter(const EventArgs& msg)
{
    if ((m_pListCtrl != nullptr) && (m_pListCtrl->GetSelectionMode() == ListCtrlSelectionMode::kCell)) {
        //单元格选择模式下，行级悬停高亮（含 hovered 渐隐/渐显动画）属于行选择模式专属视觉：
        //此处不执行基类逻辑（不置 Hovered 状态、不启动悬停动画），但仍返回 false 让事件继续冒泡给父视图，
        //保证视图（ScrollBox）能收到 MouseEnter 并淡入自动隐藏的滚动条
        return false;
    }
    return BaseClass::MouseEnter(msg);
}

bool ListCtrlItem::MouseLeave(const EventArgs& msg)
{
    if ((m_pListCtrl != nullptr) && (m_pListCtrl->GetSelectionMode() == ListCtrlSelectionMode::kCell)) {
        //单元格选择模式下，不执行基类逻辑（否则会启动 hovered 渐隐动画，使整行悬停色"闪现"一下）；
        //仅清除可能由 SetFocus()（Control::OnSetFocus 会把状态置为 Hovered）残留的行级悬停状态
        if (GetState() == kControlStateHovered) {
            SetState(kControlStateNormal);
        }
        return false;
    }
    return BaseClass::MouseLeave(msg);
}

bool ListCtrlItem::SupportCheckMode() const
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

bool ListCtrlItem::IsShowCheckBox() const
{
    //如果有CheckBox图片资源，则认为显示了CheckBox
    return !GetStateImage(kControlStateNormal).empty() && !GetSelectedStateImage(kControlStateNormal).empty();
}

bool ListCtrlItem::IsCheckBoxChecked() const
{
    return IsShowCheckBox() && IsChecked();
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
    BaseClass::Paint(pRender, rcPaint);
    if (pRender == nullptr) {
        return;
    }
    ImagePtr pItemImage = LoadItemImage();
    if (pItemImage != nullptr) {
        UiSize imageSize;
        ListCtrl* pListCtrl = GetListCtrl();
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

        UiRect rc = GetRect();
        rc.Deflate(GetControlPadding());
        int32_t nPaddingLeft = 0;
        if (IsShowCheckBox()) {
            nPaddingLeft += GetCheckBoxImageWidth();
            nPaddingLeft += GetIconSpacing();
        }
        rc.left += nPaddingLeft;
        rc.Validate();
        rc.right = rc.left + imageSize.cx;
        VAlignRect(rc, TEXT_VCENTER, imageSize.cy);
        PaintImage(pRender, pItemImage.get(), _T(""), -1, nullptr, &rc, nullptr);
    }
}

void ListCtrlItem::PaintStateColors(IRender* pRender)
{
    //单元格选择模式下，行级悬停(Enter)/按下(Pressed)产生的高亮背景属于行选择模式专属视觉，这里直接屏蔽不绘制；
    //其它状态（普通、选中、禁用等）以及行选择模式下的行为保持不变
    if ((m_pListCtrl != nullptr) &&
        (m_pListCtrl->GetSelectionMode() == ListCtrlSelectionMode::kCell) &&
        ((GetState() == kControlStateHovered) || (GetState() == kControlStatePressed))) {
        return;
    }
    BaseClass::PaintStateColors(pRender);
}

ImagePtr ListCtrlItem::LoadItemImage() const
{
    ImagePtr pItemImage;
    if (m_imageId >= 0) {
        ListCtrl* pListCtrl = GetListCtrl();
        if (pListCtrl != nullptr) {
            ImageListPtr pImageList = pListCtrl->GetImageList(ListCtrlType::Report);
            if (pImageList != nullptr) {
                pItemImage = pImageList->GetImageData(m_imageId);
                //ASSERT(pItemImage != nullptr);
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

int32_t ListCtrlItem::GetIconSpacing() const
{
    return m_nIconSpacing;
}

int32_t ListCtrlItem::GetItemPaddingLeft()
{
    int32_t nPaddingLeft = 0;
    if (IsShowCheckBox()) {
        nPaddingLeft += GetCheckBoxImageWidth();
        nPaddingLeft += GetIconSpacing();
    }
    ImagePtr pItemImage = LoadItemImage();
    if (pItemImage != nullptr) {
        UiSize imageSize;
        ListCtrl* pListCtrl = GetListCtrl();
        if (pListCtrl != nullptr) {
            ImageListPtr pImageList = pListCtrl->GetImageList(ListCtrlType::Report);
            if (pImageList != nullptr) {
                imageSize = pImageList->GetImageSize();
            }
        }
        if (imageSize.cx <= 0) {
            imageSize.cx = pItemImage->GetImageInfo()->GetWidth();
        }
        nPaddingLeft += imageSize.cx;
        nPaddingLeft += GetIconSpacing();
    }
    return nPaddingLeft;
}

}//namespace ui


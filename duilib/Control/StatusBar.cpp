#include "StatusBar.h"
#include "duilib/Render/IRender.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Control/LabelImpl.h"

namespace ui
{

StatusBar::StatusBar(Window* pWindow) :
    HBox(pWindow),
    m_nSeparatorWidth(1),
    m_separatorColor(_T("border_window")),
    m_bShowSizeGrip(false),
    m_bSizeGripResize(true),
    m_sizeGripColor(_T("border_window")),
    m_bSizeBoxBackupValid(false)
{
}

DString StatusBar::GetType() const { return DUI_CTR_STATUS_BAR; }

void StatusBar::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if ((strName == _T("separator_width")) || (strName == _T("separatorwidth"))) {
        SetSeparatorWidth(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("separator_color")) || (strName == _T("separatorcolor"))) {
        SetSeparatorColor(strValue);
    }
    else if ((strName == _T("size_grip")) || (strName == _T("sizegrip"))) {
        SetShowSizeGrip(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("size_grip_resize")) || (strName == _T("sizegripresize"))) {
        SetSizeGripResize(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("size_grip_color")) || (strName == _T("sizegripcolor"))) {
        SetSizeGripColor(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

size_t StatusBar::GetPanelCount() const
{
    size_t nCount = 0;
    const size_t nItemCount = GetItemCount();
    for (size_t nIndex = 0; nIndex < nItemCount; ++nIndex) {
        Control* pItem = GetItemAt(nIndex);
        if ((pItem != nullptr) && pItem->IsVisible()) {
            ++nCount;
        }
    }
    return nCount;
}

Control* StatusBar::GetPanel(size_t nIndex) const
{
    size_t nSeen = 0;
    const size_t nItemCount = GetItemCount();
    for (size_t i = 0; i < nItemCount; ++i) {
        Control* pItem = GetItemAt(i);
        if ((pItem == nullptr) || !pItem->IsVisible()) {
            continue;
        }
        if (nSeen == nIndex) {
            return pItem;
        }
        ++nSeen;
    }
    return nullptr;
}

int32_t StatusBar::GetPanelIndex(Control* pPanel) const
{
    if (pPanel == nullptr) {
        return -1;
    }
    int32_t nIndex = 0;
    const size_t nItemCount = GetItemCount();
    for (size_t i = 0; i < nItemCount; ++i) {
        Control* pItem = GetItemAt(i);
        if ((pItem == nullptr) || !pItem->IsVisible()) {
            continue;
        }
        if (pItem == pPanel) {
            return nIndex;
        }
        ++nIndex;
    }
    return -1;
}

Control* StatusBar::FindPanel(const DString& strName) const
{
    if (strName.empty()) {
        return nullptr;
    }
    const size_t nItemCount = GetItemCount();
    for (size_t i = 0; i < nItemCount; ++i) {
        Control* pItem = GetItemAt(i);
        if ((pItem != nullptr) && pItem->IsVisible() && (pItem->GetName() == strName)) {
            return pItem;
        }
    }
    return nullptr;
}

bool StatusBar::SetPanelText(size_t nIndex, const DString& strText)
{
    Control* pPanel = GetPanel(nIndex);
    if (pPanel == nullptr) {
        return false;
    }
    LabelOwner* pTextPanel = dynamic_cast<LabelOwner*>(pPanel);
    if (pTextPanel == nullptr) {
        //该面板不是文本类控件（如 Progress / 自定义自绘面板），无法设置文本
        return false;
    }
    pTextPanel->SetText(strText);
    return true;
}

DString StatusBar::GetPanelText(size_t nIndex) const
{
    Control* pPanel = GetPanel(nIndex);
    if (pPanel == nullptr) {
        return DString();
    }
    LabelOwner* pTextPanel = dynamic_cast<LabelOwner*>(pPanel);
    if (pTextPanel == nullptr) {
        return DString();
    }
    return pTextPanel->GetText();
}

void StatusBar::SetSeparatorWidth(int32_t nWidth)
{
    if (m_nSeparatorWidth != nWidth) {
        m_nSeparatorWidth = nWidth;
        Invalidate();
    }
}

int32_t StatusBar::GetSeparatorWidth() const
{
    return m_nSeparatorWidth;
}

void StatusBar::SetSeparatorColor(const DString& strColor)
{
    if (m_separatorColor != strColor) {
        m_separatorColor = strColor;
        Invalidate();
    }
}

DString StatusBar::GetSeparatorColor() const
{
    return m_separatorColor.c_str();
}

void StatusBar::SetShowSizeGrip(bool bShow)
{
    if (m_bShowSizeGrip != bShow) {
        m_bShowSizeGrip = bShow;
        Invalidate();
        //手柄开关变化后，需要同步刷新窗口的可拖拽缩放区域
        UpdateSizeGripHitArea();
    }
}

bool StatusBar::IsShowSizeGrip() const
{
    return m_bShowSizeGrip;
}

void StatusBar::SetSizeGripResize(bool bResize)
{
    if (m_bSizeGripResize != bResize) {
        m_bSizeGripResize = bResize;
        if (!bResize) {
            //关闭拖拽缩放时，立刻还原此前放大的窗口 size_box
            RestoreSizeBox();
        }
        else {
            UpdateSizeGripHitArea();
        }
    }
}

bool StatusBar::IsSizeGripResize() const
{
    return m_bSizeGripResize;
}

void StatusBar::SetSizeGripColor(const DString& strColor)
{
    if (m_sizeGripColor != strColor) {
        m_sizeGripColor = strColor;
        Invalidate();
    }
}

DString StatusBar::GetSizeGripColor() const
{
    return m_sizeGripColor.c_str();
}

void StatusBar::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    //先绘制全部子控件（各个状态面板）
    BaseClass::PaintChild(pRender, rcPaint);

    if (pRender == nullptr) {
        return;
    }
    UiRect rcContent = GetRect();
    if (rcContent.IsEmpty()) {
        return;
    }
    rcContent.Deflate(GetPadding());

    //分隔线在子控件之上绘制，确保不被面板背景遮盖
    PaintSeparator(pRender, rcContent);
    if (m_bShowSizeGrip) {
        PaintSizeGrip(pRender, rcContent);
    }
}

void StatusBar::PaintSeparator(IRender* pRender, const UiRect& rcContent)
{
    const int32_t nWidth = Dpi().GetScaleInt(m_nSeparatorWidth);
    if (nWidth <= 0) {
        return;
    }

    //按布局顺序收集可见子控件的矩形
    std::vector<UiRect> itemRects;
    const size_t nCount = GetItemCount();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        Control* pItem = GetItemAt(nIndex);
        if ((pItem != nullptr) && pItem->IsVisible() && !pItem->GetRect().IsEmpty()) {
            itemRects.push_back(pItem->GetRect());
        }
    }
    if (itemRects.size() < 2) {
        //少于两个面板时没有分隔线
        return;
    }

    const UiColor color = GetUiColor(m_separatorColor.c_str());

    //分隔线上下留出少量间距，避免贴满整个状态栏
    const int32_t nInset = Dpi().GetScaleInt(2);
    int32_t nTop = rcContent.top + nInset;
    int32_t nBottom = rcContent.bottom - nInset;
    if (nBottom <= nTop) {
        nTop = rcContent.top;
        nBottom = rcContent.bottom;
    }

    for (size_t nIndex = 1; nIndex < itemRects.size(); ++nIndex) {
        //取相邻两个面板的中间位置作为分隔线位置
        const int32_t x = (itemRects[nIndex - 1].right + itemRects[nIndex].left) / 2;
        UiRect rcLine;
        rcLine.left = x;
        rcLine.right = x + nWidth;
        rcLine.top = nTop;
        rcLine.bottom = nBottom;
        if ((rcLine.right <= rcContent.left) || (rcLine.left >= rcContent.right)) {
            //分隔线落在内容区之外，跳过
            continue;
        }
        pRender->FillRect(UiRectF::MakeFromRect(rcLine), color);
    }
}

void StatusBar::PaintSizeGrip(IRender* pRender, const UiRect& rcContent)
{
    const int32_t nMargin = Dpi().GetScaleInt(2);
    const int32_t nStep = Dpi().GetScaleInt(3);
    //连第一条斜线都放不下时，整个手柄不绘制
    if ((rcContent.right - nMargin - nStep <= rcContent.left) ||
        (rcContent.bottom - nMargin - nStep <= rcContent.top)) {
        return;
    }

    const UiColor color = GetUiColor(m_sizeGripColor.c_str());
    const float fLineWidth = Dpi().GetScaleFloat(1.0f);

    const int32_t xRight = rcContent.right - nMargin;
    const int32_t yBottom = rcContent.bottom - nMargin;
    for (int32_t i = 0; i < 3; ++i) {
        const int32_t nOffset = i * nStep;
        const int32_t x1 = xRight - nOffset - nStep;
        const int32_t y1 = yBottom;
        const int32_t x2 = xRight;
        const int32_t y2 = yBottom - nOffset - nStep;
        if ((x1 < rcContent.left) || (y2 < rcContent.top)) {
            //剩余空间不足，停止绘制
            break;
        }
        pRender->DrawLine(UiPointF((float)x1, (float)y1), UiPointF((float)x2, (float)y2), color, fLineWidth);
    }
}

UiRect StatusBar::GetSizeGripRect(const UiRect& rcContent) const
{
    const int32_t nMargin = Dpi().GetScaleInt(2);
    const int32_t nStep = Dpi().GetScaleInt(3);
    //三条斜线构成一个 3*nStep 见方的区域，右下角锚点见 PaintSizeGrip
    UiRect rcGrip;
    rcGrip.left = rcContent.right - nMargin - 3 * nStep;
    rcGrip.top = rcContent.bottom - nMargin - 3 * nStep;
    rcGrip.right = rcContent.right - nMargin;
    rcGrip.bottom = rcContent.bottom - nMargin;
    if ((rcGrip.right <= rcGrip.left) || (rcGrip.bottom <= rcGrip.top)) {
        //空间不足，手柄不绘制
        rcGrip.Clear();
    }
    return rcGrip;
}

int32_t StatusBar::GetSizeGripHitSize() const
{
    const int32_t nDefault = Dpi().GetScaleInt(12);
    UiRect rcContent = GetRect();
    if (rcContent.IsEmpty()) {
        return nDefault > 0 ? nDefault : 1;
    }
    rcContent.Deflate(GetPadding());

    const UiRect rcGrip = GetSizeGripRect(rcContent);
    if (rcGrip.IsEmpty()) {
        return nDefault > 0 ? nDefault : 1;
    }

    //命中区域取"手柄到内容区右下角"的距离，再加 1 逻辑像素余量
    const int32_t nDx = rcContent.right - rcGrip.left;
    const int32_t nDy = rcContent.bottom - rcGrip.top;
    int32_t nNeed = (nDx > nDy) ? nDx : nDy;
    nNeed += Dpi().GetScaleInt(1);
    return nNeed > 0 ? nNeed : 1;
}

void StatusBar::SetPos(UiRect rc)
{
    BaseClass::SetPos(rc);
    //布局位置变化后，手柄可能移入/移出窗口右下角，需要刷新拖拽命中区域
    UpdateSizeGripHitArea();
}

void StatusBar::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
    //DPI 变化后窗口会自行缩放 size_box，此前的备份值已失效，直接按新比例重新计算
    m_bSizeBoxBackupValid = false;
    UpdateSizeGripHitArea();
}

void StatusBar::UpdateSizeGripHitArea()
{
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return;
    }

    //判断本状态栏是否停靠在窗口客户区的右下角（允许 1 逻辑像素的布局误差）
    bool bAtRightBottom = false;
    if (m_bShowSizeGrip && m_bSizeGripResize && pWindow->IsWindow()) {
        const UiRect rcSelf = GetRect();
        if (!rcSelf.IsEmpty()) {
            UiRect rcClient;
            pWindow->GetClientRect(rcClient);
            const int32_t nTolerance = Dpi().GetScaleInt(1);
            const int32_t nDx = rcSelf.right > rcClient.right ? (rcSelf.right - rcClient.right)
                                                             : (rcClient.right - rcSelf.right);
            const int32_t nDy = rcSelf.bottom > rcClient.bottom ? (rcSelf.bottom - rcClient.bottom)
                                                               : (rcClient.bottom - rcSelf.bottom);
            bAtRightBottom = (nDx <= nTolerance) && (nDy <= nTolerance);
        }
    }

    if (!bAtRightBottom) {
        //不在右下角（或手柄已关闭），还原此前放大的窗口 size_box
        RestoreSizeBox();
        return;
    }

    const int32_t nNeed = GetSizeGripHitSize();
    UiRect rcSizeBox = pWindow->GetSizeBox();
    if ((rcSizeBox.right >= nNeed) && (rcSizeBox.bottom >= nNeed)) {
        //窗口的可拉伸区域已足够覆盖手柄，无需改动
        return;
    }

    if (!m_bSizeBoxBackupValid) {
        //记录原始值，便于本控件不再停靠右下角时还原
        m_rcSizeBoxBackup = rcSizeBox;
        m_bSizeBoxBackupValid = true;
    }
    if (rcSizeBox.right < nNeed) {
        rcSizeBox.right = nNeed;
    }
    if (rcSizeBox.bottom < nNeed) {
        rcSizeBox.bottom = nNeed;
    }
    m_rcSizeBoxApplied = rcSizeBox;
    //传入的已是 DPI 缩放后的像素值，不需要再次缩放
    pWindow->SetSizeBox(rcSizeBox, false);
}

void StatusBar::RestoreSizeBox()
{
    if (!m_bSizeBoxBackupValid) {
        return;
    }
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        const UiRect rcCurrent = pWindow->GetSizeBox();
        bool bUnchanged = (rcCurrent.left == m_rcSizeBoxApplied.left) &&
                          (rcCurrent.top == m_rcSizeBoxApplied.top) &&
                          (rcCurrent.right == m_rcSizeBoxApplied.right) &&
                          (rcCurrent.bottom == m_rcSizeBoxApplied.bottom);
        if (bUnchanged) {
            //只有当前值仍是我们写入的值时才还原，避免覆盖业务侧自己的设置
            pWindow->SetSizeBox(m_rcSizeBoxBackup, false);
        }
    }
    m_bSizeBoxBackupValid = false;
}

} // namespace ui

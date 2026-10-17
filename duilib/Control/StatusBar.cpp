#include "StatusBar.h"
#include "duilib/Render/IRender.h"
#include "duilib/Core/DpiManager.h"

namespace ui
{

StatusBar::StatusBar(Window* pWindow) :
    HBox(pWindow),
    m_nSeparatorWidth(1),
    m_separatorColor(_T("border_window")),
    m_bShowSizeGrip(false),
    m_sizeGripColor(_T("border_window"))
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
    else if ((strName == _T("size_grip_color")) || (strName == _T("sizegripcolor"))) {
        SetSizeGripColor(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
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
    }
}

bool StatusBar::IsShowSizeGrip() const
{
    return m_bShowSizeGrip;
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
    const UiColor color = GetUiColor(m_sizeGripColor.c_str());
    const int32_t nMargin = Dpi().GetScaleInt(2);
    const int32_t nStep = Dpi().GetScaleInt(3);
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

} // namespace ui

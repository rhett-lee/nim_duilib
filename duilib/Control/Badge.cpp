#include "Badge.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{

Badge::Badge(Window* pWindow) :
    BaseClass(pWindow),
    m_nCount(0),
    m_nMaxCount(99),
    m_bDotMode(false),
    m_strBadgeColor(_T("bg_badge"))
{
}

Badge::~Badge()
{
}

DString Badge::GetType() const
{
    return DUI_CTR_BADGE;
}

void Badge::SetAttribute(const DString& strName, const DString& strValue)
{
    if ((strName == _T("count")) || (strName == _T("badge_count"))) {
        SetCount(StringUtil::StringToInt64(strValue));
    }
    else if ((strName == _T("max_count")) || (strName == _T("maxcount"))) {
        SetMaxCount(StringUtil::StringToInt64(strValue));
    }
    else if (strName == _T("dot")) {
        SetDotMode(strValue == _T("true"));
    }
    else if ((strName == _T("badge_color")) || (strName == _T("badgecolor"))) {
        SetBadgeColor(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Badge::SetCount(int64_t nCount)
{
    if (m_nCount == nCount) {
        return;
    }
    m_nCount = nCount;
    UpdateBadge();
}

void Badge::SetMaxCount(int64_t nMaxCount)
{
    if (nMaxCount < 1) {
        nMaxCount = 1;
    }
    if (m_nMaxCount == nMaxCount) {
        return;
    }
    m_nMaxCount = nMaxCount;
    UpdateBadge();
}

void Badge::SetDotMode(bool bDotMode)
{
    if (m_bDotMode == bDotMode) {
        return;
    }
    m_bDotMode = bDotMode;
    UpdateBadge();
}

void Badge::SetBadgeColor(const DString& strColor)
{
    m_strBadgeColor = strColor;
    Invalidate();
}

void Badge::UpdateBadge()
{
    if (m_bDotMode) {
        // 红点模式：不显示数字
        SetText(_T(""));
    }
    else if (m_nCount > m_nMaxCount) {
        SetText(StringUtil::Printf(_T("%lld+"), static_cast<long long>(m_nMaxCount)));
    }
    else {
        SetText(StringUtil::Printf(_T("%lld"), static_cast<long long>(m_nCount)));
    }
    // 数字模式：数量小于等于0时自动隐藏（无未读不显示角标）；红点模式的显隐由 visible 属性控制
    SetVisible(m_bDotMode || (m_nCount > 0));
    Invalidate();
}

void Badge::PaintStateColors(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    if ((rc.Width() <= 0) || (rc.Height() <= 0)) {
        return;
    }
    // 圆角胶囊背景（红点模式下宽高相等，即为圆形）
    const float fRound = (float)rc.Height() / 2.0f;
    pRender->FillRoundRect(UiRectF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom),
                           fRound, fRound, GetUiColor(m_strBadgeColor), GetAlpha());
}

} // namespace ui

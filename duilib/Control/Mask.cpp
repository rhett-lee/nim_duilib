#include "Mask.h"
#include "duilib/Core/ControlFinder.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/StringUtil.h"

#include <algorithm>

namespace ui
{

Mask::Mask(Window* pWindow) :
    BaseClass(pWindow),
    m_strMaskColor(_T("bg_overlay")),
    m_nHighlightPadding(0),
    m_nHighlightCorner(0),
    m_bClickThrough(false)
{
    SetHighlightPadding(4);
}

Mask::~Mask()
{
}

DString Mask::GetType() const
{
    return DUI_CTR_MASK;
}

void Mask::SetAttribute(const DString& strName, const DString& strValue)
{
    if ((strName == _T("mask_color")) || (strName == _T("maskcolor"))) {
        SetMaskColor(strValue);
    }
    else if ((strName == _T("highlight_control")) || (strName == _T("highlightcontrol"))) {
        SetHighlightControl(strValue);
    }
    else if ((strName == _T("highlight_padding")) || (strName == _T("highlightpadding"))) {
        SetHighlightPadding(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("highlight_corner")) || (strName == _T("highlightcorner"))) {
        SetHighlightCorner(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("click_through")) || (strName == _T("clickthrough"))) {
        SetClickThrough(StringUtil::IsValueTrue(strValue));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Mask::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    //镂空区域内边距与圆角半径按"DPI缩放后的物理像素"存储，DPI变化时需按旧缩放比换算到新缩放比
    m_nHighlightPadding = Dpi().GetScaleInt(m_nHighlightPadding, nOldDpiScale);
    m_nHighlightCorner = Dpi().GetScaleInt(m_nHighlightCorner, nOldDpiScale);
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void Mask::SetMaskColor(const DString& strColor)
{
    if (m_strMaskColor != strColor) {
        m_strMaskColor = strColor;
        Invalidate();
    }
}

void Mask::SetHighlightControl(const DString& strControlName)
{
    if (m_strHighlightControl != strControlName) {
        m_strHighlightControl = strControlName;
        Invalidate();
    }
}

void Mask::SetHighlightPadding(int32_t nPadding, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        nPadding = Dpi().GetScaleInt(nPadding);
    }
    if (nPadding < 0) {
        nPadding = 0;
    }
    if (m_nHighlightPadding != nPadding) {
        m_nHighlightPadding = nPadding;
        Invalidate();
    }
}

void Mask::SetHighlightCorner(int32_t nCorner, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        nCorner = Dpi().GetScaleInt(nCorner);
    }
    if (nCorner < 0) {
        nCorner = 0;
    }
    if (m_nHighlightCorner != nCorner) {
        m_nHighlightCorner = nCorner;
        Invalidate();
    }
}

void Mask::SetClickThrough(bool bThrough)
{
    m_bClickThrough = bThrough;
}

bool Mask::GetHighlightRect(UiRect& rcHighlight)
{
    rcHighlight.Clear();
    if (m_strHighlightControl.empty()) {
        return false;
    }
    //注意：不能用 Control::FindControl(name)，它只在自身子树内查找；
    //高亮目标通常是 Mask 之外的兄弟控件，需从窗口根节点开始查找
    Control* pControl = nullptr;
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        pControl = pWindow->FindControl(m_strHighlightControl);
    }
    if (pControl == nullptr) {
        return false;
    }
    UiRect rc = pControl->GetRect();
    if (rc.IsEmpty()) {
        return false;
    }
    rc.Inflate(m_nHighlightPadding, m_nHighlightPadding);
    //限制在本控件范围内
    const UiRect rcSelf = GetRect();
    rc.left = std::max(rc.left, rcSelf.left);
    rc.top = std::max(rc.top, rcSelf.top);
    rc.right = std::min(rc.right, rcSelf.right);
    rc.bottom = std::min(rc.bottom, rcSelf.bottom);
    rc.Validate();
    if (rc.IsEmpty()) {
        return false;
    }
    rcHighlight = rc;
    return true;
}

void Mask::AddRoundRectPath(IPath* pPath, const UiRectF& rc, float rx, float ry)
{
    ASSERT(pPath != nullptr);
    if (pPath == nullptr) {
        return;
    }
    rx = std::min(rx, rc.Width() / 2);
    ry = std::min(ry, rc.Height() / 2);
    rx = std::max(rx, 0.0f);
    ry = std::max(ry, 0.0f);
    if ((rx <= 0.01f) || (ry <= 0.01f)) {
        pPath->AddRect(rc);
        return;
    }
    //顺时针绘制：左上角圆弧 -> 上边 -> 右上角 -> 右边 -> 右下角 -> 下边 -> 左下角 -> 左边
    pPath->AddArc(UiRectF(rc.left, rc.top, rc.left + rx * 2, rc.top + ry * 2), 180.0f, 90.0f);
    pPath->AddLine(rc.left + rx, rc.top, rc.right - rx, rc.top);
    pPath->AddArc(UiRectF(rc.right - rx * 2, rc.top, rc.right, rc.top + ry * 2), 270.0f, 90.0f);
    pPath->AddLine(rc.right, rc.top + ry, rc.right, rc.bottom - ry);
    pPath->AddArc(UiRectF(rc.right - rx * 2, rc.bottom - ry * 2, rc.right, rc.bottom), 0.0f, 90.0f);
    pPath->AddLine(rc.right - rx, rc.bottom, rc.left + rx, rc.bottom);
    pPath->AddArc(UiRectF(rc.left, rc.bottom - ry * 2, rc.left + rx * 2, rc.bottom), 90.0f, 90.0f);
    pPath->AddLine(rc.left, rc.bottom - ry, rc.left, rc.top + ry);
    pPath->Close();
}

void Mask::PaintBkColor(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    UiColor maskColor = GetUiColor(m_strMaskColor);
    if (maskColor.IsEmpty()) {
        return;
    }
    const UiRect rc = GetRect();
    if (rc.IsEmpty()) {
        return;
    }

    UiRect rcHighlight;
    if (!GetHighlightRect(rcHighlight)) {
        //无镂空：直接填充整个矩形
        pRender->FillRect(UiRectF::MakeFromRect(rc), maskColor);
        return;
    }

    //有镂空：用"外矩形 + 内圆角矩形"组成偶奇填充路径，挖出高亮区域
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory == nullptr) {
        pRender->FillRect(UiRectF::MakeFromRect(rc), maskColor);
        return;
    }
    std::unique_ptr<IPath> pPath(pRenderFactory->CreatePath());
    if (pPath == nullptr) {
        pRender->FillRect(UiRectF::MakeFromRect(rc), maskColor);
        return;
    }
    pPath->SetFillType(IPath::FillType::kEvenOdd);
    pPath->AddRect(UiRectF::MakeFromRect(rc));
    const float fRadius = (float)m_nHighlightCorner;
    AddRoundRectPath(pPath.get(), UiRectF::MakeFromRect(rcHighlight), fRadius, fRadius);

    std::unique_ptr<IBrush> pBrush(pRenderFactory->CreateBrush(maskColor));
    if (pBrush != nullptr) {
        pRender->FillPath(pPath.get(), pBrush.get());
    }
}

Control* Mask::FindControl(FINDCONTROLPROC Proc, void* pProcData,
                           uint32_t uFlags, const UiPoint& ptMouse, const UiPoint& scrollPos)
{
    if (((uFlags & UIFIND_HITTEST) != 0) && m_bClickThrough) {
        UiRect rcHighlight;
        if (GetHighlightRect(rcHighlight) && rcHighlight.ContainsPt(ptMouse)) {
            //点击落在镂空区域：返回nullptr，使外层容器继续查找下层控件（实现点击穿透）
            return nullptr;
        }
    }
    return BaseClass::FindControl(Proc, pProcData, uFlags, ptMouse, scrollPos);
}

Mask* Mask::ShowOverWindow(Window* pWindow)
{
    if (pWindow == nullptr) {
        return nullptr;
    }
    Box* pRoot = pWindow->GetXmlRoot();
    if (pRoot == nullptr) {
        return nullptr;
    }
    Mask* pMask = new Mask(pWindow);
    pMask->SetFloat(true);
    pMask->SetFixedWidth(UiFixedInt::MakeStretch(), true, false);
    pMask->SetFixedHeight(UiFixedInt::MakeStretch(), true, false);
    pMask->SetPaintOrder(255);
    if (!pRoot->AddItem(pMask)) {
        delete pMask;
        return nullptr;
    }
    return pMask;
}

void Mask::Remove()
{
    Box* pParent = GetParent();
    if (pParent == nullptr) {
        return;
    }
    //注意：如果父容器开启了自动销毁子控件，RemoveItem 会销毁本对象，此后不可再访问本指针
    pParent->RemoveItem(this);
}

} // namespace ui

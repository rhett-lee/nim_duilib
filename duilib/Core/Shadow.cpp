#include "Shadow.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ClickThrough.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"

namespace ui 
{

class ShadowBox : public Box
{
    typedef Box BaseClass;
public:
    ShadowBox(Window* pWindow, Shadow* pShadow):
        Box(pWindow),
        m_pShadow(pShadow)
    {
        //关闭控件自身的内边距，否则阴影绘制不出来
        SetEnableControlPadding(false);
    }
    virtual DString GetType() const override { return _T("ShadowBox"); }

    //绘制容器内的子控件
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override
    {
        UiRect rcTemp;
        if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
            return;
        }
        UiPadding rcPadding;
        if (m_pShadow != nullptr) {
            rcPadding = m_pShadow->GetCurrentShadowCorner();
        }
        UiRect rcRect = GetRect();
        rcRect.Deflate(rcPadding);
        //设置客户区剪辑区域，避免覆盖阴影
        AutoClip rectClip(pRender, rcRect, true);

        UiSize borderRound = m_pShadow->GetShadowBorderRound();
        if (GetWindow() != nullptr) {
            GetWindow()->Dpi().ScaleSize(borderRound);
        }
        float fRoundWidth = (float)borderRound.cx;
        float fRoundHeight = (float)borderRound.cy;
        bool bRoundClip = (borderRound.cx > 0) && (borderRound.cy > 0);
        if (rcPadding.IsEmpty()) {
            bRoundClip = false;
        }
        //设置圆角客户区剪辑区域，避免覆盖阴影
        AutoClip roundClip(pRender, rcRect, fRoundWidth, fRoundHeight, bRoundClip);

        //绘制子控件
        BaseClass::PaintChild(pRender, rcPaint);
    }

    /** 计算控件大小(宽和高)
        如果设置了图片并设置 width 或 height 任意一项为 auto，将根据图片大小和文本大小来计算最终大小
     *  @param [in] szAvailable 可用大小，不包含内边距，不包含外边距
     *  @return 控件的估算大小，包含内边距(Box)，不包含外边距
     */
    virtual UiEstSize EstimateSize(UiSize szAvailable) override
    {
        UiFixedSize fixedSize = GetFixedSize();
        if (!fixedSize.cx.IsAuto() && !fixedSize.cy.IsAuto()) {
            //如果宽高都不是auto属性，则直接返回
            return MakeEstSize(fixedSize);
        }
        szAvailable.Validate();
        if (!IsReEstimateSize(szAvailable)) {
            //使用缓存中的估算结果
            return GetEstimateSize();
        }

        //子控件的大小，包含内边距，但不包含外边距; 包含了阴影本身的大小（即Box的内边距）
        UiSize sizeByChild = GetLayout()->EstimateSizeByChild(m_items, szAvailable);

        SetReEstimateSize(false);
        for (auto pControl : m_items) {
            ASSERT(pControl != nullptr);
            if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
                continue;
            }
            if ((pControl->GetFixedWidth().IsAuto()) ||
                (pControl->GetFixedHeight().IsAuto())) {
                if (pControl->IsReEstimateSize(szAvailable)) {
                    SetReEstimateSize(true);
                    break;
                }
            }
        }
        if (fixedSize.cx.IsAuto()) {
            fixedSize.cx.SetInt32(sizeByChild.cx);
        }
        if (fixedSize.cy.IsAuto()) {
            fixedSize.cy.SetInt32(sizeByChild.cy);
        }

        UiEstSize estSize = MakeEstSize(fixedSize);
        SetEstimateSize(estSize, szAvailable);
        return estSize;
    }

    virtual void PaintBkImage(IRender* pRender) override
    {
        if (!m_pShadow->IsEnableShadowSnap() || !m_pShadow->IsShadowAttached()) {
            BaseClass::PaintBkImage(pRender);
        }
        else {
            Image* pBkImage = GetBkImagePtr();
            Window* pWindow = GetWindow();
            if ((pBkImage != nullptr) && (pWindow != nullptr)) {
                UiRect destRect = GetRect();
                UiPadding rcShadowCorner = pWindow->GetCurrentShadowCorner();
                UiPadding rcRealCorner = m_pShadow->GetShadowCorner();
                pWindow->Dpi().ScalePadding(rcRealCorner);

                //窗口贴边时，阴影需要拉伸到窗口边缘
                if (rcShadowCorner.top == 0) {
                    destRect.top -= rcRealCorner.top;
                }
                if (rcShadowCorner.left == 0) {
                    destRect.left -= rcRealCorner.left;
                }
                if (rcShadowCorner.right == 0) {
                    destRect.right += rcRealCorner.right;
                }
                if (rcShadowCorner.bottom == 0) {
                    destRect.bottom += rcRealCorner.bottom;
                }
                PaintImage(pRender, pBkImage, _T(""), DUI_NOSET_VALUE, nullptr, &destRect);
            }
        }
    }

    virtual bool ButtonDown(const EventArgs& msg) override
    {
        OnMouseDown(msg.ptMouse);
        return true;
    }

    virtual bool RButtonDown(const EventArgs& msg) override
    {
        OnMouseDown(msg.ptMouse);
        return true;
    }

private:
    /** 鼠标点击事件
    */
    void OnMouseDown(UiPoint ptMouse)
    {
        UiPoint pt = ptMouse;
        //判断是否在阴影上
        if ((m_pShadow == nullptr) || !m_pShadow->IsShadowAttached()) {
            return;
        }
        Box* pShadowRoot = dynamic_cast<Box*>(m_pShadow->GetRoot());
        if (pShadowRoot == nullptr) {
            return;
        }
        Box* pRoot = nullptr;
        if (pShadowRoot->GetItemCount() > 0) {
            pRoot = dynamic_cast<Box*>(pShadowRoot->GetItemAt(0));
        }
        if (pRoot == nullptr) {
            return;
        }
        UiRect rcRoot = pRoot->GetPos();
        if (rcRoot.ContainsPt(pt)) {
            return;
        }

        ClientToScreen(pt);
        OnMouseClickShadow(pt);
    }

    /** 鼠标点击在阴影上
    * @param [in] ptMouse 鼠标点击的点（屏幕坐标）
    */
    void OnMouseClickShadow(UiPoint ptMouse) const
    {
        ClickThrough shadowClick;
        shadowClick.ClickThroughWindow(GetWindow(), ptMouse);
    }

private:
    //关联的阴影控件
    Shadow* m_pShadow;
};

Shadow::Shadow(Window* pWindow):
    m_bShadowAttached(true),
    m_bUseDefaultShadowAttached(true),
    m_isMaximized(false),
    m_pRoot(nullptr),
    m_pWindow(pWindow),
    m_bEnableShadowSnap(true),
    m_bLeftSnap(false),
    m_bTopSnap(false),
    m_bRightSnap(false),
    m_bBottomSnap(false)
{
    SetShadowType(Shadow::ShadowType::kShadowDefault);
}

bool Shadow::IsUseDefaultShadowAttached() const
{
    return m_bUseDefaultShadowAttached;
}

void Shadow::SetUseDefaultShadowAttached(bool bDefault)
{
    m_bUseDefaultShadowAttached = bDefault;
}

Box* Shadow::AttachShadow(Box* pRoot)
{
    if (!m_bShadowAttached) {
        return pRoot;
    }
    ASSERT(m_pRoot == nullptr);
    if (m_pRoot != nullptr) {
        return pRoot;
    }

    if (pRoot == nullptr) {
        return nullptr;
    }

    m_pRoot = new ShadowBox(pRoot->GetWindow(), this);
    m_pRoot->AddItem(pRoot);
    DoAttachShadow(m_pRoot, pRoot, true, m_isMaximized);
    return m_pRoot;
}

void Shadow::DoAttachShadow(Box* pNewRoot, Box* pOrgRoot, bool bNewAttach, bool isMaximized) const
{
    //实现逻辑：按需更新
    ASSERT((pNewRoot != nullptr) && (pOrgRoot != nullptr));
    if ((pNewRoot == nullptr) || (pOrgRoot == nullptr)) {
        return;
    }
    const UiPadding rcShadowCorner = GetCurrentShadowCorner();
    if (bNewAttach && !isMaximized) {
        //Attach并且不是窗口最大化状态
        pNewRoot->SetPadding(rcShadowCorner, false);
    }
    else {
        //Detach或者窗口为最大化状态时
        pNewRoot->SetPadding(UiPadding(0, 0, 0, 0), false);
    }
    if (pOrgRoot->GetFixedWidth().IsInt32()) {
        int32_t rootWidth = pOrgRoot->GetFixedWidth().GetInt32();
        if (bNewAttach) {
            rootWidth += (rcShadowCorner.left + rcShadowCorner.right);
        }
        pNewRoot->SetFixedWidth(UiFixedInt(rootWidth), true, false);        
    }
    else {
        pNewRoot->SetFixedWidth(pOrgRoot->GetFixedWidth(), true, false);
    }
    if (pOrgRoot->GetFixedHeight().IsInt32()) {
        int32_t rootHeight = pOrgRoot->GetFixedHeight().GetInt32();
        if (bNewAttach) {
            rootHeight += (rcShadowCorner.top + rcShadowCorner.bottom);
        }       
        pNewRoot->SetFixedHeight(UiFixedInt(rootHeight), true, false);
    }
    else {
        pNewRoot->SetFixedHeight(pOrgRoot->GetFixedHeight(), true, false);
    }
    pNewRoot->SetBkImage(bNewAttach ? m_shadowImage : DString());
}

void Shadow::SetShadowAttached(bool bShadowAttached)
{
    m_bShadowAttached = bShadowAttached;
    //外部设置后，即更新为非默认值
    m_bUseDefaultShadowAttached = false;

    if (bShadowAttached) {
        if (GetShadowType() == Shadow::ShadowType::kShadowNone) {
            m_nShadowType = Shadow::ShadowType::kShadowDefault;
        }
    }
    OnShadowAttached(GetShadowType());
}

bool Shadow::IsShadowAttached() const
{ 
    return m_bShadowAttached;
}

void Shadow::SetShadowType(Shadow::ShadowType nShadowType)
{
    ASSERT(nShadowType >= Shadow::ShadowType::kShadowNone);
    ASSERT(nShadowType < Shadow::ShadowType::kShadowCount);
    if ((nShadowType >= Shadow::ShadowType::kShadowNone) &&
        (nShadowType < Shadow::ShadowType::kShadowCount)) {
        m_nShadowType = nShadowType;
    }
    else {
        return;
    }

    if (GetShadowType() != Shadow::ShadowType::kShadowNone) {
        m_bShadowAttached = true;
    }
    else {
        m_bShadowAttached = false;
    }

    //外部设置后，即更新为非默认值
    m_bUseDefaultShadowAttached = false;

    OnShadowAttached(GetShadowType());
}

Shadow::ShadowType Shadow::GetShadowType() const
{
    return m_nShadowType;
}

bool Shadow::GetShadowType(const DString& typeString, ShadowType& nShadowType)
{
    if (typeString == _T("big")) {
        nShadowType = Shadow::ShadowType::kShadowBig;
    }
    else if (typeString == _T("big_round")) {
        nShadowType = Shadow::ShadowType::kShadowBigRound;
    }
    else if (typeString == _T("small")) {
        nShadowType = Shadow::ShadowType::kShadowSmall;
    }
    else if (typeString == _T("small_round")) {
        nShadowType = Shadow::ShadowType::kShadowSmallRound;
    }
    else if (typeString == _T("menu")) {
        nShadowType = Shadow::ShadowType::kShadowMenu;
    }
    else if (typeString == _T("menu_round")) {
        nShadowType = Shadow::ShadowType::kShadowMenuRound;
    }
    else if (typeString == _T("custom")) {
        nShadowType = Shadow::ShadowType::kShadowCustom;
    }
    else if (typeString == _T("default")) {
        nShadowType = Shadow::ShadowType::kShadowDefault;
    }
    else if (typeString == _T("none")) {
        nShadowType = Shadow::ShadowType::kShadowNone;
    }
    else {
        ASSERT(0);
        return false;
    }
    return true;
}

bool Shadow::GetShadowParam(ShadowType nShadowType,
                            UiSize& szBorderRound,
                            UiPadding& rcShadowCorner,
                            DString& shadowImage)
{
    bool bRet = false;
    if (nShadowType == Shadow::ShadowType::kShadowBig) {
        bRet = true;
        szBorderRound = UiSize(0, 0);
        rcShadowCorner = UiPadding(58, 58, 62, 64);
        shadowImage = StringUtil::Printf(_T("file='public/shadow/shadow_big.svg' window_shadow_mode='true' corner='%d,%d,%d,%d'"),
                                             rcShadowCorner.left + szBorderRound.cx,
                                             rcShadowCorner.top + szBorderRound.cx,
                                             rcShadowCorner.right + szBorderRound.cx,
                                             rcShadowCorner.bottom + szBorderRound.cx);
    }
    else if (nShadowType == Shadow::ShadowType::kShadowBigRound) {
        bRet = true;
        szBorderRound = UiSize(6, 6);
        rcShadowCorner = UiPadding(58, 58, 62, 64);
        shadowImage = StringUtil::Printf(_T("file='public/shadow/shadow_big_round.svg' window_shadow_mode='true' corner='%d,%d,%d,%d'"),
                                         rcShadowCorner.left + szBorderRound.cx,
                                         rcShadowCorner.top + szBorderRound.cx,
                                         rcShadowCorner.right + szBorderRound.cx,
                                         rcShadowCorner.bottom + szBorderRound.cx);
    }
    else if (nShadowType == Shadow::ShadowType::kShadowSmall) {
        bRet = true;
        szBorderRound = UiSize(0, 0);
        rcShadowCorner = UiPadding(52, 52, 56, 58);
        shadowImage = StringUtil::Printf(_T("file='public/shadow/shadow_small.svg' window_shadow_mode='true' corner='%d,%d,%d,%d'"),
                                             rcShadowCorner.left + szBorderRound.cx,
                                             rcShadowCorner.top + szBorderRound.cx,
                                             rcShadowCorner.right + szBorderRound.cx,
                                             rcShadowCorner.bottom + szBorderRound.cx);
    }
    else if (nShadowType == Shadow::ShadowType::kShadowSmallRound) {
        bRet = true;
        szBorderRound = UiSize(6, 6);
        rcShadowCorner = UiPadding(52, 52, 56, 58);
        shadowImage = StringUtil::Printf(_T("file='public/shadow/shadow_small_round.svg' window_shadow_mode='true' corner='%d,%d,%d,%d'"),
                                         rcShadowCorner.left + szBorderRound.cx,
                                         rcShadowCorner.top + szBorderRound.cx,
                                         rcShadowCorner.right + szBorderRound.cx,
                                         rcShadowCorner.bottom + szBorderRound.cx);
    }
    else if (nShadowType == Shadow::ShadowType::kShadowMenu) {
        bRet = true;
        szBorderRound = UiSize(0, 0);
        rcShadowCorner = UiPadding(52, 52, 56, 58);
        shadowImage = StringUtil::Printf(_T("file='public/shadow/shadow_menu.svg' window_shadow_mode='true' corner='%d,%d,%d,%d'"),
                                             rcShadowCorner.left + szBorderRound.cx,
                                             rcShadowCorner.top + szBorderRound.cx,
                                             rcShadowCorner.right + szBorderRound.cx,
                                             rcShadowCorner.bottom + szBorderRound.cx);
    }
    else if (nShadowType == Shadow::ShadowType::kShadowMenuRound) {
        bRet = true;
        szBorderRound = UiSize(6, 6);
        rcShadowCorner = UiPadding(52, 52, 56, 58);
        shadowImage = StringUtil::Printf(_T("file='public/shadow/shadow_menu_round.svg' window_shadow_mode='true' corner='%d,%d,%d,%d'"),
                                         rcShadowCorner.left + szBorderRound.cx,
                                         rcShadowCorner.top + szBorderRound.cx,
                                         rcShadowCorner.right + szBorderRound.cx,
                                         rcShadowCorner.bottom + szBorderRound.cx);
    }
    else if (nShadowType == Shadow::ShadowType::kShadowCustom) {
        bRet = true;
        szBorderRound = UiSize(0, 0);
        rcShadowCorner = UiPadding(0, 0, 0, 0);
        shadowImage.clear();
    }
    else {
        szBorderRound = UiSize(0, 0);
        rcShadowCorner = UiPadding(0, 0, 0, 0);
        shadowImage.clear();
    }
    return bRet;
}

void Shadow::OnShadowAttached(Shadow::ShadowType nShadowType)
{
    UiSize szBorderRound;
    UiPadding rcShadowCorner;
    DString shadowImage;
    if (GetShadowParam(nShadowType, szBorderRound, rcShadowCorner, shadowImage)) {
        SetShadowCorner(rcShadowCorner);
        SetShadowBorderRound(szBorderRound);
        SetShadowImage(shadowImage);
    }
    UpdateShadow();
}

void Shadow::UpdateShadow()
{
    //如果已经调用了AttachShadow，需要进行些处理
    if (m_pRoot != nullptr) {
        Box* pOrgRoot = nullptr;
        if (m_pRoot->GetItemCount() > 0) {
            pOrgRoot = dynamic_cast<Box*>(m_pRoot->GetItemAt(0));
        }
        DoAttachShadow(m_pRoot, pOrgRoot, m_bShadowAttached, m_isMaximized);

        //刷新，重绘
        m_pRoot->ArrangeAncestor();
        m_pRoot->SetPos(m_pRoot->GetPos());
    }
}

void Shadow::SetShadowImage(const DString& shadowImage)
{
    if (shadowImage != m_shadowImage) {
        //阴影图片发生变化
        m_shadowImage = shadowImage;
        UpdateShadow();
    }
}

const DString& Shadow::GetShadowImage() const
{
    return m_shadowImage;
}

void Shadow::SetShadowCorner(const UiPadding& rcShadowCorner)
{
    ASSERT((rcShadowCorner.left >= 0) && (rcShadowCorner.top >= 0) && (rcShadowCorner.right >= 0) && (rcShadowCorner.bottom >= 0));
    if ((rcShadowCorner.left >= 0) && (rcShadowCorner.top >= 0) && (rcShadowCorner.right >= 0) && (rcShadowCorner.bottom >= 0)) {
        m_rcShadowCorner = rcShadowCorner;
        UpdateShadow();
    }    
}

UiPadding Shadow::GetShadowCorner() const
{
    return m_rcShadowCorner;
}

UiPadding Shadow::GetCurrentShadowCorner() const
{
    if (m_bShadowAttached && !m_isMaximized) {
        UiPadding rcShadowCorner = m_rcShadowCorner;
        ASSERT(m_pWindow != nullptr);
        if (m_pWindow != nullptr) {
            m_pWindow->Dpi().ScalePadding(rcShadowCorner);
        }

        if (m_bLeftSnap) {
            rcShadowCorner.left = 0;
        }
        if (m_bTopSnap) {
            rcShadowCorner.top = 0;
        }
        if (m_bRightSnap) {
            rcShadowCorner.right = 0;
        }
        if (m_bBottomSnap) {
            rcShadowCorner.bottom = 0;
        }
        return rcShadowCorner;
    }
    else {
        return UiPadding(0, 0, 0, 0);
    }
}

void Shadow::SetShadowBorderRound(UiSize szBorderRound)
{
    m_szBorderRound = szBorderRound;
    if (m_pRoot != nullptr) {
        m_pRoot->Invalidate();
    }
}

UiSize Shadow::GetShadowBorderRound() const
{
    return m_szBorderRound;
}

void Shadow::MaximizedOrRestored(bool isMaximized)
{
    m_isMaximized = isMaximized;
    if (!m_bShadowAttached) {
        return;
    }
    if (m_pRoot != nullptr) {
        UiPadding rcShadowCorner = GetCurrentShadowCorner();
        m_pRoot->SetPadding(rcShadowCorner, false);
    }
}

ui::Control* Shadow::GetRoot()
{
    return m_pRoot;
}

void Shadow::ClearImageCache()
{
    if (m_pRoot) {
        m_pRoot->ClearImageCache();
    }    
}

void Shadow::ChangeDpiScale(const DpiManager& dpi, uint32_t /*nOldDpiScale*/, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == dpi.GetScale());
    if (nNewDpiScale != dpi.GetScale()) {
        return;
    }
    //更新阴影图片(触发图片重新加载，根据DPI适应响应DPI值的图片)
    DString shadowImage = GetShadowImage();
    if (!shadowImage.empty()) {
        SetShadowImage(_T(""));
        SetShadowImage(shadowImage);
    }
}

void Shadow::SetWindowPosSnap(bool bLeftSnap, bool bRightSnap, bool bTopSnap, bool bBottomSnap)
{
    if (IsEnableShadowSnap() && IsShadowAttached()) {
        if ((m_bLeftSnap != bLeftSnap) || (m_bTopSnap != bTopSnap) ||
            (m_bRightSnap != bRightSnap) || (m_bBottomSnap != bBottomSnap)) {
            m_bLeftSnap = bLeftSnap;
            m_bTopSnap = bTopSnap;
            m_bRightSnap = bRightSnap;
            m_bBottomSnap = bBottomSnap;

            //更新属性
            UpdateWindowPosSnap();
        }
    }
    else {
        if ((m_bLeftSnap != 0) || (m_bTopSnap != 0) ||
            (m_bRightSnap != 0) || (m_bBottomSnap != 0)) {
            m_bLeftSnap = 0;
            m_bTopSnap = 0;
            m_bRightSnap = 0;
            m_bBottomSnap = 0;

            //更新属性
            UpdateWindowPosSnap();
        }
    }
}

void Shadow::SetEnableShadowSnap(bool bEnable)
{
    m_bEnableShadowSnap = bEnable;
}

bool Shadow::IsEnableShadowSnap() const
{
    return m_bEnableShadowSnap;
}

void Shadow::UpdateWindowPosSnap()
{
    if (m_pRoot != nullptr) {
        UiPadding rcShadowCorner = GetCurrentShadowCorner();
        if(!rcShadowCorner.Equals(m_pRoot->GetPadding())) {
            m_pRoot->SetPadding(rcShadowCorner, false);
            if (m_pWindow != nullptr) {
                m_pWindow->InvalidateAll();
            }
        }        
    }
}

} //namespace ui

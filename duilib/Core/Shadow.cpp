#include "Shadow.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ClickThrough.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"

#ifdef DUILIB_BUILD_FOR_WIN
    #include <VersionHelpers.h>
#endif

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

        UiSize borderRound;
        if (m_pShadow != nullptr) {
            borderRound = m_pShadow->GetShadowBorderRound();
        }
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
        UiFixedSize fixedSize;
        UiEstSize returnEstSize;
        if (!PreEstimateSize(szAvailable, fixedSize, returnEstSize)) {
            return returnEstSize;
        }

        //子控件的大小，包含内边距，但不包含外边距; 包含了阴影本身的大小（即Box的内边距）
        UiSize64 layoutSize = GetLayout()->EstimateLayoutSize(m_items, szAvailable);
        UiSize sizeByChild(ui::TruncateToInt32(layoutSize.cx), ui::TruncateToInt32(layoutSize.cy));

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
        if (pRender == nullptr) {
            return;
        }
        if ((m_pShadow == nullptr) || !m_pShadow->IsShadowAttached()) {
            BaseClass::PaintBkImage(pRender);
        }
        else {
            Image* pBkImage = GetBkImagePtr();
            Window* pWindow = GetWindow();
            if ((pBkImage != nullptr) && (pWindow != nullptr)) {
                UiRect destRect = GetRect();
                if (m_pShadow->IsEnableShadowSnap()) {
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
                }
                PaintImage(pRender, pBkImage, _T(""), DUI_NOSET_VALUE, nullptr, &destRect);
            }

            //绘制边框
            if (m_pShadow != nullptr) {
                Box* pXmlRoot = m_pShadow->GetAttachedXmlRoot();
                int32_t nShadowBorderSize = m_pShadow->GetShadowBorderSize();  //边框大小(以XmlRoot Box的矩形为中心线，中心线两侧各一个像素)
                DString shadowBorderColor = m_pShadow->GetShadowBorderColor(); //边框颜色（和Win11默认的窗口边框颜色接近）
                UiColor dwBorderColor;
                if (!shadowBorderColor.empty() && (nShadowBorderSize > 0)) {
                    dwBorderColor = GetUiColor(shadowBorderColor);
                }
                if ((pXmlRoot != nullptr) && (nShadowBorderSize > 0) && !dwBorderColor.IsEmpty()) {                 
                    float fBorderSize = Dpi().GetScaleFloat(nShadowBorderSize);
                    const UiSize borderRound = m_pShadow->GetShadowBorderRound();   //圆角大小

                    float rx = Dpi().GetScaleFloat(borderRound.cx);
                    float ry = Dpi().GetScaleFloat(borderRound.cy);
                    
                    UiRect rcPos = pXmlRoot->GetPos();
                    if (!rcPos.IsEmpty()) {
                        UiRectF rcRoot((float)rcPos.left, (float)rcPos.top, (float)rcPos.right, (float)rcPos.bottom);

                        if ((borderRound.cy > 0) && (borderRound.cy > 0)) {
                            pRender->DrawRoundRect(rcRoot, rx, ry, dwBorderColor, fBorderSize);
                        }
                        else {
                            pRender->DrawRect(rcRoot, dwBorderColor, fBorderSize, false);
                        }
                    }
                }
            }
        }
    }

    /** 鼠标点击事件, 实现鼠标穿透功能
    */
    void OnMouseDown(UiPoint ptMouse)
    {
        UiPoint pt = ptMouse;
        //判断是否在阴影上
        if ((m_pShadow == nullptr) || !m_pShadow->IsShadowAttached()) {
            return;
        }
        Box* pXmlRoot = m_pShadow->GetAttachedXmlRoot();
        if (pXmlRoot == nullptr) {
            return;
        }
        UiRect rcRoot = pXmlRoot->GetPos();
        if (rcRoot.ContainsPt(pt)) {
            //鼠标不在阴影上
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
        if (!m_pShadow->IsEnableClickThroughWindow()) {
            return;
        }
        ClickThrough shadowClick;
        shadowClick.ClickThroughWindow(GetWindow(), ptMouse);
    }

private:
    //关联的阴影控件
    Shadow* m_pShadow;
};

Shadow::Shadow(Window* pWindow, bool bShadowAttached):
    m_bShadowAttached(bShadowAttached),
    m_bWindowMaximized(false),
    m_pShadowBox(nullptr),
    m_pWindow(pWindow),
    m_bEnableClickThroughWindow(true),
    m_bEnableShadowSnap(true),
    m_bLeftSnap(false),
    m_bTopSnap(false),
    m_bRightSnap(false),
    m_bBottomSnap(false),
    m_nShadowBorderSize(2),
    m_shadowBorderColor(_T(""))
{
    //默认阴影边框的颜色
    ASSERT(m_pWindow != nullptr);
    const DString borderWindow = _T("border_window");
    if (!GlobalManager::Instance().Color().GetColor(borderWindow).IsEmpty()) {
        m_shadowBorderColor = borderWindow;
    }
    if (m_shadowBorderColor.empty()) {
        //若无配置，则设置默认值
        if ((m_pWindow != nullptr) && m_pWindow->IsColorThemeDarkMode()) {
            //深色主题
            m_shadowBorderColor = _T("#FF444444");
        }
        else {
            //浅色主题
            m_shadowBorderColor = _T("#FFB5B5B5");
        }
    }

    //设置默认的阴影类型
    m_nShadowType = ShadowType::kShadowDefault;
}

void Shadow::SetShadowAttached(bool bShadowAttached)
{
    ASSERT(m_pWindow->GetFullscreenControl() == nullptr);
    if (m_pWindow->GetFullscreenControl() != nullptr) {
        return;
    }
    if (m_nShadowType == ShadowType::kShadowDefault) {
        m_nShadowType = GetDefaultShadowType(m_pWindow);
    }
    m_bShadowAttached = bShadowAttached;
    OnShadowAttached(GetShadowType());
}

bool Shadow::IsShadowAttached() const
{
    return m_bShadowAttached;
}

void Shadow::SetShadowType(ShadowType nShadowType)
{
    ASSERT(m_pWindow->GetFullscreenControl() == nullptr);
    if (m_pWindow->GetFullscreenControl() != nullptr) {
        return;
    }
    m_nShadowType = GetSupportedShadowType(m_pWindow, nShadowType);

    if (!m_pWindow->IsUseSystemCaption()) {
        //如果未启用系统标题栏，则自动开启阴影
        m_bShadowAttached = true;
    }    
    OnShadowAttached(GetShadowType());
}

ShadowType Shadow::GetShadowType() const
{
    return m_nShadowType;
}

bool Shadow::IsSystemShadowEnabled() const
{
    return IsSystemShadowEnabled(m_nShadowType);
}

bool Shadow::IsSystemShadowEnabled(ShadowType nShadowType) const
{
    return (nShadowType == ShadowType::kShadowSystemDefault) ||
           (nShadowType == ShadowType::kShadowSystemDoNotRound) ||
           (nShadowType == ShadowType::kShadowSystemRound) ||
           (nShadowType == ShadowType::kShadowSystemSmallRound);
}

void Shadow::OnShadowAttached(ShadowType nShadowType)
{
    UiSize szBorderRound;
    UiPadding rcShadowCorner;
    DString shadowImage;
    if (GetShadowParam(m_pWindow, nShadowType, szBorderRound, rcShadowCorner, shadowImage, this)) {
        //用户自定义类型：不覆盖原值，以用户设置的为准
        if (nShadowType != ShadowType::kShadowCustom) {
            SetShadowCorner(rcShadowCorner);
            SetShadowBorderRound(szBorderRound);
            SetShadowImage(shadowImage);
        }
    }
    UpdateShadow();

    if (m_pWindow->NativeWnd()->IsSystemShadowSupported()) {
        if (IsShadowAttached() && IsSystemShadowEnabled(nShadowType)) {
            //使用系统阴影
            NativeWindowShadowType nativeShadowType = NativeWindowShadowType::kShadowSystemDefault;
            if (nShadowType == ShadowType::kShadowSystemDefault) {
                nativeShadowType = NativeWindowShadowType::kShadowSystemDefault;
            }
            else if (nShadowType == ShadowType::kShadowSystemDoNotRound) {
                nativeShadowType = NativeWindowShadowType::kShadowSystemDoNotRound;
            }
            else if (nShadowType == ShadowType::kShadowSystemRound) {
                nativeShadowType = NativeWindowShadowType::kShadowSystemRound;
            }
            else if (nShadowType == ShadowType::kShadowSystemSmallRound) {
                nativeShadowType = NativeWindowShadowType::kShadowSystemSmallRound;
            }
            if (m_pWindow->NativeWnd()->SetSystemShadowType(nativeShadowType)) {
                //启用系统阴影时，必须清除RGN，否则显示不正确(由调用方负责处理)
                m_pWindow->NativeWnd()->ClearWindowRgn(true);
            }
        }
        else {
#ifdef DUILIB_BUILD_FOR_MACOS
            //MacOS系统中，如果使用系统标题栏，需要开启系统阴影(必须使用直角阴影，否则客户区周围都是圆角)，否则也不显示阴影
            if (m_pWindow->IsUseSystemCaption()) {
                m_pWindow->NativeWnd()->SetSystemShadowType(NativeWindowShadowType::kShadowSystemDoNotRound);
                //启用系统阴影时，必须清除RGN，否则显示不正确(由调用方负责处理)
                m_pWindow->NativeWnd()->ClearWindowRgn(true);
            }
            else {
                m_pWindow->NativeWnd()->SetSystemShadowType(NativeWindowShadowType::kShadowSystemDisabled);
            }
#else
            m_pWindow->NativeWnd()->SetSystemShadowType(NativeWindowShadowType::kShadowSystemDisabled);
#endif
        }
    }
}

void Shadow::UpdateShadow()
{
    //如果已经调用了AttachShadow，需要进行些处理
    if (m_pShadowBox != nullptr) {
        Box* pXmlRoot = nullptr;
        if (m_pShadowBox->GetItemCount() > 0) {
            pXmlRoot = dynamic_cast<Box*>(m_pShadowBox->GetItemAt(0));
        }
        DoAttachShadow(m_pShadowBox.get(), pXmlRoot, m_bShadowAttached, m_bWindowMaximized);

        //刷新，重绘        
        UiRect rcShadow = m_pShadowBox->GetPos();
        if (!rcShadow.IsEmpty()) {
            m_pShadowBox->SetPos(rcShadow);
        }
        m_pShadowBox->ArrangeAncestor();
        m_pWindow->InvalidateAll();
    }
}

void Shadow::DoAttachShadow(Box* pShadowBox, Box* pXmlRoot, bool bShadowAttached, bool bWindowMaximized) const
{
    //实现逻辑：按需更新
    ASSERT((pShadowBox != nullptr) && (pXmlRoot != nullptr));
    if ((pShadowBox == nullptr) || (pXmlRoot == nullptr)) {
        return;
    }
    const UiPadding rcShadowCorner = GetCurrentShadowCorner();
    if (bShadowAttached && !bWindowMaximized) {
        //Attach并且不是窗口最大化状态
        pShadowBox->SetPadding(rcShadowCorner, false);
    }
    else {
        //Detach或者窗口为最大化状态时
        pShadowBox->SetPadding(UiPadding(0, 0, 0, 0), false);
    }
    if (pXmlRoot->GetFixedWidth().IsInt32()) {
        int32_t rootWidth = pXmlRoot->GetFixedWidth().GetInt32();
        if (bShadowAttached) {
            rootWidth += (rcShadowCorner.left + rcShadowCorner.right);
        }
        pShadowBox->SetFixedWidth(UiFixedInt(rootWidth), true, false);
    }
    else {
        pShadowBox->SetFixedWidth(pXmlRoot->GetFixedWidth(), true, false);
    }
    if (pXmlRoot->GetFixedHeight().IsInt32()) {
        int32_t rootHeight = pXmlRoot->GetFixedHeight().GetInt32();
        if (bShadowAttached) {
            rootHeight += (rcShadowCorner.top + rcShadowCorner.bottom);
        }
        pShadowBox->SetFixedHeight(UiFixedInt(rootHeight), true, false);
    }
    else {
        pShadowBox->SetFixedHeight(pXmlRoot->GetFixedHeight(), true, false);
    }
    pShadowBox->SetBkImage(bShadowAttached ? m_shadowImage : DString());
}

Box* Shadow::AttachShadow(Box* pXmlRoot)
{
    ASSERT(m_pShadowBox == nullptr);
    if (m_pShadowBox != nullptr) {
        return pXmlRoot;
    }
    m_pXmlRootBox = pXmlRoot;
    if (!m_bShadowAttached) {
        return pXmlRoot;
    }
    if (pXmlRoot == nullptr) {
        return nullptr;
    }

    m_pShadowBox = new ShadowBox(pXmlRoot->GetWindow(), this);
    m_pShadowBox->SetMouseEnabled(false);    //阴影容器不接收鼠标消息
    m_pShadowBox->SetNoFocus();              //阴影容器不获取焦点
    m_pShadowBox->SetKeyboardEnabled(false); //阴影容器不接收键盘消息
    m_pShadowBox->AddItem(pXmlRoot);
    DoAttachShadow(m_pShadowBox.get(), pXmlRoot, true, m_bWindowMaximized);
    return m_pShadowBox.get();
}

Box* Shadow::DettachShadow()
{
    ASSERT(m_pShadowBox != nullptr);
    if (m_pShadowBox == nullptr) {
        return nullptr;
    }
    Box* pXmlRoot = nullptr;
    if (m_pShadowBox->GetItemCount() > 0) {
        pXmlRoot = dynamic_cast<Box*>(m_pShadowBox->GetItemAt(0));
    }
    ASSERT(pXmlRoot != nullptr);
    if (pXmlRoot == nullptr) {
        return nullptr;
    }
    ASSERT(m_pXmlRootBox == pXmlRoot);
    if (m_pXmlRootBox != pXmlRoot) {
        return nullptr;
    }    
    m_pXmlRootBox.reset();
    m_pShadowBox->SetAutoDestroyChild(false);
    m_pShadowBox->RemoveItem(pXmlRoot);
    delete m_pShadowBox.get();
    m_pShadowBox = nullptr;
    return pXmlRoot;
}

Box* Shadow::GetShadowBox() const
{
    return m_pShadowBox.get();
}

Box* Shadow::GetAttachedXmlRoot() const
{
#ifdef _DEBUG
    Box* pShadowRoot = GetShadowBox();
    if (pShadowRoot == nullptr) {
        return nullptr;
    }
    Box* pXmlRoot = nullptr;
    if (pShadowRoot->GetItemCount() > 0) {
        pXmlRoot = dynamic_cast<Box*>(pShadowRoot->GetItemAt(0));
    }
    ASSERT(pXmlRoot == m_pXmlRootBox.get());
#endif
    return m_pXmlRootBox.get();
}

bool Shadow::HasShadowBox() const
{
    return m_pShadowBox != nullptr;
}

bool Shadow::GetShadowType(const DString& typeString, ShadowType& nShadowType)
{
    if (typeString == _T("big")) {
        nShadowType = ShadowType::kShadowBig;
    }
    else if (typeString == _T("big_round")) {
        nShadowType = ShadowType::kShadowBigRound;
    }
    else if (typeString == _T("small")) {
        nShadowType = ShadowType::kShadowSmall;
    }
    else if (typeString == _T("small_round")) {
        nShadowType = ShadowType::kShadowSmallRound;
    }
    else if (typeString == _T("menu")) {
        nShadowType = ShadowType::kShadowMenu;
    }
    else if (typeString == _T("menu_round")) {
        nShadowType = ShadowType::kShadowMenuRound;
    }
    else if (typeString == _T("none")) {
        nShadowType = ShadowType::kShadowNone;
    }
    else if (typeString == _T("none_round")) {
        nShadowType = ShadowType::kShadowNoneRound;
    }
    else if (typeString == _T("custom")) {
        nShadowType = ShadowType::kShadowCustom;
    }
    else if (typeString == _T("default")) {
        nShadowType = ShadowType::kShadowDefault;
    }
    else if (typeString == _T("system_default")) {
        nShadowType = ShadowType::kShadowSystemDefault;
    }
    else if (typeString == _T("system_not_round")) {
        nShadowType = ShadowType::kShadowSystemDoNotRound;
    }
    else if (typeString == _T("system_round")) {
        nShadowType = ShadowType::kShadowSystemRound;
    }
    else if (typeString == _T("system_small_round")) {
        nShadowType = ShadowType::kShadowSystemSmallRound;
    }
    else {
        ASSERT(0);
        return false;
    }
    return true;
}

ShadowType Shadow::GetDefaultShadowType(const Window* pWindow)
{
    if (pWindow != nullptr) {
        if (pWindow->IsLayeredWindow()) {
            //分层窗口，默认为自绘阴影，圆角
            return ShadowType::kShadowBigRound;
        }
        else if (pWindow->NativeWnd()->IsSystemShadowSupported()) {
            //该窗口支持系统阴影
            return ShadowType::kShadowSystemDefault;
        }        
    }
    //未关联窗口的情况
#if defined (DUILIB_BUILD_FOR_WIN)
    //Windows系统
    if (::IsWindows10OrGreater()) {
        //Windows 10 或者更新的系统，默认使用系统阴影，体验较好
        return ShadowType::kShadowSystemDefault;
    }
    else {
        //低于Windows 10的系统，默认使用自绘阴影，因为系统阴影的体验不好
        return ShadowType::kShadowBigRound;
    }
#elif defined (DUILIB_BUILD_FOR_MACOS)
    //macOS：支持系统阴影
    return ShadowType::kShadowSystemDefault;
#else
    //其他平台：默认为自绘阴影，圆角
    return ShadowType::kShadowBigRound;
#endif
}

ShadowType Shadow::GetSupportedShadowType(const Window* pWindow, ShadowType nShadowType)
{
    if (nShadowType == ShadowType::kShadowDefault) {
        nShadowType = GetDefaultShadowType(pWindow);
    }
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return nShadowType;
    }
    if (!pWindow->NativeWnd()->IsSystemShadowSupported()) {
        //当不支持系统阴影时，自动选择自绘阴影
#ifdef DUILIB_BUILD_FOR_WIN
        //只有Win7系统会到这个流程，Win8开始，DWM默认都是开启的，且无法关闭
        if (Shadow::IsSystemShadowType(nShadowType)) {
            nShadowType = ShadowType::kShadowNone;
        }
#else
        if (nShadowType == ShadowType::kShadowSystemDefault) {
            nShadowType = ShadowType::kShadowBigRound;
        }
        else if (nShadowType == ShadowType::kShadowSystemDoNotRound) {
            nShadowType = ShadowType::kShadowNone;
        }
        else if (nShadowType == ShadowType::kShadowSystemRound) {
            nShadowType = ShadowType::kShadowBigRound;
        }
        else if (nShadowType == ShadowType::kShadowSystemSmallRound) {
            nShadowType = ShadowType::kShadowSmallRound;
        }
#endif
    }
//    else {
//        if ((nShadowType == ShadowType::kShadowSystemDefault) ||
//            (nShadowType == ShadowType::kShadowSystemDoNotRound)) {
//            //这两个值时，窗口不能是分层窗口，否则会变成无阴影的状态(SDL实现在Windows平台未使用分层窗口属性)
//#ifndef DUILIB_BUILD_FOR_SDL
//            ASSERT(!pWindow->IsLayeredWindow());
//            if (pWindow->IsLayeredWindow()) {
//                nShadowType = ShadowType::kShadowSystemRound;
//            }
//#endif
//        }
//    }
    return nShadowType;
}

bool Shadow::IsSystemShadowType(ShadowType nShadowType)
{
    bool bRet = false;
    if ((nShadowType == ShadowType::kShadowSystemDefault) ||
        (nShadowType == ShadowType::kShadowSystemDoNotRound) ||
        (nShadowType == ShadowType::kShadowSystemRound) ||
        (nShadowType == ShadowType::kShadowSystemSmallRound)) {
        bRet = true;
    }
    return bRet;
}

bool Shadow::IsShadowTypeNeedLayeredWindow(ShadowType nShadowType)
{
    if (IsSystemShadowType(nShadowType)) {
        return false;
    }
    if (nShadowType == ShadowType::kShadowNone) {
        return false;
    }
    return true;
}

bool Shadow::IsShadowTypeNeedWindowRGN(ShadowType nShadowType)
{
    if (nShadowType == ShadowType::kShadowNone) {
        return true;
    }
    return false;
}

bool Shadow::GetShadowParam(const Window* pWindow,
                            ShadowType& nShadowType,
                            UiSize& szBorderRound,
                            UiPadding& rcShadowCorner,
                            DString& shadowImage,
                            Shadow* pShadowObj)
{
    nShadowType = GetSupportedShadowType(pWindow, nShadowType);
    if (nShadowType == ShadowType::kShadowDefault) {
        nShadowType = GetDefaultShadowType(pWindow);
    }
    bool bRet = false;
    //阴影边缘的颜色，与窗口边框的颜色保持一致
    DString svgReplaceColors;
    if ((pShadowObj != nullptr) && !pShadowObj->GetShadowBorderColor().empty()) {
        svgReplaceColors = StringUtil::Printf(_T("svg_replace_colors='#B5B5B5|%s'"), pShadowObj->GetShadowBorderColor().c_str());
    }
    if (nShadowType == ShadowType::kShadowBig) {
        bRet = true;
        szBorderRound = UiSize(0, 0);
        rcShadowCorner = UiPadding(30, 30, 34, 36);
        shadowImage = StringUtil::Printf(_T("file='%s/shadow/shadow_big.svg' window_shadow_mode='true' corner='%d,%d,%d,%d' %s"),
                                             DUILIB_PUBLIC_RES_DIR,
                                             rcShadowCorner.left + szBorderRound.cx,
                                             rcShadowCorner.top + szBorderRound.cx,
                                             rcShadowCorner.right + szBorderRound.cx,
                                             rcShadowCorner.bottom + szBorderRound.cx,
                                             svgReplaceColors.c_str());
    }
    else if (nShadowType == ShadowType::kShadowBigRound) {
        bRet = true;
        szBorderRound = UiSize(6, 6);
        rcShadowCorner = UiPadding(30, 30, 34, 36);
        shadowImage = StringUtil::Printf(_T("file='%s/shadow/shadow_big_round.svg' window_shadow_mode='true' corner='%d,%d,%d,%d' %s"),
                                         DUILIB_PUBLIC_RES_DIR,
                                         rcShadowCorner.left + szBorderRound.cx,
                                         rcShadowCorner.top + szBorderRound.cx,
                                         rcShadowCorner.right + szBorderRound.cx,
                                         rcShadowCorner.bottom + szBorderRound.cx,
                                         svgReplaceColors.c_str());
    }
    else if (nShadowType == ShadowType::kShadowSmall) {
        bRet = true;
        szBorderRound = UiSize(0, 0);
        rcShadowCorner = UiPadding(24, 24, 28, 30);
        shadowImage = StringUtil::Printf(_T("file='%s/shadow/shadow_small.svg' window_shadow_mode='true' corner='%d,%d,%d,%d' %s"),
                                             DUILIB_PUBLIC_RES_DIR,
                                             rcShadowCorner.left + szBorderRound.cx,
                                             rcShadowCorner.top + szBorderRound.cx,
                                             rcShadowCorner.right + szBorderRound.cx,
                                             rcShadowCorner.bottom + szBorderRound.cx,
                                             svgReplaceColors.c_str());
    }
    else if (nShadowType == ShadowType::kShadowSmallRound) {
        bRet = true;
        szBorderRound = UiSize(6, 6);
        rcShadowCorner = UiPadding(24, 24, 28, 30);
        shadowImage = StringUtil::Printf(_T("file='%s/shadow/shadow_small_round.svg' window_shadow_mode='true' corner='%d,%d,%d,%d' %s"),
                                         DUILIB_PUBLIC_RES_DIR,
                                         rcShadowCorner.left + szBorderRound.cx,
                                         rcShadowCorner.top + szBorderRound.cx,
                                         rcShadowCorner.right + szBorderRound.cx,
                                         rcShadowCorner.bottom + szBorderRound.cx,
                                         svgReplaceColors.c_str());
    }
    else if (nShadowType == ShadowType::kShadowMenu) {
        bRet = true;
        szBorderRound = UiSize(0, 0);
        rcShadowCorner = UiPadding(24, 24, 28, 30);
        shadowImage = StringUtil::Printf(_T("file='%s/shadow/shadow_menu.svg' window_shadow_mode='true' corner='%d,%d,%d,%d' %s"),
                                             DUILIB_PUBLIC_RES_DIR,
                                             rcShadowCorner.left + szBorderRound.cx,
                                             rcShadowCorner.top + szBorderRound.cx,
                                             rcShadowCorner.right + szBorderRound.cx,
                                             rcShadowCorner.bottom + szBorderRound.cx,
                                             svgReplaceColors.c_str());
    }
    else if (nShadowType == ShadowType::kShadowMenuRound) {
        bRet = true;
        szBorderRound = UiSize(6, 6);
        rcShadowCorner = UiPadding(24, 24, 28, 30);
        shadowImage = StringUtil::Printf(_T("file='%s/shadow/shadow_menu_round.svg' window_shadow_mode='true' corner='%d,%d,%d,%d' %s"),
                                         DUILIB_PUBLIC_RES_DIR,
                                         rcShadowCorner.left + szBorderRound.cx,
                                         rcShadowCorner.top + szBorderRound.cx,
                                         rcShadowCorner.right + szBorderRound.cx,
                                         rcShadowCorner.bottom + szBorderRound.cx,
                                         svgReplaceColors.c_str());
    }
    else if (nShadowType == ShadowType::kShadowNone) {
        bRet = true;
        szBorderRound = UiSize(0, 0);
        rcShadowCorner = UiPadding(0, 0, 0, 0);//设置一个像素，容纳边线（参考后续代码）
        shadowImage.clear();
    }
    else if (nShadowType == ShadowType::kShadowNoneRound) {
        bRet = true;
        szBorderRound = UiSize(6, 6);
        rcShadowCorner = UiPadding(0, 0, 0, 0);//设置一个像素，容纳边线（参考后续代码）
        shadowImage.clear();
    }
    else if (nShadowType == ShadowType::kShadowCustom) {
        bRet = true;
        szBorderRound = UiSize(0, 0);
        rcShadowCorner = UiPadding(0, 0, 0, 0);
        shadowImage.clear();
    }
    else if ((nShadowType == ShadowType::kShadowSystemDefault)    ||
             (nShadowType == ShadowType::kShadowSystemDoNotRound) ||
             (nShadowType == ShadowType::kShadowSystemRound)      ||
             (nShadowType == ShadowType::kShadowSystemSmallRound) ) {
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

    if ((pShadowObj != nullptr) && ((nShadowType == ShadowType::kShadowNone) ||
                                    (nShadowType == ShadowType::kShadowNoneRound))) {
        int32_t nShadowBorderSize = pShadowObj->GetShadowBorderSize();
        if (pShadowObj->GetShadowBorderColor().empty()) {
            nShadowBorderSize = 0;
        }
        if (nShadowBorderSize > 0) {
            nShadowBorderSize /= 2; //取边线的一半
            if (nShadowBorderSize < 1) {
                nShadowBorderSize = 1;
            }
            rcShadowCorner = UiPadding(nShadowBorderSize, nShadowBorderSize, nShadowBorderSize, nShadowBorderSize);//边线宽度
        }
        else {
            rcShadowCorner = UiPadding(0, 0, 0, 0);//禁止边线
        }
    }
    return bRet;
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

void Shadow::SetShadowBorderSize(int32_t nShadowBorderSize)
{
    ASSERT(nShadowBorderSize >= 0);
    if (nShadowBorderSize >= 0) {
        m_nShadowBorderSize = nShadowBorderSize;
    }    
}

int32_t Shadow::GetShadowBorderSize() const
{
    return m_nShadowBorderSize;
}

void Shadow::SetShadowBorderColor(const DString& shadowBorderColor)
{
    m_shadowBorderColor = shadowBorderColor;
}

const DString& Shadow::GetShadowBorderColor() const
{
    return m_shadowBorderColor;
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
    if (m_bShadowAttached && !m_bWindowMaximized) {
        UiPadding rcShadowCorner = m_rcShadowCorner;
        m_pWindow->Dpi().ScalePadding(rcShadowCorner);
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
    if (m_pShadowBox != nullptr) {
        m_pShadowBox->Invalidate();
    }
}

UiSize Shadow::GetShadowBorderRound() const
{
    return m_szBorderRound;
}

void Shadow::MaximizedOrRestored(bool bWindowMaximized)
{
    m_bWindowMaximized = bWindowMaximized;
    if (!m_bShadowAttached) {
        return;
    }
    if (m_pShadowBox != nullptr) {
        UiPadding rcShadowCorner = GetCurrentShadowCorner();
        m_pShadowBox->SetPadding(rcShadowCorner, false);
    }
}

void Shadow::ClearImageCache()
{
    if (m_pShadowBox != nullptr) {
        m_pShadowBox->ClearImageCache();
    }    
}

void Shadow::ChangeDpiScale(const DpiManager& dpi, uint32_t /*nOldDpiScale*/, uint32_t nNewDpiScale)
{
    if (!dpi.CheckDisplayScaleFactor(nNewDpiScale)) {
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
    if (IsEnableShadowSnap() && IsShadowAttached() && !GetShadowImage().empty()) {
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
    if (m_pShadowBox != nullptr) {
        UiPadding rcShadowCorner = GetCurrentShadowCorner();
        if(!rcShadowCorner.Equals(m_pShadowBox->GetPadding())) {
            m_pShadowBox->SetPadding(rcShadowCorner, false);
            m_pWindow->InvalidateAll();
        }        
    }
}

void Shadow::CheckMouseClickOnShadow(EventType eventType, const UiPoint& pt)
{
    if ((eventType != kEventMouseButtonDown) && (eventType != kEventMouseRButtonDown)) {
        //只处理鼠标左键按下和右键按下事件
        return;
    }
    if (!IsEnableClickThroughWindow()) {
        return;
    }
    if (m_pWindow->IsWindowMaximized() || m_pWindow->IsWindowFullscreen()) {
        //窗口全屏或者最大化模式
        return;
    }

    ShadowType shadowType = GetShadowType();
    if ((shadowType == ShadowType::kShadowNone) || (shadowType == ShadowType::kShadowNoneRound)) {
        //无阴影模式
        return;
    }
    if (IsSystemShadowType(shadowType)) {
        //系统阴影模式
        return;
    }

    ShadowBox* pShadowBox = nullptr;
    Box* pBox = GetShadowBox();
    if (pBox != nullptr) {
        pShadowBox = dynamic_cast<ShadowBox*>(pBox);
    }
    if (pShadowBox == nullptr) {
        return;
    }
    UiRect rcShadowBox = pShadowBox->GetRect();
    if (!rcShadowBox.ContainsPt(pt)) {
        return;
    }

    UiPadding rcShadowCorner = GetShadowCorner();
    if ((rcShadowCorner.left <= 1) && (rcShadowCorner.top <= 1) && (rcShadowCorner.right <= 1) && (rcShadowCorner.bottom <= 1)) {
        //当前无阴影
        return;
    }
    pShadowBox->Dpi().ScalePadding(rcShadowCorner);
    rcShadowBox.Deflate(rcShadowCorner);
    if (rcShadowBox.ContainsPt(pt)) {
        //鼠标不在阴影范围内
        return;
    }

    //鼠标确认点击在阴影上，处理阴影穿透逻辑
    pShadowBox->OnMouseDown(pt);
}

void Shadow::SetEnableClickThroughWindow(bool bEnable)
{
    m_bEnableClickThroughWindow = bEnable;
}

bool Shadow::IsEnableClickThroughWindow() const
{
    return m_bEnableClickThroughWindow;
}

} //namespace ui

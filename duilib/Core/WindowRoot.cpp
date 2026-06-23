#include "duilib/Core/WindowRoot.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/FullscreenBox.h"
#include "duilib/Core/Shadow.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/ControlFinder.h"
#include "duilib/Core/DpiManager.h"

#ifdef DUILIB_BUILD_FOR_WIN
    #include "duilib/Utils/ApiWrapper_Windows.h"
#endif

namespace ui
{

WindowRoot::WindowRoot(Window* pWindow):
    m_pWindow(pWindow),
    m_bControlFullscreen(false),
    m_pControlFinder(nullptr)
{
}

WindowRoot::~WindowRoot()
{
    Clear();
}

void WindowRoot::Clear()
{
    ClearShadow();
    ClearRoot();
}

void WindowRoot::ClearRoot()
{
    Box* pRoot = m_pRoot.get();
    m_pRoot.reset();
    if (pRoot != nullptr) {
        delete pRoot;
        pRoot = nullptr;
    }
}

void WindowRoot::ClearShadow()
{
    m_shadow.reset();
}

Box* WindowRoot::GetRoot() const
{
    return m_pRoot.get();
}

Box* WindowRoot::GetXmlRoot() const
{
    Box* pXmlRoot = nullptr;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pXmlRoot = pShadow->GetAttachedXmlRoot();
    }
    if (pXmlRoot == nullptr) {
        pXmlRoot = m_pRoot.get();
    }
    return pXmlRoot;
}

void WindowRoot::CreateShadow(bool bLayeredWindow)
{
    if (m_shadow != nullptr) {
        return;
    }
    m_shadow = std::make_unique<Shadow>(m_pWindow, bLayeredWindow);
}

Shadow* WindowRoot::GetShadow() const
{
    ASSERT(m_shadow != nullptr);
    if (m_bControlFullscreen) {
        FullscreenBox* pFullscreenBox = dynamic_cast<FullscreenBox*>(m_pRoot.get());
        if (pFullscreenBox != nullptr) {
            return nullptr;
        }
    }
    return m_shadow.get();
}

bool WindowRoot::IsControlFullscreen() const
{
    return m_bControlFullscreen;
}

void WindowRoot::SetControlFinder(ControlFinder* pControlFinder)
{
    m_pControlFinder = pControlFinder;
}

bool WindowRoot::AttachBox(Box* pRoot)
{
    if ((m_pRoot != nullptr) && (pRoot != m_pRoot)) {
        ClearRoot();
    }
    m_pRoot = pRoot;
    if (m_pControlFinder != nullptr) {
        m_pControlFinder->SetRoot(pRoot);
    }   
    return true;
}

Box* WindowRoot::AttachShadow(Box* pRoot)
{
    if (pRoot != nullptr) {
        //保存其Margin值
        m_rcXmlRootMargin = pRoot->GetMargin();
    }
    else {
        m_rcXmlRootMargin.Clear();
    }
    m_rcXmlRootMargin.Validate();

    Shadow* pShadow = GetShadow();
    ASSERT(pShadow != nullptr);
    if (pShadow != nullptr) {
        return pShadow->AttachShadow(pRoot);
    }
    else {
        return pRoot;
    }
}

void WindowRoot::ProcessWindowShadowTypeChanged()
{
    Shadow* pShadow = GetShadow();
    if (pShadow == nullptr) {
        return;
    }
    if (m_pRoot == nullptr) {
        return;//尚未绑定
    }
    //控件全屏时，禁止访问阴影相关操作
    if (m_bControlFullscreen) {
        return;
    }
    FullscreenBox* pFullscreenBox = dynamic_cast<FullscreenBox*>(m_pRoot.get());
    if (pFullscreenBox != nullptr) {
        return;
    }

    if (pShadow->IsShadowAttached() && !pShadow->IsSystemShadowEnabled()) {
        //需要绑定
        if (!pShadow->HasShadowBox()) {
            Box* pNewRoot = pShadow->AttachShadow(m_pRoot.get());
            if (pNewRoot != nullptr) {
                m_pRoot.reset(); //先清空，避免被释放
                AttachBox(pNewRoot);
            }
        }
    }
    else {
        //不需要绑定
        if ( pShadow->HasShadowBox() &&
             (pShadow->GetShadowBox() == m_pRoot.get()) &&
             (pShadow->GetAttachedXmlRoot() != nullptr)) {
            Box* pNewRoot = pShadow->DettachShadow();
            if (pNewRoot != nullptr) {
                m_pRoot.reset(); //先清空，避免被释放
                AttachBox(pNewRoot);
            }
        }
    }

    //检查系统阴影的边框设置
    UpdateXmlRootMargin();

    //重绘窗口，否则会有绘制异常
    m_pWindow->InvalidateAll();
}

void WindowRoot::SetShadowAttached(bool bShadowAttached)
{
    ASSERT(!IsControlFullscreen());    
    if (IsControlFullscreen()) {
        return;
    }
    if (bShadowAttached) {
        ASSERT(!m_pWindow->IsUseSystemCaption());
        if (m_pWindow->IsUseSystemCaption()) {
            return;
        }
    }
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowAttached(bShadowAttached);
        ProcessWindowShadowTypeChanged();
    }
}

bool WindowRoot::IsShadowAttached() const
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        return pShadow->IsShadowAttached();
    }
    else {
        return false;
    }
}

void WindowRoot::SetShadowType(ShadowType nShadowType)
{
    ASSERT(!IsControlFullscreen());
    if (IsControlFullscreen()) {
        return;
    }
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowType(nShadowType);
        ProcessWindowShadowTypeChanged();
    }
}

ShadowType WindowRoot::GetShadowType() const
{
    ShadowType nShadowType = ShadowType::kShadowDefault;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        nShadowType = pShadow->GetShadowType();
    }
    return nShadowType;
}

DString WindowRoot::GetShadowImage() const
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        return pShadow->GetShadowImage();
    }
    else {
        return DString();
    }
}

void WindowRoot::SetShadowImage(const DString& shadowImage)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowImage(shadowImage);
    }
}

void WindowRoot::SetShadowBorderSize(int32_t nShadowBorderSize)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowBorderSize(nShadowBorderSize);
    }
}

int32_t WindowRoot::GetShadowBorderSize() const
{
    int32_t nShadowBorderSize = 0;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        nShadowBorderSize = pShadow->GetShadowBorderSize();
    }
    return nShadowBorderSize;
}

void WindowRoot::SetShadowBorderColor(const DString& shadowBorderColor)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowBorderColor(shadowBorderColor);
    }
}

DString WindowRoot::GetShadowBorderColor() const
{
    DString shadowBorderColor;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        shadowBorderColor = pShadow->GetShadowBorderColor();
    }
    return shadowBorderColor;
}

UiPadding WindowRoot::GetCurrentShadowCorner() const
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        return pShadow->GetCurrentShadowCorner();
    }
    else {
        return UiPadding();
    }
}

UiPadding WindowRoot::GetShadowCorner() const
{
    UiPadding rcShadowCorner;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        rcShadowCorner = pShadow->GetShadowCorner();
    }
    return rcShadowCorner;
}

void WindowRoot::SetShadowCorner(const UiPadding& rcShadowCorner)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowCorner(rcShadowCorner);
    }
}

void WindowRoot::SetShadowBorderRound(UiSize szBorderRound)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetShadowBorderRound(szBorderRound);
    }
}

UiSize WindowRoot::GetShadowBorderRound() const
{
    UiSize szBorderRound;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        szBorderRound = pShadow->GetShadowBorderRound();
    }
    return szBorderRound;
}

void WindowRoot::SetEnableShadowSnap(bool bEnable)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetEnableShadowSnap(bEnable);
    }
}

bool WindowRoot::IsEnableShadowSnap() const
{
    bool bRet = false;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        bRet = pShadow->IsEnableShadowSnap();
    }
    return bRet;
}

void WindowRoot::SetWindowPosSnap(bool bLeftSnap, bool bRightSnap, bool bTopSnap, bool bBottomSnap)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->SetWindowPosSnap(bLeftSnap, bRightSnap, bTopSnap, bBottomSnap);
    }
}

void WindowRoot::ClearImageCache()
{
    Control* pRoot = nullptr;
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pRoot = pShadow->GetShadowBox();
    }
    if (pRoot == nullptr) {
        pRoot = m_pRoot.get();
    }
    if (pRoot != nullptr) {
        pRoot->ClearImageCache();
    }
}

void WindowRoot::ChangeDpiScale(const DpiManager& dpi, uint32_t nOldScaleFactor, uint32_t nNewScaleFactor)
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->ChangeDpiScale(dpi, nOldScaleFactor, nNewScaleFactor);
    }
}

UiMargin WindowRoot::GetWindowMaximizedMargin() const
{
    if (m_pWindow->IsUseSystemCaption() || !m_pWindow->IsWindowMaximized() || m_pWindow->IsWindowFullscreen()) {
        //使用系统标题栏，全屏或者不是最大化时，均不需要设置
        return UiMargin();
    }
    UiRect rcWindow;
    m_pWindow->GetWindowRect(rcWindow);
    UiRect rcClientRect;
    m_pWindow->GetClientRect(rcClientRect);
    int32_t cxClient = rcClientRect.Width();
    int32_t cyClient = rcClientRect.Height();
    const DpiManager& dpi = m_pWindow->Dpi();
    if (dpi.HasPixelDensity()) {
        dpi.UnscaleInt(cxClient);
        dpi.UnscaleInt(cyClient);
        dpi.ScaleWindowSize(cxClient);
        dpi.ScaleWindowSize(cyClient);
    }
    UiMargin rcFullscreenMargin;
    if ((cxClient == rcWindow.Width()) && (cyClient == rcWindow.Height())) { //只有客户区大小和窗口大小一致时需要处理
        //最大化时，设置外边距，避免客户区的内容溢出屏幕
        UiRect rcWork;
        m_pWindow->GetMonitorWorkRect(rcWork);
        if (rcWindow.left < rcWork.left) {
            rcFullscreenMargin.left = rcWork.left - rcWindow.left;
        }
        if (rcWindow.top < rcWork.top) {
            rcFullscreenMargin.top = rcWork.top - rcWindow.top;
        }
        if (rcWindow.right > rcWork.right) {
            rcFullscreenMargin.right = rcWindow.right - rcWork.right;
        }
        if (rcWindow.bottom > rcWork.bottom) {
            rcFullscreenMargin.bottom = rcWindow.bottom - rcWork.bottom;
        }
        if (dpi.HasPixelDensity()) {
            rcFullscreenMargin.left = (int32_t)std::round(rcFullscreenMargin.left * dpi.GetPixelDensity());
            rcFullscreenMargin.top = (int32_t)std::round(rcFullscreenMargin.top * dpi.GetPixelDensity());
            rcFullscreenMargin.right = (int32_t)std::round(rcFullscreenMargin.right * dpi.GetPixelDensity());
            rcFullscreenMargin.bottom = (int32_t)std::round(rcFullscreenMargin.bottom * dpi.GetPixelDensity());
        }        
    }
    rcFullscreenMargin.Validate();
    return rcFullscreenMargin;
}

void WindowRoot::UpdateXmlRootMargin()
{
    FullscreenBox* pFullscreenBox = dynamic_cast<FullscreenBox*>(m_pRoot.get());
    if (pFullscreenBox != nullptr) {
        return;
    }
    if (m_pWindow->IsWindowMinimized()) {
        //窗口最小化，不需要更新
        return;
    }
    Box* pXmlRoot = GetXmlRoot();
    if (pXmlRoot == nullptr) {
        return;
    }
    Shadow* pShadow = GetShadow();
    if (pShadow == nullptr) {
        return;
    }
    UiMargin rcWindowBorderMargin; //根容器的外边距
    if (pShadow->IsShadowAttached()      &&
        m_pWindow->IsUseSystemShadow()   &&
        !m_pWindow->IsChildWindow()      &&
        !m_pWindow->IsUseSystemCaption() &&
        !m_pWindow->IsWindowMaximized()  &&
        !m_pWindow->IsWindowFullscreen()) {
        //非最大化时，窗口边缘存在视觉边线，占用客户区，会覆盖客户区的内容（比如根容器设置边线时，会被覆盖掉）
        const int32_t nShadowFrameBorderSize = m_pWindow->NativeWnd()->GetSystemShadowFrameBorderSize();
        rcWindowBorderMargin.left = nShadowFrameBorderSize;
        rcWindowBorderMargin.top = nShadowFrameBorderSize;
        rcWindowBorderMargin.right = nShadowFrameBorderSize;
        rcWindowBorderMargin.bottom = nShadowFrameBorderSize;
    }
    rcWindowBorderMargin.Validate(); //窗口边线的占用所需的Margin
    UiMargin originXmlRootMargin = m_rcXmlRootMargin;       //XML Root Box原来的Margin
    UiMargin maximizedMargin = GetWindowMaximizedMargin();  //窗口最大化或者全屏时所需要设置的Margin（此时窗口边缘溢出可视区域）
    UiMargin rcRootBoxMargin;
    rcRootBoxMargin.left = originXmlRootMargin.left + maximizedMargin.left + rcWindowBorderMargin.left;
    rcRootBoxMargin.right = originXmlRootMargin.right + maximizedMargin.right + rcWindowBorderMargin.right;
    rcRootBoxMargin.top = originXmlRootMargin.top + maximizedMargin.top + rcWindowBorderMargin.top;
    rcRootBoxMargin.bottom = originXmlRootMargin.bottom + maximizedMargin.bottom + rcWindowBorderMargin.bottom;

    UiMargin rcMargin = pXmlRoot->GetMargin();
    if (rcRootBoxMargin != rcMargin) {
        pXmlRoot->SetMargin(rcRootBoxMargin, false);
    }
}

void WindowRoot::ProcessWindowMaximized()
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->MaximizedOrRestored(true);
    }
    ProcessWindowResized();
}

void WindowRoot::ProcessWindowRestored()
{
    Shadow* pShadow = GetShadow();
    if (pShadow != nullptr) {
        pShadow->MaximizedOrRestored(false);
    }
    ProcessWindowResized();
}

void WindowRoot::ProcessWindowResized()
{
    UpdateXmlRootMargin();
}

void WindowRoot::ProcessWindowEnterFullscreen()
{
    ProcessWindowResized();
}

void WindowRoot::ProcessWindowExitFullscreen()
{
    FullscreenBox* pFullscreenBox = dynamic_cast<FullscreenBox*>(m_pRoot.get());
    if (pFullscreenBox != nullptr) {
        //退出控件全屏状态
        m_pRoot = pFullscreenBox->GetOldRoot();
        ASSERT(m_pRoot != nullptr);
        if (m_pControlFinder != nullptr) {
            m_pControlFinder->SetRoot(m_pRoot.get());
        }
        pFullscreenBox->ExitControlFullscreen();
        if (m_pRoot != nullptr) {
            m_pRoot->SetVisible(true);
        }
        delete pFullscreenBox;
        pFullscreenBox = nullptr;
    }
    m_bControlFullscreen = false;
    ProcessWindowResized();
}

void WindowRoot::ProcessFullscreenButtonMouseMove(const UiPoint& pt)
{
    FullscreenBox* pFullscreenBox = dynamic_cast<FullscreenBox*>(m_pRoot.get());
    if (pFullscreenBox != nullptr) {
        pFullscreenBox->ProcessFullscreenButtonMouseMove(pt);
    }
}

bool WindowRoot::EnterControlFullscreen(Control* pFullscreenControl, const DString& exitButtonClass)
{
    FullscreenBox* pFullscreenBox = new FullscreenBox(m_pWindow);
    if (pFullscreenBox->EnterControlFullscreen(m_pRoot.get(), pFullscreenControl, exitButtonClass)) {
        //成功进入控件全屏状态
        if (m_pControlFinder != nullptr) {
            m_pControlFinder->SetRoot(pFullscreenBox);
        }
        m_pRoot = pFullscreenBox;
        m_bControlFullscreen = true;
        return true;
    }
    else {
        m_bControlFullscreen = false;
        delete pFullscreenBox;
        pFullscreenBox = nullptr;
        return false;
    }
}

bool WindowRoot::SetFullscreenControl(Control* pFullscreenControl, const DString& exitButtonClass)
{
    ASSERT(pFullscreenControl != nullptr);
    if (pFullscreenControl == nullptr) {
        return false;
    }
    ASSERT(m_pRoot != nullptr);
    if (m_pRoot == nullptr) {
        return false;
    }
    ASSERT(m_pRoot.get() != pFullscreenControl);
    if (m_pRoot.get() == pFullscreenControl) {
        return false;
    }

    bool bRet = false;
    FullscreenBox* pFullscreenBox = dynamic_cast<FullscreenBox*>(m_pRoot.get());
    if (pFullscreenBox != nullptr) {
        //当前已经是控件全屏状态
        if (pFullscreenBox->GetFullscreenControl() == pFullscreenControl) {
            return true;
        }
        ASSERT(m_bControlFullscreen);
        ASSERT(m_pWindow->IsWindowFullscreen());
        ASSERT(m_pRoot == pFullscreenBox);
        ASSERT(m_pControlFinder->GetRoot() == pFullscreenBox);
        if (m_bControlFullscreen && m_pWindow->IsWindowFullscreen() &&
            (m_pRoot == pFullscreenBox) && (m_pControlFinder->GetRoot() == pFullscreenBox)) {
            //仅切换全屏控件，不改变全屏状态
            if (pFullscreenBox->UpdateControlFullscreen(pFullscreenControl, exitButtonClass)) {
                //复位控件的状态
                m_pWindow->ClearInputStatus();
                //设置焦点
                pFullscreenControl->SetFocus();
                bRet = true;
            }
        }
    }
    else {
        //原来不是控件全屏状态
        if (EnterControlFullscreen(pFullscreenControl, exitButtonClass)) {
            //窗口进入全屏状态
            m_pWindow->EnterFullscreen();
            //复位控件的状态
            m_pWindow->ClearInputStatus();
            //设置焦点
            pFullscreenControl->SetFocus();
            bRet = true;
        }
    }
    return bRet;
}

Control* WindowRoot::GetFullscreenControl() const
{
    if (m_bControlFullscreen) {
        FullscreenBox* pFullscreenBox = dynamic_cast<FullscreenBox*>(m_pRoot.get());
        if (pFullscreenBox != nullptr) {
            return pFullscreenBox->GetFullscreenControl();
        }
    }
    return nullptr;
}

void WindowRoot::ExitControlFullscreen()
{
    if (m_bControlFullscreen) {
        FullscreenBox* pFullscreenBox = dynamic_cast<FullscreenBox*>(m_pRoot.get());
        if (pFullscreenBox != nullptr) {
            //退出控件全屏
            bool bWindowOldFullscreen = pFullscreenBox->IsWindowOldFullscreen();
            ProcessWindowExitFullscreen();
            if (bWindowOldFullscreen) {
                //窗口原来就是全屏，不需要退出窗口全屏状态
                return;
            }
        }
    }
    //退出窗口全屏
    m_pWindow->ExitFullscreen();
}

} // namespace ui

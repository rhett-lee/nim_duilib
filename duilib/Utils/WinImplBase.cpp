#include "WinImplBase.h"
#include "duilib/Core/WindowBuilder.h"
#include "duilib/Core/Box.h"
#include "duilib/Utils/FilePath.h"

namespace ui
{

WindowImplBase::WindowImplBase():
    m_pMaxButton(nullptr),
    m_pMinButton(nullptr),
    m_pRestoreButton(nullptr)
{
}

WindowImplBase::~WindowImplBase()
{
}

void WindowImplBase::PreInitWindow()
{
    BaseClass::PreInitWindow();
    if (!IsUseSystemCaption()) {
        //关闭按钮
        Control* pControl = FindControl(DUI_CTR_BUTTON_CLOSE);
        if (pControl) {
            ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
            pControl->AttachClick(UiBind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));
        }

        //最小化按钮
        pControl = FindControl(DUI_CTR_BUTTON_MIN);
        if (pControl) {
            ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
            pControl->AttachClick(UiBind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));

            m_pMinButton = pControl;
            m_minButtonFlag = pControl->GetWeakFlag();
        }

        //最大化按钮
        pControl = FindControl(DUI_CTR_BUTTON_MAX);
        if (pControl) {
            ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
            pControl->AttachClick(UiBind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));

            m_pMaxButton = pControl;
            m_maxButtonFlag = pControl->GetWeakFlag();
        }

        //还原按钮
        pControl = FindControl(DUI_CTR_BUTTON_RESTORE);
        if (pControl) {
            ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
            pControl->AttachClick(UiBind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));

            m_pRestoreButton = pControl;
            m_restoreButtonFlag = pControl->GetWeakFlag();
        }
        
        //全屏按钮
        pControl = FindControl(DUI_CTR_BUTTON_FULLSCREEN);
        if (pControl) {
            ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
            pControl->AttachClick(UiBind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));
        }

#if defined (DUILIB_BUILD_FOR_SDL) && !defined (DUILIB_BUILD_FOR_WIN)
        //标题栏: 由于使用SDL时，不支持双击标题栏最大化/还原窗口，所以自己实现此逻辑（非Windows平台）
        pControl = FindControl(DUI_CTR_CAPTION_BAR);
        if (pControl) {
            pControl->AttachBubbledEvent(ui::kEventMouseDoubleClick, UiBind(&WindowImplBase::OnTitleBarDoubleClick, this, std::placeholders::_1));
        }
#endif
    }
}

DString WindowImplBase::GetSkinFolder()
{
    return BaseClass::GetSkinFolder();
}

DString WindowImplBase::GetSkinFile()
{
    return BaseClass::GetSkinFile();
}

Control* WindowImplBase::CreateControl(const DString& strClass)
{
    return BaseClass::CreateControl(strClass);
}

void WindowImplBase::OnInitWindow()
{
    BaseClass::OnInitWindow();
}

void WindowImplBase::OnPreCloseWindow()
{
    BaseClass::OnPreCloseWindow();
}

void WindowImplBase::OnCloseWindow()
{
    BaseClass::OnCloseWindow();
}

void WindowImplBase::OnFinalMessage()
{
    BaseClass::OnFinalMessage();
}

bool WindowImplBase::OnButtonClick(const EventArgs& msg)
{
    if (IsUseSystemCaption()) {
        return true;
    }
    Control* pSender = msg.GetSender();
    ASSERT(pSender != nullptr);
    if (pSender == nullptr) {
        return false;
    }
    DString sCtrlName = pSender->GetName();
    if (sCtrlName == DUI_CTR_BUTTON_CLOSE) {
        //关闭按钮
        CloseWnd();
    }
    else if (sCtrlName == DUI_CTR_BUTTON_MIN) {
        //最小化按钮
        ShowWindow(kSW_MINIMIZE);
    }
    else if (sCtrlName == DUI_CTR_BUTTON_MAX) {
        //最大化按钮        
        ShowWindow(kSW_SHOW_MAXIMIZED);
    }
    else if (sCtrlName == DUI_CTR_BUTTON_RESTORE) {
        //还原按钮        
        ShowWindow(kSW_RESTORE);
    }
    else if (sCtrlName == DUI_CTR_BUTTON_FULLSCREEN) {
        //全屏按钮
        EnterFullScreen();
    }

    return true;
}

bool WindowImplBase::OnTitleBarDoubleClick(const EventArgs& /*param*/)
{
    Control* pControl = FindControl(DUI_CTR_BUTTON_MAX);
    if ((pControl != nullptr) && pControl->IsVisible()) {
        //最大化按钮
        if (!IsWindowMaximized()) {
            ShowWindow(kSW_SHOW_MAXIMIZED);
        }
    }
    else {
        //还原按钮
        pControl = FindControl(DUI_CTR_BUTTON_RESTORE);
        if ((pControl != nullptr) && pControl->IsVisible()) {
            if (IsWindowMaximized()) {
                ShowWindow(kSW_RESTORE);
            }            
        }
    }
    return true;
}

LRESULT WindowImplBase::OnSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, const NativeMsg& nativeMsg, bool& bHandled)
{
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    LRESULT lResult = BaseClass::OnSizeMsg(sizeType, newWindowSize, nativeMsg, bHandled);
    if (windowFlag.expired()) {
        return lResult;
    }
    if (sizeType == WindowSizeType::kSIZE_MAXIMIZED) {
        //窗口最大化
        if (!IsWindowFullScreen()) {
            OnWindowMaximized();
        }
    }
    else if (sizeType == WindowSizeType::kSIZE_RESTORED) {
        //窗口还原
        if (!IsWindowFullScreen()) {
            OnWindowRestored();
        }
    }
    else if (sizeType == WindowSizeType::kSIZE_MINIMIZED) {
        //窗口最小化
        if (!IsWindowFullScreen()) {
            OnWindowMinimized();
        }
    }
    return lResult;
}

void WindowImplBase::OnWindowEnterFullScreen()
{
    if (!IsUseSystemCaption()) {
        Control* pCaptionBar = FindControl(DUI_CTR_CAPTION_BAR);
        if (pCaptionBar != nullptr) {
            pCaptionBar->SetVisible(false);
        }
    }
}

void WindowImplBase::OnWindowExitFullScreen()
{
    if (!IsUseSystemCaption()) {
        Control* pCaptionBar = FindControl(DUI_CTR_CAPTION_BAR);
        if (pCaptionBar != nullptr) {
            pCaptionBar->SetVisible(true);
        }
        ProcessMaxRestoreStatus();
    }
}

void WindowImplBase::OnWindowMaximized()
{
    ProcessMaxRestoreStatus();
}

void WindowImplBase::OnWindowRestored()
{
    ProcessMaxRestoreStatus();
}

void WindowImplBase::OnWindowMinimized()
{
}

void WindowImplBase::OnUseSystemCaptionBarChanged()
{
    BaseClass::OnUseSystemCaptionBarChanged();
    if (GetRoot() == nullptr) {
        return;
    }
    Control* pCaptionBar = FindControl(DUI_CTR_CAPTION_BAR);
    if (pCaptionBar != nullptr) {
        pCaptionBar->SetVisible(!IsUseSystemCaption());
        if (!IsUseSystemCaption()) {
            ProcessMaxRestoreStatus();
        }
    }
}

void WindowImplBase::OnWindowDpiChanged(uint32_t nOldDPI, uint32_t nNewDPI)
{
    BaseClass::OnWindowDpiChanged(nOldDPI, nNewDPI);
}

void WindowImplBase::ProcessMaxRestoreStatus()
{
    if (IsUseSystemCaption() || (GetRoot() == nullptr)) {
        return;
    }
    Control* pMaxButton = FindControl(DUI_CTR_BUTTON_MAX);
    Control* pRestoreButton = FindControl(DUI_CTR_BUTTON_RESTORE);
    bool bWindowMax = IsWindowMaximized();
    if (pMaxButton != nullptr) {
        pMaxButton->SetFadeVisible(bWindowMax ? false : true);
    }
    if (pRestoreButton != nullptr) {
        pRestoreButton->SetFadeVisible(bWindowMax ? true : false);
    }
}

bool WindowImplBase::HasMinMaxBox(bool& bMinimizeBox, bool& bMaximizeBox) const
{
    bMinimizeBox = false;
    bMaximizeBox = false;
    if ((m_pMinButton != nullptr) && !m_minButtonFlag.expired()) {
        bMinimizeBox = true;
    }
    if ((m_pMaxButton != nullptr) && !m_maxButtonFlag.expired()) {
        bMaximizeBox = true;
    }
    return true;
}

bool WindowImplBase::IsPtInMaximizeRestoreButton(const UiPoint& pt) const
{
    bool bInButton = false;
    if ((m_pMaxButton != nullptr) && !m_maxButtonFlag.expired() && m_pMaxButton->IsVisible()) {
        bInButton = m_pMaxButton->GetRect().ContainsPt(pt);
    }
    else if ((m_pRestoreButton != nullptr) && !m_restoreButtonFlag.expired() && m_pRestoreButton->IsVisible()) {
        bInButton = m_pRestoreButton->GetRect().ContainsPt(pt);
    }
    return bInButton;
}


}

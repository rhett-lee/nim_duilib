#include "WinImplBase.h"
#include "duilib/Core/WindowBuilder.h"
#include "duilib/Core/Box.h"

namespace ui
{

WindowImplBase::WindowImplBase()
{
}

WindowImplBase::~WindowImplBase()
{
}

void WindowImplBase::InitWindow()
{
    __super::InitWindow();

    SetResourcePath(GetSkinFolder());
    DString strSkinFile;
    DString xmlFile = GetSkinFile();
    if (!xmlFile.empty() && xmlFile.front() == _T('<')) {
        //返回的内容是XML文件内容，而不是文件路径
        strSkinFile = std::move(xmlFile);
    }
    else {
        strSkinFile = GetResourcePath() + xmlFile;
    }
    auto callback = UiBind(&WindowImplBase::CreateControl, this, std::placeholders::_1);
    WindowBuilder builder;
    Box* pRoot = builder.Create(strSkinFile, callback, this);

    ASSERT(pRoot && _T("Faield to load xml file."));
    if (pRoot == nullptr) {
        return;
    }

    if (IsUseSystemCaption()) {
        //关闭阴影
        SetShadowAttached(false);
    }

    //关联窗口附加阴影
    pRoot = AttachShadow(pRoot);

    //关联Root对象
    AttachBox(pRoot);

    //更新自绘制标题栏状态
    OnUseSystemCaptionBarChanged();
    if (!IsUseSystemCaption()) {
        Control* pControl = (Control*)FindControl(DUI_CTR_BUTTON_CLOSE);
        if (pControl) {
            ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
            pControl->AttachClick(UiBind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));
        }

        pControl = (Control*)FindControl(DUI_CTR_BUTTON_MIN);
        if (pControl) {
            ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
            pControl->AttachClick(UiBind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));
        }

        pControl = (Control*)FindControl(DUI_CTR_BUTTON_MAX);
        if (pControl) {
            ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
            pControl->AttachClick(UiBind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));
        }

        pControl = (Control*)FindControl(DUI_CTR_BUTTON_RESTORE);
        if (pControl) {
            ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
            pControl->AttachClick(UiBind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));
        }
        pControl = (Control*)FindControl(DUI_CTR_BUTTON_FULLSCREEN);
        if (pControl) {
            ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
            pControl->AttachClick(UiBind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));
        }
    }
}

void WindowImplBase::OnInitWindow()
{
    __super::OnInitWindow();    
}

void WindowImplBase::OnCloseWindow()
{
    __super::OnCloseWindow();
}

void WindowImplBase::OnFinalMessage()
{
    __super::OnFinalMessage();
}

Control* WindowImplBase::CreateControl(const DString& /*strClass*/)
{
    return nullptr;
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

LRESULT WindowImplBase::OnSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, bool& bHandled)
{
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    LRESULT lResult = __super::OnSizeMsg(sizeType, newWindowSize, bHandled);
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
    __super::OnUseSystemCaptionBarChanged();
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
    __super::OnWindowDpiChanged(nOldDPI, nNewDPI);
}

void WindowImplBase::ProcessMaxRestoreStatus()
{
    if (IsUseSystemCaption() || (GetRoot() == nullptr)) {
        return;
    }
    Control* pMaxButton = (Control*)FindControl(DUI_CTR_BUTTON_MAX);
    Control* pRestoreButton = (Control*)FindControl(DUI_CTR_BUTTON_RESTORE);
    bool bWindowMax = IsWindowMaximized();
    if (pMaxButton != nullptr) {
        pMaxButton->SetFadeVisible(bWindowMax ? false : true);
    }
    if (pRestoreButton != nullptr) {
        pRestoreButton->SetFadeVisible(bWindowMax ? true : false);
    }
}

}

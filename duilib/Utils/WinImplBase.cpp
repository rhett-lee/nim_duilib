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
    __super::PreInitWindow();

    //保存资源所在路径(不支持绝对路径)
    FilePath skinFolder(GetSkinFolder());
    ASSERT(!skinFolder.IsAbsolutePath());
    if (skinFolder.IsAbsolutePath()) {
        return;
    }

    SetResourcePath(skinFolder);
    SetXmlPath(FilePath());

    //XML文件所在路径，应是相对路
    DString xmlFile = GetSkinFile();
    DString skinXmlFileData;
    FilePath skinXmlFilePath;
    if (!xmlFile.empty() && xmlFile.front() == _T('<')) {
        //返回的内容是XML文件内容，而不是文件路径        
        skinXmlFileData = std::move(xmlFile);
    }
    else {
        FilePath xmlFilePath(xmlFile);
        ASSERT(!xmlFilePath.IsAbsolutePath());
        if (xmlFilePath.IsAbsolutePath()) {
            return;
        }

        //保存XML文件所在路径
        size_t nPos = xmlFile.find_last_of(_T("/\\"));
        if (nPos != DString::npos) {
            DString xmlPath = xmlFile.substr(0, nPos);
            if (!xmlPath.empty()) {
                SetXmlPath(FilePath(xmlPath));
            }
        }
        skinXmlFilePath = GetResourcePath();
        skinXmlFilePath.JoinFilePath(xmlFilePath);
    }
    auto callback = UiBind(&WindowImplBase::CreateControl, this, std::placeholders::_1);
    WindowBuilder builder;
    Box* pRoot = nullptr;
    if (!skinXmlFileData.empty()) {
        Control* pControl = builder.CreateFromXmlData(skinXmlFileData, callback, this);
        pRoot = builder.ToBox(pControl);
    }
    else {
        ASSERT(!skinXmlFilePath.IsEmpty());
        Control* pControl = builder.CreateFromXmlFile(skinXmlFilePath, callback, this);
        pRoot = builder.ToBox(pControl);
    }

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
    }
}

DString WindowImplBase::GetSkinFolder()
{
    return m_skinFolder;
}

DString WindowImplBase::GetSkinFile()
{
    return m_skinFile;
}

void WindowImplBase::InitSkin(const DString& skinFolder, const DString& skinFile)
{
    m_skinFolder = skinFolder;
    m_skinFile = skinFile;
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

LRESULT WindowImplBase::OnSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, const NativeMsg& nativeMsg, bool& bHandled)
{
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    LRESULT lResult = __super::OnSizeMsg(sizeType, newWindowSize, nativeMsg, bHandled);
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

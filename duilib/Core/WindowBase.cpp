#include "WindowBase.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/WindowDropTarget.h"
#include "duilib/Core/WindowCreateAttributes.h"

namespace ui
{
WindowBase::WindowBase():
    m_pParentWindow(nullptr), 
    m_pWindowDropTarget(nullptr),
    m_pNativeWindow(nullptr)
{
    m_pNativeWindow = new NativeWindow(this);
}

WindowBase::~WindowBase()
{
    ClearWindowBase();
    if (m_pNativeWindow != nullptr) {
        delete m_pNativeWindow;
        m_pNativeWindow = nullptr;
    }
}

bool WindowBase::CreateWnd(WindowBase* pParentWindow, const WindowCreateParam& createParam)
{
    //解析XML，读取窗口的属性参数
    WindowCreateAttributes createAttributes;
    GetCreateWindowAttributes(createAttributes);

    m_pParentWindow = pParentWindow;
    m_parentFlag.reset();
    if (pParentWindow != nullptr) {
        m_parentFlag = pParentWindow->GetWeakFlag();
    }
    NativeWindow* pNativeWindow = pParentWindow != nullptr ? pParentWindow->NativeWnd() : nullptr;
    return m_pNativeWindow->CreateWnd(pNativeWindow, createParam, createAttributes);
}

int32_t WindowBase::DoModal(WindowBase* pParentWindow, const WindowCreateParam& createParam,
                            bool bCenterWindow, bool bCloseByEsc, bool bCloseByEnter)
{
    //解析XML，读取窗口的属性参数
    WindowCreateAttributes createAttributes;
    GetCreateWindowAttributes(createAttributes);

    m_pParentWindow = pParentWindow;
    m_parentFlag.reset();
    if (pParentWindow != nullptr) {
        m_parentFlag = pParentWindow->GetWeakFlag();
    }
    NativeWindow* pNativeWindow = pParentWindow != nullptr ? pParentWindow->NativeWnd() : nullptr;
    return m_pNativeWindow->DoModal(pNativeWindow, createParam, createAttributes, bCenterWindow, bCloseByEsc, bCloseByEnter);
}

void WindowBase::OnNativeCreateWndMsg(bool bDoModal, const NativeMsg& nativeMsg, bool& bHandled)
{
    //窗口完成创建，初始化（内部使用）
    InitWindowBase();

    //回调，让子类解析XML文件并与窗口绑定（内部使用，子类可重写）
    PreInitWindow();

    //内部初始化（内部使用）
    PostInitWindow();

    //调用子类的初始化函数
    OnInitWindow();    
    OnCreateWndMsg(bDoModal, nativeMsg, bHandled);
}

void WindowBase::ClearWindowBase()
{
    //注销拖放操作
    if (m_pWindowDropTarget != nullptr) {
        m_pWindowDropTarget->Clear();
        delete m_pWindowDropTarget;
        m_pWindowDropTarget = nullptr;
    }
    m_pParentWindow = nullptr;
    m_parentFlag.reset();
    m_dpi.reset();

    //清除Native窗口的资源
    m_pNativeWindow->ClearNativeWindow();
}

void WindowBase::SetUseSystemCaption(bool bUseSystemCaption)
{
    m_pNativeWindow->SetUseSystemCaption(bUseSystemCaption);
}

bool WindowBase::IsUseSystemCaption() const
{
    return m_pNativeWindow->IsUseSystemCaption();
}

void WindowBase::SetLayeredWindowAlpha(int32_t nAlpha)
{
    m_pNativeWindow->SetLayeredWindowAlpha(nAlpha);
    OnWindowAlphaChanged();
}

uint8_t WindowBase::GetLayeredWindowAlpha() const
{
    return m_pNativeWindow->GetLayeredWindowAlpha();
}

void WindowBase::SetLayeredWindowOpacity(int32_t nAlpha)
{
    m_pNativeWindow->SetLayeredWindowOpacity(nAlpha);
    OnWindowAlphaChanged();
}

uint8_t WindowBase::GetLayeredWindowOpacity() const
{
    return m_pNativeWindow->GetLayeredWindowOpacity();
}


bool WindowBase::SetLayeredWindow(bool bIsLayeredWindow, bool bRedraw)
{
    bool bRet = m_pNativeWindow->SetLayeredWindow(bIsLayeredWindow, bRedraw);
    OnLayeredWindowChanged();
    return bRet;
}

bool WindowBase::IsLayeredWindow() const
{
    return m_pNativeWindow->IsLayeredWindow();
}

void WindowBase::CloseWnd(int32_t nRet)
{
    m_pNativeWindow->CloseWnd(nRet);
}

void WindowBase::Close()
{
    m_pNativeWindow->Close();
}

bool WindowBase::IsClosingWnd() const
{
    return m_pNativeWindow->IsClosingWnd();
}

int32_t WindowBase::GetCloseParam() const
{
    return m_pNativeWindow->GetCloseParam();
}

bool WindowBase::AddMessageFilter(IUIMessageFilter* pFilter)
{
    if (std::find(m_aMessageFilters.begin(), m_aMessageFilters.end(), pFilter) != m_aMessageFilters.end()) {
        ASSERT(false);
        return false;
    }
    if (pFilter != nullptr) {
        m_aMessageFilters.push_back(pFilter);
    }
    return true;
}
bool WindowBase::RemoveMessageFilter(IUIMessageFilter* pFilter)
{
    auto iter = std::find(m_aMessageFilters.begin(), m_aMessageFilters.end(), pFilter);
    if (iter != m_aMessageFilters.end()) {
        m_aMessageFilters.erase(iter);
        return true;
    }
    return false;
}

LRESULT WindowBase::PostMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return m_pNativeWindow->PostMsg(uMsg, wParam, lParam);
}

void WindowBase::PostQuitMsg(int32_t nExitCode)
{
    return m_pNativeWindow->PostQuitMsg(nExitCode);
}

WindowBase* WindowBase::WindowBaseFromPoint(const UiPoint& pt)
{
    WindowBase* pWindowBase = nullptr;
    INativeWindow* pNativeWindow = m_pNativeWindow->WindowBaseFromPoint(pt);
    if (pNativeWindow != nullptr) {
        pWindowBase = dynamic_cast<WindowBase*>(pNativeWindow);
    }
    return pWindowBase;
}

NativeWindow* WindowBase::NativeWnd() const
{
    return m_pNativeWindow;
}

WindowBase* WindowBase::GetParentWindow() const
{
    if (!m_parentFlag.expired()) {
        return m_pParentWindow;
    }
    else {
        return nullptr;
    }
}

bool WindowBase::IsWindow() const
{
    return m_pNativeWindow->IsWindow();
}

void WindowBase::InitWindowBase()
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return;
    }
    //初始化窗口自身的DPI管理器
    const DpiManager& dpiManager = GlobalManager::Instance().Dpi();
    if (!dpiManager.IsUserDefineDpi() && dpiManager.IsPerMonitorDpiAware()) {
        //每个显示器，有独立的DPI：初始化窗口自己的DPI管理器
        m_dpi = std::make_unique<DpiManager>();
        m_dpi->SetDpiByWindow(this);
    }
}

void WindowBase::GetClientRect(UiRect& rcClient) const
{
    m_pNativeWindow->GetClientRect(rcClient);
}

void WindowBase::GetWindowRect(UiRect& rcWindow) const
{
    m_pNativeWindow->GetWindowRect(rcWindow);
}

void WindowBase::ScreenToClient(UiPoint& pt) const
{
    m_pNativeWindow->ScreenToClient(pt);
}

void WindowBase::ClientToScreen(UiPoint& pt) const
{
    m_pNativeWindow->ClientToScreen(pt);
}

void WindowBase::ClientToScreen(UiRect& rc) const
{
    UiPoint pt;
    pt.x = rc.left;
    pt.y = rc.top;
    ClientToScreen(pt);
    rc.right = pt.x + rc.Width();
    rc.left = pt.x;
    rc.bottom = pt.y + rc.Height();
    rc.top = pt.y;
}

void WindowBase::GetCursorPos(UiPoint& pt) const
{
    m_pNativeWindow->GetCursorPos(pt);
}

bool WindowBase::GetMonitorRect(UiRect& rcMonitor) const
{
    return m_pNativeWindow->GetMonitorRect(rcMonitor);
}

bool WindowBase::GetPrimaryMonitorWorkRect(UiRect& rcWork)
{
    return NativeWindow::GetPrimaryMonitorWorkRect(rcWork);
}

bool WindowBase::GetMonitorWorkRect(UiRect& rcWork) const
{
    return m_pNativeWindow->GetMonitorWorkRect(rcWork);
}

bool WindowBase::GetMonitorWorkRect(const UiPoint& pt, UiRect& rcWork) const
{
    return m_pNativeWindow->GetMonitorWorkRect(pt, rcWork);
}

void WindowBase::SetCapture()
{
    m_pNativeWindow->SetCapture();
}

void WindowBase::ReleaseCapture()
{
    m_pNativeWindow->ReleaseCapture();
}

bool WindowBase::IsCaptured() const
{
    return m_pNativeWindow->IsCaptured();
}

bool WindowBase::ShowWindow(ShowWindowCommands nCmdShow)
{
    return m_pNativeWindow->ShowWindow(nCmdShow);
}

void WindowBase::ShowModalFake()
{
    NativeWindow* pNativeWindow = GetParentWindow() != nullptr ? GetParentWindow()->NativeWnd() : nullptr;
    m_pNativeWindow->ShowModalFake(pNativeWindow);
}

bool WindowBase::IsFakeModal() const
{
    return m_pNativeWindow->IsFakeModal();
}

bool WindowBase::IsDoModal() const
{
    return m_pNativeWindow->IsDoModal();
}

void WindowBase::CenterWindow()
{
    m_pNativeWindow->CenterWindow();
}

void WindowBase::SetWindowAlwaysOnTop(bool bOnTop)
{
    m_pNativeWindow->SetWindowAlwaysOnTop(bOnTop);
}

bool WindowBase::IsWindowAlwaysOnTop() const
{
    return m_pNativeWindow->IsWindowAlwaysOnTop();
}

bool WindowBase::SetWindowForeground()
{
    return m_pNativeWindow->SetWindowForeground();
}

bool WindowBase::IsWindowForeground() const
{
    return m_pNativeWindow->IsWindowForeground();
}

bool WindowBase::SetWindowFocus()
{
    return m_pNativeWindow->SetWindowFocus();
}

bool WindowBase::KillWindowFocus()
{
    return m_pNativeWindow->KillWindowFocus();
}

bool WindowBase::IsWindowFocused() const
{
    return m_pNativeWindow->IsWindowFocused();
}

void WindowBase::CheckSetWindowFocus()
{
    return m_pNativeWindow->CheckSetWindowFocus();
}

bool WindowBase::EnterFullScreen()
{
    return m_pNativeWindow->EnterFullScreen();
}

bool WindowBase::ExitFullScreen()
{
    return m_pNativeWindow->ExitFullScreen();
}

bool WindowBase::IsWindowMaximized() const
{
    return m_pNativeWindow->IsWindowMaximized();
}

bool WindowBase::IsWindowMinimized() const
{
    return m_pNativeWindow->IsWindowMinimized();
}

bool WindowBase::IsWindowFullScreen() const
{
    return m_pNativeWindow->IsWindowFullScreen();
}

bool WindowBase::EnableWindow(bool bEnable)
{
    return m_pNativeWindow->EnableWindow(bEnable);
}

bool WindowBase::IsWindowEnabled() const
{
    return m_pNativeWindow->IsWindowEnabled();
}

bool WindowBase::IsWindowVisible() const
{
    return m_pNativeWindow->IsWindowVisible();
}

bool WindowBase::SetWindowPos(const InsertAfterWnd& insertAfter, int32_t X, int32_t Y, int32_t cx, int32_t cy, uint32_t uFlags)
{
    NativeWindow* pNativeWindow = insertAfter.m_pWindow != nullptr ? insertAfter.m_pWindow->NativeWnd() : nullptr;
    return m_pNativeWindow->SetWindowPos(pNativeWindow, insertAfter.m_hwndFlag, X, Y, cx, cy, uFlags);
}

bool WindowBase::MoveWindow(int32_t X, int32_t Y, int32_t nWidth, int32_t nHeight, bool bRepaint)
{
    return m_pNativeWindow->MoveWindow(X, Y, nWidth, nHeight, bRepaint);
}

UiRect WindowBase::GetWindowPos(bool bContainShadow) const
{
    ASSERT(IsWindow());
    UiRect rcPos;
    GetWindowRect(rcPos);
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        rcPos.Deflate(rcShadow);
    }
    return rcPos;
}

void WindowBase::Resize(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
    ASSERT(cx >= 0 && cy >= 0);
    if (cx < 0) {
        cx = 0;
    }
    if (cy < 0) {
        cy = 0;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(cy);
        Dpi().ScaleInt(cx);
    }

    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        cx += rcShadow.left + rcShadow.right;
        cy += rcShadow.top + rcShadow.bottom;
    }
    ASSERT(IsWindow());
    WindowBase::SetWindowPos(InsertAfterWnd(), 0, 0, cx, cy, kSWP_NOZORDER | kSWP_NOMOVE | kSWP_NOACTIVATE);
}

bool WindowBase::SetWindowIcon(const FilePath& iconFilePath)
{
    return m_pNativeWindow->SetWindowIcon(iconFilePath);
}

bool WindowBase::SetWindowIcon(const std::vector<uint8_t>& iconFileData, const DString& iconFileName)
{
    return m_pNativeWindow->SetWindowIcon(iconFileData, iconFileName);
}

void WindowBase::SetText(const DString& strText)
{
    m_pNativeWindow->SetText(strText);
    m_text = strText;
}

const DString& WindowBase::GetText() const
{
    return m_text;
}

void WindowBase::SetTextId(const DString& strTextId)
{
    ASSERT(IsWindow());
    m_pNativeWindow->SetText(GlobalManager::Instance().Lang().GetStringViaID(strTextId));
    m_textId = strTextId;
}

const DString& WindowBase::GetTextId() const
{
    return m_textId;
}

const DpiManager& WindowBase::Dpi() const
{
    return (m_dpi != nullptr) ? *m_dpi : GlobalManager::Instance().Dpi();
}

bool WindowBase::ChangeDpi(uint32_t nNewDPI)
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    //DPI值限制在60到300之间(小于50的时候，会出问题，比如原来是1的，经过DPI转换后，会变成0，导致很多逻辑失效)
    ASSERT((nNewDPI >= 60) && (nNewDPI <= 300)) ;
    if ((nNewDPI < 60) || (nNewDPI > 300)) {
        return false;
    }

    uint32_t nOldDPI = Dpi().GetDPI();
    uint32_t nOldDpiScale = Dpi().GetScale();
    if (m_dpi == nullptr) {
        m_dpi = std::make_unique<DpiManager>();
    }
    //更新窗口的DPI值为新值
    m_dpi->SetDPI(nNewDPI);

    ASSERT(nNewDPI == m_dpi->GetDPI());
    uint32_t nNewDpiScale = m_dpi->GetScale();

    //按新的DPI更新窗口布局
    OnDpiScaleChanged(nOldDpiScale, nNewDpiScale);

    //更新窗口大小和位置
    UiRect rcOldWindow;
    GetWindowRect(rcOldWindow);
    UiRect rcNewWindow = Dpi().GetScaleRect(rcOldWindow, nOldDpiScale);
    m_pNativeWindow->MoveWindow(rcOldWindow.left, rcOldWindow.top, rcNewWindow.Width(), rcNewWindow.Height(), true);
    OnWindowDpiChanged(nOldDPI, nNewDPI);
    return true;
}

void WindowBase::ProcessDpiChangedMsg(uint32_t nNewDPI, const UiRect& /*rcNewWindow*/)
{
    //此消息必须处理，否则窗口大小与界面的比例将失调
    const DpiManager& dpiManager = GlobalManager::Instance().Dpi();
    if ((m_dpi != nullptr) && dpiManager.IsPerMonitorDpiAware()) {
        //调整DPI值
        uint32_t nOldDPI = m_dpi->GetDPI();
        uint32_t nOldDpiScale = m_dpi->GetScale();

        //更新窗口的DPI值为新值
        m_dpi->SetDPI(nNewDPI);
        ASSERT(nNewDPI == m_dpi->GetDPI());
        uint32_t nNewDpiScale = m_dpi->GetScale();

        //按新的DPI更新窗口布局
        OnDpiScaleChanged(nOldDpiScale, nNewDpiScale);
        OnWindowDpiChanged(nOldDPI, nNewDPI);
    }
}

void WindowBase::OnDpiScaleChanged(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if ((nOldDpiScale == nNewDpiScale) || (nNewDpiScale == 0)) {
        return;
    }
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    UiSize szMinWindow = NativeWnd()->GetWindowMinimumSize();
    szMinWindow = Dpi().GetScaleSize(szMinWindow, nOldDpiScale);
    NativeWnd()->SetWindowMinimumSize(szMinWindow);

    UiSize szMaxWindow = NativeWnd()->GetWindowMaximumSize();    
    szMaxWindow = Dpi().GetScaleSize(szMaxWindow, nOldDpiScale);    
    NativeWnd()->SetWindowMaximumSize(szMaxWindow);

    m_rcSizeBox = Dpi().GetScaleRect(m_rcSizeBox, nOldDpiScale);
    m_szRoundCorner = Dpi().GetScaleSize(m_szRoundCorner, nOldDpiScale);
    m_rcCaption = Dpi().GetScaleRect(m_rcCaption, nOldDpiScale);
}

bool WindowBase::SetWindowRoundRectRgn(const UiRect& rcWnd, const UiSize& szRoundCorner, bool bRedraw)
{
    return m_pNativeWindow->SetWindowRoundRectRgn(rcWnd, szRoundCorner, bRedraw);
}

void WindowBase::ClearWindowRgn(bool bRedraw)
{
    m_pNativeWindow->ClearWindowRgn(bRedraw);
}

void WindowBase::Invalidate(const UiRect& rcItem)
{
    GlobalManager::Instance().AssertUIThread();
    m_pNativeWindow->Invalidate(rcItem);
}

bool WindowBase::UpdateWindow() const
{
    return m_pNativeWindow->UpdateWindow();
}

void WindowBase::KeepParentActive()
{
    m_pNativeWindow->KeepParentActive();
}

const UiRect& WindowBase::GetSizeBox() const
{
    return m_rcSizeBox;
}

void WindowBase::SetSizeBox(const UiRect& rcSizeBox, bool bNeedDpiScale)
{
    m_rcSizeBox = rcSizeBox;
    if (bNeedDpiScale) {
        Dpi().ScaleRect(m_rcSizeBox);
    }
    if (m_rcSizeBox.left < 0) {
        m_rcSizeBox.left = 0;
    }
    if (m_rcSizeBox.top < 0) {
        m_rcSizeBox.top = 0;
    }
    if (m_rcSizeBox.right < 0) {
        m_rcSizeBox.right = 0;
    }
    if (m_rcSizeBox.bottom < 0) {
        m_rcSizeBox.bottom = 0;
    }
}

const UiRect& WindowBase::GetCaptionRect() const
{
    return m_rcCaption;
}

void WindowBase::SetCaptionRect(const UiRect& rcCaption, bool bNeedDpiScale)
{
    m_rcCaption = rcCaption;
    if (bNeedDpiScale) {
        Dpi().ScaleRect(m_rcCaption);
    }
}

const UiRect& WindowBase::GetSysMenuRect() const
{
    return m_rcSysMenuRect;
}

void WindowBase::SetSysMenuRect(const UiRect& rcSysMenuRect, bool bNeedDpiScale)
{
    m_rcSysMenuRect = rcSysMenuRect;
    if (bNeedDpiScale) {
        Dpi().ScaleRect(m_rcSysMenuRect);
    }
}

void WindowBase::SetEnableSnapLayoutMenu(bool bEnable)
{
    m_pNativeWindow->SetEnableSnapLayoutMenu(bEnable);
}

bool WindowBase::IsEnableSnapLayoutMenu() const
{
    return m_pNativeWindow->IsEnableSnapLayoutMenu();
}

void WindowBase::SetEnableSysMenu(bool bEnable)
{
    m_pNativeWindow->SetEnableSysMenu(bEnable);
}

bool WindowBase::IsEnableSysMenu() const
{
    return m_pNativeWindow->IsEnableSysMenu();
}

const UiSize& WindowBase::GetRoundCorner() const
{
    return m_szRoundCorner;
}

void WindowBase::SetRoundCorner(int cx, int cy, bool bNeedDpiScale)
{
    ASSERT(cx >= 0);
    ASSERT(cy >= 0);
    if ((cx < 0) || (cy < 0)) {
        return;
    }
    //两个参数要么同时等于0，要么同时大于0，否则参数无效
    ASSERT(((cx > 0) && (cy > 0)) || ((cx == 0) && (cy == 0)));
    if (cx == 0) {
        if (cy != 0) {
            return;
        }
    }
    else {
        if (cy == 0) {
            return;
        }
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(cx);
        Dpi().ScaleInt(cy);
    }
    m_szRoundCorner.cx = cx;
    m_szRoundCorner.cy = cy;
}

void WindowBase::SetWindowMaximumSize(const UiSize& szMinWindow, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        NativeWnd()->SetWindowMaximumSize(Dpi().GetScaleSize(szMinWindow));
    }
    else {
        NativeWnd()->SetWindowMaximumSize(szMinWindow);
    }
}

const UiSize& WindowBase::GetWindowMaximumSize() const
{
    return NativeWnd()->GetWindowMaximumSize();
}

void WindowBase::SetWindowMinimumSize(const UiSize& szMaxWindow, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        NativeWnd()->SetWindowMinimumSize(Dpi().GetScaleSize(szMaxWindow));
    }
    else {
        NativeWnd()->SetWindowMinimumSize(szMaxWindow);
    }
}

const UiSize& WindowBase::GetWindowMinimumSize() const
{
    return NativeWnd()->GetWindowMaximumSize();
}

int32_t WindowBase::SetWindowHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers)
{
    return m_pNativeWindow->SetWindowHotKey(wVirtualKeyCode, wModifiers);
}

bool WindowBase::GetWindowHotKey(uint8_t& wVirtualKeyCode, uint8_t& wModifiers) const
{
    return m_pNativeWindow->GetWindowHotKey(wVirtualKeyCode, wModifiers);
}

bool WindowBase::RegisterHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers, int32_t id)
{
    return m_pNativeWindow->RegisterHotKey(wVirtualKeyCode, wModifiers, id);
}

bool WindowBase::UnregisterHotKey(int32_t id)
{
    return m_pNativeWindow->UnregisterHotKey(id);
}

bool WindowBase::RegisterDragDrop(ControlDropTarget* pDropTarget)
{
    if (m_pWindowDropTarget == nullptr) {
        m_pWindowDropTarget = new WindowDropTarget;
        m_pWindowDropTarget->SetWindow(this);
    }
    return m_pWindowDropTarget->RegisterDragDrop(pDropTarget);
}

bool WindowBase::UnregisterDragDrop(ControlDropTarget* pDropTarget)
{
    if (m_pWindowDropTarget == nullptr) {
        return false;
    }
    return m_pWindowDropTarget->UnregisterDragDrop(pDropTarget);
}

const UiPoint& WindowBase::GetLastMousePos() const
{
    return m_pNativeWindow->GetLastMousePos();
}

void WindowBase::SetLastMousePos(const UiPoint& pt)
{
    m_pNativeWindow->SetLastMousePos(pt);
}

void* WindowBase::GetWindowHandle() const
{
    return m_pNativeWindow->GetWindowHandle();
}

#ifdef DUILIB_BUILD_FOR_SDL
DString WindowBase::GetVideoDriverName() const
{
    return m_pNativeWindow->GetVideoDriverName();
}

DString WindowBase::GetWindowRenderName() const
{
    return m_pNativeWindow->GetWindowRenderName();
}
#endif

void WindowBase::OnWindowSize(WindowSizeType sizeType)
{
    UiSize szRoundCorner = GetRoundCorner();
    bool isIconic = IsWindowMinimized();
    if (!isIconic && (sizeType != WindowSizeType::kSIZE_MAXIMIZED) && (szRoundCorner.cx > 0 && szRoundCorner.cy > 0)) {
        //最大化、最小化时，均不设置圆角RGN，只有普通状态下设置
        UiRect rcWnd;
        GetWindowRect(rcWnd);
        rcWnd.Offset(-rcWnd.left, -rcWnd.top);
        rcWnd.right++;
        rcWnd.bottom++;
        SetWindowRoundRectRgn(rcWnd, szRoundCorner, true);
    }
    else if (!isIconic) {
        //不需要设置RGN的时候，清除原RGN设置，避免最大化以后显示不正确
        ClearWindowRgn(true);
    }
}

void WindowBase::OnNativeWindowEnterFullScreen()
{
    OnWindowEnterFullScreen();
}

void WindowBase::OnNativeWindowExitFullScreen()
{
    OnWindowExitFullScreen();
}

UiRect WindowBase::OnNativeGetSizeBox() const
{
    return GetSizeBox();
}

void WindowBase::OnNativeGetShadowCorner(UiPadding& rcShadow) const
{
    GetShadowCorner(rcShadow);
}

const DpiManager& WindowBase::OnNativeGetDpi() const
{
    return Dpi();
}

void WindowBase::OnNativeGetCaptionRect(UiRect& captionRect) const
{
    captionRect = GetCaptionRect();
}

void WindowBase::OnNativeGetSysMenuRect(UiRect& sysMenuRect) const
{
    sysMenuRect = GetSysMenuRect();
}

bool WindowBase::OnNativeIsPtInCaptionBarControl(const UiPoint& pt) const
{
    return IsPtInCaptionBarControl(pt);
}

bool WindowBase::OnNativeHasMinMaxBox(bool& bMinimizeBox, bool& bMaximizeBox) const
{
    return HasMinMaxBox(bMinimizeBox, bMaximizeBox);
}

bool WindowBase::OnNativeIsPtInMaximizeRestoreButton(const UiPoint& pt) const
{
    return IsPtInMaximizeRestoreButton(pt);
}

void WindowBase::OnNativePreCloseWindow()
{
    NativeWindow* pNativeWindow = GetParentWindow() != nullptr ? GetParentWindow()->NativeWnd() : nullptr;
    m_pNativeWindow->OnCloseModalFake(pNativeWindow);
    PreCloseWindow();
}

void WindowBase::OnNativePostCloseWindow()
{
    PostCloseWindow();
}

void WindowBase::OnNativeUseSystemCaptionBarChanged()
{
    OnUseSystemCaptionBarChanged();
}

bool WindowBase::OnNativePreparePaint()
{
    return OnPreparePaint();
}

IRender* WindowBase::OnNativeGetRender() const
{
    return GetRender();
}

void WindowBase::OnNativeProcessDpiChangedMsg(uint32_t nNewDPI, const UiRect& rcNewWindow)
{
    ProcessDpiChangedMsg(nNewDPI, rcNewWindow);
}

void WindowBase::OnNativeFinalMessage()
{
    FinalMessage();
}

LRESULT WindowBase::OnNativeWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    LRESULT lResult = 0;
    //第一优先级：将消息发给过滤器进行过滤（可以通过设置bHandled为true来截获消息处理）
    for (auto filter : m_aMessageFilters) {
        if (filter == nullptr) {
            continue;
        }
        lResult = filter->FilterMessage(uMsg, wParam, lParam, bHandled);
        if (bHandled) {
            //过滤器处理后截获此消息，不再进行派发
            return lResult;
        }
    }

    //第二优先级：派发给子类回调函数（子类可以通过设置bHandled为true来截获消息处理）   
    if (!bHandled) {
        lResult = OnWindowMessage(uMsg, wParam, lParam, bHandled);
    }
    return lResult;
}

LRESULT WindowBase::OnNativeSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, const NativeMsg& nativeMsg, bool& bHandled)
{
    OnWindowSize(sizeType);
    return OnSizeMsg(sizeType, newWindowSize, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeMoveMsg(const UiPoint& ptTopLeft, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnMoveMsg(ptTopLeft, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeShowWindowMsg(bool bShow, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnShowWindowMsg(bShow, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativePaintMsg(const UiRect& rcPaint, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnPaintMsg(rcPaint, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeSetFocusMsg(INativeWindow* pLostFocusWindow, const NativeMsg& nativeMsg, bool& bHandled)
{
    WindowBase* pLostFocusWindowBase = nullptr;
    if (pLostFocusWindow != nullptr) {
        pLostFocusWindowBase = dynamic_cast<WindowBase*>(pLostFocusWindow);
    }
    return OnSetFocusMsg(pLostFocusWindowBase, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeKillFocusMsg(INativeWindow* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled)
{
    WindowBase* pSetFocusWindowBase = nullptr;
    if (pSetFocusWindow != nullptr) {
        pSetFocusWindowBase = dynamic_cast<WindowBase*>(pSetFocusWindow);
    }
    return OnKillFocusMsg(pSetFocusWindowBase, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeImeStartCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnImeStartCompositionMsg(nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeImeEndCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnImeEndCompositionMsg(nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeSetCursorMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnSetCursorMsg(nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeContextMenuMsg(const UiPoint& pt, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnContextMenuMsg(pt, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeKeyUpMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnKeyUpMsg(vkCode, modifierKey, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeCharMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnCharMsg(vkCode, modifierKey, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeHotKeyMsg(int32_t hotkeyId, VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnHotKeyMsg(hotkeyId, vkCode, modifierKey, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeMouseWheelMsg(int32_t wheelDelta, const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnMouseWheelMsg(wheelDelta, pt, modifierKey, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeMouseMoveMsg(const UiPoint& pt, uint32_t modifierKey, bool bFromNC, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnMouseMoveMsg(pt, modifierKey, bFromNC, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeMouseHoverMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnMouseHoverMsg(pt, modifierKey, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeMouseLeaveMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnMouseLeaveMsg(nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeMouseLButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnMouseLButtonDownMsg(pt, modifierKey, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeMouseLButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnMouseLButtonUpMsg(pt, modifierKey, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeMouseLButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnMouseLButtonDbClickMsg(pt, modifierKey, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeMouseRButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnMouseRButtonDownMsg(pt, modifierKey, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeMouseRButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnMouseRButtonUpMsg(pt, modifierKey, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeMouseRButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnMouseRButtonDbClickMsg(pt, modifierKey, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeCaptureChangedMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnCaptureChangedMsg(nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeWindowCloseMsg(uint32_t wParam, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnWindowCloseMsg(wParam, nativeMsg, bHandled);
}

} // namespace ui

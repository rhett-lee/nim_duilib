#include "WindowBase.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/WindowDropTarget.h"

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

bool WindowBase::CreateWnd(WindowBase* pParentWindow,
                           const WindowCreateParam& createParam,
                           const UiRect& rc)
{
    m_pParentWindow = pParentWindow;
    m_parentFlag.reset();
    if (pParentWindow != nullptr) {
        m_parentFlag = pParentWindow->GetWeakFlag();
    }
    bool bRet = m_pNativeWindow->CreateWnd(pParentWindow, createParam, rc);
    if (bRet) {
        InitWindowBase();
        InitWindow();
        OnInitWindow();
    }    
    return bRet;
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

void WindowBase::SetWindowAlpha(int nAlpha)
{
    m_pNativeWindow->SetWindowAlpha(nAlpha);
    OnWindowAlphaChanged();
}

uint8_t WindowBase::GetWindowAlpha() const
{
    return m_pNativeWindow->GetWindowAlpha();
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

void WindowBase::CloseWnd(UINT nRet)
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

LRESULT WindowBase::SendMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return m_pNativeWindow->SendMsg(uMsg, wParam, lParam);
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

void WindowBase::GetCursorPos(UiPoint& pt) const
{
    m_pNativeWindow->GetCursorPos(pt);
}

void WindowBase::MapWindowDesktopRect(UiRect& rc) const
{
    m_pNativeWindow->MapWindowDesktopRect(rc);
}

bool WindowBase::GetMonitorRect(UiRect& rcMonitor, UiRect& rcWork) const
{
    return m_pNativeWindow->GetMonitorRect(rcMonitor, rcWork);
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
    m_pNativeWindow->ShowModalFake(GetParentWindow());
}

bool WindowBase::IsFakeModal() const
{
    return m_pNativeWindow->IsFakeModal();
}

void WindowBase::CenterWindow()
{
    m_pNativeWindow->CenterWindow();
}

void WindowBase::ToTopMost()
{
    m_pNativeWindow->ToTopMost();
}

void WindowBase::BringToTop()
{
    m_pNativeWindow->BringToTop();
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

bool WindowBase::SetOwnerWindowFocus()
{
    return m_pNativeWindow->SetOwnerWindowFocus();
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
    return m_pNativeWindow->SetWindowPos(insertAfter, X, Y, cx, cy, uFlags);
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

void WindowBase::SetIcon(UINT nRes)
{
    m_pNativeWindow->SetIcon(nRes);
}

void WindowBase::SetText(const DString& strText)
{
    m_pNativeWindow->SetText(strText);
    m_text = strText;
}

DString WindowBase::GetText() const
{
    return m_text;
}

void WindowBase::SetTextId(const DString& strTextId)
{
    ASSERT(IsWindow());
    m_pNativeWindow->SetText(GlobalManager::Instance().Lang().GetStringViaID(strTextId).c_str());
    m_textId = strTextId;
}

DString WindowBase::GetTextId() const
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
    m_szMinWindow = Dpi().GetScaleSize(m_szMinWindow, nOldDpiScale);
    m_szMaxWindow = Dpi().GetScaleSize(m_szMaxWindow, nOldDpiScale);
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

bool WindowBase::BeginPaint(UiRect& rcPaint)
{
    return m_pNativeWindow->BeginPaint(rcPaint);
}

bool WindowBase::EndPaint(const UiRect& rcPaint, IRender* pRender)
{
    return m_pNativeWindow->EndPaint(rcPaint, pRender);
}

bool WindowBase::GetUpdateRect(UiRect& rcPaint)
{
    return m_pNativeWindow->GetUpdateRect(rcPaint);
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

UiSize WindowBase::GetMinInfo(bool bContainShadow) const
{
    UiSize xy = m_szMinWindow;
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        if (xy.cx != 0) {
            xy.cx -= rcShadow.left + rcShadow.right;
        }
        if (xy.cy != 0) {
            xy.cy -= rcShadow.top + rcShadow.bottom;
        }
    }
    return xy;
}

void WindowBase::SetMinInfo(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
    ASSERT(cx >= 0 && cy >= 0);
    if (cx < 0) {
        cx = 0;
    }
    if (cy < 0) {
        cy = 0;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(cx);
        Dpi().ScaleInt(cy);
    }
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        if (cx != 0) {
            cx += rcShadow.left + rcShadow.right;
        }
        if (cy != 0) {
            cy += rcShadow.top + rcShadow.bottom;
        }
    }
    m_szMinWindow.cx = cx;
    m_szMinWindow.cy = cy;
}

UiSize WindowBase::GetMaxInfo(bool bContainShadow) const
{
    UiSize xy = m_szMaxWindow;
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        if (xy.cx != 0) {
            xy.cx -= rcShadow.left + rcShadow.right;
        }
        if (xy.cy != 0) {
            xy.cy -= rcShadow.top + rcShadow.bottom;
        }
    }

    return xy;
}

void WindowBase::SetMaxInfo(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
    ASSERT(cx >= 0 && cy >= 0);
    if (cx < 0) {
        cx = 0;
    }
    if (cy < 0) {
        cy = 0;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(cx);
        Dpi().ScaleInt(cy);
    }
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        if (cx != 0) {
            cx += rcShadow.left + rcShadow.right;
        }
        if (cy != 0) {
            cy += rcShadow.top + rcShadow.bottom;
        }
    }
    m_szMaxWindow.cx = cx;
    m_szMaxWindow.cy = cy;
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

const UiRect& WindowBase::OnNativeGetCaptionRect() const
{
    return GetCaptionRect();
}

bool WindowBase::OnNativeIsPtInCaptionBarControl(const UiPoint& pt) const
{
    return IsPtInCaptionBarControl(pt);
}

UiSize WindowBase::OnNativeGetMinInfo(bool bContainShadow) const
{
    return GetMinInfo(bContainShadow);
}

UiSize WindowBase::OnNativeGetMaxInfo(bool bContainShadow) const
{
    return GetMaxInfo(bContainShadow);
}

void WindowBase::OnNativePreCloseWindow()
{
    m_pNativeWindow->OnCloseModalFake(GetParentWindow());
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

LRESULT WindowBase::OnNativeUserMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    return HandleUserMessage(uMsg, wParam, lParam, bHandled);
}

LRESULT WindowBase::OnNativeSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, bool& bHandled)
{
    OnWindowSize(sizeType);
    return OnSizeMsg(sizeType, newWindowSize, bHandled);
}

LRESULT WindowBase::OnNativeMoveMsg(const UiPoint& ptTopLeft, bool& bHandled)
{
    return OnMoveMsg(ptTopLeft, bHandled);
}

LRESULT WindowBase::OnNativePaintMsg(bool& bHandled)
{
    return OnPaintMsg(bHandled);
}

LRESULT WindowBase::OnNativeSetFocusMsg(INativeWindow* pLostFocusWindow, bool& bHandled)
{
    WindowBase* pLostFocusWindowBase = nullptr;
    if (pLostFocusWindow != nullptr) {
        pLostFocusWindowBase = dynamic_cast<WindowBase*>(pLostFocusWindow);
    }
    return OnSetFocusMsg(pLostFocusWindowBase, bHandled);
}

LRESULT WindowBase::OnNativeKillFocusMsg(INativeWindow* pSetFocusWindow, bool& bHandled)
{
    WindowBase* pSetFocusWindowBase = nullptr;
    if (pSetFocusWindow != nullptr) {
        pSetFocusWindowBase = dynamic_cast<WindowBase*>(pSetFocusWindow);
    }
    return OnKillFocusMsg(pSetFocusWindowBase, bHandled);
}

LRESULT WindowBase::OnNativeImeStartCompositionMsg(bool& bHandled)
{
    return OnImeStartCompositionMsg(bHandled);
}

LRESULT WindowBase::OnNativeImeEndCompositionMsg(bool& bHandled)
{
    return OnImeEndCompositionMsg(bHandled);
}

LRESULT WindowBase::OnNativeSetCursorMsg(bool& bHandled)
{
    return OnSetCursorMsg(bHandled);
}

LRESULT WindowBase::OnNativeContextMenuMsg(const UiPoint& pt, bool& bHandled)
{
    return OnContextMenuMsg(pt, bHandled);
}

LRESULT WindowBase::OnNativeKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled)
{
    return OnKeyDownMsg(vkCode, modifierKey, bHandled);
}

LRESULT WindowBase::OnNativeKeyUpMsg(VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled)
{
    return OnKeyUpMsg(vkCode, modifierKey, bHandled);
}

LRESULT WindowBase::OnNativeCharMsg(VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled)
{
    return OnCharMsg(vkCode, modifierKey, bHandled);
}

LRESULT WindowBase::OnNativeHotKeyMsg(int32_t hotkeyId, VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled)
{
    return OnHotKeyMsg(hotkeyId, vkCode, modifierKey, bHandled);
}

LRESULT WindowBase::OnNativeMouseWheelMsg(int32_t wheelDelta, const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    return OnMouseWheelMsg(wheelDelta, pt, modifierKey, bHandled);
}

LRESULT WindowBase::OnNativeMouseMoveMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    return OnMouseMoveMsg(pt, modifierKey, bHandled);
}

LRESULT WindowBase::OnNativeMouseHoverMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    return OnMouseHoverMsg(pt, modifierKey, bHandled);
}

LRESULT WindowBase::OnNativeMouseLeaveMsg(bool& bHandled)
{
    return OnMouseLeaveMsg(bHandled);
}

LRESULT WindowBase::OnNativeMouseLButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    return OnMouseLButtonDownMsg(pt, modifierKey, bHandled);
}

LRESULT WindowBase::OnNativeMouseLButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    return OnMouseLButtonUpMsg(pt, modifierKey, bHandled);
}

LRESULT WindowBase::OnNativeMouseLButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    return OnMouseLButtonDbClickMsg(pt, modifierKey, bHandled);
}

LRESULT WindowBase::OnNativeMouseRButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    return OnMouseRButtonDownMsg(pt, modifierKey, bHandled);
}

LRESULT WindowBase::OnNativeMouseRButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    return OnMouseRButtonUpMsg(pt, modifierKey, bHandled);
}

LRESULT WindowBase::OnNativeMouseRButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    return OnMouseRButtonDbClickMsg(pt, modifierKey, bHandled);
}

LRESULT WindowBase::OnNativeCaptureChangedMsg(bool& bHandled)
{
    return OnCaptureChangedMsg(bHandled);
}

LRESULT WindowBase::OnNativeWindowCloseMsg(uint32_t wParam, bool& bHandled)
{
    return OnWindowCloseMsg(wParam, bHandled);
}

} // namespace ui

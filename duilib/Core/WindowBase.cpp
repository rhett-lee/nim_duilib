#include "WindowBase.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/WindowCreateAttributes.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/MonitorUtil.h"
#include <random>

namespace ui
{
WindowBase::WindowBase():
    m_pParentWindow(nullptr), 
    m_pNativeWindow(nullptr),
    m_bWindowFirstShown(false),
    m_windowSizeState(WindowSizeState::kUnknown)
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
    SetWindowId(createParam.m_windowId);
    m_windowClassName = createParam.m_className;
    m_pParentWindow = pParentWindow;

    NativeWindow* pNativeWindow = pParentWindow != nullptr ? pParentWindow->NativeWnd() : nullptr;
    return m_pNativeWindow->CreateWnd(pNativeWindow, createParam, createAttributes);
}

int32_t WindowBase::DoModal(WindowBase* pParentWindow, const WindowCreateParam& createParam,
                            bool bCloseByEsc, bool bCloseByEnter)
{
    //解析XML，读取窗口的属性参数
    WindowCreateAttributes createAttributes;
    GetCreateWindowAttributes(createAttributes);
    SetWindowId(createParam.m_windowId);
    m_windowClassName = createParam.m_className;
    m_pParentWindow = pParentWindow;

    NativeWindow* pNativeWindow = pParentWindow != nullptr ? pParentWindow->NativeWnd() : nullptr;
    return m_pNativeWindow->DoModal(pNativeWindow, createParam, createAttributes, bCloseByEsc, bCloseByEnter);
}

bool WindowBase::CreateChildWnd(WindowBase* pParentWindow, int32_t nX, int32_t nY, int32_t nWidth, int32_t nHeight)
{
    SetWindowId(_T(""));
    m_pParentWindow = pParentWindow;
    NativeWindow* pNativeWindow = pParentWindow != nullptr ? pParentWindow->NativeWnd() : nullptr;
    return m_pNativeWindow->CreateChildWnd(pNativeWindow, nX, nY, nWidth, nHeight);
}

void WindowBase::OnNativeCreateWndMsg(bool bDoModal, const NativeMsg& nativeMsg, bool& bHandled)
{
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();

    //窗口完成创建，初始化（内部使用）
    InitWindowBase();

    //回调，让子类解析XML文件并与窗口绑定（内部使用，子类可重写）
    PreInitWindow();

    //内部初始化（内部使用）
    PostInitWindow();

    //调用子类的初始化函数
    OnInitWindow();

    if (!windowFlag.expired()) {
        OnWindowCreateMsg(bDoModal, nativeMsg, bHandled);
    }    
    if (!windowFlag.expired()) {
        //给应用层回调
        SendWindowEvent(kWindowCreateMsg, (WPARAM)bDoModal ? 1 : 0);
    }
}

void WindowBase::ClearWindowBase()
{
    m_pParentWindow = nullptr;
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
    return NativeWindow::PostQuitMsg(nExitCode);
}

WindowBase* WindowBase::WindowBaseFromPoint(const UiPoint& pt, bool bIgnoreChildWindow)
{
    WindowBase* pWindowBase = nullptr;
    INativeWindow* pNativeWindow = m_pNativeWindow->WindowBaseFromPoint(pt, bIgnoreChildWindow);
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
    return m_pParentWindow.get();
}

bool WindowBase::SetParentWindow(WindowBase* pParentWindow)
{
    ASSERT((pParentWindow != nullptr) && pParentWindow->IsWindow());
    if ((pParentWindow == nullptr) || !pParentWindow->IsWindow()) {
        return false;
    }
    m_pParentWindow = pParentWindow;
    return m_pNativeWindow->SetParentWindow(pParentWindow->NativeWnd());
}

bool WindowBase::IsWindow() const
{
    return m_pNativeWindow->IsWindow();
}

bool WindowBase::IsChildWindow() const
{
    return m_pNativeWindow->IsChildWindow();
}

void WindowBase::InitWindowBase()
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return;
    }
    //初始化窗口自身的DPI管理器
    m_dpi = std::make_unique<DpiManager>();
    m_dpi->SetDisplayScaleForWindow(this);
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

    int32_t cx = rc.Width();
    Dpi().ClientSizeToWindowSize(cx);
    int32_t cy = rc.Height();
    Dpi().ClientSizeToWindowSize(cy);

    rc.right = pt.x + cx;
    rc.left = pt.x;
    rc.bottom = pt.y + cy;
    rc.top = pt.y;
}

void WindowBase::ScreenToClient(UiRect& rc) const
{
    UiPoint pt;
    pt.x = rc.left;
    pt.y = rc.top;
    ScreenToClient(pt);

    int32_t cx = rc.Width();
    Dpi().WindowSizeToClientSize(cx);
    int32_t cy = rc.Height();
    Dpi().WindowSizeToClientSize(cy);

    rc.right = pt.x + cx;
    rc.left = pt.x;
    rc.bottom = pt.y + cy;
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

bool WindowBase::EnterFullscreen()
{
    return m_pNativeWindow->EnterFullscreen();
}

bool WindowBase::ExitFullscreen()
{
    return m_pNativeWindow->ExitFullscreen();
}

bool WindowBase::IsWindowMaximized() const
{
    return m_pNativeWindow->IsWindowMaximized();
}

bool WindowBase::IsWindowMinimized() const
{
    return m_pNativeWindow->IsWindowMinimized();
}

bool WindowBase::IsWindowFullscreen() const
{
    return m_pNativeWindow->IsWindowFullscreen();
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
        GetCurrentShadowCorner(rcShadow);
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
        Dpi().ScaleWindowSize(cy);
        Dpi().ScaleWindowSize(cx);
    }

    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        Dpi().ScaleWindowSize(rcShadow.left);
        Dpi().ScaleWindowSize(rcShadow.top);
        Dpi().ScaleWindowSize(rcShadow.right);
        Dpi().ScaleWindowSize(rcShadow.bottom);
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
}

DString WindowBase::GetText() const
{
    return m_pNativeWindow->GetText();
}

void WindowBase::SetTextId(const DString& strTextId)
{
    ASSERT(IsWindow());
    m_textId = strTextId;
    m_pNativeWindow->SetText(GlobalManager::Instance().Lang().GetStringViaID(strTextId));  
}

const DString& WindowBase::GetTextId() const
{
    return m_textId;
}

const DString& WindowBase::GetWindowId() const
{
    return m_windowId;
}

//生成12位的随机字符串
static std::string generate_12digit_random()
{
    std::random_device rd;
    std::mt19937_64 generator(rd());
    std::uniform_int_distribution<int64_t> distribution(
        100000000000,  // 最小12位数
        999999999999   // 最大12位数
    );

    return std::to_string(distribution(generator));
}

void WindowBase::SetWindowId(const DString& windowId)
{
    m_windowId = windowId;
    if (m_windowId.empty()) {
        //随机生成一个Id
        m_windowId = StringConvert::UTF8ToT(generate_12digit_random());
    }
}

const DString& WindowBase::GetWindowClassName() const
{
    return m_windowClassName;
}

const DpiManager& WindowBase::Dpi() const
{
    return (m_dpi != nullptr) ? *m_dpi : GlobalManager::Instance().Dpi();
}

bool WindowBase::ChangeDisplayScale(uint32_t nNewDisplayScaleFactor, bool bDisableDpiAware)
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    //DPI缩放比值限制在60到300之间(小于50的时候，会出问题，比如原来是1的，经过DPI转换后，会变成0，导致很多逻辑失效)
    const uint32_t nDisplayScaleFactorMin = (uint32_t)(DUILIB_DISPLAY_SCALE_MIN * 100 + 0.5f);
    const uint32_t nDisplayScaleFactorMax = (uint32_t)(DUILIB_DISPLAY_SCALE_MAX * 100 + 0.5f);
    ASSERT((nNewDisplayScaleFactor >= nDisplayScaleFactorMin) && (nNewDisplayScaleFactor <= nDisplayScaleFactorMax)) ;
    if ((nNewDisplayScaleFactor < nDisplayScaleFactorMin) || (nNewDisplayScaleFactor > nDisplayScaleFactorMax)) {
        return false;
    }

    uint32_t nOldScaleFactor = Dpi().GetDisplayScaleFactor();
    if (m_dpi == nullptr) {
        m_dpi = std::make_unique<DpiManager>();
    }
    //更新窗口的DPI值为新值
    m_dpi->SetDisplayScale(nNewDisplayScaleFactor / 100.0f, m_dpi->GetPixelDensity());

    //标记为用户自定义DPI，不再跟随屏幕DPI变化
    if (bDisableDpiAware) {
        m_dpi->SetUserDefinedDpi(true);
    }

    uint32_t nNewScaleFactor = m_dpi->GetDisplayScaleFactor();

    //按新的DPI更新窗口布局
    OnDisplayScaleChanged(nOldScaleFactor, nNewScaleFactor);
    OnWindowDisplayScaleChanged(nOldScaleFactor, nNewScaleFactor);

    //更新窗口大小：需要按照显示比例大小同比例缩放，但不需要按窗口像素密度处理
    if (nNewScaleFactor != nOldScaleFactor) {
        UiRect rcOldWindow;
        GetWindowRect(rcOldWindow);
        int32_t cx = Dpi().GetScaleInt(rcOldWindow.Width(), nOldScaleFactor);
        int32_t cy = Dpi().GetScaleInt(rcOldWindow.Height(), nOldScaleFactor);
        m_pNativeWindow->MoveWindow(rcOldWindow.left, rcOldWindow.top, cx, cy, true);
    }

    //重绘窗口内容
    UiRect rcClient;
    GetClientRect(rcClient);
    Invalidate(rcClient);
    return true;
}

void WindowBase::OnDisplayScaleChangedMsg(float fNewDisplayScale, float fNewPixelDensity)
{
    if ((fNewDisplayScale < 0.9999f) || (fNewPixelDensity < 0.9999f)) {
        //无效值
        return;
    }
    //更新全局DPI管理器的DPI值
    float fMonitorDisplayScale = MonitorUtil::GetPrimaryMonitorDisplayScale();
    DpiManager& dpi = GlobalManager::Instance().Dpi();
    if (!dpi.IsUserDefinedDpi() && dpi.IsDpiAware() && !IsFloatEqual(fMonitorDisplayScale, dpi.GetDisplayScale())) {
        dpi.SetDisplayScale(fMonitorDisplayScale, dpi.GetPixelDensity());
    }

    //此消息必须处理，否则窗口大小与界面的比例将失调
    uint32_t nOldScaleFactor = 0;
    uint32_t nNewScaleFactor = 0;
    if (m_dpi != nullptr) {
        nOldScaleFactor = m_dpi->GetDisplayScaleFactor();
        nNewScaleFactor = nOldScaleFactor;
    }
    if ((m_dpi != nullptr) && !m_dpi->IsUserDefinedDpi() && m_dpi->IsDpiAware()) {
        if (!m_dpi->IsDisplayScaleChanged(fNewDisplayScale, fNewPixelDensity)) {
            //无变化，不处理
            return;
        }
        //更新窗口的DPI缩放比为新值
        nOldScaleFactor = m_dpi->GetDisplayScaleFactor();
        m_dpi->SetDisplayScale(fNewDisplayScale, fNewPixelDensity);
        nNewScaleFactor = m_dpi->GetDisplayScaleFactor();

        //按新的DPI更新窗口布局
        if (nOldScaleFactor != nNewScaleFactor) {
            OnDisplayScaleChanged(nOldScaleFactor, nNewScaleFactor);            
        }
    }

    //该回调，无论是否变化，都需要通知应用层
    OnWindowDisplayScaleChanged(nOldScaleFactor, nNewScaleFactor);
}

void WindowBase::OnDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor)
{
    if ((nOldScaleFactor == nNewScaleFactor) || (nNewScaleFactor == 0)) {
        return;
    }
    if (!Dpi().CheckDisplayScaleFactor(nNewScaleFactor)) {
        return;
    }
    UiSize szMinWindow = NativeWnd()->GetWindowMinimumSize();
    szMinWindow = Dpi().GetScaleSize(szMinWindow, nOldScaleFactor);
    NativeWnd()->SetWindowMinimumSize(szMinWindow);

    UiSize szMaxWindow = NativeWnd()->GetWindowMaximumSize();
    szMaxWindow = Dpi().GetScaleSize(szMaxWindow, nOldScaleFactor);
    NativeWnd()->SetWindowMaximumSize(szMaxWindow);

    m_rcSizeBox = Dpi().GetScaleRect(m_rcSizeBox, nOldScaleFactor);
    m_szRoundCorner = Dpi().GetScaleSize(m_szRoundCorner, nOldScaleFactor);
    m_rcCaption = Dpi().GetScaleRect(m_rcCaption, nOldScaleFactor);
}

bool WindowBase::NeedSetWindowRgnOnWindowResized()
{
    if (IsChildWindow()) {
        //子窗口，不自动设置RGN
        return false;
    }
    return true;
}

bool WindowBase::SetWindowRoundRectRgn(const UiRect& rcWnd, float rx, float ry, bool bRedraw)
{
    return m_pNativeWindow->SetWindowRoundRectRgn(rcWnd, rx, ry, bRedraw);
}

bool WindowBase::SetWindowRectRgn(const UiRect& rcWnd, bool bRedraw)
{
    return m_pNativeWindow->SetWindowRectRgn(rcWnd, bRedraw);
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
        NativeWnd()->SetWindowMaximumSize(Dpi().GetScaleWindowSize(szMinWindow));
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
        NativeWnd()->SetWindowMinimumSize(Dpi().GetScaleWindowSize(szMaxWindow));
    }
    else {
        NativeWnd()->SetWindowMinimumSize(szMaxWindow);
    }
}

const UiSize& WindowBase::GetWindowMinimumSize() const
{
    return NativeWnd()->GetWindowMinimumSize();
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

void WindowBase::OnWindowSize(WindowSizeType /*sizeType*/)
{
    //此函数的主要功能：设置窗口的RGN，从而实现窗口的圆角或者直角功能
    if (!NeedSetWindowRgnOnWindowResized()) {
        //不支持，立即返回
        return;
    }
    if (IsUseSystemCaption() || IsWindowMinimized() || IsWindowMaximized()) {
        //使用系统工具栏，窗口最小化，窗口最大化的情况下，关闭RGN设置
        ClearWindowRgn(true);
    }
    else {
        //其他情况下
        UiSize szRoundCorner = GetRoundCorner();
        if (szRoundCorner.cx > 0 && szRoundCorner.cy > 0) {
            //该窗口的配置为圆角窗口
            UiRect rcWnd;
            GetWindowRect(rcWnd);
            rcWnd.Offset(-rcWnd.left, -rcWnd.top);
            SetWindowRoundRectRgn(rcWnd, (float)szRoundCorner.cx, (float)szRoundCorner.cy, true);
        }
        else {
            //配置为直角窗口
            //不需要设置RGN的时候，使用与窗口大小相同的矩形RGN，而不是使用默认值（因为默认情况下，窗口的左上角和右上角是圆角，左下角和右下角是直角）
            UiRect rcWnd;
            GetWindowRect(rcWnd);
            rcWnd.Offset(-rcWnd.left, -rcWnd.top);
            rcWnd.right++;
            rcWnd.bottom++;
            SetWindowRectRgn(rcWnd, true);
        }
    }
}

void WindowBase::OnNativeWindowEnterFullscreen()
{
    NotifyWindowEnterFullscreen(); //供Window子类处理业务
    OnWindowEnterFullscreen();     //供应用层处理业务

    m_windowSizeState = WindowSizeState::kFullscreen;
    SendWindowEvent(kWindowEnterFullscreenMsg);
}

void WindowBase::OnNativeWindowExitFullscreen()
{
    NotifyWindowExitFullscreen();   //供Window子类处理业务
    OnWindowExitFullscreen();       //供应用层处理业务
    SendWindowEvent(kWindowExitFullscreenMsg);
}

UiRect WindowBase::OnNativeGetSizeBox() const
{
    return GetSizeBox();
}

void WindowBase::OnNativeGetShadowCorner(UiPadding& rcShadow) const
{
    GetCurrentShadowCorner(rcShadow);
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

Control* WindowBase::OnNativeFindControl(const UiPoint& pt) const
{
    return OnFindControl(pt);
}

void WindowBase::OnNativeDisplayScaleChangedMsg(float fNewDisplayScale, float fNewPixelDensity)
{
    OnDisplayScaleChangedMsg(fNewDisplayScale, fNewPixelDensity);
}

void WindowBase::OnNativeFinalMessage()
{
    if (IsChildWindow() || GlobalManager::Instance().Windows().HasWindowBase(this)) {
        //发送一个窗口关闭事件
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        WPARAM wParam = (WPARAM)GetCloseParam();
        SendWindowEvent(kWindowCloseMsg, wParam);
        if (windowFlag.expired()) {
            return;
        }
    }
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

LRESULT WindowBase::OnNativeWindowPosChangedMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnWindowPosChangedMsg(nativeMsg, bHandled);
    SendWindowEvent(kWindowPosChangedMsg);
    return lResult;
}

LRESULT WindowBase::OnNativeSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, const NativeMsg& nativeMsg, bool& bHandled)
{
    OnWindowSize(sizeType);
    LRESULT lResult = OnSizeMsg(sizeType, newWindowSize, nativeMsg, bHandled);
    SendWindowEvent(kWindowSizeMsg, (WPARAM)sizeType);

    //窗口大小改变时，主动触发重绘（避免分层窗口的情况下，窗口不绘制的现象出现）
    UiRect rcClient;
    GetClientRect(rcClient);
    Invalidate(rcClient);

    //通知最终的窗口状态事件
    if (sizeType == WindowSizeType::kSIZE_MAXIMIZED) {
        //最大化
        if (!IsWindowFullscreen()) {
            //非全屏状态
            m_windowSizeState = WindowSizeState::kMaximized;
            SendWindowEvent(kWindowMaximizedMsg);
        }
    }
    else if (sizeType == WindowSizeType::kSIZE_RESTORED) {
        //还原
        if (m_windowSizeState != WindowSizeState::kRestored) {
            m_windowSizeState = WindowSizeState::kRestored;
            SendWindowEvent(kWindowRestoredMsg);
        }
    }
    else if (sizeType == WindowSizeType::kSIZE_MINIMIZED) {
        //最小化
        m_windowSizeState = WindowSizeState::kMinimized;
        SendWindowEvent(kWindowMaximizedMsg);
    }
    return lResult;
}

LRESULT WindowBase::OnNativeMoveMsg(const UiPoint& ptTopLeft, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnMoveMsg(ptTopLeft, nativeMsg, bHandled);
    SendWindowEvent(kWindowMoveMsg);
    return lResult;
}

LRESULT WindowBase::OnNativeShowWindowMsg(bool bShow, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnShowWindowMsg(bShow, nativeMsg, bHandled);
    SendWindowEvent(kWindowShowWindowMsg, bShow ? 1 : 0);
    return lResult;
}

LRESULT WindowBase::OnNativePaintMsg(const UiRect& rcPaint, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnPaintMsg(rcPaint, nativeMsg, bHandled);
    SendWindowEvent(kWindowPaintMsg);

    //首次绘制事件, 给一次回调
    if (!IsWindowFirstShown()) {
        m_bWindowFirstShown = true;

        //触发第一次绘制事件
        SendWindowEvent(kWindowFirstShown);
    }
    return lResult;
}

LRESULT WindowBase::OnNativeSetFocusMsg(INativeWindow* pLostFocusWindow, const NativeMsg& nativeMsg, bool& bHandled)
{
    WindowBase* pLostFocusWindowBase = nullptr;
    if (pLostFocusWindow != nullptr) {
        pLostFocusWindowBase = dynamic_cast<WindowBase*>(pLostFocusWindow);
    }
    LRESULT lResult = OnSetFocusMsg(pLostFocusWindowBase, nativeMsg, bHandled);
    SendWindowEvent(kWindowSetFocusMsg, (WPARAM)pLostFocusWindowBase);
    return lResult;
}

LRESULT WindowBase::OnNativeKillFocusMsg(INativeWindow* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled)
{
    WindowBase* pSetFocusWindowBase = nullptr;
    if (pSetFocusWindow != nullptr) {
        pSetFocusWindowBase = dynamic_cast<WindowBase*>(pSetFocusWindow);
    }
    LRESULT lResult = OnKillFocusMsg(pSetFocusWindowBase, nativeMsg, bHandled);
    SendWindowEvent(kWindowKillFocusMsg, (WPARAM)pSetFocusWindowBase);
    return lResult;
}

LRESULT WindowBase::OnNativeImeSetContextMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnImeSetContextMsg(nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeImeStartCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnImeStartCompositionMsg(nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeImeCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnImeCompositionMsg(nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeImeEndCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnImeEndCompositionMsg(nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeSetCursorMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnSetCursorMsg(nativeMsg, bHandled);
    SendWindowEvent(kWindowSetCursorMsg);
    return lResult;
}

LRESULT WindowBase::OnNativeContextMenuMsg(const UiPoint& pt, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnContextMenuMsg(pt, nativeMsg, bHandled);
}

LRESULT WindowBase::OnNativeKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
    if (!m_windowEventMap.empty()) {
        EventArgs msg;
        msg.SetSenderWeakFlag(GetWeakFlag());
        msg.eventType = kWindowKeyDownMsg;
        msg.vkCode = vkCode;
        msg.modifierKey = modifierKey;
        SendWindowEvent(msg);
    }
    return lResult;
}

LRESULT WindowBase::OnNativeKeyUpMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnKeyUpMsg(vkCode, modifierKey, nativeMsg, bHandled);
    if (!m_windowEventMap.empty()) {
        EventArgs msg;
        msg.SetSenderWeakFlag(GetWeakFlag());
        msg.eventType = kWindowKeyUpMsg;
        msg.vkCode = vkCode;
        msg.modifierKey = modifierKey;
        SendWindowEvent(msg);
    }
    return lResult;
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
    LRESULT lResult = OnMouseWheelMsg(wheelDelta, pt, modifierKey, nativeMsg, bHandled);
    if (!m_windowEventMap.empty()) {
        EventArgs msg;
        msg.SetSenderWeakFlag(GetWeakFlag());
        msg.eventType = kWindowMouseWheelMsg;
        msg.ptMouse = pt;
        msg.modifierKey = modifierKey;
        msg.eventData = wheelDelta;
        SendWindowEvent(msg);
    }
    return lResult;
}

LRESULT WindowBase::OnNativeMouseMoveMsg(const UiPoint& pt, uint32_t modifierKey, bool bFromNC, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnMouseMoveMsg(pt, modifierKey, bFromNC, nativeMsg, bHandled);
    SendWindowMouseEvent(kWindowMouseMoveMsg, pt, modifierKey);
    return lResult;
}

LRESULT WindowBase::OnNativeMouseHoverMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnMouseHoverMsg(pt, modifierKey, nativeMsg, bHandled);
    SendWindowMouseEvent(kWindowMouseHoverMsg, pt, modifierKey);
    return lResult;
}

LRESULT WindowBase::OnNativeMouseLeaveMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnMouseLeaveMsg(nativeMsg, bHandled);
    SendWindowEvent(kWindowMouseLeaveMsg);
    return lResult;
}

LRESULT WindowBase::OnNativeMouseLButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnMouseLButtonDownMsg(pt, modifierKey, nativeMsg, bHandled);
    SendWindowMouseEvent(kWindowLButtonDownMsg, pt, modifierKey);
    return lResult;
}

LRESULT WindowBase::OnNativeMouseLButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnMouseLButtonUpMsg(pt, modifierKey, nativeMsg, bHandled);
    SendWindowMouseEvent(kWindowLButtonUpMsg, pt, modifierKey);
    return lResult;
}

LRESULT WindowBase::OnNativeMouseLButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnMouseLButtonDbClickMsg(pt, modifierKey, nativeMsg, bHandled);
    SendWindowMouseEvent(kWindowLButtonDbClickMsg, pt, modifierKey);
    return lResult;
}

LRESULT WindowBase::OnNativeMouseRButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnMouseRButtonDownMsg(pt, modifierKey, nativeMsg, bHandled);
    SendWindowMouseEvent(kWindowRButtonDownMsg, pt, modifierKey);
    return lResult;
}

LRESULT WindowBase::OnNativeMouseRButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnMouseRButtonUpMsg(pt, modifierKey, nativeMsg, bHandled);
    SendWindowMouseEvent(kWindowRButtonUpMsg, pt, modifierKey);
    return lResult;
}

LRESULT WindowBase::OnNativeMouseRButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnMouseRButtonDbClickMsg(pt, modifierKey, nativeMsg, bHandled);
    SendWindowMouseEvent(kWindowRButtonDbClickMsg, pt, modifierKey);
    return lResult;
}

LRESULT WindowBase::OnNativeMouseMButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnMouseMButtonDownMsg(pt, modifierKey, nativeMsg, bHandled);
    SendWindowMouseEvent(kWindowMButtonDownMsg, pt, modifierKey);
    return lResult;
}

LRESULT WindowBase::OnNativeMouseMButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnMouseMButtonUpMsg(pt, modifierKey, nativeMsg, bHandled);
    SendWindowMouseEvent(kWindowMButtonUpMsg, pt, modifierKey);
    return lResult;
}

LRESULT WindowBase::OnNativeMouseMButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnMouseMButtonDbClickMsg(pt, modifierKey, nativeMsg, bHandled);
    SendWindowMouseEvent(kWindowMButtonDbClickMsg, pt, modifierKey);
    return lResult;
}

LRESULT WindowBase::OnNativeCaptureChangedMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = OnCaptureChangedMsg(nativeMsg, bHandled);
    SendWindowEvent(kWindowCaptureChangedMsg);
    return lResult;
}

LRESULT WindowBase::OnNativeWindowCloseMsg(uint32_t wParam, const NativeMsg& nativeMsg, bool& bHandled)
{
    return OnWindowCloseMsg(wParam, nativeMsg, bHandled);
}

void WindowBase::OnNativeWindowPosSnapped(bool bLeftSnap, bool bRightSnap, bool bTopSnap, bool bBottomSnap)
{
    OnWindowPosSnapped(bLeftSnap, bRightSnap, bTopSnap, bBottomSnap);
}

bool WindowBase::IsWindowFirstShown() const
{
    return m_bWindowFirstShown;
}

bool WindowBase::SendWindowEvent(EventType eventType, WPARAM wParam, LPARAM lParam)
{
    if (m_windowEventMap.empty()) {
        return true;
    }
    EventArgs msg;
    msg.SetSenderWeakFlag(GetWeakFlag());
    msg.eventType = eventType;
    msg.ptMouse = GetLastMousePos();
    msg.wParam = wParam;
    msg.lParam = lParam;
    return SendWindowEvent(msg);
}

bool WindowBase::SendWindowMouseEvent(EventType eventType, const UiPoint& pt, uint32_t modifierKey)
{
    if (m_windowEventMap.empty()) {
        return true;
    }
    EventArgs msg;
    msg.SetSenderWeakFlag(GetWeakFlag());
    msg.eventType = eventType;
    msg.ptMouse = pt;
    msg.modifierKey = modifierKey;
    return SendWindowEvent(msg);
}

bool WindowBase::SendWindowEvent(const EventArgs& msg)
{
    if (!m_windowEventMap.empty()) {
        auto callback = m_windowEventMap.find(msg.eventType);
        if (callback != m_windowEventMap.end()) {
            callback->second(msg);
        }
    }
    return true;
}

bool WindowBase::HasWindowEventCallback(EventType eventType) const
{
    return m_windowEventMap.find(eventType) != m_windowEventMap.end();
}

bool WindowBase::HasWindowEventCallbackByID(EventCallbackID callbackID) const
{
    for (auto iter = m_windowEventMap.begin(); iter != m_windowEventMap.end(); ++iter) {
        if (iter->second.HasEventCallbackByID(callbackID)) {
            return true;
        }
    }
    return false;
}

void WindowBase::DetachWindowEventCallback(EventType eventType)
{
    auto iter = m_windowEventMap.find(eventType);
    if (iter != m_windowEventMap.end()) {
        m_windowEventMap.erase(iter);
    }
}

void WindowBase::DetachWindowEventCallbackByID(EventCallbackID callbackID)
{
    EventUtils::RemoveEventCallbackByID(m_windowEventMap, callbackID);
}

void WindowBase::AttachWindowCreateMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowCreateMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowCloseMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowCloseMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowFirstShown(const EventCallback& callback, EventCallbackID callbackID)
{
    ASSERT(!IsWindowFirstShown());
    m_windowEventMap[kWindowFirstShown].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowEnterFullscreenMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowEnterFullscreenMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowExitFullscreenMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowExitFullscreenMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowMaximizedMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowMaximizedMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowMinimizedMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowMinimizedMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowRestoredMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowRestoredMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowPosChangedMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowPosChangedMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowSizeMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowSizeMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowMoveMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowMoveMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowShowWindowMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowShowWindowMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowPaintMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowPaintMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowSetFocusMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowSetFocusMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowKillFocusMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowKillFocusMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowSetCursorMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowSetCursorMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowKeyDownMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowKeyDownMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowKeyUpMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowKeyUpMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowMouseWheelMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowMouseWheelMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowMouseMoveMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowMouseMoveMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowMouseHoverMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowMouseHoverMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowMouseLeaveMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowMouseLeaveMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowLButtonDownMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowLButtonDownMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowLButtonUpMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowLButtonUpMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowLButtonDbClickMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowLButtonDbClickMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowRButtonDownMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowRButtonDownMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowRButtonUpMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowRButtonUpMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowRButtonDbClickMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowRButtonDbClickMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowMButtonDownMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowMButtonDownMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowMButtonUpMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowMButtonUpMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowMButtonDbClickMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowMButtonDbClickMsg].AddEventCallback(callback, callbackID);
}

void WindowBase::AttachWindowCaptureChangedMsg(const EventCallback& callback, EventCallbackID callbackID)
{
    m_windowEventMap[kWindowCaptureChangedMsg].AddEventCallback(callback, callbackID);
}

} // namespace ui

#include "duilib/Control/Flyout.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Control.h"
#include "duilib/Box/ScrollBox.h"
#include "duilib/Core/ScrollBar.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/WindowCreateParam.h"
#include "duilib/Core/WindowCreateAttributes.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Core/Callback.h"
#include "duilib/Animation/PopupAnimation.h"
#include "duilib/Animation/AnimationPlayer.h"

#if defined (DUILIB_BUILD_FOR_SDL)
    #include <SDL3/SDL.h>
#endif

namespace ui
{

namespace
{

/** 判断鼠标的任意键是否处于按下状态（轮询使用，跨平台）
*   SDL 模式下需要使用全局鼠标状态：浮层窗口不抢焦点，点击可能落在浮层之外的窗口
*   （例如 CEF 的原生子窗口）上，此时 SDL_GetMouseState 无法反映按键状态
*/
bool IsMouseButtonDownAny()
{
#if defined (DUILIB_BUILD_FOR_SDL)
    const SDL_MouseButtonFlags nButtonState = SDL_GetGlobalMouseState(nullptr, nullptr);
    return (nButtonState & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK | SDL_BUTTON_MMASK)) != 0;
#else
    return ((::GetAsyncKeyState(VK_LBUTTON) |
             ::GetAsyncKeyState(VK_RBUTTON) |
             ::GetAsyncKeyState(VK_MBUTTON)) & 0x8000) != 0;
#endif
}

/** 判断 Esc 键是否处于按下状态（轮询使用，跨平台）
*/
bool IsEscapeKeyDown()
{
    return Keyboard::IsKeyDown(kVK_ESCAPE);
}

#if defined(DUILIB_BUILD_FOR_WIN) && !defined(DUILIB_BUILD_FOR_SDL)
/** 阴影区域的命中测试过滤器（仅 Windows 平台）
*   浮层窗口的阴影区域不显示内容，却会拦截鼠标消息，导致其下方控件无法悬停、无法点击；
*   该过滤器把落在阴影（可视区域之外）上的 WM_NCHITTEST 返回为 HTTRANSPARENT，
*   使系统把鼠标消息派发给浮层下面的窗口（父窗口与其在同一线程，可正常接收）。
*/
class ShadowHitTestFilter : public IUIMessageFilter
{
public:
    explicit ShadowHitTestFilter(Flyout* pFlyout) : m_pFlyout(pFlyout) {}

    virtual LRESULT FilterMessage(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled) override
    {
        if ((uMsg == WM_NCHITTEST) && (m_pFlyout != nullptr) && m_pFlyout->IsShadowClickThrough()) {
            const int32_t xPos = (int32_t)GET_X_LPARAM(lParam);
            const int32_t yPos = (int32_t)GET_Y_LPARAM(lParam);
            if (!m_pFlyout->IsPointInVisualArea(UiPoint(xPos, yPos))) {
                bHandled = true;
                return HTTRANSPARENT;
            }
        }
        return 0;
    }

private:
    //所属浮层（本过滤器由该浮层持有，生命周期与其一致）
    Flyout* m_pFlyout;
};
#endif // DUILIB_BUILD_FOR_WIN && !DUILIB_BUILD_FOR_SDL

#if defined(DUILIB_BUILD_FOR_WIN) && !defined(DUILIB_BUILD_FOR_SDL)
/** 父窗口的消息过滤器（浮层打开期间安装在父窗口上）
*   不抢焦点模式下用于感知"点击浮层外部"：消息到达时同步判定，不受以下因素影响：
*   1. 鼠标按下消息被其它进程/子窗口（如 WebView2、CEF 子窗口）处理；
*   2. 点击非客户区（标题栏/边框）时系统进入模态移动循环，定时器无法按时触发；
*   3. 定时轮询的周期与被点击时长不匹配，导致整个按下过程被漏采。
*   同时通过父窗口的焦点/激活变化感知"焦点转移到其它窗口"（含子窗口、其它进程窗口）。
*/
class ParentWindowFilter : public IUIMessageFilter
{
public:
    explicit ParentWindowFilter(Flyout* pFlyout) : m_pFlyout(pFlyout) {}

    virtual LRESULT FilterMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& /*bHandled*/) override
    {
        if (m_pFlyout != nullptr) {
            m_pFlyout->OnParentWindowMessage(uMsg, wParam, lParam);
        }
        return 0;
    }

private:
    //所属浮层（本过滤器由该浮层持有，生命周期与其一致）
    Flyout* m_pFlyout;
};
#endif // DUILIB_BUILD_FOR_WIN && !DUILIB_BUILD_FOR_SDL

/** 获取与给定方位相反主方位的方位（用于空间不足时翻转，保留对齐方式）
*/
Flyout::Placement FlipPlacement(Flyout::Placement placement)
{
    switch (placement) {
    case Flyout::Placement::Bottom:       return Flyout::Placement::Top;
    case Flyout::Placement::BottomCenter: return Flyout::Placement::TopCenter;
    case Flyout::Placement::BottomEnd:    return Flyout::Placement::TopEnd;
    case Flyout::Placement::Top:          return Flyout::Placement::Bottom;
    case Flyout::Placement::TopCenter:    return Flyout::Placement::BottomCenter;
    case Flyout::Placement::TopEnd:       return Flyout::Placement::BottomEnd;
    case Flyout::Placement::Right:        return Flyout::Placement::Left;
    case Flyout::Placement::RightCenter:  return Flyout::Placement::LeftCenter;
    case Flyout::Placement::RightEnd:     return Flyout::Placement::LeftEnd;
    case Flyout::Placement::Left:         return Flyout::Placement::Right;
    case Flyout::Placement::LeftCenter:   return Flyout::Placement::RightCenter;
    case Flyout::Placement::LeftEnd:      return Flyout::Placement::RightEnd;
    default:                              return placement;
    }
}
} // namespace

//静态成员初始化
Flyout* Flyout::s_pActiveFlyout = nullptr;
const int32_t Flyout::kDetectIntervalMs = 50;

Flyout::Flyout(Window* pParentWindow) :
    m_pParentWindow(pParentWindow),
    m_placement(Placement::Bottom),
    m_actualPlacement(Placement::Bottom),
    m_bAutoDismiss(true),
    m_bClickOutsideDismiss(true),
    m_bShadowClickThrough(true),
    m_bFollowAnchor(true),
    m_bAnchorSuspended(false),
    m_bNoFocus(true),
    m_nGap(6),
    m_bAllowFlip(true),
    m_bOpen(false),
    m_bClosing(false),
    m_nDetectTaskId(0),
    m_bPointerDownLast(false),
    m_bEscapeDownLast(false)
{
    if (m_pParentWindow != nullptr) {
        m_skinFolder = m_pParentWindow->GetResourcePath().ToString();
    }
}

Flyout::~Flyout()
{
    StopDetectTimer();
    RemoveParentWindowFilter();
    if (m_pFadeInPlayer != nullptr) {
        m_pFadeInPlayer->Stop();
        m_pFadeInPlayer.reset();
    }
    //兜底：确保析构后活动浮层指针不悬空（正常关闭路径已在 DoClose/OnFinalMessage 清空）
    if (s_pActiveFlyout == this) {
        s_pActiveFlyout = nullptr;
    }
}

void Flyout::SetSkinFolder(const DString& skinFolder)
{
    m_skinFolder = skinFolder;
}

bool Flyout::ShowAt(Control* pAnchor, const DString& xmlFile, Placement placement)
{
    ASSERT(pAnchor != nullptr);
    ASSERT(!xmlFile.empty());
    if ((pAnchor == nullptr) || xmlFile.empty()) {
        return false;
    }
    Window* pAnchorWindow = pAnchor->GetWindow();
    ASSERT(pAnchorWindow != nullptr);
    if (pAnchorWindow == nullptr) {
        return false;
    }

    //同一时刻只保留一个活动浮层：先关闭已有的浮层
    if ((s_pActiveFlyout != nullptr) && (s_pActiveFlyout != this)) {
        s_pActiveFlyout->DoClose(CloseReason::kManual);
    }

    m_pAnchor = pAnchor;
    m_xml = xmlFile;
    m_placement = placement;
    m_actualPlacement = placement;
    m_bClosing = false;
    m_bOpen = false;
    m_bAnchorSuspended = false;

    //不抢焦点模式下，记录创建浮层前父窗口是否处于前台：
    //浮层窗口的创建过程可能使其意外成为前台窗口，从而使父窗口失去激活状态与键盘焦点
    const bool bParentWasForeground = (m_pParentWindow != nullptr) && m_pParentWindow->IsWindowForeground();

    //窗口初始位置设为锚点左上角（屏幕坐标），避免首次显示时在默认位置闪烁
    UiRect rcAnchorInit;
    GetAnchorScreenRect(rcAnchorInit);
    WindowCreateParam createWndParam;
    createWndParam.m_dwStyle = kWS_POPUP;
    createWndParam.m_dwExStyle = kWS_EX_LAYERED;
    if (m_bNoFocus) {
        //不抢焦点：显示后父窗口保持激活
        createWndParam.m_dwExStyle |= kWS_EX_NOACTIVATE;
    }
    if (m_pParentWindow == nullptr) {
        //没有父窗口时作为工具窗口，避免出现在任务栏和 Alt+Tab 列表中
        createWndParam.m_dwExStyle |= kWS_EX_TOOLWINDOW;
    }
    //计算创建时的初始位置：按方位预估目标位置（屏幕坐标），直接以目标位置创建窗口。
    //macOS(Cocoa)在窗口显示时可能先把窗口摆到"创建位置"，再应用之后的定位；
    //若创建位置不是目标位置，就会出现"先在错误位置显示一帧，再跳转到正常位置"的现象。
    UiPoint ptInitPos(rcAnchorInit.left, rcAnchorInit.top);
    UiSize szInitWindow;
    UiPadding rcInitShadow;
    if (GetPrefetchWindowSize(szInitWindow, rcInitShadow)) {
        Placement actualInit = m_placement;
        ptInitPos = CalcWindowPos(rcAnchorInit, szInitWindow, rcInitShadow, m_placement, m_bAllowFlip, actualInit);
    }
    createWndParam.m_nX = ptInitPos.x;
    createWndParam.m_nY = ptInitPos.y;

    if (!CreateWnd(m_pParentWindow, createWndParam)) {
        //创建失败（通常是 XML 资源路径错误）：销毁对象并返回，避免泄漏。
        //注意：若本对象此前已是活动浮层（重复调用 ShowAt 且此前 ShowAt 成功过），
        //直接 delete this 会留下指向已释放内存的悬空静态指针，须先清空。
        if (s_pActiveFlyout == this) {
            s_pActiveFlyout = nullptr;
        }
        delete this;
        return false;
    }

    //父窗口使用私有颜色主题（OpenColorTheme）时，浮层继承同一套配色，避免深色窗口弹出浅色卡片
    if (m_pParentWindow != nullptr) {
        const std::string& parentThemeData = m_pParentWindow->GetColorThemeXmlData();
        if (!parentThemeData.empty()) {
            OpenColorThemeData(parentThemeData);
        }
    }

    //计算锚点位置
    PositionAroundAnchor();
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (m_bNoFocus && bParentWasForeground && (m_pParentWindow != nullptr)) {
        //浮层窗口的创建过程可能使浮层成为前台窗口并抢走键盘焦点：
        //浮层不应持有键盘焦点，立即把前台状态与输入焦点恢复给父窗口
        //（仅Windows平台：SDL实现创建窗口时不抢前台与输入焦点）
        const HWND hFlyout = (HWND)GetWindowHandle();
        if ((::GetForegroundWindow() == hFlyout) || (::GetFocus() == hFlyout)) {
            m_pParentWindow->SetWindowForeground();
            m_pParentWindow->SetWindowFocus();
        }
    }
#endif
    //记录锚点的初始屏幕矩形，供跟随锚点时检测位移
    GetAnchorScreenRect(m_rcAnchorScreenLast);

    //弹出层淡入动画：在窗口显示前把 Alpha 置 0，避免出现一帧全不透明的闪烁
    PopupAnimation::PlayFadeIn(this, m_pFadeInPlayer);

    if (!m_bNoFocus) {
        //显示窗口，并且需要设置焦点
        ShowWindow(ui::kSW_SHOW_NORMAL);

        //设置为置顶窗口
        SetWindowAlwaysOnTop(true);

        SetWindowForeground();
        KeepParentActive();
    }
    else {
        //显示窗口，并且不需要设置焦点
        ShowWindow(ui::kSW_SHOW_NA);

        //设置为置顶窗口
        SetWindowAlwaysOnTop(true);

        if (bParentWasForeground && (m_pParentWindow != nullptr) && !m_pParentWindow->IsWindowForeground()) {
            //不抢焦点模式：浮层窗口创建后可能成为前台窗口，导致父窗口失去激活状态与键盘焦点
            //（例如自动完成浮层弹出后无法继续输入）；此处显式恢复父窗口的前台状态与输入焦点
            m_pParentWindow->SetWindowForeground();
            m_pParentWindow->SetWindowFocus();
        }
    }

    m_bOpen = true;
    s_pActiveFlyout = this;

    //安装父窗口消息过滤器：同步感知外部点击/焦点转移（安装在 ShowAt 完成之后，
    //避免浮层窗口创建过程中的瞬时焦点变化被误判为"点击外部"）
    InstallParentWindowFilter();

    if (m_bNoFocus || m_bFollowAnchor) {
        //不抢焦点模式需要检测外部点击；跟随锚点需要轮询锚点位置
        StartDetectTimer();
    }

    //触发打开回调
    std::vector<FlyoutEvent> callbacks = m_openedCallbacks;
    for (const FlyoutEvent& callback : callbacks) {
        callback(CloseReason::kManual);
    }
    return true;
}

void Flyout::Dismiss()
{
    DoClose(CloseReason::kManual);
}

void Flyout::SetAutoDismiss(bool bAutoDismiss)
{
    m_bAutoDismiss = bAutoDismiss;
}

bool Flyout::IsAutoDismiss() const
{
    return m_bAutoDismiss;
}

void Flyout::SetClickOutsideDismiss(bool bDismiss)
{
    m_bClickOutsideDismiss = bDismiss;
}

bool Flyout::IsClickOutsideDismiss() const
{
    return m_bClickOutsideDismiss;
}

void Flyout::SetShadowClickThrough(bool bClickThrough)
{
    m_bShadowClickThrough = bClickThrough;
}

bool Flyout::IsShadowClickThrough() const
{
    return m_bShadowClickThrough;
}

void Flyout::GetVisualRect(UiRect& rcVisual) const
{
    rcVisual.Clear();
    UiRect rcWindow;
    GetWindowRect(rcWindow);
    if (rcWindow.IsEmpty()) {
        return;
    }

    //窗口阴影占据的边缘区域不显示内容，也不参与命中判断
    //（GetWindowRect 返回屏幕坐标，阴影内边距是布局坐标，按像素密度换算后再扣除）
    UiPadding rcShadow = GetCurrentShadowCorner();
    Dpi().ClientSizeToWindowSize(rcShadow);
    UiRect rcTemp = rcWindow;
    rcTemp.Deflate(rcShadow);
    rcVisual = rcTemp.IsEmpty() ? rcWindow : rcTemp;
}

bool Flyout::IsPointInVisualArea(const UiPoint& ptScreen) const
{
    UiRect rcVisual;
    GetVisualRect(rcVisual);
    return !rcVisual.IsEmpty() && rcVisual.ContainsPt(ptScreen);
}

bool Flyout::IsHitTestTransparent(const UiPoint& pt) const
{
    if (!m_bShadowClickThrough) {
        return false;
    }
    UiRect rcClient;
    GetClientRect(rcClient);
    if (rcClient.IsEmpty()) {
        return false;
    }
    //窗口阴影占据的边缘区域不显示内容，也不接收鼠标消息
    UiRect rcVisual = rcClient;
    rcVisual.Deflate(GetCurrentShadowCorner());
    if (rcVisual.IsEmpty()) {
        return false;
    }
    return rcClient.ContainsPt(pt) && !rcVisual.ContainsPt(pt);
}

void Flyout::SetFollowAnchor(bool bFollow)
{
    if (m_bFollowAnchor == bFollow) {
        return;
    }
    m_bFollowAnchor = bFollow;
    if (m_bFollowAnchor && m_bOpen && (m_nDetectTaskId == 0)) {
        //运行中开启跟随：补启动轮询任务
        StartDetectTimer();
    }
}

bool Flyout::IsFollowAnchor() const
{
    return m_bFollowAnchor;
}

bool Flyout::IsAnchorVisible() const
{
    Control* pAnchor = m_pAnchor.get();
    if (pAnchor == nullptr) {
        return false;
    }
    Window* pAnchorWindow = pAnchor->GetWindow();
    if (pAnchorWindow == nullptr) {
        return false;
    }
    if (!pAnchor->IsVisible()) {
        return false;
    }

    //锚点在窗口客户区视觉坐标（布局坐标，物理像素）下的矩形：布局坐标扣除各级滚动容器累计的滚动偏移
    UiRect rcAnchor = pAnchor->GetPos();
    const UiPoint ptScrollOffset = pAnchor->GetScrollOffsetInScrollBox();
    rcAnchor.Offset(-ptScrollOffset.x, -ptScrollOffset.y);
    if (rcAnchor.IsEmpty()) {
        return false;
    }

    //被窗口客户区裁剪（例如滚动到客户区外）：GetClientRect 与 GetPos 同为布局坐标，无需再做换算
    UiRect rcClient;
    pAnchorWindow->GetClientRect(rcClient);
    UiRect rcVisible = rcAnchor;
    if (!rcVisible.Intersect(rcClient)) {
        return false;
    }

    //被滚动容器裁剪：锚点被滚出滚动区域后应当视为不可见
    Control* pParent = pAnchor->GetParent();
    while (pParent != nullptr) {
        if (dynamic_cast<ScrollBox*>(pParent) != nullptr) {
            //滚动容器自身在客户区视觉坐标下的矩形（还需扣除其上层滚动容器的偏移）
            UiRect rcParent = pParent->GetPos();
            const UiPoint ptParentScrollOffset = pParent->GetScrollOffsetInScrollBox();
            rcParent.Offset(-ptParentScrollOffset.x, -ptParentScrollOffset.y);
            if (!rcVisible.Intersect(rcParent)) {
                return false;
            }
        }
        pParent = pParent->GetParent();
    }
    return true;
}

bool Flyout::UpdateAnchorPosition()
{
    if (!m_bOpen || m_bClosing || (m_pAnchor == nullptr) || (GetRoot() == nullptr)) {
        return false;
    }
    const Placement oldPlacement = m_actualPlacement;
    PositionAroundAnchor();
    GetAnchorScreenRect(m_rcAnchorScreenLast);
    InvokeLayoutCallbacks();
    return m_actualPlacement != oldPlacement;
}

void Flyout::InvokeLayoutCallbacks()
{
    std::vector<FlyoutLayoutEvent> callbacks = m_layoutedCallbacks;
    for (const FlyoutLayoutEvent& callback : callbacks) {
        callback();
    }
}

void Flyout::SetNoFocus(bool bNoFocus)
{
    m_bNoFocus = bNoFocus;
}

bool Flyout::IsNoFocus() const
{
    return m_bNoFocus;
}

void Flyout::SetGap(int32_t nGap)
{
    m_nGap = nGap;
}

int32_t Flyout::GetGap() const
{
    return m_nGap;
}

void Flyout::SetAllowFlip(bool bAllowFlip)
{
    m_bAllowFlip = bAllowFlip;
}

bool Flyout::IsAllowFlip() const
{
    return m_bAllowFlip;
}

Control* Flyout::GetAnchor() const
{
    return m_pAnchor.get();
}

Flyout::Placement Flyout::GetPlacement() const
{
    return m_placement;
}

Flyout::Placement Flyout::GetActualPlacement() const
{
    return m_actualPlacement;
}

bool Flyout::IsOpen() const
{
    return m_bOpen;
}

void Flyout::AttachOpened(const FlyoutEvent& callback)
{
    if (callback) {
        m_openedCallbacks.push_back(callback);
    }
}

void Flyout::AttachClosed(const FlyoutEvent& callback)
{
    if (callback) {
        m_closedCallbacks.push_back(callback);
    }
}

void Flyout::AttachLayouted(const FlyoutLayoutEvent& callback)
{
    if (callback) {
        m_layoutedCallbacks.push_back(callback);
    }
}

Flyout* Flyout::GetActiveFlyout()
{
    return s_pActiveFlyout;
}

void Flyout::DismissActive()
{
    if (s_pActiveFlyout != nullptr) {
        s_pActiveFlyout->Dismiss();
    }
}

DString Flyout::GetSkinFolder()
{
    return m_skinFolder.c_str();
}

DString Flyout::GetSkinFile()
{
    return m_xml.c_str();
}

void Flyout::OnInitWindow()
{
    BaseClass::OnInitWindow();
#if defined(DUILIB_BUILD_FOR_WIN) && !defined(DUILIB_BUILD_FOR_SDL)
    //窗口阴影区域不拦截鼠标：安装命中测试过滤器，把阴影上的鼠标消息交给浮层下面的窗口
    if (m_pShadowHitTestFilter == nullptr) {
        m_pShadowHitTestFilter = std::make_unique<ShadowHitTestFilter>(this);
        AddMessageFilter(m_pShadowHitTestFilter.get());
    }
#endif
}

void Flyout::OnInitLayout()
{
    BaseClass::OnInitLayout();
    //布局已完成（首次绘制前）：通知回调按控件的实际布局矩形调整内容位置
    InvokeLayoutCallbacks();
}

void Flyout::OnFinalMessage()
{
    if (m_pShadowHitTestFilter != nullptr) {
        RemoveMessageFilter(m_pShadowHitTestFilter.get());
        m_pShadowHitTestFilter.reset();
    }
    StopDetectTimer();
    RemoveParentWindowFilter();
    m_bOpen = false;
    if (s_pActiveFlyout == this) {
        s_pActiveFlyout = nullptr;
    }

    //非模态窗口基类在 OnFinalMessage 返回后会自动 delete this
    BaseClass::OnFinalMessage();
}

LRESULT Flyout::OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = BaseClass::OnKillFocusMsg(pSetFocusWindow, nativeMsg, bHandled);
    //抢焦点模式下：焦点落到本窗口以外即关闭
    //不抢焦点模式（m_bNoFocus）下，浮层不持有键盘焦点，收到失去焦点消息属于正常运行状态
    //（例如父窗口重新成为前台窗口时），此时不应关闭浮层；
    //该模式下的外部点击关闭由 OnDetectTick 的定时检测负责
    if (!m_bNoFocus && m_bAutoDismiss && m_bClickOutsideDismiss && (pSetFocusWindow != this)) {
        DoClose(CloseReason::kClickOutside);
    }
    return lResult;
}

LRESULT Flyout::OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_bAutoDismiss && (vkCode == kVK_ESCAPE)) {
        bHandled = true;
        DoClose(CloseReason::kEscape);
        return 0;
    }
    return BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
}

void Flyout::OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor)
{
    BaseClass::OnWindowDisplayScaleChanged(nOldScaleFactor, nNewScaleFactor);
    if ((nNewScaleFactor == 0) || (nOldScaleFactor == nNewScaleFactor)) {
        //仅窗口的像素密度发生变化（界面显示比例不变）时，框架也会发出该通知：
        //例如macOS在Retina屏幕上显示窗口时，窗口像素密度会从初始值同步为屏幕的真实值，
        //但界面显示比例并未改变。此时浮层不需要关闭（否则会出现打开后立即自动关闭）。
        return;
    }
    //DPI 变化后锚点位置与窗口尺寸均需重算，直接关闭浮层更稳妥
    DoClose(CloseReason::kAnchorLost);
}

bool Flyout::GetAnchorScreenRect(UiRect& rcAnchor) const
{
    rcAnchor.Clear();
    Control* pAnchor = m_pAnchor.get();
    if (pAnchor == nullptr) {
        return false;
    }
    Window* pAnchorWindow = pAnchor->GetWindow();
    if (pAnchorWindow == nullptr) {
        return false;
    }

    //锚点控件在窗口客户区中的布局矩形（物理像素）
    rcAnchor = pAnchor->GetPos();
    //扣除各级滚动容器累计的滚动偏移，换算为窗口客户区视觉坐标
    //（GetPos 为滚动内容坐标，视觉坐标 = 内容坐标 - 滚动偏移，与 Menu/MenuBar 的换算约定一致）
    UiPoint ptScrollOffset = pAnchor->GetScrollOffsetInScrollBox();
    rcAnchor.Offset(-ptScrollOffset.x, -ptScrollOffset.y);

    //客户区布局坐标 -> 屏幕坐标：ClientToScreen 内部已按窗口像素密度换算，
    //此处不能再做一次密度换算（否则高DPI下锚点矩形会被重复缩小/放大）
    pAnchorWindow->ClientToScreen(rcAnchor);
    return true;
}

bool Flyout::IsPointInAnchor(const UiPoint& ptScreen) const
{
    UiRect rcAnchor;
    if (!GetAnchorScreenRect(rcAnchor)) {
        return false;
    }
    return rcAnchor.ContainsPt(ptScreen);
}

bool Flyout::IsPointOnAnchorScrollBar(const UiPoint& ptScreen) const
{
    if ((m_pParentWindow == nullptr) || (m_pAnchor == nullptr)) {
        return false;
    }
    Control* pAnchor = m_pAnchor.get();
    if (pAnchor == nullptr) {
        return false;
    }

    //屏幕物理坐标 -> 父窗口客户区布局坐标（与 FindControl 的入参坐标系一致）
    //ScreenToClient 内部已按窗口像素密度换算为布局坐标，此处不能再做一次密度换算
    UiPoint ptClient = ptScreen;
    m_pParentWindow->ScreenToClient(ptClient);
    Control* pControl = m_pParentWindow->FindControl(ptClient);
    if (pControl == nullptr) {
        return false;
    }

    //鼠标落点必须在滚动条上；滚动条的父控件即它所属的滚动容器
    Control* pScrollBox = pControl->GetParent();
    if ((dynamic_cast<ScrollBar*>(pControl) == nullptr) || (pScrollBox == nullptr)) {
        return false;
    }

    //该滚动容器必须是锚点的祖先，才属于"锚点所在滚动容器"的滚动条
    Control* pParent = pAnchor->GetParent();
    while (pParent != nullptr) {
        if (pParent == pScrollBox) {
            return true;
        }
        pParent = pParent->GetParent();
    }
    return false;
}

UiPoint Flyout::CalcWindowPos(const UiRect& rcAnchor, const UiSize& szWindow, const UiPadding& rcShadow,
                              Placement placement, bool bAllowFlip, Placement& actualPlacement) const
{
    actualPlacement = placement;

    //默认值：以锚点左上角作为窗口位置（后续按方位修正）
    UiPoint ptWindowPos(rcAnchor.left - rcShadow.left, rcAnchor.top - rcShadow.top);

    Control* pAnchor = m_pAnchor.get();
    Window* pAnchorWindow = (pAnchor != nullptr) ? pAnchor->GetWindow() : nullptr;
    if (pAnchorWindow == nullptr) {
        return ptWindowPos;
    }
    //坐标换算统一使用锚点窗口的DPI：浮层与锚点必然在同一显示器上，
    //以锚点窗口为基准可避免两个窗口的DPI状态不一致导致的坐标偏差
    const DpiManager& anchorDpi = pAnchorWindow->Dpi();

    //m_nGap 是逻辑像素：先按缩放比换算为物理像素，再换算为屏幕坐标
    const int32_t nGap = anchorDpi.GetScaleWindowSize(m_nGap);

    //以锚点中心点所在显示器的工作区为边界（屏幕坐标）
    const UiPoint ptAnchorCenter((rcAnchor.left + rcAnchor.right) / 2, (rcAnchor.top + rcAnchor.bottom) / 2);
    UiRect rcWork;
    if (!pAnchorWindow->GetMonitorWorkRect(ptAnchorCenter, rcWork) || rcWork.IsEmpty()) {
        return ptWindowPos;
    }

    //视觉区域的宽高（不含阴影，屏幕坐标）
    const int32_t nClientWidth = szWindow.cx - rcShadow.left - rcShadow.right;
    const int32_t nClientHeight = szWindow.cy - rcShadow.top - rcShadow.bottom;

    //按期望方位计算视觉区域左上角（屏幕坐标）
    int32_t nClientX = rcAnchor.left;
    int32_t nClientY = rcAnchor.top;
    switch (placement)
    {
    case Placement::Bottom:
        nClientX = rcAnchor.left;
        nClientY = rcAnchor.bottom + nGap;
        break;
    case Placement::BottomCenter:
        nClientX = (rcAnchor.left + rcAnchor.right - nClientWidth) / 2;
        nClientY = rcAnchor.bottom + nGap;
        break;
    case Placement::BottomEnd:
        nClientX = rcAnchor.right - nClientWidth;
        nClientY = rcAnchor.bottom + nGap;
        break;
    case Placement::Top:
        nClientX = rcAnchor.left;
        nClientY = rcAnchor.top - nGap - nClientHeight;
        break;
    case Placement::TopCenter:
        nClientX = (rcAnchor.left + rcAnchor.right - nClientWidth) / 2;
        nClientY = rcAnchor.top - nGap - nClientHeight;
        break;
    case Placement::TopEnd:
        nClientX = rcAnchor.right - nClientWidth;
        nClientY = rcAnchor.top - nGap - nClientHeight;
        break;
    case Placement::Right:
        nClientX = rcAnchor.right + nGap;
        nClientY = rcAnchor.top;
        break;
    case Placement::RightCenter:
        nClientX = rcAnchor.right + nGap;
        nClientY = (rcAnchor.top + rcAnchor.bottom - nClientHeight) / 2;
        break;
    case Placement::RightEnd:
        nClientX = rcAnchor.right + nGap;
        nClientY = rcAnchor.bottom - nClientHeight;
        break;
    case Placement::Left:
        nClientX = rcAnchor.left - nGap - nClientWidth;
        nClientY = rcAnchor.top;
        break;
    case Placement::LeftCenter:
        nClientX = rcAnchor.left - nGap - nClientWidth;
        nClientY = (rcAnchor.top + rcAnchor.bottom - nClientHeight) / 2;
        break;
    case Placement::LeftEnd:
        nClientX = rcAnchor.left - nGap - nClientWidth;
        nClientY = rcAnchor.bottom - nClientHeight;
        break;
    default:
        break;
    }

    //空间不足时翻转到对侧
    if (bAllowFlip) {
        const bool bBottomSide = (placement == Placement::Bottom) ||
                                 (placement == Placement::BottomCenter) ||
                                 (placement == Placement::BottomEnd);
        const bool bTopSide = (placement == Placement::Top) ||
                              (placement == Placement::TopCenter) ||
                              (placement == Placement::TopEnd);
        const bool bRightSide = (placement == Placement::Right) ||
                                (placement == Placement::RightCenter) ||
                                (placement == Placement::RightEnd);
        const bool bLeftSide = (placement == Placement::Left) ||
                               (placement == Placement::LeftCenter) ||
                               (placement == Placement::LeftEnd);

        if (bBottomSide && (nClientY + nClientHeight > rcWork.bottom)) {
            //下方放不下：尝试翻转到上方
            int32_t nFlippedY = rcAnchor.top - nGap - nClientHeight;
            if (nFlippedY >= rcWork.top) {
                nClientY = nFlippedY;
                actualPlacement = FlipPlacement(placement);
            }
        }
        else if (bTopSide && (nClientY < rcWork.top)) {
            //上方放不下：尝试翻转到下方
            int32_t nFlippedY = rcAnchor.bottom + nGap;
            if (nFlippedY + nClientHeight <= rcWork.bottom) {
                nClientY = nFlippedY;
                actualPlacement = FlipPlacement(placement);
            }
        }
        else if (bRightSide && (nClientX + nClientWidth > rcWork.right)) {
            //右方放不下：尝试翻转到左方
            int32_t nFlippedX = rcAnchor.left - nGap - nClientWidth;
            if (nFlippedX >= rcWork.left) {
                nClientX = nFlippedX;
                actualPlacement = FlipPlacement(placement);
            }
        }
        else if (bLeftSide && (nClientX < rcWork.left)) {
            //左方放不下：尝试翻转到右方
            int32_t nFlippedX = rcAnchor.right + nGap;
            if (nFlippedX + nClientWidth <= rcWork.right) {
                nClientX = nFlippedX;
                actualPlacement = FlipPlacement(placement);
            }
        }
    }

    //翻转后仍然溢出工作区时，夹持到工作区内
    if (nClientX < rcWork.left) {
        nClientX = rcWork.left;
    }
    else if (nClientX + nClientWidth > rcWork.right) {
        nClientX = rcWork.right - nClientWidth;
    }
    if (nClientY < rcWork.top) {
        nClientY = rcWork.top;
    }
    else if (nClientY + nClientHeight > rcWork.bottom) {
        nClientY = rcWork.bottom - nClientHeight;
    }

    //窗口位置需要加上阴影内边距（均为屏幕坐标）
    ptWindowPos.x = nClientX - rcShadow.left;
    ptWindowPos.y = nClientY - rcShadow.top;
    return ptWindowPos;
}

bool Flyout::GetPrefetchWindowSize(UiSize& szWindow, UiPadding& rcShadow)
{
    Control* pAnchor = m_pAnchor.get();
    Window* pAnchorWindow = (pAnchor != nullptr) ? pAnchor->GetWindow() : nullptr;
    if (pAnchorWindow == nullptr) {
        return false;
    }

    //解析窗口XML属性（解析结果在窗口内部缓存，CreateWnd时会复用，不会重复解析）
    WindowCreateAttributes createAttributes;
    GetCreateWindowAttributes(createAttributes);
    if ((createAttributes.m_szInitSize.cx <= 0) || (createAttributes.m_szInitSize.cy <= 0)) {
        return false;
    }

    //窗口大小（含阴影区域）为窗口坐标
    szWindow = createAttributes.m_szInitSize;

    //阴影边距为DIP值，需要换算为窗口坐标（屏幕坐标）
    const DpiManager& anchorDpi = pAnchorWindow->Dpi();
    rcShadow.left = anchorDpi.GetScaleWindowSize(createAttributes.m_rcShadowCorner.left);
    rcShadow.top = anchorDpi.GetScaleWindowSize(createAttributes.m_rcShadowCorner.top);
    rcShadow.right = anchorDpi.GetScaleWindowSize(createAttributes.m_rcShadowCorner.right);
    rcShadow.bottom = anchorDpi.GetScaleWindowSize(createAttributes.m_rcShadowCorner.bottom);
    return true;
}

void Flyout::PositionAroundAnchor()
{
    Box* pRoot = GetRoot();
    ASSERT(pRoot != nullptr);
    if (pRoot == nullptr) {
        return;
    }
    Control* pAnchor = m_pAnchor.get();
    if (pAnchor == nullptr) {
        return;
    }

    //锚点屏幕矩形（屏幕坐标）
    UiRect rcAnchor;
    if (!GetAnchorScreenRect(rcAnchor)) {
        return;
    }
    Window* pAnchorWindow = pAnchor->GetWindow();
    if (pAnchorWindow == nullptr) {
        return;
    }
    //坐标换算统一使用锚点窗口的DPI：浮层与锚点必然在同一显示器上，
    //以锚点窗口为基准可避免两个窗口的DPI状态不一致导致的坐标偏差
    const DpiManager& anchorDpi = pAnchorWindow->Dpi();

    //以锚点中心点所在显示器的工作区为边界（屏幕坐标）
    UiPoint ptAnchorCenter((rcAnchor.left + rcAnchor.right) / 2, (rcAnchor.top + rcAnchor.bottom) / 2);
    UiRect rcWork;
    if (!GetMonitorWorkRect(ptAnchorCenter, rcWork) || rcWork.IsEmpty()) {
        return;
    }

    //估算窗口大小：EstimateSize 与控件布局使用同一坐标系（物理像素），
    //需要先把工作区的屏幕坐标换算为该坐标系下的可用空间
    UiRect rcWorkLayout = rcWork;
    anchorDpi.WindowSizeToClientSize(rcWorkLayout);
    UiSize szAvailable(rcWorkLayout.Width(), rcWorkLayout.Height());
    UiEstSize estSize = pRoot->EstimateSize(szAvailable);
    UiSize szWindowLayout = szAvailable;
    if (estSize.cx.IsInt32()) {
        szWindowLayout.cx = estSize.cx.GetInt32();
    }
    if (estSize.cy.IsInt32()) {
        szWindowLayout.cy = estSize.cy.GetInt32();
    }
    if (szWindowLayout.cx > rcWorkLayout.Width()) {
        szWindowLayout.cx = rcWorkLayout.Width();
    }
    if (szWindowLayout.cy > rcWorkLayout.Height()) {
        szWindowLayout.cy = rcWorkLayout.Height();
    }

    //窗口尺寸（包含阴影区域）与阴影内边距换算为屏幕坐标，参与下面的定位与夹持计算
    UiSize szWindow = szWindowLayout;
    anchorDpi.ClientSizeToWindowSize(szWindow);
    UiPadding rcShadow = pRoot->GetPadding();
    anchorDpi.ClientSizeToWindowSize(rcShadow);

    Placement actual = m_placement;
    const UiPoint ptWindowPos = CalcWindowPos(rcAnchor, szWindow, rcShadow, m_placement, m_bAllowFlip, actual);
    m_actualPlacement = actual;

    SetWindowPos(InsertAfterWnd(),
                 ptWindowPos.x, ptWindowPos.y,
                 szWindow.cx, szWindow.cy,
                 kSWP_NOZORDER | kSWP_NOACTIVATE);
}

void Flyout::StartDetectTimer()
{
    StopDetectTimer();

#if defined(DUILIB_BUILD_FOR_WIN) && !defined(DUILIB_BUILD_FOR_SDL)
    //以当前按键状态为初始值，避免触发浮层的那次点击在抬起前被误判为新的外部按下
    m_bPointerDownLast = (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    m_bEscapeDownLast = (::GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
#else
    //SDL 模式：同样以当前按键状态为初始值（避免触发浮层的那次点击被误判）
    m_bPointerDownLast = IsMouseButtonDownAny();
    m_bEscapeDownLast = IsEscapeKeyDown();
#endif

    //该轮询任务同时用于：不抢焦点模式下的外部点击检测、跟随锚点时的位置检测
    m_nDetectTaskId = GlobalManager::Instance().Thread().PostDelayedTask(
        kThreadUI,
        UiBind(this, [this]() {
            m_nDetectTaskId = 0;
            OnDetectTick();
        }),
        kDetectIntervalMs);
}

void Flyout::StopDetectTimer()
{
    if (m_nDetectTaskId != 0) {
        GlobalManager::Instance().Thread().CancelTask(m_nDetectTaskId);
        m_nDetectTaskId = 0;
    }
}

void Flyout::InstallParentWindowFilter()
{
#if defined(DUILIB_BUILD_FOR_WIN) && !defined(DUILIB_BUILD_FOR_SDL)
    if ((m_pParentWindow != nullptr) && (m_pParentWindowFilter == nullptr)) {
        m_pParentWindowFilter = std::make_unique<ParentWindowFilter>(this);
        m_pParentWindow->AddMessageFilter(m_pParentWindowFilter.get());
    }
#endif
}

void Flyout::RemoveParentWindowFilter()
{
    if (m_pParentWindowFilter != nullptr) {
        if (m_pParentWindow != nullptr) {
            m_pParentWindow->RemoveMessageFilter(m_pParentWindowFilter.get());
        }
        m_pParentWindowFilter.reset();
    }
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
void Flyout::OnParentWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!m_bOpen || m_bClosing) {
        return;
    }
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
    {
        //客户区鼠标按下：lParam 为客户区坐标，转换为屏幕物理坐标后判定
        if (!m_bAutoDismiss || !m_bClickOutsideDismiss || (m_pParentWindow == nullptr)) {
            return;
        }
        UiPoint ptScreen((int32_t)GET_X_LPARAM(lParam), (int32_t)GET_Y_LPARAM(lParam));
        m_pParentWindow->ClientToScreen(ptScreen);
        CheckCloseOnOutsidePoint(ptScreen);
        return;
    }
    case WM_NCLBUTTONDOWN:
    case WM_NCRBUTTONDOWN:
    case WM_NCMBUTTONDOWN:
    case WM_NCXBUTTONDOWN:
    {
        //非客户区（标题栏/边框等）鼠标按下：lParam 已经是屏幕物理坐标
        if (!m_bAutoDismiss || !m_bClickOutsideDismiss) {
            return;
        }
        CheckCloseOnOutsidePoint(UiPoint((int32_t)GET_X_LPARAM(lParam), (int32_t)GET_Y_LPARAM(lParam)));
        return;
    }
    case WM_KILLFOCUS:
    {
        //父窗口失去键盘焦点：焦点转移到本浮层时不算外部点击（例如点击候选项，
        //浮层随后由候选项选中流程正常关闭），转移到其它窗口/子窗口（CEF 子窗口、
        //WebView2 子窗口、其它进程窗口）时关闭浮层
        if (!m_bAutoDismiss || !m_bClickOutsideDismiss) {
            return;
        }
        HWND hFocus = (HWND)wParam;
        if (hFocus != nullptr) {
            HWND hFocusRoot = ::GetAncestor(hFocus, GA_ROOT);
            if ((hFocusRoot != nullptr) && (hFocusRoot == (HWND)GetWindowHandle())) {
                return;
            }
        }
        DoClose(CloseReason::kClickOutside);
        return;
    }
    case WM_ACTIVATE:
    {
        //父窗口被其它窗口激活取代：关闭浮层
        if (!m_bAutoDismiss || !m_bClickOutsideDismiss) {
            return;
        }
        if (LOWORD(wParam) == WA_INACTIVE) {
            //激活目标是本浮层（点击浮层内容）时，不属于"点击外部"
            HWND hNewActive = (HWND)lParam;
            if ((hNewActive != nullptr) && (::GetAncestor(hNewActive, GA_ROOT) == (HWND)GetWindowHandle())) {
                return;
            }
            //浮层创建/显示期间可能产生瞬时激活，该通知在浮层显示完成、父窗口已恢复前台后
            //才被处理；此时父窗口仍是前台窗口，不属于"被其它窗口激活取代"，需要忽略，
            //否则浮层刚展开就会被误判为"点击外部"而关闭
            HWND hForeground = ::GetForegroundWindow();
            if (m_pParentWindow != nullptr) {
                HWND hParent = (HWND)m_pParentWindow->GetWindowHandle();
                if ((hForeground != nullptr) && (hForeground == hParent)) {
                    return;
                }
            }
            if ((hForeground != nullptr) && (::GetAncestor(hForeground, GA_ROOT) == (HWND)GetWindowHandle())) {
                return;
            }
            DoClose(CloseReason::kClickOutside);
        }
        return;
    }
    case WM_KEYDOWN:
    {
        //父窗口收到 Esc：关闭浮层（不截获该按键，仍由父窗口继续处理）
        if (m_bAutoDismiss && (wParam == VK_ESCAPE)) {
            DoClose(CloseReason::kEscape);
        }
        return;
    }
    default:
        return;
    }
}
#endif // DUILIB_BUILD_FOR_WIN && !DUILIB_BUILD_FOR_SDL

bool Flyout::CheckCloseOnOutsidePoint(const UiPoint& ptScreen)
{
    //落点判定使用可视区域（不含窗口阴影），阴影区域按"外部"处理
    if (IsPointInVisualArea(ptScreen)) {
        return false;
    }
    if (IsPointInAnchor(ptScreen)) {
        return false;
    }
    //跟随锚点时，按下落在"锚点所在滚动容器的滚动条"上属于滚动操作（滚动过程中浮层会跟随锚点移动），
    //此时不按"点击外部"关闭，否则拖动滚动条会把正在跟随的卡片关掉
    if (m_bFollowAnchor && IsPointOnAnchorScrollBar(ptScreen)) {
        return false;
    }
    DoClose(CloseReason::kClickOutside);
    return true;
}

void Flyout::OnDetectTick()
{
    if (!m_bOpen || m_bClosing) {
        return;
    }

    //窗口或父窗口失效（销毁/隐藏/最小化）时关闭
    //（因锚点不可见而挂起时，窗口是本模块主动隐藏的，不能按"窗口失效"关闭）
    if (!IsWindow() || (!m_bAnchorSuspended && !IsWindowVisible())) {
        DoClose(CloseReason::kAnchorLost);
        return;
    }
    if (m_pParentWindow != nullptr) {
        if (!m_pParentWindow->IsWindow() || !m_pParentWindow->IsWindowVisible() ||
            m_pParentWindow->IsWindowMinimized()) {
            DoClose(CloseReason::kAnchorLost);
            return;
        }
    }
    if (m_pAnchor == nullptr) {
        DoClose(CloseReason::kAnchorLost);
        return;
    }

    //跟随锚点：锚点因滚动/布局变化而位移时重新定位浮层；锚点被裁剪（滚出可视区域）时挂起浮层
    if (m_bFollowAnchor) {
        if (!IsAnchorVisible()) {
            //锚点已不可见（例如被滚出滚动容器的可视区域）：暂时隐藏浮层窗口，但保留浮层对象与打开状态；
            //锚点重新可见后自动恢复显示（若在此直接关闭，滚回后卡片不会重新出现，状态也会与用户预期不一致）
            if (!m_bAnchorSuspended) {
                m_bAnchorSuspended = true;
                ShowWindow(kSW_HIDE);
            }
        }
        else if (m_bAnchorSuspended) {
            //锚点重新可见：先按新位置重新定位，再显示窗口，避免"先显示后跳动"
            m_bAnchorSuspended = false;
            UpdateAnchorPosition();
            ShowWindow(m_bNoFocus ? kSW_SHOW_NA : kSW_SHOW_NORMAL);
        }
        else {
            UiRect rcAnchorNow;
            if (GetAnchorScreenRect(rcAnchorNow) && (rcAnchorNow != m_rcAnchorScreenLast)) {
                //重新定位时保持打开时的方位、不做翻转：浮层内容（例如尖角箭头与阴影）是按该方位生成的，
                //翻转到另一侧会与内容不一致；方位这一侧空间不足时由 PositionAroundAnchor 夹持到工作区内
                const bool bAllowFlipOld = m_bAllowFlip;
                m_bAllowFlip = false;
                UpdateAnchorPosition();
                m_bAllowFlip = bAllowFlipOld;
            }
        }
    }

    if (m_bAutoDismiss) {
        //鼠标任意按键的按下边沿，且落点既不在浮层内也不在锚点上时关闭
        //（不吞掉这次点击：消息会继续发给点击的目标窗口，例如另一个触发按钮）
        const bool bPointerDown = IsMouseButtonDownAny();
        const bool bPointerPressed = bPointerDown && !m_bPointerDownLast;
        m_bPointerDownLast = bPointerDown;
        if (bPointerPressed && m_bClickOutsideDismiss) {
            UiPoint ptCursor;
            GetCursorPos(ptCursor);
            //兜底路径：正常情况下已由父窗口消息过滤器同步关闭；这里保留定时检测，
            //用于父窗口收不到消息的场景（例如点击由其它进程的窗口或原生子窗口直接消费）
            if (CheckCloseOnOutsidePoint(ptCursor)) {
                return;
            }
        }

        //Esc 键的按下边沿
        const bool bEscapeDown = IsEscapeKeyDown();
        const bool bEscapePressed = bEscapeDown && !m_bEscapeDownLast;
        m_bEscapeDownLast = bEscapeDown;
        if (bEscapePressed) {
            DoClose(CloseReason::kEscape);
            return;
        }
    }

    if (m_bOpen && !m_bClosing) {
        m_nDetectTaskId = GlobalManager::Instance().Thread().PostDelayedTask(
            kThreadUI,
            UiBind(this, [this]() {
                m_nDetectTaskId = 0;
                OnDetectTick();
            }),
            kDetectIntervalMs);
    }
}

void Flyout::DoClose(CloseReason reason)
{
    if (m_bClosing) {
        return;
    }
    m_bClosing = true;
    m_bOpen = false;

    StopDetectTimer();
    RemoveParentWindowFilter();

    if (s_pActiveFlyout == this) {
        s_pActiveFlyout = nullptr;
    }

    //窗口实际关闭前触发回调
    std::vector<FlyoutEvent> callbacks = m_closedCallbacks;
    for (const FlyoutEvent& callback : callbacks) {
        callback(reason);
    }

    //不抢焦点模式下，鼠标点击浮层内容时框架会把原生键盘焦点移到浮层窗口
    //（Window::OnButtonDown 中“确保被点击的窗口有输入焦点”的逻辑），而窗口关闭是
    //异步的（CloseWnd 内部为 PostMessage WM_CLOSE）：若不在关闭前把焦点交还父窗口，
    //浮层销毁后原生焦点丢失，父窗口中的编辑框虽持有框架虚拟焦点也收不到键盘消息
    //（表现为“点击候选项后无法继续输入”）。
    //仅在“浮层自身持有焦点、父窗口仍为前台且尚未持有焦点”时恢复，避免抢走其它窗口
    //或父窗口内子窗口（如 CEF 原生子窗口）的焦点。
    if (m_bNoFocus && IsWindowForeground() &&
        (m_pParentWindow != nullptr) && !m_pParentWindow->IsWindowFocused()) {
        if (!m_pParentWindow->IsWindowForeground()) {
            m_pParentWindow->SetWindowForeground();
        }
        m_pParentWindow->SetWindowFocus();
    }

    if (IsWindow()) {
        CloseWnd();
    }
}

} // namespace ui

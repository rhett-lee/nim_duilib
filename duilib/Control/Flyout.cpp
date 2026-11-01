#include "duilib/Control/Flyout.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Control.h"
#include "duilib/Box/ScrollBox.h"
#include "duilib/Core/ScrollBar.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/WindowCreateParam.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Core/Callback.h"

namespace ui
{

namespace
{
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
    m_bFollowAnchor(false),
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

    //窗口初始位置设为锚点左上角（物理像素），避免首次显示时在默认位置闪烁
    UiRect rcAnchorInit;
    GetAnchorScreenRect(rcAnchorInit, true);
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
    createWndParam.m_nX = rcAnchorInit.left;
    createWndParam.m_nY = rcAnchorInit.top;

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
    //记录锚点的初始屏幕矩形，供跟随锚点时检测位移
    GetAnchorScreenRect(m_rcAnchorScreenLast, true);

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
    UiRect rcTemp = rcWindow;
    rcTemp.Deflate(GetCurrentShadowCorner());
    rcVisual = rcTemp.IsEmpty() ? rcWindow : rcTemp;
}

bool Flyout::IsPointInVisualArea(const UiPoint& ptScreen) const
{
    UiRect rcVisual;
    GetVisualRect(rcVisual);
    return !rcVisual.IsEmpty() && rcVisual.ContainsPt(ptScreen);
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

    //锚点在窗口客户区视觉坐标（DIP）下的矩形：布局坐标扣除各级滚动容器累计的滚动偏移
    UiRect rcAnchor = pAnchor->GetPos();
    const UiPoint ptScrollOffset = pAnchor->GetScrollOffsetInScrollBox();
    rcAnchor.Offset(-ptScrollOffset.x, -ptScrollOffset.y);
    if (rcAnchor.IsEmpty()) {
        return false;
    }

    //被窗口客户区裁剪（例如滚动到客户区外）
    UiRect rcClient;
    pAnchorWindow->GetClientRect(rcClient);
    pAnchorWindow->Dpi().WindowSizeToClientSize(rcClient);
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
    GetAnchorScreenRect(m_rcAnchorScreenLast, true);
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
    //DPI 变化后锚点位置与窗口尺寸均需重算，直接关闭浮层更稳妥
    DoClose(CloseReason::kAnchorLost);
}

bool Flyout::GetAnchorScreenRect(UiRect& rcAnchor, bool bPhysical) const
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

    //锚点控件在窗口客户区内容坐标系（DIP）下的布局矩形
    rcAnchor = pAnchor->GetPos();
    //扣除各级滚动容器累计的滚动偏移，换算为窗口客户区视觉坐标
    //（GetPos 为滚动内容坐标，视觉坐标 = 内容坐标 - 滚动偏移，与 Menu/MenuBar 的换算约定一致）
    UiPoint ptScrollOffset = pAnchor->GetScrollOffsetInScrollBox();
    rcAnchor.Offset(-ptScrollOffset.x, -ptScrollOffset.y);

    if (bPhysical) {
        //DIP -> 物理像素 -> 屏幕坐标，与 GetCursorPos/GetWindowRect 的物理坐标系一致
        pAnchorWindow->Dpi().ClientSizeToWindowSize(rcAnchor);
        pAnchorWindow->ClientToScreen(rcAnchor);
    }
    else {
        //先按 DIP 值加窗口原点（物理），再整体缩放回 DIP（与 Menu 的坐标换算约定保持一致）
        pAnchorWindow->ClientToScreen(rcAnchor);
        pAnchorWindow->Dpi().WindowSizeToClientSize(rcAnchor);
    }
    return true;
}

bool Flyout::IsPointInAnchor(const UiPoint& ptScreen) const
{
    UiRect rcAnchor;
    if (!GetAnchorScreenRect(rcAnchor, true)) {
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
    UiPoint ptClient = ptScreen;
    m_pParentWindow->ScreenToClient(ptClient);
    m_pParentWindow->Dpi().WindowSizeToClientSize(ptClient);
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

    //锚点屏幕矩形（DIP）
    UiRect rcAnchor;
    if (!GetAnchorScreenRect(rcAnchor, false)) {
        return;
    }

    //以锚点中心点所在显示器的工作区为边界（DIP）
    UiPoint ptAnchorCenter((rcAnchor.left + rcAnchor.right) / 2, (rcAnchor.top + rcAnchor.bottom) / 2);
    Dpi().ClientSizeToWindowSize(ptAnchorCenter);
    UiRect rcWork;
    if (!GetMonitorWorkRect(ptAnchorCenter, rcWork)) {
        return;
    }
    Dpi().WindowSizeToClientSize(rcWork);

    //估算窗口大小（返回值包含阴影区域）
    UiSize szAvailable(rcWork.Width(), rcWork.Height());
    UiEstSize estSize = pRoot->EstimateSize(szAvailable);
    UiSize szWindow = szAvailable;
    if (estSize.cx.IsInt32()) {
        szWindow.cx = estSize.cx.GetInt32();
    }
    if (estSize.cy.IsInt32()) {
        szWindow.cy = estSize.cy.GetInt32();
    }
    if (szWindow.cx > rcWork.Width()) {
        szWindow.cx = rcWork.Width();
    }
    if (szWindow.cy > rcWork.Height()) {
        szWindow.cy = rcWork.Height();
    }

    //窗口阴影所占据的边缘区域，视觉有效区域需要扣除
    UiPadding rcShadow = pRoot->GetPadding();
    int32_t nClientWidth = szWindow.cx - rcShadow.left - rcShadow.right;
    int32_t nClientHeight = szWindow.cy - rcShadow.top - rcShadow.bottom;

    const int32_t nGap = Dpi().GetScaleInt(m_nGap);

    //按期望方位计算视觉区域左上角（屏幕 DIP 坐标）
    Placement actual = m_placement;
    int32_t nClientX = rcAnchor.left;
    int32_t nClientY = rcAnchor.top;
    switch (m_placement)
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
    if (m_bAllowFlip) {
        const bool bBottomSide = (m_placement == Placement::Bottom) ||
                                 (m_placement == Placement::BottomCenter) ||
                                 (m_placement == Placement::BottomEnd);
        const bool bTopSide = (m_placement == Placement::Top) ||
                              (m_placement == Placement::TopCenter) ||
                              (m_placement == Placement::TopEnd);
        const bool bRightSide = (m_placement == Placement::Right) ||
                                (m_placement == Placement::RightCenter) ||
                                (m_placement == Placement::RightEnd);
        const bool bLeftSide = (m_placement == Placement::Left) ||
                               (m_placement == Placement::LeftCenter) ||
                               (m_placement == Placement::LeftEnd);

        if (bBottomSide && (nClientY + nClientHeight > rcWork.bottom)) {
            //下方放不下：尝试翻转到上方
            int32_t nFlippedY = rcAnchor.top - nGap - nClientHeight;
            if (nFlippedY >= rcWork.top) {
                nClientY = nFlippedY;
                actual = FlipPlacement(m_placement);
            }
        }
        else if (bTopSide && (nClientY < rcWork.top)) {
            //上方放不下：尝试翻转到下方
            int32_t nFlippedY = rcAnchor.bottom + nGap;
            if (nFlippedY + nClientHeight <= rcWork.bottom) {
                nClientY = nFlippedY;
                actual = FlipPlacement(m_placement);
            }
        }
        else if (bRightSide && (nClientX + nClientWidth > rcWork.right)) {
            //右方放不下：尝试翻转到左方
            int32_t nFlippedX = rcAnchor.left - nGap - nClientWidth;
            if (nFlippedX >= rcWork.left) {
                nClientX = nFlippedX;
                actual = FlipPlacement(m_placement);
            }
        }
        else if (bLeftSide && (nClientX < rcWork.left)) {
            //左方放不下：尝试翻转到右方
            int32_t nFlippedX = rcAnchor.right + nGap;
            if (nFlippedX + nClientWidth <= rcWork.right) {
                nClientX = nFlippedX;
                actual = FlipPlacement(m_placement);
            }
        }
    }
    m_actualPlacement = actual;

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

    //窗口位置需要加上阴影内边距，并从 DIP 换算回物理像素
    int32_t nWindowX = nClientX - rcShadow.left;
    int32_t nWindowY = nClientY - rcShadow.top;
    Dpi().ClientSizeToWindowSize(nWindowX);
    Dpi().ClientSizeToWindowSize(nWindowY);
    UiSize szWindowPhysical = szWindow;
    Dpi().ClientSizeToWindowSize(szWindowPhysical);

    SetWindowPos(InsertAfterWnd(),
                 nWindowX, nWindowY,
                 szWindowPhysical.cx, szWindowPhysical.cy,
                 kSWP_NOZORDER | kSWP_NOACTIVATE);
}

void Flyout::StartDetectTimer()
{
    StopDetectTimer();

#if defined(DUILIB_BUILD_FOR_WIN) && !defined(DUILIB_BUILD_FOR_SDL)
    //以当前按键状态为初始值，避免触发浮层的那次点击在抬起前被误判为新的外部按下
    m_bPointerDownLast = (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    m_bEscapeDownLast = (::GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
#endif

    //该轮询任务同时用于：不抢焦点模式下的外部点击检测（仅 Windows）、跟随锚点时的位置检测
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
            if (GetAnchorScreenRect(rcAnchorNow, true) && (rcAnchorNow != m_rcAnchorScreenLast)) {
                //重新定位时保持打开时的方位、不做翻转：浮层内容（例如尖角箭头与阴影）是按该方位生成的，
                //翻转到另一侧会与内容不一致；方位这一侧空间不足时由 PositionAroundAnchor 夹持到工作区内
                const bool bAllowFlipOld = m_bAllowFlip;
                m_bAllowFlip = false;
                UpdateAnchorPosition();
                m_bAllowFlip = bAllowFlipOld;
            }
        }
    }

#if defined(DUILIB_BUILD_FOR_WIN) && !defined(DUILIB_BUILD_FOR_SDL)
    if (m_bAutoDismiss) {
        //鼠标任意按键的按下边沿，且落点既不在浮层内也不在锚点上时关闭
        //（不吞掉这次点击：消息会继续发给点击的目标窗口，例如另一个触发按钮）
        const bool bPointerDown = ((::GetAsyncKeyState(VK_LBUTTON) |
                                    ::GetAsyncKeyState(VK_RBUTTON) |
                                    ::GetAsyncKeyState(VK_MBUTTON)) & 0x8000) != 0;
        const bool bPointerPressed = bPointerDown && !m_bPointerDownLast;
        m_bPointerDownLast = bPointerDown;
        if (bPointerPressed && m_bClickOutsideDismiss) {
            UiPoint ptCursor;
            GetCursorPos(ptCursor);

            //落点判定使用可视区域（不含窗口阴影），阴影区域按"外部"处理
            UiRect rcVisual;
            GetVisualRect(rcVisual);
            const bool bInsideFlyout = rcVisual.ContainsPt(ptCursor);

            if (!bInsideFlyout && !IsPointInAnchor(ptCursor)) {
                //跟随锚点时，按下落在"锚点所在滚动容器的滚动条"上属于滚动操作（滚动过程中浮层会跟随锚点移动），
                //此时不按"点击外部"关闭，否则拖动滚动条会把正在跟随的卡片关掉
                const bool bScrollOperation = m_bFollowAnchor && IsPointOnAnchorScrollBar(ptCursor);
                if (!bScrollOperation) {
                    DoClose(CloseReason::kClickOutside);
                    return;
                }
            }
        }

        //Esc 键的按下边沿
        const bool bEscapeDown = (::GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
        const bool bEscapePressed = bEscapeDown && !m_bEscapeDownLast;
        m_bEscapeDownLast = bEscapeDown;
        if (bEscapePressed) {
            DoClose(CloseReason::kEscape);
            return;
        }
    }
#endif

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

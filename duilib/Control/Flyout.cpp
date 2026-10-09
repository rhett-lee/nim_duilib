#include "duilib/Control/Flyout.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/WindowCreateParam.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Core/Callback.h"

namespace ui
{

//静态成员初始化
Flyout* Flyout::s_pActiveFlyout = nullptr;
const int32_t Flyout::kDetectIntervalMs = 50;

Flyout::Flyout(Window* pParentWindow) :
    m_pParentWindow(pParentWindow),
    m_placement(Placement::Bottom),
    m_actualPlacement(Placement::Bottom),
    m_bAutoDismiss(true),
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

    //窗口初始位置设为锚点左上角（物理像素），避免首次显示时在默认位置闪烁
    UiRect rcAnchorInit;
    GetAnchorScreenRect(rcAnchorInit, true);
    WindowCreateParam createWndParam;
    createWndParam.m_dwStyle = kWS_POPUP;
    createWndParam.m_dwExStyle = kWS_EX_TOPMOST | kWS_EX_LAYERED;
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

    //计算锚点位置并显示窗口
    PositionAroundAnchor();

    if (!m_bNoFocus) {
        SetWindowForeground();
    }

    m_bOpen = true;
    s_pActiveFlyout = this;

    if (m_bNoFocus) {
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
}

void Flyout::OnFinalMessage()
{
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
    if (m_bAutoDismiss && (pSetFocusWindow != this)) {
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
    case Placement::BottomEnd:
        nClientX = rcAnchor.right - nClientWidth;
        nClientY = rcAnchor.bottom + nGap;
        break;
    case Placement::Top:
        nClientX = rcAnchor.left;
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
    case Placement::RightEnd:
        nClientX = rcAnchor.right + nGap;
        nClientY = rcAnchor.bottom - nClientHeight;
        break;
    case Placement::Left:
        nClientX = rcAnchor.left - nGap - nClientWidth;
        nClientY = rcAnchor.top;
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
        const bool bBottomSide = (m_placement == Placement::Bottom) || (m_placement == Placement::BottomEnd);
        const bool bTopSide = (m_placement == Placement::Top) || (m_placement == Placement::TopEnd);
        const bool bRightSide = (m_placement == Placement::Right) || (m_placement == Placement::RightEnd);
        const bool bLeftSide = (m_placement == Placement::Left) || (m_placement == Placement::LeftEnd);

        if (bBottomSide && (nClientY + nClientHeight > rcWork.bottom)) {
            //下方放不下：尝试翻转到上方
            int32_t nFlippedY = rcAnchor.top - nGap - nClientHeight;
            if (nFlippedY >= rcWork.top) {
                nClientY = nFlippedY;
                actual = (m_placement == Placement::BottomEnd) ? Placement::TopEnd : Placement::Top;
            }
        }
        else if (bTopSide && (nClientY < rcWork.top)) {
            //上方放不下：尝试翻转到下方
            int32_t nFlippedY = rcAnchor.bottom + nGap;
            if (nFlippedY + nClientHeight <= rcWork.bottom) {
                nClientY = nFlippedY;
                actual = (m_placement == Placement::TopEnd) ? Placement::BottomEnd : Placement::Bottom;
            }
        }
        else if (bRightSide && (nClientX + nClientWidth > rcWork.right)) {
            //右方放不下：尝试翻转到左方
            int32_t nFlippedX = rcAnchor.left - nGap - nClientWidth;
            if (nFlippedX >= rcWork.left) {
                nClientX = nFlippedX;
                actual = (m_placement == Placement::RightEnd) ? Placement::LeftEnd : Placement::Left;
            }
        }
        else if (bLeftSide && (nClientX < rcWork.left)) {
            //左方放不下：尝试翻转到右方
            int32_t nFlippedX = rcAnchor.right + nGap;
            if (nFlippedX + nClientWidth <= rcWork.right) {
                nClientX = nFlippedX;
                actual = (m_placement == Placement::LeftEnd) ? Placement::RightEnd : Placement::Right;
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

    SetWindowPos(InsertAfterWnd(InsertAfterFlag::kHWND_TOPMOST),
                 nWindowX, nWindowY,
                 szWindowPhysical.cx, szWindowPhysical.cy,
                 kSWP_SHOWWINDOW | (m_bNoFocus ? kSWP_NOACTIVATE : 0));
}

void Flyout::StartDetectTimer()
{
    StopDetectTimer();

#if defined(DUILIB_BUILD_FOR_WIN) && !defined(DUILIB_BUILD_FOR_SDL)
    //以当前按键状态为初始值，避免触发浮层的那次点击在抬起前被误判为新的外部按下
    m_bPointerDownLast = (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    m_bEscapeDownLast = (::GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;

    m_nDetectTaskId = GlobalManager::Instance().Thread().PostDelayedTask(
        kThreadUI,
        UiBind(this, [this]() {
            m_nDetectTaskId = 0;
            OnDetectTick();
        }),
        kDetectIntervalMs);
#else
    //非 Windows 平台暂不支持不抢焦点模式下的外部点击检测，可通过 SetNoFocus(false) 使用焦点模式
#endif
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
    if (!IsWindow() || !IsWindowVisible()) {
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

#if defined(DUILIB_BUILD_FOR_WIN) && !defined(DUILIB_BUILD_FOR_SDL)
    if (m_bAutoDismiss) {
        //鼠标任意按键的按下边沿，且落点既不在浮层内也不在锚点上时关闭
        //（不吞掉这次点击：消息会继续发给点击的目标窗口，例如另一个触发按钮）
        const bool bPointerDown = ((::GetAsyncKeyState(VK_LBUTTON) |
                                    ::GetAsyncKeyState(VK_RBUTTON) |
                                    ::GetAsyncKeyState(VK_MBUTTON)) & 0x8000) != 0;
        const bool bPointerPressed = bPointerDown && !m_bPointerDownLast;
        m_bPointerDownLast = bPointerDown;
        if (bPointerPressed) {
            UiPoint ptCursor;
            GetCursorPos(ptCursor);

            UiRect rcFlyout;
            GetWindowRect(rcFlyout);
            const bool bInsideFlyout = rcFlyout.ContainsPt(ptCursor);

            if (!bInsideFlyout && !IsPointInAnchor(ptCursor)) {
                DoClose(CloseReason::kClickOutside);
                return;
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

    if (IsWindow()) {
        CloseWnd();
    }
}

} // namespace ui

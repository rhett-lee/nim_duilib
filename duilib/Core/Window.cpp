#include "Window.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Shadow.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ToolTip.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Utils/ApiWrapper.h"
#include "duilib/Utils/PerformanceUtil.h"

#include <VersionHelpers.h>
#include <Olectl.h>

namespace ui
{
Window::Window() :
    m_pRoot(nullptr),
    m_OnEvent(),
    m_renderOffset(),
    m_pFocus(nullptr),
    m_pEventHover(nullptr),
    m_pEventClick(nullptr),
    m_pEventKey(nullptr),
    m_ptLastMousePos(-1, -1),
    m_rcAlphaFix(0, 0, 0, 0),
    m_bFirstLayout(true),
    m_bIsArranged(false),
    m_aDelayedCleanup(),
    m_mOptionGroup(),
    m_defaultAttrHash(),
    m_strResourcePath()
{
    m_toolTip = std::make_unique<ToolTip>();
}

Window::~Window()
{
    ClearWindow(false);
}

Window* Window::GetParentWindow() const
{
    if (!m_parentFlag.expired()) {
        return dynamic_cast<Window*>(m_pParentWindow);
    }
    else {
        return nullptr;
    }
}

void Window::AttachWindowClose(const EventCallback& callback)
{
    m_OnEvent[kEventWindowClose] += callback;
}

void Window::InitWindow()
{
    __super::InitWindow();
    if (!IsWindow()) {
        return;
    }
    //根据窗口是否为层窗口，重新初始化阴影附加属性值(层窗口为true，否则为false)
    ASSERT(m_shadow == nullptr);
    if (m_shadow != nullptr) {
        return;
    }

    //创建窗口阴影
    m_shadow = std::make_unique<Shadow>(this);
    if (m_shadow->IsUseDefaultShadowAttached()) {
        m_shadow->SetShadowAttached(m_bIsLayeredWindow);
        m_shadow->SetUseDefaultShadowAttached(true);
    }

    //添加到全局管理器
    GlobalManager::Instance().AddWindow(this);

    //创建渲染接口
    ASSERT(m_render == nullptr);
    if (m_render == nullptr) {
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactory != nullptr);
        if (pRenderFactory != nullptr) {
            m_render.reset(pRenderFactory->CreateRender(this));
        }
    }
    ASSERT(m_render != nullptr);
    if ((m_render != nullptr) && (m_render->GetWidth() == 0)) {
        //在估算控件大小的时候，需要Render有宽高等数据，所以需要进行Resize初始化
        UiRect rcClient;
        GetClientRect(rcClient);
        if ((rcClient.Width() > 0) && (rcClient.Height() > 0)) {
            m_render->Resize(rcClient.Width(), rcClient.Height());
        }
    }
}

void Window::OnFinalMessage()
{
    ClearWindow(true);
    WindowBase::OnFinalMessage();
}

void Window::ClearWindow(bool bSendClose)
{
    if (!IsWindow()) {
        return;
    }
    //发送关闭事件
    if (bSendClose) {
        SendNotify(kEventWindowClose);
    }
    
    //回收控件
    GlobalManager::Instance().RemoveWindow(this);
    ReapObjects(GetRoot());

    //删除清理的控件
    for (auto it = m_aDelayedCleanup.begin(); it != m_aDelayedCleanup.end(); ++it) {
        delete* it;
    }
    m_aDelayedCleanup.clear();

    if (m_pRoot != nullptr) {
        delete m_pRoot;
        m_pRoot = nullptr;
    }

    RemoveAllClass();
    RemoveAllOptionGroups();

    m_toolTip.reset();
    m_shadow.reset();
}

bool Window::AttachBox(Box* pRoot)
{
    ASSERT(::IsWindow(m_hWnd));
    SetFocusControl(nullptr); //设置m_pFocus相关的状态
    m_pEventKey = nullptr;
    m_pEventHover = nullptr;
    m_pEventClick = nullptr;
    // Remove the existing control-tree. We might have gotten inside this function as
    // a result of an event fired or similar, so we cannot just delete the objects and
    // pull the internal memory of the calling code. We'll delay the cleanup.
    if ((m_pRoot != nullptr) && (pRoot != m_pRoot)) {
        AddDelayedCleanup(m_pRoot);
    }
    // Set the dialog root element
    m_pRoot = pRoot;
    m_controlFinder.SetRoot(pRoot);
    // Go ahead...
    m_bIsArranged = true;
    m_bFirstLayout = true;
    // Initiate all control
    bool isInit = InitControls(m_pRoot);
    AutoResizeWindow(false);
    return isInit;
}

bool Window::InitControls(Control* pControl)
{
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return false;
    }
    m_controlFinder.AddControl(pControl);
    pControl->SetWindow(this);   
    return true;
}

void Window::ReapObjects(Control* pControl)
{
    if (pControl == nullptr) {
        return;
    }
    if (pControl == m_pEventKey) {
        m_pEventKey = nullptr;
    }
    if (pControl == m_pEventHover) {
        m_pEventHover = nullptr;
    }
    if (pControl == m_pEventClick) {
        m_pEventClick = nullptr;
    }
    if (pControl == m_pFocus) {
        m_pFocus = nullptr;
    }
    m_controlFinder.RemoveControl(pControl);
}

const DString& Window::GetResourcePath() const
{
    return m_strResourcePath;
}

void Window::SetResourcePath(const DString& strPath)
{
    m_strResourcePath = strPath;
    if (!m_strResourcePath.empty()) {
        //确保路径最后字符是分割字符
        wchar_t cEnd = m_strResourcePath.back();
        if (cEnd != _T('\\') && cEnd != _T('/')) {
            m_strResourcePath += _T('\\');
        }
    }
}

void Window::AddClass(const DString& strClassName, const DString& strControlAttrList)
{
    ASSERT(!strClassName.empty());
    ASSERT(!strControlAttrList.empty());
    m_defaultAttrHash[strClassName] = strControlAttrList;
}

DString Window::GetClassAttributes(const DString& strClassName) const
{
    auto it = m_defaultAttrHash.find(strClassName);
    if (it != m_defaultAttrHash.end()) {
        return it->second;
    }
    return _T("");
}

bool Window::RemoveClass(const DString& strClassName)
{
    auto it = m_defaultAttrHash.find(strClassName);
    if (it != m_defaultAttrHash.end()) {
        m_defaultAttrHash.erase(it);
        return true;
    }
    return false;
}

void Window::RemoveAllClass()
{
    m_defaultAttrHash.clear();
}

void Window::AddTextColor(const DString& strName, const DString& strValue)
{
    m_colorMap.AddColor(strName, strValue);
}

void Window::AddTextColor(const DString& strName, UiColor argb)
{
    m_colorMap.AddColor(strName, argb);
}

UiColor Window::GetTextColor(const DString& strName) const
{
    return m_colorMap.GetColor(strName);
}

bool Window::AddOptionGroup(const DString& strGroupName, Control* pControl)
{
    ASSERT(!strGroupName.empty());
    ASSERT(pControl != nullptr);
    if ((pControl == nullptr) || strGroupName.empty()) {
        return false;
    }
    auto it = m_mOptionGroup.find(strGroupName);
    if (it != m_mOptionGroup.end()) {
        auto it2 = std::find(it->second.begin(), it->second.end(), pControl);
        if (it2 != it->second.end()) {
            return false;
        }
        it->second.push_back(pControl);
    }
    else {
        m_mOptionGroup[strGroupName].push_back(pControl);
    }
    return true;
}

std::vector<Control*>* Window::GetOptionGroup(const DString& strGroupName)
{
    auto it = m_mOptionGroup.find(strGroupName);
    if (it != m_mOptionGroup.end()) {
        return &(it->second);
    }
    return nullptr;
}

void Window::RemoveOptionGroup(const DString& strGroupName, Control* pControl)
{
    ASSERT(!strGroupName.empty());
    ASSERT(pControl != nullptr);
    auto it = m_mOptionGroup.find(strGroupName);
    if (it != m_mOptionGroup.end()) {
        auto it2 = std::find(it->second.begin(), it->second.end(), pControl);
        if (it2 != it->second.end()) {
            it->second.erase(it2);
        }

        if (it->second.empty()) {
            m_mOptionGroup.erase(it);
        }
    }
}

void Window::RemoveAllOptionGroups()
{
    m_mOptionGroup.clear();
}

void Window::ClearImageCache()
{
    Control* pRoot = nullptr;
    ASSERT(m_shadow != nullptr);
    if (m_shadow != nullptr) {
        pRoot = m_shadow->GetRoot();
    }
    if (pRoot) {
        pRoot->ClearImageCache();
    }
    else if (m_pRoot != nullptr) {
        m_pRoot->ClearImageCache();
    }
}

const UiPoint& Window::GetLastMousePos() const
{
    return m_ptLastMousePos;
}

void Window::OnUseSystemCaptionBarChanged()
{
    WindowBase::OnUseSystemCaptionBarChanged();
    if (IsUseSystemCaption()) {
        //关闭阴影
        SetShadowAttached(false);
    }
}

void Window::OnWindowAlphaChanged()
{
    WindowBase::OnWindowAlphaChanged();
    if (m_pRoot != nullptr) {
        Invalidate(m_pRoot->GetPos());
    }
}

void Window::OnLayeredWindowChanged()
{
    //根据窗口是否为层窗口，重新初始化阴影附加属性值(层窗口为true，否则为false)
    if ((m_shadow != nullptr) && m_shadow->IsUseDefaultShadowAttached()) {
        m_shadow->SetShadowAttached(m_bIsLayeredWindow);
        m_shadow->SetUseDefaultShadowAttached(true);
    }
    if (m_pRoot != nullptr) {
        Invalidate(m_pRoot->GetPos());
    }
}

void Window::GetShadowCorner(UiPadding& rcShadow) const
{
    rcShadow.Clear();
    ASSERT(m_shadow != nullptr);
    if (m_shadow != nullptr) {
        rcShadow = m_shadow->GetShadowCorner();
    }
}

const UiRect& Window::GetAlphaFixCorner() const
{
    return m_rcAlphaFix;
}

void Window::SetAlphaFixCorner(const UiRect& rc, bool bNeedDpiScale)
{
    ASSERT((rc.left >= 0) && (rc.top >= 0) && (rc.right >= 0) && (rc.bottom >= 0));
    if ((rc.left >= 0) && (rc.top >= 0) && (rc.right >= 0) && (rc.bottom >= 0)) {
        m_rcAlphaFix = rc;
        if (bNeedDpiScale) {
            Dpi().ScaleRect(m_rcAlphaFix);
        }
    }
}

Box* Window::AttachShadow(Box* pRoot)
{
    //将阴影附加到窗口
    ASSERT(m_shadow != nullptr);
    if (m_shadow != nullptr) {
        return m_shadow->AttachShadow(pRoot);
    }
    else {
        return pRoot;
    }
}

void Window::SetShadowAttached(bool bShadowAttached)
{
    ASSERT(m_shadow != nullptr);
    if (m_shadow != nullptr) {
        m_shadow->SetShadowAttached(bShadowAttached);
    }
}

DString Window::GetShadowImage() const
{
    ASSERT(m_shadow != nullptr);
    if (m_shadow != nullptr) {
        return m_shadow->GetShadowImage();
    }
    else {
        return DString();
    }
}

void Window::SetShadowImage(const DString& strImage)
{
    ASSERT(m_shadow != nullptr);
    if (m_shadow != nullptr) {
        m_shadow->SetShadowImage(strImage);
    }
}

UiPadding Window::GetShadowCorner() const
{
    ASSERT(m_shadow != nullptr);
    if (m_shadow != nullptr) {
        return m_shadow->GetShadowCorner();
    }
    else {
        return UiPadding();
    }
}

bool Window::IsShadowAttached() const
{
    ASSERT(m_shadow != nullptr);
    if (m_shadow != nullptr) {
        return m_shadow->IsShadowAttached();
    }
    else {
        return false;
    }
}

bool Window::IsUseDefaultShadowAttached() const
{
    ASSERT(m_shadow != nullptr);
    if (m_shadow != nullptr) {
        return m_shadow->IsUseDefaultShadowAttached();
    } 
    else {
        return false;
    }    
}

void Window::SetUseDefaultShadowAttached(bool isDefault)
{
    ASSERT(m_shadow != nullptr);
    if (m_shadow != nullptr) {
        m_shadow->SetUseDefaultShadowAttached(isDefault);
    }
}

void Window::SetShadowCorner(const UiPadding& padding, bool bNeedDpiScale)
{
    ASSERT(m_shadow != nullptr);
    if (m_shadow != nullptr) {
        m_shadow->SetShadowCorner(padding, bNeedDpiScale);
    }
}

void Window::SetInitSize(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
    ASSERT(IsWindow());
    if (m_pRoot == nullptr) {
        Resize(cx, cy, bContainShadow, bNeedDpiScale);
    }
}

LRESULT Window::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    LRESULT lResult = 0;
    bHandled = false;
    // Custom handling of events
    switch (uMsg)
    {
    case WM_APP + 1:
    {
        for (Control* pControl : m_aDelayedCleanup) {
            delete pControl;
        }
        m_aDelayedCleanup.clear();
        bHandled = true;
    }
    break;

    case WM_NCACTIVATE:            lResult = OnNcActivateMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NCCALCSIZE:            lResult = OnNcCalcSizeMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NCHITTEST:            lResult = OnNcHitTestMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NCLBUTTONDBLCLK:    lResult = OnNcLButtonDbClickMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_CLOSE:                lResult = OnCloseMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_GETMINMAXINFO:        lResult = OnGetMinMaxInfoMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_WINDOWPOSCHANGING:    lResult = OnWindowPosChangingMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_SIZE:                lResult = OnSizeMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_DPICHANGED:            lResult = OnDpiChangedMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_MOVE:                lResult = OnMoveMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_ERASEBKGND:            lResult = OnEraseBkGndMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_PAINT:                lResult = OnPaintMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_MOUSEHOVER:            lResult = OnMouseHoverMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_MOUSELEAVE:            lResult = OnMouseLeaveMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_MOUSEMOVE:            lResult = OnMouseMoveMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_MOUSEWHEEL:            lResult = OnMouseWheelMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_LBUTTONDOWN:        lResult = OnLButtonDownMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_RBUTTONDOWN:        lResult = OnRButtonDownMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_LBUTTONDBLCLK:        lResult = OnLButtonDoubleClickMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_RBUTTONDBLCLK:        lResult = OnRButtonDoubleClickMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_LBUTTONUP:            lResult = OnLButtonUpMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_RBUTTONUP:            lResult = OnRButtonUpMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_CONTEXTMENU:        lResult = OnContextMenuMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_SETFOCUS:            lResult = OnSetFocusMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_KILLFOCUS:            lResult = OnKillFocusMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_CHAR:                lResult = OnCharMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_KEYDOWN:            lResult = OnKeyDownMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_KEYUP:                lResult = OnKeyUpMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_SYSKEYDOWN:            lResult = OnSysKeyDownMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_SYSKEYUP:            lResult = OnSysKeyUpMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_IME_STARTCOMPOSITION: lResult = OnIMEStartCompositionMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_IME_ENDCOMPOSITION:      lResult = OnIMEEndCompositionMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_SETCURSOR:            lResult = OnSetCusorMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NOTIFY:                lResult = OnNotifyMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_COMMAND:            lResult = OnCommandMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_SYSCOMMAND:            lResult = OnSysCommandMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_HOTKEY:                lResult = OnHotKeyMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC:
        lResult = OnCtlColorMsgs(uMsg, wParam, lParam, bHandled);
        break;
    case WM_TOUCH:
        lResult = OnTouchMsg(uMsg, wParam, lParam, bHandled);
        break;
    case WM_POINTERDOWN:
    case WM_POINTERUP:
    case WM_POINTERUPDATE:
    case WM_POINTERLEAVE:
    case WM_POINTERWHEEL:
    case WM_POINTERCAPTURECHANGED:
        lResult = OnPointerMsgs(uMsg, wParam, lParam, bHandled);
        break;

    default:
        break;
    }//end of switch
    return lResult;
}

LRESULT Window::OnNcActivateMsg(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCACTIVATE);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    LRESULT lResult = 0;
    if (IsWindowMinimized()) {
        bHandled = false;
    }
    else {
        //MSDN: wParam 参数为 FALSE 时，应用程序应返回 TRUE 以指示系统应继续执行默认处理
        bHandled = true;
        lResult = (wParam == FALSE) ? TRUE : FALSE;
    }
    return lResult;
}

LRESULT Window::OnNcCalcSizeMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCCALCSIZE);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    //截获，让系统不处理此消息
    bHandled = true;
    return 0;
}

LRESULT Window::OnNcHitTestMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCHITTEST);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    bHandled = true;
    UiPoint pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    ScreenToClient(pt);

    UiRect rcClient;
    GetClientRect(rcClient);

    //客户区域，排除掉阴影部分区域
    if (m_shadow != nullptr) {
        UiPadding rcCorner = m_shadow->GetShadowCorner();
        rcClient.Deflate(rcCorner);
    }

    if (!IsWindowMaximized()) {
        //非最大化状态
        UiRect rcSizeBox = GetSizeBox();
        if (pt.y < rcClient.top + rcSizeBox.top) {
            if (pt.y >= rcClient.top) {
                if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                    return HTTOPLEFT;//在窗口边框的左上角。
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return HTTOPRIGHT;//在窗口边框的右上角
                }
                else {
                    return HTTOP;//在窗口的上水平边框中
                }
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
        else if (pt.y > rcClient.bottom - rcSizeBox.bottom) {
            if (pt.y <= rcClient.bottom) {
                if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                    return HTBOTTOMLEFT;//在窗口边框的左下角
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return HTBOTTOMRIGHT;//在窗口边框的右下角
                }
                else {
                    return HTBOTTOM;//在窗口的下水平边框中
                }
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }

        if (pt.x < rcClient.left + rcSizeBox.left) {
            if (pt.x >= rcClient.left) {
                return HTLEFT;//在窗口的左边框
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
        if (pt.x > rcClient.right - rcSizeBox.right) {
            if (pt.x <= rcClient.right) {
                return HTRIGHT;//在窗口的右边框中
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
    }

    UiRect rcCaption = GetCaptionRect();
    if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
        && pt.y >= rcClient.top + rcCaption.top && pt.y < rcClient.top + rcCaption.bottom) {
        Control* pControl = FindControl(pt);
        if (pControl) {
            if (pControl->CanPlaceCaptionBar()) {
                return HTCLIENT;//在工作区中（放在标题栏上的控件，视为工作区）
            }
            else {
                return HTCAPTION;//在标题栏中
            }
        }
    }
    //其他，在工作区中
    return HTCLIENT;
}

LRESULT Window::OnNcLButtonDbClickMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCLBUTTONDBLCLK);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    bHandled = true;
    if (!IsWindowMaximized()) {
        //最大化
        Maximized();
    }
    else {
        //还原
        Restore();
    }
    return 0;
}

LRESULT Window::OnCloseMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_CLOSE);
    bHandled = false;
    if (m_bFakeModal) {
        Window* pParentWindow = GetParentWindow();
        if (pParentWindow != nullptr) {
            pParentWindow->EnableWindow(true);
            pParentWindow->SetWindowFocus();
        }
        m_bFakeModal = false;
    }

    ClearStatus();
    if (IsWindowFocused()) {
        SetOwnerWindowFocus();        
    }
    return 0;
}

LRESULT Window::OnGetMinMaxInfoMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_GETMINMAXINFO);
    bHandled = false;
    LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
    UiRect rcWork;
    UiRect rcMonitor;
    GetMonitorRect(rcMonitor, rcWork);
    rcWork.Offset(-rcMonitor.left, -rcMonitor.top);

    //最大化时，默认设置为当前屏幕的最大区域
    lpMMI->ptMaxPosition.x = rcWork.left;
    lpMMI->ptMaxPosition.y = rcWork.top;
    lpMMI->ptMaxSize.x = rcWork.Width();
    lpMMI->ptMaxSize.y = rcWork.Height();

    if (GetMaxInfo(false).cx != 0) {
        lpMMI->ptMaxTrackSize.x = GetMaxInfo(true).cx;
    }
    if (GetMaxInfo(false).cy != 0) {
        lpMMI->ptMaxTrackSize.y = GetMaxInfo(true).cy;
    }
    if (GetMinInfo(false).cx != 0) {
        lpMMI->ptMinTrackSize.x = GetMinInfo(true).cx;
    }
    if (GetMinInfo(false).cy != 0) {
        lpMMI->ptMinTrackSize.y = GetMinInfo(true).cy;
    }
    return 0;
}

LRESULT Window::OnWindowPosChangingMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_WINDOWPOSCHANGING);
    bHandled = false;
    if (IsWindowMaximized()) {
        //最大化状态
        LPWINDOWPOS lpPos = (LPWINDOWPOS)lParam;
        if (lpPos->flags & SWP_FRAMECHANGED) // 第一次最大化，而不是最大化之后所触发的WINDOWPOSCHANGE
        {
            POINT pt = { 0, 0 };
            HMONITOR hMontorPrimary = ::MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
            HMONITOR hMonitorTo = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY);

            // 先把窗口最大化，再最小化，然后恢复，此时MonitorFromWindow拿到的HMONITOR不准确
            // 判断GetWindowRect的位置如果不正确（最小化时得到的位置信息是-38000），则改用normal状态下的位置，来获取HMONITOR
            UiRect rc;
            GetWindowRect(rc);
            if (rc.left < -10000 && rc.top < -10000 && rc.bottom < -10000 && rc.right < -10000) {
                WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
                ::GetWindowPlacement(m_hWnd, &wp);
                hMonitorTo = ::MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTOPRIMARY);
            }
            if (hMonitorTo != hMontorPrimary) {
                // 解决无边框窗口在双屏下面（副屏分辨率大于主屏）时，最大化不正确的问题
                MONITORINFO  miTo = { sizeof(miTo), 0 };
                ::GetMonitorInfo(hMonitorTo, &miTo);

                lpPos->x = miTo.rcWork.left;
                lpPos->y = miTo.rcWork.top;
                lpPos->cx = miTo.rcWork.right - miTo.rcWork.left;
                lpPos->cy = miTo.rcWork.bottom - miTo.rcWork.top;
            }
        }
    }
    return 0;
}

LRESULT Window::OnSizeMsg(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_SIZE);
    bHandled = false;
    UiSize szRoundCorner = GetRoundCorner();
    bool isIconic = IsWindowMinimized();
    if (!isIconic && (wParam != SIZE_MAXIMIZED) && (szRoundCorner.cx > 0 && szRoundCorner.cy > 0)) {
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
    if (m_pRoot != nullptr) {
        m_pRoot->Arrange();
    }
    if (wParam == SIZE_MAXIMIZED) {
        if (m_shadow != nullptr) {
            m_shadow->MaximizedOrRestored(true);
        }
    }
    else if (wParam == SIZE_RESTORED) {
        if (m_shadow != nullptr) {
            m_shadow->MaximizedOrRestored(false);
        }
    }
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventWindowSize);
    }
    return 0;
}

LRESULT Window::OnDpiChangedMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_DPICHANGED);
    bHandled = false;//需要重新测试

    uint32_t nNewDPI = HIWORD(wParam);
    UiRect rcNewWindow;
    const RECT* prcNewWindow = (RECT*)lParam;
    if (prcNewWindow != nullptr) {
        rcNewWindow.left = prcNewWindow->left;
        rcNewWindow.top = prcNewWindow->top;
        rcNewWindow.right = prcNewWindow->right;
        rcNewWindow.bottom = prcNewWindow->bottom;
    }
    ProcessDpiChangedMsg(nNewDPI, rcNewWindow);
    return 0;
}

void Window::OnDpiScaleChanged(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if ((nOldDpiScale == nNewDpiScale) || (nNewDpiScale == 0)) {
        return;
    }
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    WindowBase::OnDpiScaleChanged(nOldDpiScale, nNewDpiScale);
    //窗口阴影
    if (m_shadow != nullptr) {
        m_shadow->ChangeDpiScale(Dpi(), nOldDpiScale, nNewDpiScale);
    }

    //更新窗口自身的DPI关联属性
    m_rcAlphaFix = Dpi().GetScaleRect(m_rcAlphaFix, nOldDpiScale);
    m_renderOffset = Dpi().GetScalePoint(m_renderOffset, nOldDpiScale);
    m_ptLastMousePos = Dpi().GetScalePoint(m_ptLastMousePos, nOldDpiScale);

    //更新布局和控件的DPI关联属性
    SetArrange(true);

    Box* pRoot = GetRoot();
    if (pRoot != nullptr) {
        pRoot->ChangeDpiScale(nOldDpiScale, nNewDpiScale);
        pRoot->Arrange();
        Invalidate(m_pRoot->GetPos());
    }
}

LRESULT Window::OnMoveMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_MOVE);
    bHandled = false;
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventWindowMove);
    }
    return 0;
}

LRESULT Window::OnEraseBkGndMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_ERASEBKGND);
    bHandled = true;
    return 1;
}

LRESULT Window::OnPaintMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_PAINT);
    bHandled = true;
    PerformanceStat statPerformance(_T("Window::OnPaintMsg"));
    Paint();
    return 0;
}

LRESULT Window::OnMouseHoverMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_MOUSEHOVER);
    bHandled = false;
    m_toolTip->SetMouseTracking(this, false);
    if (IsCaptured()) {
        //如果处于Capture状态，不显示ToolTip
        return 0;
    }

    UiPoint trackPos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    Control* pHover = FindControl(trackPos);
    if (pHover == nullptr) {
        return 0;
    }

    Control* pOldHover = GetHoverControl();
    if (pHover != nullptr) {
        pHover->SendEvent(kEventMouseHover, 0, 0, 0, trackPos);
    }

    if (pOldHover == GetHoverControl()) {
        //检查按需显示ToolTip信息    
        UiRect rect = pHover->GetPos();
        uint32_t maxWidth = pHover->GetToolTipWidth();
        DString toolTipText = pHover->GetToolTipText();
        m_toolTip->ShowToolTip(this, rect, maxWidth, trackPos, toolTipText);
    }
    return 0;
}

LRESULT Window::OnMouseLeaveMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_MOUSELEAVE);
    bHandled = false;
    m_toolTip->HideToolTip();
    m_toolTip->ClearMouseTracking();
    return 0;
}

void Window::UpdateToolTip()
{
    //隐藏现有的，等待重新显示即会更新
    m_toolTip->HideToolTip();
    m_toolTip->ClearMouseTracking();
}

LRESULT Window::OnMouseMoveMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_MOUSEMOVE);
    bHandled = false;
    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    OnMouseMove(wParam, lParam, pt);
    return 0;
}

LRESULT Window::OnMouseWheelMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_MOUSEWHEEL);
    bHandled = false;
    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    ScreenToClient(pt);
    OnMouseWheel(wParam, lParam, pt);
    return 0;
}

LRESULT Window::OnLButtonDownMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_LBUTTONDOWN);

    bHandled = false;
    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    OnButtonDown(kEventMouseButtonDown, wParam, lParam, pt);
    return 0;
}

LRESULT Window::OnRButtonDownMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_RBUTTONDOWN);
    bHandled = false;
    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    OnButtonDown(kEventMouseRButtonDown, wParam, lParam, pt);
    return 0;
}

LRESULT Window::OnLButtonDoubleClickMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_LBUTTONDBLCLK);
    bHandled = false;
    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    OnButtonDown(kEventMouseDoubleClick, wParam, lParam, pt);
    return 0;
}

LRESULT Window::OnRButtonDoubleClickMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_RBUTTONDBLCLK);
    bHandled = false;
    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    OnButtonDown(kEventMouseRDoubleClick, wParam, lParam, pt);
    return 0;
}

LRESULT Window::OnLButtonUpMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_LBUTTONUP);
    bHandled = false;
    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    OnButtonUp(kEventMouseButtonUp, wParam, lParam, pt);
    return 0;
}

LRESULT Window::OnRButtonUpMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_RBUTTONUP);
    bHandled = false;
    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    OnButtonUp(kEventMouseRButtonUp, wParam, lParam, pt);
    return 0;
}

LRESULT Window::OnContextMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_CONTEXTMENU);
    bHandled = false;
    ReleaseCapture();

    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    if ((pt.x != -1) && (pt.y != -1)) {
        ScreenToClient(pt);
        m_ptLastMousePos = pt;
        Control* pControl = FindContextMenuControl(&pt);
        if (pControl != nullptr) {
            Control* ptControl = FindControl(pt);//当前点击点所在的控件
            pControl->SendEvent(kEventMouseMenu, wParam, (LPARAM)ptControl, 0, UiPoint(pt));
        }
    }
    else {
        //如果用户键入 SHIFT+F10，则上下文菜单为 -1, -1，
        //应用程序应在当前所选内容的位置（而不是 (xPos、yPos) ）显示上下文菜单。
        Control* pControl = FindContextMenuControl(nullptr);
        if (pControl != nullptr) {
            pControl->SendEvent(kEventMouseMenu, wParam, 0, 0, UiPoint(pt));
        }
    }
    return 0;
}

void Window::OnButtonDown(EventType eventType, WPARAM wParam, LPARAM lParam, const UiPoint& pt)
{
    ASSERT(eventType == kEventMouseButtonDown || 
           eventType == kEventMouseRButtonDown || 
           eventType == kEventMouseDoubleClick ||
           eventType == kEventMouseRDoubleClick);
    CheckSetWindowFocus();
    m_ptLastMousePos = pt;
    Control* pControl = FindControl(pt);
    if (pControl != nullptr) {
        Control* pOldEventClick = m_pEventClick;
        m_pEventClick = pControl;
        pControl->SetFocus();
        SetCapture();
        pControl->SendEvent(eventType, wParam, lParam, 0, pt);
        if ((pOldEventClick != nullptr) && (pOldEventClick != pControl)) {
            pOldEventClick->SendEvent(kEventMouseClickChanged);
        }
    }
}

void Window::OnButtonUp(EventType eventType, WPARAM wParam, LPARAM lParam, const UiPoint& pt)
{
    ASSERT(eventType == kEventMouseButtonUp || eventType == kEventMouseRButtonUp);
    m_ptLastMousePos = pt;
    ReleaseCapture();
    if (m_pEventClick != nullptr) {
        m_pEventClick->SendEvent(eventType, wParam, lParam, 0, pt);
        m_pEventClick = nullptr;
    }
}

void Window::OnMouseMove(WPARAM wParam, LPARAM lParam, const UiPoint& pt)
{
    m_toolTip->SetMouseTracking(this, true);
    m_ptLastMousePos = pt;

    // Do not move the focus to the new control when the mouse is pressed
    if (!IsCaptured()) {
        if (!HandleMouseEnterLeave(pt, wParam, lParam)) {
            return;
        }
    }

    if (m_pEventClick != nullptr) {
        m_pEventClick->SendEvent(kEventMouseMove, 0, lParam, 0, pt);
    }
    else if (m_pEventHover != nullptr) {
        m_pEventHover->SendEvent(kEventMouseMove, 0, lParam, 0, pt);
    }
}

void Window::OnMouseWheel(WPARAM wParam, LPARAM lParam, const UiPoint& pt)
{
    m_ptLastMousePos = pt;
    Control* pControl = FindControl(pt);
    if (pControl != nullptr) {
        pControl->SendEvent(kEventMouseWheel, wParam, lParam, 0, pt);
    }
}

void Window::ClearStatus()
{
    if (m_pEventHover != nullptr) {
        m_pEventHover->SendEvent(kEventMouseLeave);
        m_pEventHover = nullptr;
    }
    if (m_pEventClick != nullptr) {
        m_pEventClick->SendEvent(kEventMouseLeave);
        m_pEventClick = nullptr;
    }
    if (m_pEventKey != nullptr) {
        m_pEventKey->SendEvent(kEventMouseLeave);
        m_pEventKey = nullptr;
    }
    KillFocusControl();
}

bool Window::HandleMouseEnterLeave(const UiPoint& pt, WPARAM wParam, LPARAM lParam)
{
    Control* pNewHover = FindControl(pt);
    //设置为新的Hover控件
    Control* pOldHover = m_pEventHover;
    m_pEventHover = pNewHover;

    if ((pNewHover != pOldHover) && (pOldHover != nullptr)) {
        //Hover状态的控件发生变化，原来Hover控件的Tooltip应消失
        pOldHover->SendEvent(kEventMouseLeave, 0, 0, 0, pt);
        m_toolTip->HideToolTip();
    }
    ASSERT(pNewHover == m_pEventHover);
    if (pNewHover != m_pEventHover) {
        return false;
    }

    if ((pNewHover != pOldHover) && (pNewHover != nullptr)) {
        pNewHover->SendEvent(kEventMouseEnter, wParam, lParam, 0, pt);
    }
    return true;
}

LRESULT Window::OnIMEStartCompositionMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_IME_STARTCOMPOSITION);
    bHandled = false;
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventImeStartComposition, wParam, lParam, static_cast<TCHAR>(wParam));
    }
    return 0;
}

LRESULT Window::OnIMEEndCompositionMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_IME_ENDCOMPOSITION);
    bHandled = false;
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventImeEndComposition, wParam, lParam, static_cast<TCHAR>(wParam));
    }
    return 0;
}

LRESULT Window::OnSetFocusMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_SETFOCUS);
    bHandled = false;
    return 0;
}

LRESULT Window::OnKillFocusMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_KILLFOCUS);
    bHandled = false;
    Control* pEventClick = m_pEventClick;
    m_pEventClick = nullptr;
    ReleaseCapture();
    if (pEventClick != nullptr) {
        pEventClick->SendEvent(kEventWindowKillFocus, wParam, lParam, 0, UiPoint());
    }
    Control* pFocus = m_pFocus;
    if ((pFocus != nullptr) && (pFocus != pEventClick)){
        pFocus->SendEvent(kEventWindowKillFocus, wParam, lParam, 0, UiPoint());
    }
    return 0;
}

LRESULT Window::OnCharMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_CHAR);
    bHandled = false;
    if (m_pEventKey != nullptr) {
        m_pEventKey->SendEvent(kEventChar, wParam, lParam, static_cast<TCHAR>(wParam));
    }
    return 0;
}

LRESULT Window::OnKeyDownMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_KEYDOWN);
    bHandled = false;
    if (IsWindowFullScreen() && (wParam == kVK_ESCAPE)) {
        //按ESC键时，退出全屏
        ExitFullScreen();
        return 0;
    }

    if (m_pFocus != nullptr) {
        if (wParam == kVK_TAB) {
            if (m_pFocus->IsVisible() &&
                m_pFocus->IsEnabled() &&
                m_pFocus->IsWantTab()) {
                return 0;
            }
            else {
                //通过TAB键切换焦点控件
                SetNextTabControl(!Keyboard::IsKeyDown(kVK_SHIFT));
            }
        }
        else {
            m_pEventKey = m_pFocus;
            m_pFocus->SendEvent(kEventKeyDown, wParam, lParam, static_cast<TCHAR>(wParam));
        }
    }
    if ((wParam == kVK_ESCAPE) && (m_pEventClick != nullptr)) {
        m_pEventClick->SendEvent(kEventMouseClickEsc);
    }
    return 0;
}

LRESULT Window::OnKeyUpMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_KEYUP);
    bHandled = false;
    if (m_pEventKey != nullptr) {
        m_pEventKey->SendEvent(kEventKeyUp, wParam, lParam, static_cast<TCHAR>(wParam));
        m_pEventKey = nullptr;
    }
    return 0;
}

LRESULT Window::OnSysKeyDownMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_SYSKEYDOWN);
    bHandled = false;
    m_pEventKey = m_pFocus;
    if (m_pEventKey != nullptr) {
        m_pEventKey->SendEvent(kEventSysKeyDown, wParam, lParam, static_cast<TCHAR>(wParam));
    }
    return 0;
}

LRESULT Window::OnSysKeyUpMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_SYSKEYUP);
    bHandled = false;
    if (m_pEventKey != nullptr) {
        m_pEventKey->SendEvent(kEventSysKeyUp, wParam, lParam, static_cast<TCHAR>(wParam));
        m_pEventKey = nullptr;
    }
    return 0;
}

LRESULT Window::OnSetCusorMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_SETCURSOR);
    bHandled = false;
    if (LOWORD(lParam) != HTCLIENT) {
        return 0;
    }
    if (m_pEventClick != nullptr) {
        bHandled = true;
        return 0;
    }

    UiPoint pt;
    GetCursorPos(pt);
    ScreenToClient(pt);
    m_ptLastMousePos = pt;
    Control* pControl = FindControl(pt);
    if (pControl != nullptr) {
        pControl->SendEvent(kEventSetCursor, wParam, lParam, 0, pt);
        bHandled = true;
    }
    return 0;
}

LRESULT Window::OnNotifyMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NOTIFY);
    bHandled = false;
    LPNMHDR lpNMHDR = (LPNMHDR)lParam;
    if (lpNMHDR != nullptr) {
        bHandled = true;
        return ::SendMessage(lpNMHDR->hwndFrom, OCM__BASE + uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT Window::OnCommandMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_COMMAND);
    bHandled = false;
    if (lParam == 0) {
        return 0;
    }
    HWND hWndChild = (HWND)lParam;
    bHandled = true;
    return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

LRESULT Window::OnSysCommandMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_SYSCOMMAND);
    UNUSED_VARIABLE(wParam);
    UNUSED_VARIABLE(lParam);
    bHandled = false;
    return 0;
}

LRESULT Window::OnHotKeyMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_HOTKEY);
    UNUSED_VARIABLE(wParam);
    UNUSED_VARIABLE(lParam);
    bHandled = false;
    return 0;
}

LRESULT Window::OnCtlColorMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_CTLCOLOREDIT || uMsg == WM_CTLCOLORSTATIC);
    bHandled = false;
    // Refer To: http://msdn.microsoft.com/en-us/library/bb761691(v=vs.85).aspx
    // Read-only or disabled edit controls do not send the WM_CTLCOLOREDIT message; instead, they send the WM_CTLCOLORSTATIC message.
    if (lParam == 0) {
        return 0;
    }
    HWND hWndChild = (HWND)lParam;
    bHandled = true;
    return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

LRESULT Window::OnTouchMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_TOUCH);
    bHandled = false;
    unsigned int nNumInputs = LOWORD(wParam);
    if (nNumInputs < 1) {
        nNumInputs = 1;
    }
    TOUCHINPUT* pInputs = new TOUCHINPUT[nNumInputs];
    // 只关心第一个触摸位置
    if (!GetTouchInputInfoWrapper((HTOUCHINPUT)lParam, nNumInputs, pInputs, sizeof(TOUCHINPUT))) {
        delete[] pInputs;
        return 0;
    }
    else {
        CloseTouchInputHandleWrapper((HTOUCHINPUT)lParam);
        if (pInputs[0].dwID == 0) {
            return 0;
        }
    }
    //获取触摸点的坐标，并转换为窗口内的客户区坐标
    UiPoint pt = { TOUCH_COORD_TO_PIXEL(pInputs[0].x) , TOUCH_COORD_TO_PIXEL(pInputs[0].y) };
    ScreenToClient(pt);

    DWORD dwFlags = pInputs[0].dwFlags;
    delete[] pInputs;
    pInputs = nullptr;

    if (dwFlags & TOUCHEVENTF_DOWN) {
        OnButtonDown(kEventMouseButtonDown, 0, 0, pt);
    }
    else if (dwFlags & TOUCHEVENTF_MOVE) {
        UiPoint lastMousePos = m_ptLastMousePos;
        OnMouseMove(0, 0, pt);
        int detaValue = pt.y - lastMousePos.y;
        if (detaValue != 0) {
            //触发滚轮功能（lParam参数故意设置为0，有特殊含义）
            OnMouseWheel((WPARAM)detaValue, 0, pt);
        }
    }
    else if (dwFlags & TOUCHEVENTF_UP) {
        OnButtonUp(kEventMouseButtonUp, 0, 0, pt);
    }
    return 0;
}

LRESULT Window::OnPointerMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_POINTERDOWN ||
        uMsg == WM_POINTERUP ||
        uMsg == WM_POINTERUPDATE ||
        uMsg == WM_POINTERLEAVE ||
        uMsg == WM_POINTERCAPTURECHANGED ||
        uMsg == WM_POINTERWHEEL);

    bHandled = false;
    // 只关心第一个触摸点
    if (!IS_POINTER_PRIMARY_WPARAM(wParam)) {
        bHandled = true;
        return 0;
    }
    //获取指针位置，并且将屏幕坐标转换为窗口客户区坐标
    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    ScreenToClient(pt);
    UiPoint lastMousePos = m_ptLastMousePos;

    switch (uMsg)
    {
    case WM_POINTERDOWN:
        OnButtonDown(kEventMouseButtonDown, wParam, lParam, pt);
        bHandled = true;
        break;
    case WM_POINTERUPDATE:
        OnMouseMove(wParam, lParam, pt);
        bHandled = true;
        break;
    case WM_POINTERUP:
        OnButtonUp(kEventMouseButtonUp, wParam, lParam, pt);
        bHandled = true;
        break;
    case WM_POINTERWHEEL:
        OnMouseWheel(wParam, lParam, pt);
        bHandled = true;
        break;
    case WM_POINTERLEAVE:
    case WM_POINTERCAPTURECHANGED:
        m_ptLastMousePos = pt;
        if (uMsg != WM_POINTERLEAVE) {
            // Refer to LBUTTONUP and MOUSELEAVE，LBUTTOUP ReleaseCapture while MOUSELEAVE DONOT ReleaseCapture
            ReleaseCapture();
        }
        if (m_pEventClick) {
            //如果没有收到WM_POINTERUP消息，需要补一个（TODO：检查是否有副作用）
            m_pEventClick->SendEvent(kEventMouseButtonUp, wParam, lParam, 0, lastMousePos);
            m_pEventClick = nullptr;
        }
        //如果不设置bHandled，程序会转换为WM_BUTTON类消息
        bHandled = true;
        break;
    default:
        break;
    }
    return 0;
}

Control* Window::GetFocusControl() const
{
    return m_pFocus;
}

Control* Window::GetEventClick() const
{
    return m_pEventClick;
}

void Window::SetFocusControl(Control* pControl)
{
    // Paint manager window has focus?
    if (pControl != nullptr) {
        CheckSetWindowFocus();
    }
    // Already has focus?
    if (pControl == m_pFocus) {
        return;
    }
    // Remove focus from old control
    if (m_pFocus != nullptr) {
        //WPARAM 是新的焦点控件接口
        m_pFocus->SendEvent(kEventKillFocus, (WPARAM)pControl);
        m_pFocus = nullptr;
    }
    // Set focus to new control    
    if ((pControl != nullptr) && pControl->IsVisible() && pControl->IsEnabled()) {
        ASSERT(pControl->GetWindow() == this);
        ASSERT(::GetFocus() == m_hWnd);

        m_pFocus = pControl;
        m_pFocus->SendEvent(kEventSetFocus);
    }
}

void Window::KillFocusControl()
{
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventKillFocus);
        m_pFocus = nullptr;
    }
}

Window* Window::WindowFromPoint(const UiPoint& pt)
{
    WindowBase* pWindow = WindowBaseFromPoint(pt);
    if (pWindow != nullptr) {
        return dynamic_cast<Window*>(pWindow);
    }
    return nullptr;
}

Control* Window::GetHoverControl() const
{
    return m_pEventHover;
}

bool Window::SetNextTabControl(bool bForward)
{
    if (m_pRoot == nullptr) {
        return false;
    }
    // Find next/previous tabbable control
    FINDTABINFO info1 = { 0 };
    info1.pFocus = m_pFocus;
    info1.bForward = bForward;
    Control* pControl = m_pRoot->FindControl(ControlFinder::__FindControlFromTab, &info1, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
    if (pControl == nullptr) {
        if (bForward) {
            // Wrap around
            FINDTABINFO info2 = { 0 };
            info2.pFocus = bForward ? NULL : info1.pLast;
            info2.bForward = bForward;
            pControl = m_pRoot->FindControl(ControlFinder::__FindControlFromTab, &info2, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
        }
        else {
            pControl = info1.pLast;
        }
    }
    if (pControl != nullptr) {
        SetFocusControl(pControl);
    }
    return true;
}

Box* Window::GetRoot() const
{
    return m_pRoot;
}

void Window::SetArrange(bool bArrange)
{
    m_bIsArranged = bArrange;
}

void Window::AddDelayedCleanup(Control* pControl)
{
    if (pControl != nullptr) {
        pControl->SetWindow(this);
        m_aDelayedCleanup.push_back(pControl);
        ::PostMessage(m_hWnd, WM_APP + 1, 0, 0L);
    }
}

bool Window::SendNotify(EventType eventType, WPARAM wParam, LPARAM lParam)
{
    EventArgs msg;
    msg.pSender = nullptr;
    msg.Type = eventType;
    msg.ptMouse = GetLastMousePos();
    msg.dwTimestamp = ::GetTickCount();
    msg.wParam = wParam;
    msg.lParam = lParam;

    auto callback = m_OnEvent.find(msg.Type);
    if (callback != m_OnEvent.end()) {
        callback->second(msg);
    }

    callback = m_OnEvent.find(kEventAll);
    if (callback != m_OnEvent.end()) {
        callback->second(msg);
    }

    return true;
}

ui::IRender* Window::GetRender() const
{
    if ((m_render.get() != nullptr) && 
        ((m_render->GetWidth() <= 0) || (m_render->GetHeight() <= 0))) {
        //在估算控件大小的时候，需要Render有宽高等数据，所以需要进行Resize初始化
        UiRect rcClient;
        GetClientRect(rcClient);
        if ((rcClient.Width() > 0) && (rcClient.Height() > 0)) {
            m_render->Resize(rcClient.Width(), rcClient.Height());
        }
        else {
            m_render->Resize(1, 1);
        }
    }
    return m_render.get();
}

void Window::Paint()
{
    GlobalManager::Instance().AssertUIThread();

    if (IsWindowMinimized() || (m_pRoot == nullptr)) {
        PAINTSTRUCT ps = { 0 };
        ::BeginPaint(m_hWnd, &ps);
        ::EndPaint(m_hWnd, &ps);
        return;
    }

    if (m_render->GetWidth() == 0) {
        //在估算控件大小的时候，需要Render有宽高等数据，所以需要进行Resize初始化
        UiRect rcClient;
        GetClientRect(rcClient);
        if ((rcClient.Width() > 0) && (rcClient.Height() > 0)) {
            m_render->Resize(rcClient.Width(), rcClient.Height());
        }
    }
    bool bFirstLayout = m_bFirstLayout;
    if (m_bIsArranged && m_pRoot->IsArranged()) {
        //如果root配置的宽度和高度是auto类型的，自动调整窗口大小
        AutoResizeWindow(true);
    }

    //对控件进行布局
    ArrangeRoot();

    UiRect rcClient;
    GetClientRect(rcClient);
    UiRect rcWindow;
    GetWindowRect(rcWindow);
    if (rcClient.IsEmpty()) {
        return;
    }
    if (!m_render->Resize(rcClient.Width(), rcClient.Height())) {
        ASSERT(!"m_render->Resize resize failed!");
        return;
    }

    // Should we paint?
    RECT rectPaint = { 0, };
    if (!::GetUpdateRect(m_hWnd, &rectPaint, FALSE) && !bFirstLayout) {
        return;
    }
    UiRect rcPaint(rectPaint.left, rectPaint.top, rectPaint.right, rectPaint.bottom);

    //使用层窗口时，窗口部分在屏幕外时，获取到的无效区域仅仅是屏幕内的部分，这里做修正处理
    if (m_bIsLayeredWindow) {
        int32_t xScreen = GetSystemMetricsForDpiWrapper(SM_XVIRTUALSCREEN, Dpi().GetDPI());
        int32_t yScreen = GetSystemMetricsForDpiWrapper(SM_YVIRTUALSCREEN, Dpi().GetDPI());
        int32_t cxScreen = GetSystemMetricsForDpiWrapper(SM_CXVIRTUALSCREEN, Dpi().GetDPI());
        int32_t cyScreen = GetSystemMetricsForDpiWrapper(SM_CYVIRTUALSCREEN, Dpi().GetDPI());
        if (rcWindow.left < xScreen && rcWindow.left + rcPaint.left == xScreen) {
            rcPaint.left = rcClient.left;
        }
        if (rcWindow.top < yScreen && rcWindow.top + rcPaint.top == yScreen) {
            rcPaint.top = rcClient.top;
        }
        if (rcWindow.right > cxScreen && rcWindow.left + rcPaint.right == xScreen + cxScreen) {
            rcPaint.right = rcClient.right;
        }
        if (rcWindow.bottom > cyScreen && rcWindow.top + rcPaint.bottom == yScreen + cyScreen) {
            rcPaint.bottom = rcClient.bottom;
        }
    }

    PAINTSTRUCT ps = { 0 };
    ::BeginPaint(m_hWnd, &ps);

    // 去掉alpha通道
    if (m_bIsLayeredWindow) {
        m_render->ClearAlpha(rcPaint);
    }

    // 绘制    
    if (m_pRoot->IsVisible()) {
        AutoClip rectClip(m_render.get(), rcPaint, true);
        UiPoint ptOldWindOrg = m_render->OffsetWindowOrg(m_renderOffset);
        m_pRoot->Paint(m_render.get(), rcPaint);
        m_pRoot->PaintChild(m_render.get(), rcPaint);
        m_render->SetWindowOrg(ptOldWindOrg);
    }
    else {
        UiColor bkColor = UiColor(UiColors::LightGray);
        if (!m_pRoot->GetBkColor().empty()) {
            bkColor = m_pRoot->GetUiColor(m_pRoot->GetBkColor());
        }
        m_render->FillRect(rcPaint, bkColor);
    }

    // alpha修复
    if (m_bIsLayeredWindow) {
        if ((m_shadow != nullptr) && m_shadow->IsShadowAttached() && m_renderOffset.x == 0 && m_renderOffset.y == 0) {
            //补救由于Gdi绘制造成的alpha通道为0
            UiRect rcNewPaint = rcPaint;
            rcNewPaint.Intersect(m_pRoot->GetPosWithoutPadding());
            UiPadding rcRootPadding = m_pRoot->GetPadding();

            //考虑圆角
            rcRootPadding.left += 1;
            rcRootPadding.top += 1;
            rcRootPadding.right += 1;
            rcRootPadding.bottom += 1;

            m_render->RestoreAlpha(rcNewPaint, rcRootPadding);
        }
        else {
            UiRect rcAlphaFixCorner = GetAlphaFixCorner();
            if (rcAlphaFixCorner.left > 0 || rcAlphaFixCorner.top > 0 || rcAlphaFixCorner.right > 0 || rcAlphaFixCorner.bottom > 0)
            {
                UiRect rcNewPaint = rcPaint;
                UiRect rcRootPaddingPos = m_pRoot->GetPosWithoutPadding();
                rcRootPaddingPos.Deflate(rcAlphaFixCorner.left, rcAlphaFixCorner.top,
                    rcAlphaFixCorner.right, rcAlphaFixCorner.bottom);
                rcNewPaint.Intersect(rcRootPaddingPos);

                UiPadding rcRootPadding;
                m_render->RestoreAlpha(rcNewPaint, rcRootPadding);
            }
        }
    }

    // 渲染到窗口
    if (m_bIsLayeredWindow) {
        POINT pt = { rcWindow.left, rcWindow.top };
        SIZE szWindow = { rcClient.right - rcClient.left, rcClient.bottom - rcClient.top };
        POINT ptSrc = { 0, 0 };
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, static_cast<BYTE>(m_nWindowAlpha), AC_SRC_ALPHA };
        HDC hdc = m_render->GetDC();
        ::UpdateLayeredWindow(m_hWnd, NULL, &pt, &szWindow, hdc, &ptSrc, 0, &bf, ULW_ALPHA);
        m_render->ReleaseDC(hdc);
    }
    else {
        HDC hdc = m_render->GetDC();
        ::BitBlt(ps.hdc, rcPaint.left, rcPaint.top, rcPaint.Width(), rcPaint.Height(),
                 hdc, rcPaint.left, rcPaint.top, SRCCOPY);
        m_render->ReleaseDC(hdc);
    }

    ::EndPaint(m_hWnd, &ps);
}

void Window::AutoResizeWindow(bool bRepaint)
{
    if ((m_pRoot != nullptr) && (m_pRoot->GetFixedWidth().IsAuto() || m_pRoot->GetFixedHeight().IsAuto())) {
        UiSize maxSize(999999, 999999);
        UiEstSize estSize = m_pRoot->EstimateSize(maxSize);
        if (!estSize.cx.IsStretch() && !estSize.cy.IsStretch()) {
            UiSize needSize = MakeSize(estSize);
            if (needSize.cx < m_pRoot->GetMinWidth()) {
                needSize.cx = m_pRoot->GetMinWidth();
            }
            if (needSize.cx > m_pRoot->GetMaxWidth()) {
                needSize.cx = m_pRoot->GetMaxWidth();
            }
            if (needSize.cy < m_pRoot->GetMinHeight()) {
                needSize.cy = m_pRoot->GetMinHeight();
            }
            if (needSize.cy > m_pRoot->GetMaxHeight()) {
                needSize.cy = m_pRoot->GetMaxHeight();
            }
            UiRect rect;
            GetWindowRect(rect);
            if ((rect.Width() != needSize.cx) || (rect.Height() != needSize.cy)) {
                ::MoveWindow(m_hWnd, rect.left, rect.top, needSize.cx, needSize.cy, bRepaint ? TRUE : FALSE);
            }
        }
    }
}

void Window::ArrangeRoot()
{
    if (m_bIsArranged) {
        m_bIsArranged = false;
        UiRect rcClient;
        GetClientRect(rcClient);
        if (!rcClient.IsEmpty()) {
            if (m_pRoot->IsArranged()) {
                m_pRoot->SetPos(rcClient);
            }
            else {
                Control* pControl = m_pRoot->FindControl(ControlFinder::__FindControlFromUpdate, nullptr, UIFIND_VISIBLE | UIFIND_ME_FIRST);
                while (pControl != nullptr) {
                    pControl->SetPos(pControl->GetPos());
                    //ASSERT(!pControl->IsArranged());
                    pControl = m_pRoot->FindControl(ControlFinder::__FindControlFromUpdate, nullptr, UIFIND_VISIBLE | UIFIND_ME_FIRST);
                }
            }

            if (m_bFirstLayout) {
                m_bFirstLayout = false;
                OnInitLayout();
            }
        }
    }
}

bool Window::IsRenderTransparent() const
{
    return m_render->IsRenderTransparent();
}

bool Window::SetRenderTransparent(bool bCanvasTransparent)
{
    return m_render->SetRenderTransparent(bCanvasTransparent);
}

void Window::SetRenderOffset(UiPoint renderOffset)
{
    if (m_pRoot != nullptr) {
        m_renderOffset = renderOffset;
        Invalidate(m_pRoot->GetPos());
    }
}

void Window::SetRenderOffsetX(int renderOffsetX)
{
    if (m_pRoot != nullptr) {
        m_renderOffset.x = renderOffsetX;
        Invalidate(m_pRoot->GetPos());
    }
}

void Window::SetRenderOffsetY(int renderOffsetY)
{
    if (m_pRoot != nullptr) {
        m_renderOffset.y = renderOffsetY;
        Invalidate(m_pRoot->GetPos());
    }
}

void Window::OnInitLayout()
{
    if ((m_pRoot != nullptr) && ::IsWindowsVistaOrGreater()) {
        if (m_pRoot->IsVisible()) {
            m_pRoot->SetFadeVisible(true);
        }
    }
}

Control* Window::FindControl(const UiPoint& pt) const
{
    Control* pControl = m_controlFinder.FindControl(pt);
    if ((pControl != nullptr) && (pControl->GetWindow() != this)) {
        ASSERT(FALSE);
        pControl = nullptr;
    }
    return pControl;
}

Control* Window::FindContextMenuControl(const UiPoint* pt) const
{
    Control* pControl = m_controlFinder.FindContextMenuControl(pt);
    if ((pControl != nullptr) && (pControl->GetWindow() != this)) {
        ASSERT(FALSE);
        pControl = nullptr;
    }
    return pControl;
}

Box* Window::FindDroppableBox(const UiPoint& pt, uint8_t nDropInId) const
{
    Box* pControl = m_controlFinder.FindDroppableBox(pt, nDropInId);
    if ((pControl != nullptr) && (pControl->GetWindow() != this)) {
        ASSERT(FALSE);
        pControl = nullptr;
    }
    return pControl;
}

Control* Window::FindControl(const DString& strName) const
{
    return m_controlFinder.FindSubControlByName(m_pRoot, strName);
}

Control* Window::FindSubControlByPoint(Control* pParent, const UiPoint& pt) const
{
    return m_controlFinder.FindSubControlByPoint(pParent, pt);
}

Control* Window::FindSubControlByName(Control* pParent, const DString& strName) const
{
    return m_controlFinder.FindSubControlByName(pParent, strName);
}

} // namespace ui

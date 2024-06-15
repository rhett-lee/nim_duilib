#include "Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Shadow.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ToolTip.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Utils/PerformanceUtil.h"

//清理控件资源的自定义消息
#define WM_CLEANUP_MSG  (WM_USER + 1)

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
    m_rcAlphaFix(0, 0, 0, 0),
    m_bFirstLayout(true),
    m_bIsArranged(false),
    m_aDelayedCleanup(),
    m_mOptionGroup(),
    m_defaultAttrHash(),
    m_strResourcePath(),
    m_bPostQuitMsgWhenClosed(false)
{
    m_toolTip = std::make_unique<ToolTip>();
}

Window::~Window()
{
    ASSERT(!IsWindow());
    ClearWindow(false);
}

Window* Window::GetParentWindow() const
{
    WindowBase* pWindowBase = WindowBase::GetParentWindow();
    if (pWindowBase != nullptr) {
        return dynamic_cast<Window*>(pWindowBase);
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
        m_shadow->SetShadowAttached(IsLayeredWindow());
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

void Window::PreCloseWindow()
{
    ClearStatus();
}

void Window::PostCloseWindow()
{
    if (m_bPostQuitMsgWhenClosed) {
        this->PostQuitMsg(0);
    }
    OnCloseWindow();
}

void Window::FinalMessage()
{
    //按倒序清理资源
    ClearWindow(true);
    ClearWindowBase();

    //回调Final接口
    OnFinalMessage();
}

void Window::OnFinalMessage()
{
    delete this;
}

void Window::ClearWindow(bool bSendClose)
{
    if (bSendClose && IsWindow()) {
        //发送关闭事件
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        SendNotify(kEventWindowClose);
        if (windowFlag.expired()) {
            return;
        }
    }
    
    //回收控件
    GlobalManager::Instance().RemoveWindow(this);
    ReapObjects(GetRoot());

    if (m_pRoot != nullptr) {
        delete m_pRoot;
        m_pRoot = nullptr;
    }

    RemoveAllClass();
    RemoveAllOptionGroups();

    m_toolTip.reset();
    m_shadow.reset();
    m_render.reset();
    m_controlFinder.Clear();

    //删除清理的控件
    while (!m_aDelayedCleanup.empty()) {
        std::vector<Control*> aDelayedCleanup;
        aDelayedCleanup.swap(m_aDelayedCleanup);
        for (Control* pControl : aDelayedCleanup) {
            //移除过程中，还是会有删除控件向m_aDelayedCleanup里面添加
            delete pControl;
        }
        aDelayedCleanup.clear();
    }    
    ASSERT(m_aDelayedCleanup.empty());
}

bool Window::AttachBox(Box* pRoot)
{
    ASSERT(IsWindow());
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
        DString::value_type cEnd = m_strResourcePath.back();
        if (cEnd != _T('\\') && cEnd != _T('/')) {
#ifdef DUILIB_PLATFORM_WIN
            m_strResourcePath += _T('\\');
#else
            m_strResourcePath += _T('/');
#endif // DUILIB_PLATFORM_WIN
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

void Window::OnUseSystemCaptionBarChanged()
{
    if (IsUseSystemCaption()) {
        //关闭阴影
        SetShadowAttached(false);
    }
}

void Window::OnLayeredWindowChanged()
{
    //根据窗口是否为层窗口，重新初始化阴影附加属性值(层窗口为true，否则为false)
    if ((m_shadow != nullptr) && m_shadow->IsUseDefaultShadowAttached()) {
        m_shadow->SetShadowAttached(IsLayeredWindow());
        m_shadow->SetUseDefaultShadowAttached(true);
    }
    if (m_pRoot != nullptr) {
        Invalidate(m_pRoot->GetPos());
    }
}

void Window::OnWindowAlphaChanged()
{
    if (m_pRoot != nullptr) {
        Invalidate(m_pRoot->GetPos());
    }
}

void Window::OnWindowEnterFullScreen()
{
}

void Window::OnWindowExitFullScreen()
{
}

void Window::OnWindowDpiChanged(uint32_t /*nOldDPI*/, uint32_t /*nNewDPI*/)
{
}

void Window::GetShadowCorner(UiPadding& rcShadow) const
{
    rcShadow.Clear();
    ASSERT(m_shadow != nullptr);
    if (m_shadow != nullptr) {
        rcShadow = m_shadow->GetShadowCorner();
    }
}

bool Window::IsPtInCaptionBarControl(const UiPoint& pt) const
{
    Control* pControl = FindControl(pt);
    if (pControl) {
        if (pControl->CanPlaceCaptionBar()) {
            return true;
        }
    }
    return false;
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

    //更新布局和控件的DPI关联属性
    SetArrange(true);

    Box* pRoot = GetRoot();
    if (pRoot != nullptr) {
        pRoot->ChangeDpiScale(nOldDpiScale, nNewDpiScale);
        pRoot->Arrange();
        Invalidate(m_pRoot->GetPos());
    }
}

LRESULT Window::HandleUserMessage(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    LRESULT lResult = 0;
    bHandled = false;
    switch (uMsg)
    {
    case WM_CLEANUP_MSG:
        {
            for (Control* pControl : m_aDelayedCleanup) {
                delete pControl;
            }
            m_aDelayedCleanup.clear();
            bHandled = true;
        }
        break;
    default:
        break;
    }
    return lResult;
}

LRESULT Window::OnWindowMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT Window::OnSizeMsg(WindowSizeType sizeType, const UiSize& /*newWindowSize*/, bool& bHandled)
{
    bHandled = false;
    if (m_pRoot != nullptr) {
        m_pRoot->Arrange();
    }
    if (sizeType == WindowSizeType::kSIZE_MAXIMIZED) {
        if (m_shadow != nullptr) {
            m_shadow->MaximizedOrRestored(true);
        }
    }
    else if (sizeType == WindowSizeType::kSIZE_RESTORED) {
        if (m_shadow != nullptr) {
            m_shadow->MaximizedOrRestored(false);
        }
    }
    if (m_pFocus != nullptr) {
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        EventArgs msgData;
        msgData.eventData = (int32_t)sizeType;
        m_pFocus->SendEvent(kEventWindowSize, msgData);
        if (windowFlag.expired()) {
            return 0;
        }
    }
    return 0;
}

LRESULT Window::OnMoveMsg(const UiPoint& ptTopLeft, bool& bHandled)
{
    bHandled = false;
    if (m_pFocus != nullptr) {
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        EventArgs msgData;
        msgData.ptMouse = ptTopLeft;
        m_pFocus->SendEvent(kEventWindowMove, msgData);
        if (windowFlag.expired()) {
            return 0;
        }
    }
    return 0;
}

LRESULT Window::OnPaintMsg(bool& bHandled)
{
    bHandled = false;
    PerformanceStat statPerformance(_T("Window::OnPaintMsg"));
    if (Paint()) {
        bHandled = true;
    }
    return 0;
}

LRESULT Window::OnSetFocusMsg(WindowBase* /*pLostFocusWindow*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT Window::OnKillFocusMsg(WindowBase* /*pSetFocusWindow*/, bool& bHandled)
{
    bHandled = false;
    Control* pEventClick = m_pEventClick;
    m_pEventClick = nullptr;
    ReleaseCapture();

    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    if (pEventClick != nullptr) {
        pEventClick->SendEvent(kEventWindowKillFocus);
        if (windowFlag.expired()) {
            return 0;
        }
    }
    Control* pFocus = m_pFocus;
    if ((pFocus != nullptr) && (pFocus != pEventClick)) {
        pFocus->SendEvent(kEventWindowKillFocus);
        if (windowFlag.expired()) {
            return 0;
        }
    }
    return 0;
}

LRESULT Window::OnImeStartCompositionMsg(bool& bHandled)
{
    bHandled = false;
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventImeStartComposition);
    }
    return 0;
}

LRESULT Window::OnImeEndCompositionMsg(bool& bHandled)
{
    bHandled = false;
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventImeEndComposition);
    }
    return 0;
}

LRESULT Window::OnSetCursorMsg(bool& bHandled)
{
    bHandled = false;
    if (m_pEventClick != nullptr) {
        bHandled = true;
        return 0;
    }

    UiPoint pt;
    GetCursorPos(pt);
    ScreenToClient(pt);
    SetLastMousePos(pt);
    Control* pControl = FindControl(pt);
    if (pControl != nullptr) {
        //返回值待确认：如果应用程序处理了此消息，它应返回 TRUE 以停止进一步处理或 FALSE 以继续。
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        EventArgs msgData;
        msgData.ptMouse = pt;
        pControl->SendEvent(kEventSetCursor, msgData);
        bHandled = true;
        if (windowFlag.expired()) {
            return 0;
        }
    }
    return 0;
}

LRESULT Window::OnContextMenuMsg(const UiPoint& pt, bool& bHandled)
{
    bHandled = false;
    ReleaseCapture();

    if ((pt.x != -1) && (pt.y != -1)) {
        SetLastMousePos(pt);
        Control* pControl = FindContextMenuControl(&pt);
        if (pControl != nullptr) {
            Control* ptControl = FindControl(pt);//当前点击点所在的控件
            std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
            EventArgs msgData;
            msgData.ptMouse = pt;
            msgData.lParam = (LPARAM)ptControl;
            pControl->SendEvent(kEventContextMenu, msgData);
            if (windowFlag.expired()) {
                return 0;
            }
        }
    }
    else {
        //如果用户键入 SHIFT+F10，则上下文菜单为 -1, -1，
        //应用程序应在当前所选内容的位置（而不是 (xPos、yPos) ）显示上下文菜单。
        Control* pControl = FindContextMenuControl(nullptr);
        if (pControl != nullptr) {
            std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
            EventArgs msgData;
            msgData.ptMouse = pt;
            msgData.lParam = 0;
            pControl->SendEvent(kEventContextMenu, msgData);
            if (windowFlag.expired()) {
                return 0;
            }
        }
    }
    return 0;
}

LRESULT Window::OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled)
{
    bHandled = false;
    LRESULT lResult = 0;
    if (modifierKey & (ModifierKey::kShift | ModifierKey::kControl | ModifierKey::kAlt | ModifierKey::kWin)) {
        //含有组合按键
        m_pEventKey = m_pFocus;
        if (m_pEventKey != nullptr) {
            std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
            EventArgs msgData;
            msgData.vkCode = vkCode;
            msgData.modifierKey = modifierKey;
            m_pEventKey->SendEvent(kEventKeyDown, msgData);
            if (windowFlag.expired()) {
                return lResult;
            }
        }
        return lResult;
    }

    if ((vkCode == kVK_ESCAPE) && IsWindowFullScreen()) {
        //按ESC键时，退出全屏
        ExitFullScreen();
        return lResult;
    }
    if (m_pFocus != nullptr) {
        if (vkCode == kVK_TAB) {
            if (m_pFocus->IsVisible() &&
                m_pFocus->IsEnabled() &&
                m_pFocus->IsWantTab()) {
                return lResult;
            }
            else {
                //通过TAB键切换焦点控件
                SetNextTabControl(!Keyboard::IsKeyDown(kVK_SHIFT));
            }
        }
        else {
            m_pEventKey = m_pFocus;
            std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
            EventArgs msgData;
            msgData.vkCode = vkCode;
            msgData.modifierKey = modifierKey;
            m_pFocus->SendEvent(kEventKeyDown, msgData);
            if (windowFlag.expired()) {
                return lResult;
            }
        }
    }
    else {
        m_pEventKey = nullptr;
    }
    if ((vkCode == kVK_ESCAPE) && (m_pEventClick != nullptr)) {
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        m_pEventClick->SendEvent(kEventMouseClickEsc);
        if (windowFlag.expired()) {
            return lResult;
        }
    }
    return lResult;
}

LRESULT Window::OnKeyUpMsg(VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled)
{
    bHandled = false;
    LRESULT lResult = 0;
    if (m_pEventKey != nullptr) {
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        EventArgs msgData;
        msgData.vkCode = vkCode;
        msgData.modifierKey = modifierKey;
        m_pEventKey->SendEvent(kEventKeyUp, msgData);
        if (windowFlag.expired()) {
            return lResult;
        }
        m_pEventKey = nullptr;
    }
    return lResult;
}

LRESULT Window::OnCharMsg(VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled)
{
    bHandled = false;
    LRESULT lResult = 0;
    if (m_pEventKey != nullptr) {
        EventArgs msgData;
        msgData.vkCode = vkCode;
        msgData.modifierKey = modifierKey;
        m_pEventKey->SendEvent(kEventChar, msgData);
    }
    return lResult;
}

LRESULT Window::OnHotKeyMsg(int32_t /*hotkeyId*/, VirtualKeyCode /*vkCode*/, uint32_t /*modifierKey*/, bool& bHandled)
{
    //待添加（需确认，应该是要加在窗口上的）
    bHandled = false;
    return 0;
}

LRESULT Window::OnMouseWheelMsg(int32_t wheelDelta, const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    bHandled = false;
    LRESULT lResult = 0;
    SetLastMousePos(pt);
    Control* pControl = FindControl(pt);
    if (pControl != nullptr) {
        EventArgs msgData;
        msgData.eventData = wheelDelta;
        msgData.modifierKey = modifierKey;
        msgData.ptMouse = pt;
        pControl->SendEvent(kEventMouseWheel, msgData);
    }
    return lResult;
}

LRESULT Window::OnMouseMoveMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    bHandled = false;
    LRESULT lResult = 0;
    m_toolTip->SetMouseTracking(this, true);
    SetLastMousePos(pt);

    // Do not move the focus to the new control when the mouse is pressed
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    if (!IsCaptured()) {
        if (!HandleMouseEnterLeave(pt, modifierKey)) {
            return lResult;
        }
    }
    if (windowFlag.expired()) {
        return lResult;
    }

    EventArgs msgData;
    msgData.modifierKey = modifierKey;
    msgData.ptMouse = pt;
    if (m_pEventClick != nullptr) {        
        m_pEventClick->SendEvent(kEventMouseMove, msgData);
    }
    else if (m_pEventHover != nullptr) {
        m_pEventHover->SendEvent(kEventMouseMove, msgData);
    }
    return lResult;
}


bool Window::HandleMouseEnterLeave(const UiPoint& pt, uint32_t modifierKey)
{
    Control* pNewHover = FindControl(pt);
    //设置为新的Hover控件
    Control* pOldHover = m_pEventHover;
    m_pEventHover = pNewHover;
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();

    if ((pNewHover != pOldHover) && (pOldHover != nullptr)) {
        //Hover状态的控件发生变化，原来Hover控件的Tooltip应消失
        EventArgs msgData;
        msgData.modifierKey = modifierKey;
        msgData.ptMouse = pt;
        pOldHover->SendEvent(kEventMouseLeave, msgData);
        if (windowFlag.expired()) {
            return false;
        }
        m_toolTip->HideToolTip();
    }
    ASSERT(pNewHover == m_pEventHover);
    if (pNewHover != m_pEventHover) {
        return false;
    }

    if ((pNewHover != pOldHover) && (pNewHover != nullptr)) {
        EventArgs msgData;
        msgData.modifierKey = modifierKey;
        msgData.ptMouse = pt;
        pNewHover->SendEvent(kEventMouseEnter, msgData);
        if (windowFlag.expired()) {
            return false;
        }
    }
    return true;
}

LRESULT Window::OnMouseHoverMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    bHandled = false;
    LRESULT lResult = 0;
    m_toolTip->SetMouseTracking(this, false);
    if (IsCaptured()) {
        //如果处于Capture状态，不显示ToolTip
        return lResult;
    }
    Control* pHover = FindControl(pt);
    if (pHover == nullptr) {
        return lResult;
    }

    Control* pOldHover = GetHoverControl();
    if (pHover != nullptr) {
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        EventArgs msgData;
        msgData.modifierKey = modifierKey;
        msgData.ptMouse = pt;
        pHover->SendEvent(kEventMouseHover, msgData);
        if (windowFlag.expired()) {
            return lResult;
        }
    }

    if (pOldHover == GetHoverControl()) {
        //检查按需显示ToolTip信息    
        UiRect rect = pHover->GetPos();
        uint32_t maxWidth = pHover->GetToolTipWidth();
        DString toolTipText = pHover->GetToolTipText();
        m_toolTip->ShowToolTip(this, rect, maxWidth, pt, toolTipText);
    }
    return lResult;
}

LRESULT Window::OnMouseLeaveMsg(bool& bHandled)
{
    bHandled = false;
    m_toolTip->HideToolTip();
    m_toolTip->ClearMouseTracking();
    return 0;
}

LRESULT Window::OnMouseLButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    bHandled = false;
    OnButtonDown(kEventMouseButtonDown, pt, modifierKey);
    return 0;
}

LRESULT Window::OnMouseLButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    bHandled = false;
    OnButtonUp(kEventMouseButtonUp, pt, modifierKey);
    return 0;
}

LRESULT Window::OnMouseLButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    bHandled = false;
    OnButtonDown(kEventMouseDoubleClick, pt, modifierKey);
    return 0;
}

LRESULT Window::OnMouseRButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    bHandled = false;
    OnButtonDown(kEventMouseRButtonDown, pt, modifierKey);
    return 0;
}

LRESULT Window::OnMouseRButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    bHandled = false;
    OnButtonUp(kEventMouseRButtonUp, pt, modifierKey);
    return 0;
}

LRESULT  Window::OnMouseRButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    bHandled = false;
    OnButtonDown(kEventMouseRDoubleClick, pt, modifierKey);
    return 0;
}

LRESULT Window::OnCaptureChangedMsg(bool& bHandled)
{
    bHandled = false;

    //if (m_pEventClick) {
    //    //如果没有收到WM_POINTERUP消息，需要补一个（TODO：检查是否有副作用）
    //    m_pEventClick->SendEvent(kEventMouseButtonUp, wParam, lParam, 0, lastMousePos);
    //    m_pEventClick = nullptr;
    //}

    ReleaseCapture();
    return 0;
}

LRESULT Window::OnWindowCloseMsg(uint32_t /*wParam*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

void Window::OnButtonDown(EventType eventType, const UiPoint& pt, uint32_t modifierKey)
{
    ASSERT(eventType == kEventMouseButtonDown || 
           eventType == kEventMouseRButtonDown || 
           eventType == kEventMouseDoubleClick ||
           eventType == kEventMouseRDoubleClick);
    CheckSetWindowFocus();
    SetLastMousePos(pt);
    Control* pControl = FindControl(pt);
    if (pControl != nullptr) {
        Control* pOldEventClick = m_pEventClick;
        m_pEventClick = pControl;
        pControl->SetFocus();
        SetCapture();
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        EventArgs msgData;
        msgData.modifierKey = modifierKey;
        msgData.ptMouse = pt;
        pControl->SendEvent(eventType, msgData);
        if (windowFlag.expired()) {
            return;
        }
        if ((pOldEventClick != nullptr) && (pOldEventClick != pControl)) {
            pOldEventClick->SendEvent(kEventMouseClickChanged);
            if (windowFlag.expired()) {
                return;
            }
        }
    }
}

void Window::OnButtonUp(EventType eventType, const UiPoint& pt, uint32_t modifierKey)
{
    ASSERT(eventType == kEventMouseButtonUp || eventType == kEventMouseRButtonUp);
    SetLastMousePos(pt);
    ReleaseCapture();
    if (m_pEventClick != nullptr) {
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        EventArgs msgData;
        msgData.modifierKey = modifierKey;
        msgData.ptMouse = pt;
        m_pEventClick->SendEvent(eventType, msgData);
        if (windowFlag.expired()) {
            return;
        }
        m_pEventClick = nullptr;
    }
}

void Window::ClearStatus()
{
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    if (m_pEventHover != nullptr) {
        m_pEventHover->SendEvent(kEventMouseLeave);
        if (windowFlag.expired()) {
            return;
        }
        m_pEventHover = nullptr;
    }
    if (m_pEventClick != nullptr) {
        m_pEventClick->SendEvent(kEventMouseLeave);
        if (windowFlag.expired()) {
            return;
        }
        m_pEventClick = nullptr;
    }
    if (m_pEventKey != nullptr) {
        m_pEventKey->SendEvent(kEventMouseLeave);
        if (windowFlag.expired()) {
            return;
        }
        m_pEventKey = nullptr;
    }
    KillFocusControl();
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
    if (pControl != nullptr) {
        //确保窗口有焦点
        CheckSetWindowFocus();
    }
    if (pControl == m_pFocus) {
        return;
    }
    if (m_pFocus != nullptr) {
        //WPARAM 是新的焦点控件接口
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        m_pFocus->SendEvent(kEventKillFocus, (WPARAM)pControl);
        if (windowFlag.expired()) {
            return;
        }
        m_pFocus = nullptr;
    }
    if ((pControl != nullptr) && pControl->IsVisible() && pControl->IsEnabled()) {
        ASSERT(pControl->GetWindow() == this);
        ASSERT(IsWindowFocused());

        m_pFocus = pControl;

        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        m_pFocus->SendEvent(kEventSetFocus);
        if (windowFlag.expired()) {
            return;
        }
    }
}

void Window::KillFocusControl()
{
    if (m_pFocus != nullptr) {
        std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
        m_pFocus->SendEvent(kEventKillFocus);
        if (windowFlag.expired()) {
            return;
        }
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

void Window::UpdateToolTip()
{
    //隐藏现有的，等待重新显示即会更新
    m_toolTip->HideToolTip();
    m_toolTip->ClearMouseTracking();
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
    ASSERT(IsWindow());
    ASSERT(std::find(m_aDelayedCleanup.begin(), m_aDelayedCleanup.end(), pControl) == m_aDelayedCleanup.end());

    if (pControl != nullptr) {
        pControl->SetWindow(this);
        m_aDelayedCleanup.push_back(pControl);
        PostMsg(WM_CLEANUP_MSG);
    }
}

bool Window::SendNotify(EventType eventType, WPARAM wParam, LPARAM lParam)
{
    EventArgs msg;
    msg.SetSender(nullptr);
    msg.eventType = eventType;
    msg.ptMouse = GetLastMousePos();
    msg.wParam = wParam;
    msg.lParam = lParam;

    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    auto callback = m_OnEvent.find(msg.eventType);
    if (callback != m_OnEvent.end()) {
        callback->second(msg);
    }
    if (windowFlag.expired()) {
        return false;
    }

    callback = m_OnEvent.find(kEventAll);
    if (callback != m_OnEvent.end()) {
        callback->second(msg);
    }

    return true;
}

void Window::PostQuitMsgWhenClosed(bool bPostQuitMsg)
{
    m_bPostQuitMsgWhenClosed = bPostQuitMsg;
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

bool Window::Paint()
{
    GlobalManager::Instance().AssertUIThread();
    if (!IsWindow()) {
        return false;
    }
    if (IsWindowMinimized() || (m_pRoot == nullptr)) {
        return false;
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
    if (rcClient.IsEmpty()) {
        return false;
    }
    if (!m_render->Resize(rcClient.Width(), rcClient.Height())) {
        ASSERT(!"m_render->Resize resize failed!");
        return false;
    }

    UiRect updateRect;
    if (!GetUpdateRect(updateRect) && !bFirstLayout) {
        return false;
    }

    //开始绘制
    UiRect rcPaint;
    if (!BeginPaint(rcPaint)) {
        return false;
    }
   
    // 去掉alpha通道
    if (IsLayeredWindow()) {
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
    if (IsLayeredWindow()) {
        if ((m_shadow != nullptr) && m_shadow->IsShadowAttached() &&
            (m_renderOffset.x == 0) && (m_renderOffset.y == 0)) {
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
            if ((rcAlphaFixCorner.left > 0) || (rcAlphaFixCorner.top > 0) ||
                (rcAlphaFixCorner.right > 0) || (rcAlphaFixCorner.bottom > 0)) {
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

    //结束绘制，渲染到窗口
    return EndPaint(rcPaint, m_render.get());
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
                MoveWindow(rect.left, rect.top, needSize.cx, needSize.cy, bRepaint);
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
    if ((m_pRoot != nullptr) && m_pRoot->IsVisible()) {
        m_pRoot->SetFadeVisible(true);
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

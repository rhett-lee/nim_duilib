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

void Window::ClosingWindow()
{
    __super::ClosingWindow();
    ClearStatus();
}

void Window::OnInitWindow()
{
    __super::OnInitWindow();
}

void Window::OnCloseWindow()
{
    __super::OnCloseWindow(); 
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
    for (Control* pControl : m_aDelayedCleanup) {
        delete pControl;
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
        m_shadow->SetShadowAttached(IsLayeredWindow());
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

LRESULT Window::OnSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, bool& bHandled)
{
    LRESULT lResult = __super::OnSizeMsg(sizeType, newWindowSize, bHandled);
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
        m_pFocus->SendEvent(kEventWindowSize);
    }
    return lResult;
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

LRESULT Window::OnMoveMsg(const UiPoint& ptTopLeft, bool& bHandled)
{
    LRESULT lResult = __super::OnMoveMsg(ptTopLeft, bHandled);
    bHandled = false;
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventWindowMove);
    }
    return lResult;
}

LRESULT Window::OnPaintMsg(bool& bHandled)
{
    bHandled = true;
    PerformanceStat statPerformance(_T("Window::OnPaintMsg"));
    Paint();
    LRESULT lResult = __super::OnPaintMsg(bHandled);
    return lResult;
}

LRESULT Window::OnMouseHoverMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    LRESULT lResult = __super::OnMouseHoverMsg(pt, modifierKey, bHandled);
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
        pHover->SendEvent(kEventMouseHover, 0, 0, 0, pt);
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

LRESULT Window::OnMouseLeaveMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    LRESULT lResult = __super::OnMouseLeaveMsg(pt, modifierKey, bHandled);
    m_toolTip->HideToolTip();
    m_toolTip->ClearMouseTracking();
    return lResult;
}

void Window::UpdateToolTip()
{
    //隐藏现有的，等待重新显示即会更新
    m_toolTip->HideToolTip();
    m_toolTip->ClearMouseTracking();
}

LRESULT Window::OnMouseMoveMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    LRESULT lResult = __super::OnMouseMoveMsg(pt, modifierKey, bHandled);

    //TODO: 待实现
    int wParam = 0;
    int lParam = 0;
    OnMouseMove(wParam, lParam, pt);
    return lResult;
}

LRESULT Window::OnMouseWheelMsg(int32_t wheelDelta, const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    LRESULT lResult = __super::OnMouseWheelMsg(wheelDelta, pt, modifierKey, bHandled);
    //TODO: 待实现
    int wParam = 0;
    int lParam = 0;
    
    OnMouseWheel(wParam, lParam, pt);
    return lResult;
}

LRESULT Window::OnMouseLButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    LRESULT lResult = __super::OnMouseLButtonDownMsg(pt, modifierKey, bHandled);
    //TODO: 待实现
    int wParam = 0;
    int lParam = 0;
    OnButtonDown(kEventMouseButtonDown, wParam, lParam, pt);
    return lResult;
}

LRESULT Window::OnMouseRButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    LRESULT lResult = __super::OnMouseRButtonDownMsg(pt, modifierKey, bHandled);
    //TODO: 待实现
    int wParam = 0;
    int lParam = 0;
    OnButtonDown(kEventMouseRButtonDown, wParam, lParam, pt);
    return lResult;
}

LRESULT Window::OnMouseLButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    LRESULT lResult = __super::OnMouseLButtonDbClickMsg(pt, modifierKey, bHandled);
    //TODO: 待实现
    int wParam = 0;
    int lParam = 0;
    OnButtonDown(kEventMouseDoubleClick, wParam, lParam, pt);
    return lResult;
}

LRESULT  Window::OnMouseRButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    LRESULT lResult = __super::OnMouseRButtonDbClickMsg(pt, modifierKey, bHandled);
    //TODO: 待实现
    int wParam = 0;
    int lParam = 0;
    OnButtonDown(kEventMouseRDoubleClick, wParam, lParam, pt);
    return lResult;
}

LRESULT Window::OnMouseLButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    LRESULT lResult = __super::OnMouseLButtonUpMsg(pt, modifierKey, bHandled);
    //TODO: 待实现
    int wParam = 0;
    int lParam = 0;
    OnButtonUp(kEventMouseButtonUp, wParam, lParam, pt);
    return lResult;
}

LRESULT Window::OnMouseRButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled)
{
    LRESULT lResult = __super::OnMouseRButtonUpMsg(pt, modifierKey, bHandled);
    //TODO: 待实现
    int wParam = 0;
    int lParam = 0;
    OnButtonUp(kEventMouseRButtonUp, wParam, lParam, pt);
    return lResult;
}

LRESULT Window::OnCaptureChangedMsg(bool& bHandled)
{
    //if (uMsg != WM_POINTERLEAVE) {
    //    // Refer to LBUTTONUP and MOUSELEAVE，LBUTTOUP ReleaseCapture while MOUSELEAVE DONOT ReleaseCapture
    //    ReleaseCapture();
    //}
    //if (m_pEventClick) {
    //    //如果没有收到WM_POINTERUP消息，需要补一个（TODO：检查是否有副作用）
    //    m_pEventClick->SendEvent(kEventMouseButtonUp, wParam, lParam, 0, lastMousePos);
    //    m_pEventClick = nullptr;
    //}

    LRESULT lResult = __super::OnCaptureChangedMsg(bHandled);
    ReleaseCapture();
    return lResult;
}

LRESULT Window::OnWindowCloseMsg(uint32_t wParam, bool& bHandled)
{
    return __super::OnWindowCloseMsg(wParam, bHandled);
}

LRESULT Window::OnContextMenuMsg(const UiPoint& pt, bool& bHandled)
{
    LRESULT lResult = __super::OnContextMenuMsg(pt, bHandled);
    ReleaseCapture();

    if ((pt.x != -1) && (pt.y != -1)) {
        SetLastMousePos(pt);
        Control* pControl = FindContextMenuControl(&pt);
        if (pControl != nullptr) {
            Control* ptControl = FindControl(pt);//当前点击点所在的控件
            pControl->SendEvent(kEventMouseMenu, 0, (LPARAM)ptControl, 0, UiPoint(pt));
        }
    }
    else {
        //如果用户键入 SHIFT+F10，则上下文菜单为 -1, -1，
        //应用程序应在当前所选内容的位置（而不是 (xPos、yPos) ）显示上下文菜单。
        Control* pControl = FindContextMenuControl(nullptr);
        if (pControl != nullptr) {
            pControl->SendEvent(kEventMouseMenu, 0, 0, 0, UiPoint(pt));
        }
    }
    return lResult;
}

void Window::OnButtonDown(EventType eventType, WPARAM wParam, LPARAM lParam, const UiPoint& pt)
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
        pControl->SendEvent(eventType, wParam, lParam, 0, pt);
        if ((pOldEventClick != nullptr) && (pOldEventClick != pControl)) {
            pOldEventClick->SendEvent(kEventMouseClickChanged);
        }
    }
}

void Window::OnButtonUp(EventType eventType, WPARAM wParam, LPARAM lParam, const UiPoint& pt)
{
    ASSERT(eventType == kEventMouseButtonUp || eventType == kEventMouseRButtonUp);
    SetLastMousePos(pt);
    ReleaseCapture();
    if (m_pEventClick != nullptr) {
        m_pEventClick->SendEvent(eventType, wParam, lParam, 0, pt);
        m_pEventClick = nullptr;
    }
}

void Window::OnMouseMove(WPARAM wParam, LPARAM lParam, const UiPoint& pt)
{
    m_toolTip->SetMouseTracking(this, true);
    SetLastMousePos(pt);

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
    SetLastMousePos(pt);
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

LRESULT Window::OnImeStartCompositionMsg(bool& bHandled)
{
    LRESULT lResult = __super::OnImeStartCompositionMsg(bHandled);
    bHandled = false;
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventImeStartComposition);
    }
    return lResult;
}

LRESULT Window::OnImeEndCompositionMsg(bool& bHandled)
{
    LRESULT lResult = __super::OnImeEndCompositionMsg(bHandled);
    bHandled = false;
    if (m_pFocus != nullptr) {
        m_pFocus->SendEvent(kEventImeEndComposition);
    }
    return lResult;
}

LRESULT Window::OnSetFocusMsg(bool& bHandled)
{
    return __super::OnSetFocusMsg(bHandled);
}

LRESULT Window::OnKillFocusMsg(bool& bHandled)
{
    LRESULT lResult = __super::OnKillFocusMsg(bHandled);
    bHandled = false;
    Control* pEventClick = m_pEventClick;
    m_pEventClick = nullptr;
    ReleaseCapture();
    if (pEventClick != nullptr) {
        pEventClick->SendEvent(kEventWindowKillFocus);
    }
    Control* pFocus = m_pFocus;
    if ((pFocus != nullptr) && (pFocus != pEventClick)){
        pFocus->SendEvent(kEventWindowKillFocus);
    }
    return lResult;
}

LRESULT Window::OnCharMsg(VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled)
{
    LRESULT lResult = __super::OnCharMsg(vkCode, modifierKey, bHandled);
    //TODO: 待实现
    WPARAM wParam = 0;
    LPARAM lParam = 0;
    bHandled = false;
    if (m_pEventKey != nullptr) {
        m_pEventKey->SendEvent(kEventChar, wParam, lParam, static_cast<TCHAR>(wParam));
    }
    return lResult;
}

LRESULT Window::OnHotKeyMsg(int32_t hotkeyId, VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled)
{
    return __super::OnHotKeyMsg(hotkeyId, vkCode, modifierKey, bHandled);
}

LRESULT Window::OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled)
{
    LRESULT lResult = __super::OnKeyDownMsg(vkCode, modifierKey, bHandled);
    if (modifierKey & (ModifierKey::kShift | ModifierKey::kControl | ModifierKey::kAlt | ModifierKey::kWin)) {
        //TODO: 待实现(kEventSysKeyDown 已经删除 WM_SYSKEYDOWN)
        WPARAM wParam = 0;
        LPARAM lParam = 0;
        m_pEventKey = m_pFocus;
        if (m_pEventKey != nullptr) {
            m_pEventKey->SendEvent(kEventSysKeyDown, wParam, lParam, static_cast<TCHAR>(wParam));
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

            //TODO: 待实现(kEventSysKeyDown 已经删除 WM_SYSKEYDOWN)
            WPARAM wParam = 0;
            LPARAM lParam = 0;
            m_pFocus->SendEvent(kEventKeyDown, wParam, lParam, static_cast<TCHAR>(wParam));
        }
    }
    else {
        m_pEventKey = nullptr;
    }
    if ((vkCode == kVK_ESCAPE) && (m_pEventClick != nullptr)) {
        m_pEventClick->SendEvent(kEventMouseClickEsc);
    }
    return lResult;
}

LRESULT Window::OnKeyUpMsg(VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled)
{
    LRESULT lResult = __super::OnKeyUpMsg(vkCode, modifierKey, bHandled);
    //TODO: 待实现(kEventSysKeyUp 已经删除 WM_SYSKEYUP)
    WPARAM wParam = 0;
    LPARAM lParam = 0;
    bHandled = false;
    if (m_pEventKey != nullptr) {
        m_pEventKey->SendEvent(kEventKeyUp, wParam, lParam, static_cast<TCHAR>(wParam));
        m_pEventKey = nullptr;
    }
    return lResult;
}

LRESULT Window::OnSetCursorMsg(bool& bHandled)
{
    LRESULT lResult = __super::OnSetCursorMsg(bHandled);
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
        pControl->SendEvent(kEventSetCursor, 0, 0, 0, pt);
        bHandled = true;
    }
    return lResult;
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
        ASSERT(IsWindowFocused());

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
        PostMsg(WM_CLEANUP_MSG);
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
    HWND hWnd = GetHWND();
    if (hWnd == nullptr) {
        return;
    }
    if (IsWindowMinimized() || (m_pRoot == nullptr)) {
        PAINTSTRUCT ps = { 0 };
        ::BeginPaint(hWnd, &ps);
        ::EndPaint(hWnd, &ps);
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
    if (!::GetUpdateRect(hWnd, &rectPaint, FALSE) && !bFirstLayout) {
        return;
    }
    UiRect rcPaint(rectPaint.left, rectPaint.top, rectPaint.right, rectPaint.bottom);

    //使用层窗口时，窗口部分在屏幕外时，获取到的无效区域仅仅是屏幕内的部分，这里做修正处理
    if (IsLayeredWindow()) {
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
    ::BeginPaint(hWnd, &ps);

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
    if (IsLayeredWindow()) {
        POINT pt = { rcWindow.left, rcWindow.top };
        SIZE szWindow = { rcClient.right - rcClient.left, rcClient.bottom - rcClient.top };
        POINT ptSrc = { 0, 0 };
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, static_cast<BYTE>(GetWindowAlpha()), AC_SRC_ALPHA };
        HDC hdc = m_render->GetDC();
        ::UpdateLayeredWindow(hWnd, NULL, &pt, &szWindow, hdc, &ptSrc, 0, &bf, ULW_ALPHA);
        m_render->ReleaseDC(hdc);
    }
    else {
        HDC hdc = m_render->GetDC();
        ::BitBlt(ps.hdc, rcPaint.left, rcPaint.top, rcPaint.Width(), rcPaint.Height(),
                 hdc, rcPaint.left, rcPaint.top, SRCCOPY);
        m_render->ReleaseDC(hdc);
    }

    ::EndPaint(hWnd, &ps);
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

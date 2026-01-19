#include "ChildWindow.h"
#include "ChildWindowImpl.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui
{

ChildWindow::ChildWindow(Window* pWindow) :
    Box(pWindow)
{
    m_callbackID = (EventCallbackID)(Control*)this;
}

ChildWindow::~ChildWindow()
{
    if ((m_pChildWnd != nullptr) && m_pChildWnd->IsWindow()) {
        //同步关闭窗口
        m_pChildWnd->Close();
    }
    m_pChildWnd.reset();
}

DString ChildWindow::GetType() const { return DUI_CTR_CHILD_WINDOW; }

void ChildWindow::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("child_window_margin")) {
        UiMargin rcMargin;
        AttributeUtil::ParseMarginValue(strValue.c_str(), rcMargin);
        SetChildWindowMargin(rcMargin, true);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

UiMargin ChildWindow::GetChildWindowMargin() const
{
    return m_childWindowMargin;
}

void ChildWindow::SetChildWindowMargin(UiMargin rcMargin, bool bNeedDpiScale)
{
    ASSERT((rcMargin.left >= 0) && (rcMargin.top >= 0) && (rcMargin.right >= 0) && (rcMargin.bottom >= 0));
    rcMargin.Validate();
    if (bNeedDpiScale) {
        Dpi().ScaleMargin(rcMargin);
    }
    if (rcMargin.left < 0) {
        rcMargin.left = 0;
    }
    if (rcMargin.top < 0) {
        rcMargin.top = 0;
    }
    if (rcMargin.right < 0) {
        rcMargin.right = 0;
    }
    if (rcMargin.bottom < 0) {
        rcMargin.bottom = 0;
    }
    if (rcMargin != m_childWindowMargin) {
        m_childWindowMargin = rcMargin;
        ArrangeAncestor();
    }
}

void ChildWindow::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t /*nNewDpiScale*/)
{
    UiMargin rcMargin = GetChildWindowMargin();
    rcMargin = Dpi().GetScaleMargin(rcMargin, nOldDpiScale);
    SetChildWindowMargin(rcMargin, false);
}

void ChildWindow::SetWindow(Window* pWindow)
{
    Window* pOldWindow = GetWindow();
    BaseClass::SetWindow(pWindow);
    if (m_pChildWnd != nullptr) {
        m_pChildWnd->SetParentWindow(pWindow);
    }
    if (pOldWindow != pWindow) {
        UnregisterWindowCallbacks(pOldWindow);
        RegisterWindowCallbacks(pWindow);
    }
}

void ChildWindow::OnSetVisible(bool bChanged)
{
    BaseClass::OnSetVisible(bChanged);
    if (m_pChildWnd != nullptr) {
        m_pChildWnd->SetChildWindowVisible(IsVisible());
    }
}

void ChildWindow::OnSetEnabled(bool bChanged)
{
    BaseClass::OnSetEnabled(bChanged);
    if (m_pChildWnd != nullptr) {
        m_pChildWnd->SetChildWindowEnabled(IsEnabled());
    }
}

void ChildWindow::SetPos(UiRect rc)
{
    BaseClass::SetPos(rc);
    AdjustChildWindowPos();
}

void ChildWindow::AdjustChildWindowPos()
{
    if ((m_pChildWnd != nullptr) && m_pChildWnd->IsWindow()) {
        UiRect rc = GetRect();
        rc.Deflate(GetControlPadding());
        rc.Deflate(GetChildWindowMargin());
        rc.Validate();

        //窗口大小限制（部分平台不允许为0）
        if (rc.Width() < 1) {
            rc.right = rc.left + 1;
        }
        if (rc.Height() < 1) {
            rc.bottom = rc.top + 1;
        }
        UiPoint pt;
        pt.x = rc.left;
        pt.y = rc.top;
#if defined (DUILIB_BUILD_FOR_SDL) && !defined (DUILIB_BUILD_FOR_WIN)
        //SDL使用的是屏幕坐标，Windows SDK使用的是客户区坐标
        this->ClientToScreen(pt);
#endif
        m_pChildWnd->MoveWindow(pt.x, pt.y, rc.Width(), rc.Height(), true);
    }
}

bool ChildWindow::CreateChildWindow(ChildWindowEvents* pChildWindowEvents)
{
    ASSERT(m_pChildWnd == nullptr);
    if (m_pChildWnd != nullptr) {
        return false;
    }
    Window* pWindow = GetWindow();
    ASSERT((pWindow != nullptr) && pWindow->IsWindow());
    if ((pWindow == nullptr) || !pWindow->IsWindow()) {
        return false;
    }

    m_pChildWnd = std::make_unique<ChildWindowImpl>(pChildWindowEvents);

    //创建子窗口
    bool bRet = m_pChildWnd->CreateChildWnd(pWindow, 0, 0, 1, 1);
    if (!bRet) {
        return false;
    }
    m_pChildWnd->ShowWindow(ShowWindowCommands::kSW_SHOW);

    RegisterWindowCallbacks(pWindow);
    return true;
}

void ChildWindow::RegisterWindowCallbacks(Window* pWindow)
{
    ASSERT((pWindow != nullptr) && pWindow->IsWindow());
    if ((pWindow == nullptr) || !pWindow->IsWindow()) {
        return;
    }
    pWindow->AttachWindowMoveMsg([this, pWindow](const EventArgs&) {
        if (GetWindow() == pWindow) {
            AdjustChildWindowPos();
        }
        return true;
        }, m_callbackID);
    pWindow->AttachWindowPosChangedMsg([this, pWindow](const EventArgs&) {
        if (GetWindow() == pWindow) {
            AdjustChildWindowPos();
        }
        return true;
        }, m_callbackID);
}

void ChildWindow::UnregisterWindowCallbacks(Window* pWindow)
{
    ASSERT((pWindow != nullptr) && pWindow->IsWindow());
    if ((pWindow == nullptr) || !pWindow->IsWindow()) {
        return;
    }
    pWindow->DetachWindowEventCallbackByID(m_callbackID);
}

void ChildWindow::CloseChildWindow()
{
    if (m_pChildWnd != nullptr) {
        m_pChildWnd->Close();
    }
}

void ChildWindow::SetChildWindowEvents(ChildWindowEvents* pChildWindowEvents)
{
    ASSERT(m_pChildWnd != nullptr);
    if (m_pChildWnd == nullptr) {
        return;
    }
    m_pChildWnd->SetChildWindowEvents(pChildWindowEvents);
}

void ChildWindow::InvalidateChildWindowRect(const UiRect& rect)
{
    if (m_pChildWnd != nullptr) {
        m_pChildWnd->Invalidate(rect);
    }
}

void ChildWindow::InvalidateChildWindow()
{
    if (m_pChildWnd != nullptr) {
        UiRect rect;
        m_pChildWnd->GetClientRect(rect);
        m_pChildWnd->Invalidate(rect);
    }
}

void ChildWindow::UpdateChildWindow() const
{
    if (m_pChildWnd != nullptr) {
        m_pChildWnd->UpdateWindow();
    }
}

void ChildWindow::GetChildWindowRect(UiRect& rect) const
{
    rect = UiRect();
    if (m_pChildWnd != nullptr) {
        UiRect rc = GetRect();
        rc.Deflate(GetControlPadding());
        rc.Deflate(GetChildWindowMargin());
        m_pChildWnd->GetClientRect(rect);
        rect.left = rc.left;
        rect.top = rc.top;
    }
}

void ChildWindow::SetChildWindowLayered(bool bWindowLayered)
{
    if (m_pChildWnd != nullptr) {
        m_pChildWnd->SetLayeredWindow(bWindowLayered, true);
    }
}

}//namespace ui

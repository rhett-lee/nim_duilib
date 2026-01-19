#include "ChildWindowImpl.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"

namespace ui
{
ChildWindowImpl::ChildWindowImpl(ChildWindowEvents* pChildWindowEvents):
    m_pChildWindowEvents(pChildWindowEvents)
{
}

ChildWindowImpl::~ChildWindowImpl()
{
}

void ChildWindowImpl::SetChildWindowVisible(bool bVisible)
{
    if (IsWindowVisible() != bVisible) {
        ShowWindow(bVisible ? ShowWindowCommands::kSW_SHOW : ShowWindowCommands::kSW_HIDE);
    }
}

void ChildWindowImpl::SetChildWindowEnabled(bool bEnabled)
{
    if (IsWindowEnabled() != bEnabled) {
        EnableWindow(bEnabled);
    }
}

void ChildWindowImpl::SetChildWindowEvents(ChildWindowEvents* pChildWindowEvents)
{
    m_pChildWindowEvents = pChildWindowEvents;
}

void ChildWindowImpl::PreInitWindow()
{
    // 空实现
}

void ChildWindowImpl::OnInitWindow()
{
    // 空实现
}

void ChildWindowImpl::PostInitWindow()
{
    // 空实现
}

void ChildWindowImpl::OnInitLayout()
{
    // 空实现
}

void ChildWindowImpl::PreCloseWindow()
{
    // 空实现
}

void ChildWindowImpl::PostCloseWindow()
{
    // 空实现
}

void ChildWindowImpl::FinalMessage()
{
    // 空实现
}

void ChildWindowImpl::NotifyWindowEnterFullscreen()
{
    // 空实现
    ASSERT(0);
}

void ChildWindowImpl::NotifyWindowExitFullscreen()
{
    // 空实现
    ASSERT(0);
}

// ------------------------------ 窗口属性变更相关纯虚函数 ------------------------------
void ChildWindowImpl::OnUseSystemCaptionBarChanged()
{
    // 空实现
}

bool ChildWindowImpl::OnPreparePaint()
{
    // 空实现，默认返回true表示继续绘制
    return true;
}

void ChildWindowImpl::OnLayeredWindowChanged()
{
    // 空实现
}

void ChildWindowImpl::OnWindowAlphaChanged()
{
    // 空实现
}

void ChildWindowImpl::OnWindowEnterFullscreen()
{
    // 空实现
    ASSERT(0);
}

void ChildWindowImpl::OnWindowExitFullscreen()
{
    // 空实现
    ASSERT(0);
}

void ChildWindowImpl::OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnWindowDisplayScaleChanged(nOldScaleFactor, nNewScaleFactor);
    }
}

void ChildWindowImpl::GetShadowCorner(UiPadding& rcShadow) const
{
    // 空实现，清空输出参数
    rcShadow = UiPadding();
}

void ChildWindowImpl::GetCurrentShadowCorner(UiPadding& rcShadow) const
{
    // 空实现，清空输出参数
    rcShadow = UiPadding();
}

bool ChildWindowImpl::IsPtInCaptionBarControl(const UiPoint& /*pt*/) const
{
    // 空实现，默认返回false
    return false;
}

bool ChildWindowImpl::HasMinMaxBox(bool& bMinimizeBox, bool& bMaximizeBox) const
{
    // 空实现，默认设置为false
    bMinimizeBox = false;
    bMaximizeBox = false;
    return false;
}

bool ChildWindowImpl::IsPtInMaximizeRestoreButton(const UiPoint& /*pt*/) const
{
    // 空实现，默认返回false
    return false;
}

void ChildWindowImpl::GetCreateWindowAttributes(WindowCreateAttributes& createAttributes)
{
    // 空实现，清空输出参数
    createAttributes = WindowCreateAttributes();
}

// 非纯虚函数的实现
void ChildWindowImpl::OnDisplayScaleChanged(uint32_t /*nOldScaleFactor*/, uint32_t /*nNewScaleFactor*/)
{
    // 空实现
}

IRender* ChildWindowImpl::GetRender() const
{
    // 空实现，默认返回nullptr
    return nullptr;
}

Control* ChildWindowImpl::OnFindControl(const UiPoint& /*pt*/) const
{
    // 空实现
    return nullptr;
}

// ------------------------------ 窗口消息处理相关纯虚函数 ------------------------------
LRESULT ChildWindowImpl::OnWindowMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    // 空实现，默认标记为未处理，返回0
    bHandled = false;
    return 0;
}

void ChildWindowImpl::OnWindowCreateMsg(bool /*bDoModal*/, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        m_pChildWindowEvents->OnWindowCreateMsg(nativeMsg, bHandled);
    }
}

LRESULT ChildWindowImpl::OnWindowCloseMsg(uint32_t wParam, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnWindowCloseMsg(wParam, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnShowWindowMsg(bool bShow, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnShowWindowMsg(bShow, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnPaintMsg(const UiRect& rcPaint, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnPaintMsg(rcPaint, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnWindowPosChangedMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnWindowPosChangedMsg(nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnSizeMsg(sizeType, newWindowSize, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnMoveMsg(const UiPoint& ptTopLeft, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMoveMsg(ptTopLeft, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnSetFocusMsg(WindowBase* pLostFocusWindow, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnSetFocusMsg(pLostFocusWindow, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnKillFocusMsg(pSetFocusWindow, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnImeSetContextMsg(const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT ChildWindowImpl::OnImeStartCompositionMsg(const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT ChildWindowImpl::OnImeCompositionMsg(const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT ChildWindowImpl::OnImeEndCompositionMsg(const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT ChildWindowImpl::OnSetCursorMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnSetCursorMsg(nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnContextMenuMsg(const UiPoint& /*pt*/, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT ChildWindowImpl::OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnKeyUpMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnKeyUpMsg(vkCode, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnCharMsg(VirtualKeyCode /*vkCode*/, uint32_t /*modifierKey*/, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT ChildWindowImpl::OnHotKeyMsg(int32_t /*hotkeyId*/, VirtualKeyCode /*vkCode*/, uint32_t /*modifierKey*/, const NativeMsg& /*nativeMsg*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

LRESULT ChildWindowImpl::OnMouseWheelMsg(int32_t wheelDelta, const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMouseWheelMsg(wheelDelta, pt, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnMouseMoveMsg(const UiPoint& pt, uint32_t modifierKey, bool /*bFromNC*/, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMouseMoveMsg(pt, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnMouseHoverMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMouseHoverMsg(pt, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnMouseLeaveMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMouseLeaveMsg(nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnMouseLButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMouseLButtonDownMsg(pt, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnMouseLButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMouseLButtonUpMsg(pt, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnMouseLButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMouseLButtonDbClickMsg(pt, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnMouseRButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMouseRButtonDownMsg(pt, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnMouseRButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMouseRButtonUpMsg(pt, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnMouseRButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMouseRButtonDbClickMsg(pt, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnMouseMButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMouseMButtonDownMsg(pt, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnMouseMButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMouseMButtonUpMsg(pt, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnMouseMButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnMouseMButtonDbClickMsg(pt, modifierKey, nativeMsg, bHandled);
    }
    return 0;
}

LRESULT ChildWindowImpl::OnCaptureChangedMsg(const NativeMsg& nativeMsg, bool& bHandled)
{
    if (m_pChildWindowEvents != nullptr) {
        return m_pChildWindowEvents->OnCaptureChangedMsg(nativeMsg, bHandled);
    }
    return 0;
}

void ChildWindowImpl::OnWindowPosSnapped(bool /*bLeftSnap*/, bool /*bRightSnap*/, bool /*bTopSnap*/, bool /*bBottomSnap*/)
{
    // 空实现
}

}//namespace ui

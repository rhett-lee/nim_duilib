#include "ShadowWnd.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Shadow.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

namespace ui {

/**@brief 一个附加到基础窗口周围的窗口，带有 WS_EX_LAYERED 属性来实现阴影
    * @copyright (c) 2016, NetEase Inc. All rights reserved
    * @date 2019-03-22
    */
class ShadowWndBase : public ui::WindowImplBase, public ui::IUIMessageFilter
{
public:
    ShadowWndBase();

    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    virtual LRESULT FilterMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;
    bool Create(Window* window);
    bool UpdateShadowPos(bool bCheckZOrder);
private:
    Window* m_pWindow;
    //标记被跟随的窗口是否完成首次绘制
    bool m_isFirstPainted;
};

ShadowWndBase::ShadowWndBase():
    m_pWindow(nullptr),
    m_isFirstPainted(false)
{
}

DString ShadowWndBase::GetSkinFolder()
{
    return _T("");
}

DString ShadowWndBase::GetSkinFile()
{
    return _T("public/shadow/shadow.xml");
}

bool ShadowWndBase::Create(Window* window)
{
    m_pWindow = window;
    WindowCreateParam createParam;
    createParam.m_dwExStyle = kWS_EX_TRANSPARENT | kWS_EX_LAYERED | kWS_EX_TOOLWINDOW | kWS_EX_NOACTIVATE;
    createParam.m_className = _T("ShadowWnd");
    return Window::CreateWnd(nullptr, createParam);
}

LRESULT ShadowWndBase::FilterMessage(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
    bHandled = false;
    if ((m_pWindow == nullptr) || !m_pWindow->IsWindow()) {
        return 0;
    }
    switch (uMsg)
    {
        case WM_ERASEBKGND:        
        case WM_MOVE:
        case WM_MOVING:
        case WM_SIZE:
        case WM_SIZING:
        case WM_WINDOWPOSCHANGING:
        case WM_WINDOWPOSCHANGED:
        case WM_CAPTURECHANGED:
        case WM_ACTIVATE:
        case WM_NCACTIVATE:
            UpdateShadowPos(false);
            break;
        case WM_PAINT:
            if (!m_isFirstPainted) {
                if (UpdateShadowPos(false)) {
                    m_isFirstPainted = true;
                }
            }
            else {
                UpdateShadowPos(true);
            }
            break;
        case WM_CLOSE:
            ShowWindow(kSW_HIDE);
            //关联窗口关闭时，销毁阴影
            CloseWnd();
            break;
        case WM_SHOWWINDOW:
            if (wParam == 0) {
                ShowWindow(kSW_HIDE);
            }
            else {
                ShowWindow(kSW_SHOW_NA);
                UpdateShadowPos(false);
            }
            break;
        default:
            UpdateShadowPos(true);
            break;
    }
    return 0;
}

bool ShadowWndBase::UpdateShadowPos(bool bCheckZOrder)
{
    if ((m_pWindow == nullptr) || !m_pWindow->IsWindow()) {
        return false;
    }
    if (!m_pWindow->IsWindowVisible()) {
        ShowWindow(kSW_HIDE);
        return true;
    }
    if (bCheckZOrder) {
        if (::GetWindow(m_pWindow->NativeWnd()->GetHWND(), GW_HWNDNEXT) == NativeWnd()->GetHWND()) {
            return true;
        }
    }

    UiRect rc;
    m_pWindow->GetWindowRect(rc);
    UiPadding rcShadow;
    GetCurrentShadowCorner(rcShadow);
    rc.Inflate(rcShadow);
    uint32_t uFlags = kSWP_NOACTIVATE;
    if (!IsWindowVisible()) {
        uFlags |= kSWP_SHOWWINDOW;
    }
    SetWindowPos(InsertAfterWnd(m_pWindow), rc.left, rc.top, rc.Width(), rc.Height(), uFlags);
    return true;
}

ShadowWnd::ShadowWnd():
    m_pShadowWnd(nullptr)
{
}

ShadowWnd::~ShadowWnd()
{
}

Box* ShadowWnd::AttachShadow(Box* pRoot)
{
    if (pRoot == nullptr) {
        return nullptr;
    }
    ASSERT(m_pShadowWnd == nullptr);
    if (m_pShadowWnd != nullptr) {
        return BaseClass::AttachShadow(pRoot);
    }

    bool needCreateShadowWnd = NeedCreateShadowWnd();
    if (!needCreateShadowWnd) {
        //不满足附加透明窗口阴影条件，走原来逻辑
        return BaseClass::AttachShadow(pRoot);
    }
    else {
        //不附加默认的阴影，而是创建自己的透明窗口，作为阴影
        m_pShadowWnd = new ShadowWndBase;
        AddMessageFilter(m_pShadowWnd);

        //外置的阴影窗口需要将原窗口设置为圆角，避免圆角处出现黑色背景
        ASSERT(pRoot->GetWindow() == this);
        UiSize borderRound = GetShadowBorderRound();
        SetRoundCorner(borderRound.cx, borderRound.cy, true);
        pRoot->SetBorderRound(borderRound);
        InitShadow();
        return pRoot;
    }
}

bool ShadowWnd::NeedCreateShadowWnd() const
{
    if (IsLayeredWindow()) {
        //设置了层窗口属性，不创建
        return false;
    }
    if (IsUseDefaultShadowAttached()) {
        //配置文件中，或者外部接口中，未设置ShadowAttached属性，创建
        return !GetShadowImage().empty();
    }
    else {
        //配置文件中有设置，以配置文件中的属性为准
        return IsShadowAttached() && !GetShadowImage().empty();
    }    
}

void ShadowWnd::InitShadow()
{
    //关闭默认的窗口阴影
    if (IsShadowAttached()) {
        if (IsUseDefaultShadowAttached()) {
            SetShadowAttached(false);
            SetUseDefaultShadowAttached(true);
        }
        else {
            SetShadowAttached(false);
        }
    }

    //取消层窗口属性
    if (IsLayeredWindow()) {
        SetLayeredWindow(false, true);
    }

    //通过XML配置文件<Window>如下配置时，开启非透明窗口的阴影：
    //   layered_window = "false" shadow_attached = "true";
    if (m_pShadowWnd != nullptr) {
        m_pShadowWnd->Create(this);
        ASSERT(m_pShadowWnd->IsWindow());
        //阴影窗口不接受鼠标和键盘消息
        m_pShadowWnd->EnableWindow(false);
        m_pShadowWnd->SetShadowType(GetShadowType());
        if (IsWindowVisible()) {
            m_pShadowWnd->ShowWindow(kSW_SHOW_NA);
        }
    }
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN

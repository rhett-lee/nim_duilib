#include "ShadowWnd.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Shadow.h"

#ifdef DUILIB_BUILD_FOR_SDL
#include <SDL3/SDL.h>

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

    /** 获取关联的窗口ID
    */
    SDL_WindowID GetWindowID() const
    {
        if (m_pWindow != nullptr) {
            return SDL_GetWindowID((SDL_Window*)m_pWindow->GetWindowHandle());
        }
        return 0;
    }
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
//
//int SDLCALL ShadowWndBaseEventFilter(void* userdata, SDL_Event* event)
//{
//    ShadowWndBase* pShadowWndBase = (ShadowWndBase*)userdata;
//    if ((event != nullptr) && (pShadowWndBase != nullptr)) {
//        if (event->type == SDL_EVENT_WINDOW_MOVING) {
//            if (event->window.windowID == pShadowWndBase->GetWindowID()) {
//                bool bHandled = false;
//                pShadowWndBase->FilterMessage(SDL_EVENT_WINDOW_MOVING, 0, 0, bHandled);
//            }
//        }
//    }
//    return 0;
//}

bool ShadowWndBase::Create(Window* window)
{
    m_pWindow = window;
    WindowCreateParam createParam;
    createParam.m_dwStyle = kWS_POPUP;
    createParam.m_dwExStyle = kWS_EX_TRANSPARENT | kWS_EX_LAYERED | kWS_EX_NOACTIVATE | kWS_EX_TOOLWINDOW;
    createParam.m_className = _T("ShadowWnd");
    createParam.m_windowTitle = _T("ShadowWnd");

    //支持鼠标按住标题栏移动窗口时的阴影跟随效果
    //由于目前SDL的消息循环机制，在鼠标按住标题栏移动窗口时，窗口产生的消息对应的SDL事件是不会被应用层处理的。
    //SDL_AddEventWatch(ShadowWndBaseEventFilter, this);

    return Window::CreateWnd(nullptr, createParam);
}

LRESULT ShadowWndBase::FilterMessage(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    bHandled = false;
    if ((m_pWindow == nullptr) || !m_pWindow->IsWindow()) {
        return 0;
    }
    bool bAdjustWindowPos = false;
    if (uMsg == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        //窗口关闭
        ShowWindow(kSW_HIDE);
    }
    else if (uMsg == SDL_EVENT_WINDOW_SHOWN) {
        ShowWindow(kSW_SHOW_NA);
    }
    else if (uMsg == SDL_EVENT_WINDOW_HIDDEN) {
        ShowWindow(kSW_HIDE);
    }
    else if (uMsg == SDL_EVENT_WINDOW_MINIMIZED) {
        ShowWindow(kSW_HIDE);
    }
    else if ((uMsg == SDL_EVENT_WINDOW_EXPOSED) && !m_isFirstPainted) {
        bAdjustWindowPos = true;
    }
    else if ((uMsg >= SDL_EVENT_WINDOW_FIRST) && (uMsg <= SDL_EVENT_WINDOW_LAST)) {
        if (uMsg != SDL_EVENT_WINDOW_EXPOSED) {
            bAdjustWindowPos = true;
        }
    }
    if (bAdjustWindowPos) {
        if (m_pWindow->IsWindowVisible() && !m_pWindow->IsWindowMinimized()) {
            UiRect rc;
            m_pWindow->GetWindowRect(rc);
            UiPadding rcShadow;
            GetCurrentShadowCorner(rcShadow);
            rc.Inflate(rcShadow);
            //TODO: 由于SDL没有调整窗口Z-Order的功能，所以阴影窗口的位置不对，经常在其他窗口的后面，导致看不到阴影。
            SetWindowPos(InsertAfterWnd(m_pWindow), rc.left, rc.top, rc.Width(), rc.Height(), kSWP_SHOWWINDOW | kSWP_NOACTIVATE);
            if (uMsg == SDL_EVENT_WINDOW_EXPOSED) {
                m_isFirstPainted = true;
            }
        }
        else {
            ShowWindow(kSW_HIDE);
        }
    }
    return 0;
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
        return true;
    }
    else {
        //配置文件中有设置，以配置文件中的属性为准
        return IsShadowAttached();
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
        if (IsWindowVisible()) {
            m_pShadowWnd->ShowWindow(kSW_SHOW_NA);
        }
    }
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN

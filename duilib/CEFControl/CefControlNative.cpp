#include "CefControlNative.h"
#include "duilib/CEFControl/CefWindowUtils.h"
#include "duilib/CEFControl/internal/CefBrowserHandler.h"

#include "duilib/Core/Box.h"
#include "duilib/Core/GlobalManager.h"

namespace ui {

CefControlNative::CefControlNative(ui::Window* pWindow):
    CefControl(pWindow),
    m_bWindowFirstShown(false),
    m_bSetCefWindowParentNull(false)
{
}

CefControlNative::~CefControlNative(void)
{
    DoCloseAllNativeBrowsers(true);
    if (m_pBrowserHandler.get()) {
        m_pBrowserHandler->SetHostWindow(nullptr);
        m_pBrowserHandler->SetHandlerDelegate(nullptr);
    }
}

void CefControlNative::Init()
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pBrowserHandler.get() == nullptr) {
#ifdef DUILIB_BUILD_FOR_WIN
        //检测是否在分层窗口中创建控件
        HWND hWnd = GetWindow()->NativeWnd()->GetHWND();
        LONG style = ::GetWindowLong(hWnd, GWL_STYLE);
        ::SetWindowLong(hWnd, GWL_STYLE, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        ASSERT((::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_LAYERED) == 0 && _T("CefControlNative：无法在分层窗口内使用本控件"));
#endif
        m_pBrowserHandler = new CefBrowserHandler;
        m_pBrowserHandler->SetHostWindow(GetWindow());
        m_pBrowserHandler->SetHandlerDelegate(this);

        //异步创建Browser对象, 避免阻塞主界面的解析和显示速度
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefControlNative::ReCreateBrowser, this));
    }

    if (!m_jsBridge.get()) {
        m_jsBridge.reset(new ui::CefJSBridge);
    }
    BaseClass::Init();
}

void CefControlNative::ReCreateBrowser()
{
    GlobalManager::Instance().AssertUIThread();
    Window* pWindow = GetWindow();
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return;
    }
    ASSERT(pWindow->IsWindow());
    ASSERT(m_pBrowserHandler != nullptr);
    if (m_pBrowserHandler == nullptr) {
        return;
    }
    if (m_pBrowserHandler->GetBrowser() != nullptr) {
        //已经完成创建，不再重复创建Browser对象
        return;
    }

    //如果窗口没有完成首次显示，则等首次显示时再创建，避免出现闪黑屏现象
    if (!pWindow->IsWindowFirstShown()) {
        if (!m_bWindowFirstShown) {
            std::weak_ptr<WeakFlag> weakFlag = GetWeakFlag();
            pWindow->AttachWindowFirstShown([weakFlag, this](const EventArgs& /*args*/) {
                if (!weakFlag.expired()) {
                    ReCreateBrowser();
                }
                return true;
                });
            m_bWindowFirstShown = true;
        }
        return;
    }

    // 使用有窗模式
    CefWindowInfo window_info;
    //该参数必须显示初始化，不能使用默认值
#if CEF_VERSION_MAJOR > 109
    window_info.runtime_style = CEF_RUNTIME_STYLE_ALLOY;
#endif
    CefRect rect = { GetRect().left, GetRect().top, GetRect().right, GetRect().bottom};
#ifdef DUILIB_BUILD_FOR_WIN
    //Windows
    window_info.SetAsChild(pWindow->NativeWnd()->GetHWND(), rect);
#elif defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)
    //Linux
    window_info.SetAsChild(pWindow->NativeWnd()->GetX11WindowNumber(), rect);
#elif defined DUILIB_BUILD_FOR_MACOS
    //MacOS
    window_info.SetAsChild(pWindow->NativeWnd()->GetNSView(), rect);
#endif

    CefBrowserSettings browser_settings;
    CefString url = GetInitURL();//创建成功后，立即加载的URL
    CefBrowserHost::CreateBrowser(window_info, m_pBrowserHandler, url, browser_settings, nullptr, nullptr);
}

void CefControlNative::SetPos(ui::UiRect rc)
{
    GlobalManager::Instance().AssertUIThread();
    BaseClass::SetPos(rc);

    //更新页面子窗口的位置
    SetCefWindowPos(GetCefWindowHandle(), this);
}

bool CefControlNative::OnSetFocus(const EventArgs& msg)
{
    CefRefPtr<CefBrowserHost> browserHost = GetCefBrowserHost();
    if (browserHost != nullptr) {
        browserHost->SetFocus(true);
    }
    return BaseClass::OnSetFocus(msg);
}

bool CefControlNative::OnKillFocus(const EventArgs& msg)
{
    CefRefPtr<CefBrowserHost> browserHost = GetCefBrowserHost();
    if (browserHost != nullptr) {
        browserHost->SetFocus(false);
    }
    return BaseClass::OnKillFocus(msg);
}

void CefControlNative::OnSetVisible(bool bChanged)
{
    GlobalManager::Instance().AssertUIThread();
    BaseClass::OnSetVisible(bChanged);

    //更新页面子窗口的可见性
    SetCefWindowVisible(GetCefWindowHandle(), this);
}

void CefControlNative::UpdateCefWindowPos()
{
    if (GetCefWindowHandle() == 0) {
        //CEF页面的窗口还没完成创建
        return;
    }

    BaseClass::UpdateCefWindowPos();
    //在Browser控件创建完成后，更新窗口位置和可见性
    SetPos(GetPos());
    SetVisible(IsVisible());
}

void CefControlNative::DoCloseAllNativeBrowsers(bool bForceClose)
{
    //解除CEF子窗口与父窗口的父子关系（避免其关闭的时候，同时也关闭父窗口，导致程序退出）
    if (!m_bSetCefWindowParentNull) {
        m_bSetCefWindowParentNull = true;
        RemoveCefWindowFromParent(GetCefWindowHandle());
    }
    DoCloseAllBrowsers(bForceClose);
}

void CefControlNative::CloseAllBrowsers()
{
    DoCloseAllNativeBrowsers(true);
}

void CefControlNative::SetWindow(ui::Window* pWindow)
{
    GlobalManager::Instance().AssertUIThread();
    if ((pWindow == nullptr) || (BaseClass::GetWindow() == pWindow)) {
        return;
    }
    BaseClass::SetWindow(pWindow);

    if (m_pBrowserHandler) {
        m_pBrowserHandler->SetHostWindow(pWindow);
        m_pBrowserHandler->SetHandlerDelegate(this);
    }

    //更新页面子窗口的父窗口
    SetCefWindowParent(GetCefWindowHandle(), this);
}

std::shared_ptr<IBitmap> CefControlNative::MakeImageSnapshot()
{
    std::vector<uint8_t> bitmap;
    int32_t width = 0;
    int32_t height = 0;
    bool bRet = CaptureCefWindowBitmap(GetCefWindowHandle(), bitmap, width, height);
    if (bRet && (width > 0) && (height > 0) && ((int32_t)bitmap.size() == (width * height * 4))) {
        std::shared_ptr<IBitmap> spBitmap;
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactory != nullptr);
        if (pRenderFactory != nullptr) {
            spBitmap.reset(pRenderFactory->CreateBitmap());
            if (spBitmap != nullptr) {
                if (!spBitmap->Init(width, height, true, bitmap.data())) {
                    spBitmap.reset();
                }
            }
        }
        return spBitmap;
    }
    return nullptr;
}

bool CefControlNative::IsCefNative() const
{
    return true;
}

void CefControlNative::OnGotFocus()
{
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        //页面获取焦点时，禁止主界面输入文字（解决的问题：macOS下：在页面输入文字，按键一次，会触发多次输入，应该是SDL内部又触发了输入）
        pWindow->NativeWnd()->SetImeOpenStatus(false);
        pWindow->NativeWnd()->SetTextInputArea(nullptr, 0);
    }

    BaseClass::OnGotFocus();
}

} //namespace ui

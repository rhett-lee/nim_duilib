#include "CefControlNative.h"
#include "duilib/CEFControl/CefWindowUtils.h"
#include "duilib/CEFControl/internal/CefBrowserHandler.h"

#include "duilib/Core/Box.h"
#include "duilib/Core/GlobalManager.h"

#ifdef DUILIB_BUILD_FOR_LINUX
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
#endif

namespace ui {

CefControlNative::CefControlNative(ui::Window* pWindow):
    CefControl(pWindow),
    m_bWindowFirstShown(false)
{
}

CefControlNative::~CefControlNative(void)
{
    CefControlNative::CloseAllBrowsers();
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        m_pBrowserHandler->SetHostWindow(nullptr);
        m_pBrowserHandler->SetHandlerDelegate(nullptr);
        // Request that the main browser close.
        if (m_pBrowserHandler->GetBrowserHost() != nullptr) {
            m_pBrowserHandler->GetBrowserHost()->CloseBrowser(true);
        }        
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

void CefControlNative::SetVisible(bool bVisible)
{
    GlobalManager::Instance().AssertUIThread();
    BaseClass::SetVisible(bVisible);

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

void CefControlNative::CloseAllBrowsers()
{
#ifdef DUILIB_BUILD_FOR_WIN
    //关闭窗口时，取消父子关系，避免导致退出时的崩溃问题
    HWND hWnd = GetCefWindowHandle();
    if (::IsWindow(hWnd)) {
        ::SetParent(hWnd, nullptr);
    }
#endif
    BaseClass::CloseAllBrowsers();
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

void CefControlNative::OnFocusedNodeChanged(CefRefPtr<CefBrowser> /*browser*/,
                                            CefRefPtr<CefFrame> /*frame*/,
                                            CefDOMNode::Type /*type*/,
                                            bool /*bText*/,
                                            bool /*bEditable*/,
                                            const CefRect& /*nodeRect*/)
{
    if (!IsVisible() || !IsEnabled()) {
        return;
    }
    if (!IsFocused()) {
        //避免双焦点控件的出现
        SetFocus();
    }
}

std::shared_ptr<IBitmap> CefControlNative::MakeImageSnapshot()
{
    std::vector<uint8_t> bitmap;
    int32_t width = 0;
    int32_t height = 0;
    bool bRet = false;
#ifdef DUILIB_BUILD_FOR_WIN
    bRet = CaptureWindowBitmap_Win32(bitmap, width, height);
#elif defined (DUILIB_BUILD_FOR_MACOS)
    bRet = CaptureWindowBitmap_Mac(bitmap, width, height);
#elif defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)
    bRet = CaptureWindowBitmap_X11(bitmap, width, height);
#endif
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

#if defined (DUILIB_BUILD_FOR_WIN)
bool CefControlNative::CaptureWindowBitmap_Win32(std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height)
{
    HWND hwnd = GetCefWindowHandle();
    if (!::IsWindow(hwnd)) {
        return false;
    }
    // 获取窗口尺寸
    RECT rect = { 0, 0, 0, 0 };
    if (!GetClientRect(hwnd, &rect)) {
        return false;
    }

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    if (width <= 0 || height <= 0) {
        return false;
    }

    // 创建设备上下文
    HDC hdcScreen = ::GetDC(nullptr);
    if (hdcScreen == nullptr) {
        return false;
    }
    HDC hdcWindow = ::GetDC(hwnd);
    if (hdcWindow == nullptr) {
        ::ReleaseDC(nullptr, hdcScreen);
        return false;
    }

    HDC hdcMemDC = ::CreateCompatibleDC(hdcWindow);
    if (hdcMemDC == nullptr) {
        ::ReleaseDC(nullptr, hdcScreen);
        ::ReleaseDC(hwnd, hdcWindow);
        return false;
    }

    // 创建位图
    HBITMAP hBitmap = ::CreateCompatibleBitmap(hdcWindow, width, height);
    if (hBitmap == nullptr) {
        ::DeleteDC(hdcMemDC);
        ::ReleaseDC(nullptr, hdcScreen);
        ::ReleaseDC(hwnd, hdcWindow);
        return false;
    }

    HGDIOBJ hOldObj = ::SelectObject(hdcMemDC, hBitmap);

    // 拷贝屏幕内容到位图
    ::BitBlt(hdcMemDC, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);

    // 获取位图信息
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  // 正数表示从下到上，负数表示从上到下
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // 分配内存并获取位图数据
    bitmap.resize(width * height * 4);
    ::GetDIBits(hdcMemDC, hBitmap, 0, height, bitmap.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // 清理资源
    ::SelectObject(hdcMemDC, hOldObj);
    ::DeleteObject(hBitmap);
    ::DeleteDC(hdcMemDC);
    ::ReleaseDC(nullptr, hdcScreen);
    ::ReleaseDC(hwnd, hdcWindow);

    return true;
}
#elif defined (DUILIB_BUILD_FOR_MACOS)
bool CefControlNative::CaptureWindowBitmap_Mac(std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height)
{
    return false;
}
#elif defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)
bool CefControlNative::CaptureWindowBitmap_X11(std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height)
{
    // 检查X11环境更加健壮
    const char* sessionType = std::getenv("XDG_SESSION_TYPE");
    if (!sessionType || (std::string(sessionType) != "x11" && std::string(sessionType) != "X11")) {
        // 尝试使用DISPLAY环境变量进行二次检查
        const char* displayEnv = std::getenv("DISPLAY");
        if (!displayEnv || !*displayEnv) {
            return false;
        }
    }

    Display* display = ::XOpenDisplay(nullptr);
    if (!display) {
        return false;
    }

    // RAII资源管理
    struct DisplayCloser {
        Display* d;
        ~DisplayCloser() { if(d) ::XCloseDisplay(d); }
    } closer{display};

    ::Window x11Window = GetCefWindowHandle();

    // 获取窗口尺寸
    ::XWindowAttributes gwa;
    if (!::XGetWindowAttributes(display, x11Window, &gwa)) {
        return false;
    }
    width = gwa.width;
    height = gwa.height;
    if (width <= 0 || height <= 0) {
        return false;
    }

    // 获取窗口内容
    XImage* image = XGetImage(display, x11Window, 0, 0, width, height, AllPlanes, ZPixmap);
    if (!image) {
        return false;
    }

    // RAII管理XImage资源
    struct ImageDestroyer {
        XImage* img;
        ~ImageDestroyer() { if(img) XDestroyImage(img); }
    } imgDestroyer{image};

    // 分配内存并复制像素数据
    bitmap.resize(width * height * 4);

    // 使用更安全的像素格式转换
    bool isRgbOrder = (image->red_mask == 0xFF0000);
    bool isBgrOrder = (image->blue_mask == 0xFF0000);
    
    // 使用XGetPixel作为安全的像素获取方式
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned long pixel = XGetPixel(image, x, y);
            unsigned char r, g, b;
            
            if (isRgbOrder) {
                r = (pixel >> 16) & 0xFF;
                g = (pixel >> 8) & 0xFF;
                b = pixel & 0xFF;
            } else if (isBgrOrder) {
                r = pixel & 0xFF;
                g = (pixel >> 8) & 0xFF;
                b = (pixel >> 16) & 0xFF;
            } else {
                // 无法确定顺序，使用灰度
                r = g = b = (pixel * 255) / ((1 << image->bits_per_pixel) - 1);
            }
            
            int index = (y * width + x) * 4;
            bitmap[index] = r;
            bitmap[index + 1] = g;
            bitmap[index + 2] = b;
            bitmap[index + 3] = 255;
        }
    }

    return true;
}
#endif

}

#include "CefWindowUtils.h"
#include "duilib/Core/Window.h"

#if defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)
//Linux/FreeBSD OS

#include "include/cef_task.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace ui
{
//判断窗口是否有效
static bool IsX11WindowValid(Display* display, ::Window window)
{
    // 尝试获取窗口属性
    XWindowAttributes attrs;        
    Status status = XGetWindowAttributes(display, window, &attrs);
    return (status != 0);  // 返回1有效，0无效
}

//设置X窗口的父窗口
class SetX11WindowParentWindowTask : public CefTask
{
    IMPLEMENT_REFCOUNTING(SetX11WindowParentWindowTask);
public:
    explicit SetX11WindowParentWindowTask(CefControl* pCefControl):
        m_pCefControl(pCefControl)
    {
        m_pCefControlFlag = pCefControl->GetWeakFlag();
    }
public:
    virtual void Execute() override
    {
        if (m_pCefControlFlag.expired()) {
            return;
        }
        CefWindowHandle hParentHandle = 0;
        Window* pWindow = m_pCefControl->GetWindow();
        if (pWindow != nullptr) {
            hParentHandle = pWindow->NativeWnd()->GetX11WindowNumber();
        }
        CefWindowHandle handle = m_pCefControl->GetCefWindowHandle();
        if ((handle != 0) && (hParentHandle != 0)) {
            Display* display = XOpenDisplay(nullptr);
            // RAII资源管理
            struct DisplayCloser {
                Display* d;
                ~DisplayCloser() { if (d) ::XCloseDisplay(d); }
            } closer{ display };
        
            if ((display != nullptr) && IsX11WindowValid(display, handle) && IsX11WindowValid(display, hParentHandle)) {
                UiRect rc = m_pCefControl->GetPos();
                XReparentWindow(display, handle, hParentHandle, rc.left, rc.top);
                XFlush(display);
            }
        }
    }
private:
    CefControl* m_pCefControl;
    std::weak_ptr<WeakFlag> m_pCefControlFlag;
};

//设置X11窗口的显示或者隐藏
class SetX11WindowVisibleTask : public CefTask
{
    IMPLEMENT_REFCOUNTING(SetX11WindowVisibleTask);
public:
    explicit SetX11WindowVisibleTask(CefControl* pCefControl):
        m_pCefControl(pCefControl)
    {
        m_pCefControlFlag = pCefControl->GetWeakFlag();
    }
public:
    virtual void Execute() override
    {
        if (m_pCefControlFlag.expired()) {
            return;
        }
        CefWindowHandle handle = m_pCefControl->GetCefWindowHandle();
        if (handle != 0) {
            Display* display = XOpenDisplay(nullptr);
            // RAII资源管理
            struct DisplayCloser {
                Display* d;
                ~DisplayCloser() { if (d) ::XCloseDisplay(d); }
            } closer{ display };
            
            if ((display != nullptr) && IsX11WindowValid(display, handle)){
                if (m_pCefControl->IsVisible()) {
                    XMapWindow(display, handle);
                }
                else {
                    XUnmapWindow(display, handle);
                }
                XFlush(display);
            }
        }
    }
private:
    CefControl* m_pCefControl;
    std::weak_ptr<WeakFlag> m_pCefControlFlag;
};

//设置X11窗口的位置和大小
class SetX11WindowPosTask : public CefTask
{
    IMPLEMENT_REFCOUNTING(SetX11WindowPosTask);
public:
    explicit SetX11WindowPosTask(CefControl* pCefControl):
        m_pCefControl(pCefControl)
    {
        m_pCefControlFlag = pCefControl->GetWeakFlag();
    }
public:
    virtual void Execute() override
    {
        if (m_pCefControlFlag.expired() || !m_pCefControl->IsVisible()) {
            //窗口隐藏的时候，不需要设置；如果设置的话，会导致程序崩溃
            return;
        }
        CefWindowHandle handle = m_pCefControl->GetCefWindowHandle();
        ui::UiRect rc = m_pCefControl->GetPos();
        if (handle != 0) {
            Display* display = XOpenDisplay(nullptr);
            // RAII资源管理
            struct DisplayCloser {
                Display* d;
                ~DisplayCloser() { if (d) ::XCloseDisplay(d); }
            } closer{ display };

            if ((display != nullptr) && IsX11WindowValid(display, handle)){
                XMoveResizeWindow(display, handle, rc.left, rc.top, rc.Width(), rc.Height());
                XFlush(display);
            }
        }
    }
private:
    CefControl* m_pCefControl;
    std::weak_ptr<WeakFlag> m_pCefControlFlag;
};

void SetCefWindowPos(CefWindowHandle cefWindow, CefControl* pCefControl)
{
    if ((cefWindow == 0) || (pCefControl == nullptr)) {
        return;
    }
    Window* pWindow = pCefControl->GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    CefPostTask(TID_UI, new SetX11WindowPosTask(pCefControl));
}

void SetCefWindowVisible(CefWindowHandle cefWindow, CefControl* pCefControl)
{
    if ((cefWindow == 0) || (pCefControl == nullptr)) {
        return;
    }
    Window* pWindow = pCefControl->GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    //需要在CEF的UI线程调整
    CefPostTask(TID_UI, new SetX11WindowVisibleTask(pCefControl));
}

void SetCefWindowParent(CefWindowHandle cefWindow, CefControl* pCefControl)
{
    if ((cefWindow == 0) || (pCefControl == nullptr)) {
        return;
    }
    Window* pWindow = pCefControl->GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    //需要在CEF的UI线程调整
    CefPostTask(TID_UI, new SetX11WindowParentWindowTask(pCefControl));
}

bool CaptureCefWindowBitmap(CefWindowHandle cefWindow, std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height)
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
        ~DisplayCloser() { if (d) ::XCloseDisplay(d); }
    } closer{ display };

    ::Window x11Window = cefWindow;

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
        ~ImageDestroyer() { if (img) XDestroyImage(img); }
    } imgDestroyer{ image };

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
            }
            else if (isBgrOrder) {
                r = pixel & 0xFF;
                g = (pixel >> 8) & 0xFF;
                b = (pixel >> 16) & 0xFF;
            }
            else {
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

void SetCefWindowCursor(CefWindowHandle cefWindow, CefCursorHandle cursor)
{
    if ((cefWindow == 0) || (cursor == 0)) {
        return;
    }
    Display* display = ::XOpenDisplay(nullptr);
    if (display != nullptr) {
        // RAII资源管理
        struct DisplayCloser {
            Display* d;
            ~DisplayCloser() { if (d) ::XCloseDisplay(d); }
        } closer{ display };

        ::Window x11Window = cefWindow;
        XDefineCursor(display, x11Window, cursor);
    }
}

void RemoveCefWindowFromParent(CefWindowHandle cefWindow)
{
    if (cefWindow == 0) {
        return;
    }
    //不需要实现，对业务无影响
}

} //namespace ui

#endif //defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)

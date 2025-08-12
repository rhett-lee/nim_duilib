#include "ScreenCapture.h"
#include "duilib/Core/GlobalManager.h"

#if defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)
//Linux/FreeBSD OS

#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace ui
{
// 捕获指定窗口所在屏幕的图像
// 参数:
//   display      - 与X服务器的连接
//   targetWindow - 目标窗口的X11窗口ID
//   bitmap       - 输出的RGB图像数据
//   width        - 输出图像的宽度
//   height       - 输出图像的高度
// 返回值:
//   成功返回true，失败返回false
static bool CaptureScreenBitmap(Display* display, ::Window targetWindow, std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height)
{
    bitmap.clear();
    height = 0;
    height = 0;
    if (display == nullptr) {
        return false;
    }
    // 获取目标窗口所在的屏幕
    XWindowAttributes attr;
    if (!XGetWindowAttributes(display, targetWindow, &attr)) {
        return false;
    }
    Screen* screen = attr.screen;
    if (!screen) {
        return false;
    }

    // 获取屏幕的根窗口
    ::Window rootWindow = RootWindowOfScreen(screen);  // 从屏幕对象获取根窗口
    width = screen->width;       // 屏幕宽度（直接从Screen结构体获取）
    height = screen->height;     // 屏幕高度
    if ((width < 1) || (height < 1)) {
        return false;
    }
    
    // 捕获屏幕内容到XImage（请求包含Alpha通道的格式）
    XImage* ximage = XGetImage(
        display,
        rootWindow,
        0, 0,               // 捕获区域起始坐标（左上角）
        width, height,      // 捕获区域宽度和高度
        AllPlanes,          // 包含所有平面（包括Alpha）
        ZPixmap             // 像素格式（通常为32位BGRA或ARGB）
    );

    if (!ximage) {
        return false;
    }

    // RAII管理XImage资源
    struct ImageDestroyer {
        XImage* img;
        ~ImageDestroyer() { if (img) XDestroyImage(img); }
    } imgDestroyer{ ximage };

    // 为RGBA数据分配内存 (每个像素4字节: R, G, B, A)
    bitmap.resize(width * height * 4);

    // 转换XImage数据为RGBA格式
    // X11的ZPixmap通常是32位，格式为BGRA（字节顺序：蓝、绿、红、Alpha）
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // 获取32位像素值（包含Alpha通道）
            uint32_t pixel = XGetPixel(ximage, x, y);

            // 提取BGRA分量（根据X11实际像素格式调整）
            uint8_t blue = (pixel >> 0) & 0xFF;    // 蓝色分量
            uint8_t green = (pixel >> 8) & 0xFF;   // 绿色分量
            uint8_t red = (pixel >> 16) & 0xFF;    // 红色分量
            uint8_t alpha = (pixel >> 24) & 0xFF;  // Alpha分量（透明度）

            // 对于没有Alpha通道的屏幕，强制设置为完全不透明
            if (ximage->depth < 32) {
                alpha = 0xFF;  // 255表示完全不透明
            }

            // 存储为RGBA格式（红、绿、蓝、Alpha）
            size_t index = (y * width + x) * 4;
            bitmap[index] = red;
            bitmap[index + 1] = green;
            bitmap[index + 2] = blue;
            bitmap[index + 3] = alpha;
        }
    }
    return true;
}

std::shared_ptr<IBitmap> ScreenCapture::CaptureBitmap(const Window* pWindow)
{
    if (pWindow == nullptr) {
        return nullptr;
    }
    // 打开与X服务器的连接
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        return nullptr;
    }
    // RAII资源管理
    struct DisplayCloser {
        Display* d;
        ~DisplayCloser() { if (d) ::XCloseDisplay(d); }
    } closer{ display };

    ::Window targetWindow = pWindow->NativeWnd()->GetX11WindowNumber();

    std::vector<uint8_t> bitmap;
    int32_t width = 0;
    int32_t height = 0;
    if (!CaptureScreenBitmap(display, targetWindow, bitmap, width, height)) {
        return nullptr;
    }
    if ((width > 0) && (height > 0) && ((int32_t)bitmap.size() == (width * height * 4))) {
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

} // namespace ui

#endif //defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)

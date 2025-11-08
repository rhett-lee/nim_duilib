#include "ScreenCapture_X11.h"
#include "duilib/Core/GlobalManager.h"

#if defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)
//Linux/FreeBSD OS

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cassert>
#include <stdexcept>
#include <cstdint>
#include <cstdlib>

namespace ui
{
// 线程局部变量：存储X11错误状态（避免多线程冲突）
static thread_local bool s_x11ErrorOccurred = false;

// 自定义X11错误处理器
static int X11ErrorHandler(Display* display, XErrorEvent* event)
{
    (void)display; // 未使用参数，避免编译警告
    (void)event;   // 未使用参数，避免编译警告
    
    // 标记发生X11错误
    s_x11ErrorOccurred = true;
    return 0; // 返回0表示已处理错误，避免程序崩溃
}

// 辅助函数：计算掩码的有效位数（用于判断是否存在Alpha通道）
static int MaskBitCount(uint32_t mask)
{
    if (mask == 0) {
        return 0;
    }
    return 32 - __builtin_clz(mask); // 计算掩码最高位1的位置（适用于GCC/Clang）
}

// 辅助函数：从X11像素中提取RGBA分量（兼容无alpha_mask的旧版Xlib）
static void ExtractRGBA(XImage* ximage, uint32_t pixel, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a)
{
    r = g = b = 0xFF;
    a = 0xFF; // 默认完全不透明

    if (ximage->depth >= 24) {
        // 动态提取RGB分量（根据显示的颜色掩码）
        r = (pixel & ximage->red_mask) >> ((ximage->red_mask != 0) ? __builtin_ctz(ximage->red_mask) : 0);
        g = (pixel & ximage->green_mask) >> ((ximage->green_mask != 0) ? __builtin_ctz(ximage->green_mask) : 0);
        b = (pixel & ximage->blue_mask) >> ((ximage->blue_mask != 0) ? __builtin_ctz(ximage->blue_mask) : 0);

        // 处理Alpha通道（不依赖alpha_mask，通过深度和RGB掩码总位数推断）
        if (ximage->depth == 32) {
            // 计算RGB三个掩码的总位数
            int rgbTotalBits = MaskBitCount(ximage->red_mask) + 
                               MaskBitCount(ximage->green_mask) + 
                               MaskBitCount(ximage->blue_mask);
            
            // 如果RGB总位数 < 32，说明存在Alpha通道（剩余位数为Alpha）
            if (rgbTotalBits < 32) {
                // 查找Alpha通道的位置（未被RGB占用的高位）
                uint32_t alphaMask = ~(ximage->red_mask | ximage->green_mask | ximage->blue_mask) & 0xFFFFFFFF;
                if (alphaMask != 0) {
                    a = (pixel & alphaMask) >> __builtin_ctz(alphaMask);
                }
            }
            // 否则（RGB占满32位），无Alpha通道，保持a=0xFF
        }
        // 24位深度：无Alpha通道，保持a=0xFF
    }
}

// 捕获指定窗口所在屏幕的图像
// 参数:
//   display      - 与X服务器的连接
//   targetWindow - 目标窗口的X11窗口ID
//   bitmap       - 输出的RGB图像数据
//   width        - 输出图像的宽度
//   height       - 输出图像的高度
// 返回值:
//   成功返回true，失败返回false
static bool CaptureScreenBitmap(::Display* display, ::Window targetWindow, std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height)
{
    bitmap.clear();
    width = 0;
    height = 0;

    if (display == nullptr) {
        return false;
    }

    // 保存原始错误处理器，并设置自定义错误处理器
    XErrorHandler originalErrorHandler = XSetErrorHandler(X11ErrorHandler);
    if (originalErrorHandler == nullptr) {
        return false;
    }

    // 重置错误状态
    s_x11ErrorOccurred = false;

    bool result = false;
    try {
        // 获取目标窗口属性
        ::XWindowAttributes attr;
        if (s_x11ErrorOccurred || !::XGetWindowAttributes(display, targetWindow, &attr)) {
            throw std::runtime_error("Failed to get window attributes");
        }

        // 获取窗口所在的屏幕
        ::Screen* screen = attr.screen;
        if (s_x11ErrorOccurred || !screen) {
            throw std::runtime_error("Invalid screen");
        }

        // 获取屏幕的根窗口（整个屏幕）
        ::Window rootWindow = RootWindowOfScreen(screen);
        width = static_cast<int32_t>(screen->width);    // 类型转换安全
        height = static_cast<int32_t>(screen->height);  // 类型转换安全

        if (width < 1 || height < 1) {
            throw std::runtime_error("Invalid screen size");
        }

        // 捕获屏幕内容（使用AllPlanes确保获取所有通道）
        ::XImage* ximage = ::XGetImage(
            display,
            rootWindow,
            0, 0,               // 捕获整个屏幕
            width, height,      // 捕获区域大小
            AllPlanes,          // 捕获所有颜色平面
            ZPixmap             // 像素格式（32位对齐）
        );

        if (s_x11ErrorOccurred || !ximage) {
            throw std::runtime_error("Failed to get XImage");
        }

        // RAII管理XImage资源（确保异常安全）
        struct ImageDestroyer {
            XImage* img;
            ~ImageDestroyer() { 
                if (img) XDestroyImage(img);
            }
        } imgDestroyer{ ximage };

        // 验证XImage格式（必须是24位或32位）
        if (ximage->depth != 24 && ximage->depth != 32) {
            throw std::runtime_error("Unsupported image depth");
        }

        // 分配RGBA缓冲区（4字节/像素）
        const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
        bitmap.resize(pixelCount * 4);

        // 转换XImage数据到RGBA格式
        for (int32_t y = 0; y < height && !s_x11ErrorOccurred; ++y) {
            for (int32_t x = 0; x < width && !s_x11ErrorOccurred; ++x) {
                // 获取像素值（XGetPixel兼容性最好，大图像可优化为直接访问data缓冲区）
                const uint32_t pixel = XGetPixel(ximage, x, y);
                const size_t index = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * 4;

                uint8_t r, g, b, a;
                ExtractRGBA(ximage, pixel, r, g, b, a);

                bitmap[index] = r;
                bitmap[index + 1] = g;
                bitmap[index + 2] = b;
                bitmap[index + 3] = a;
            }
        }

        // 检查转换过程中是否发生错误
        if (s_x11ErrorOccurred) {
            throw std::runtime_error("Error during pixel conversion");
        }

        result = true;
    }
    catch (...) {
        // 捕获所有异常，确保资源正确释放
        bitmap.clear();
        width = 0;
        height = 0;
        result = false;
    }

    // 恢复原始错误处理器
    XSetErrorHandler(originalErrorHandler);
    // 重置错误状态
    s_x11ErrorOccurred = false;

    return result;
}

std::shared_ptr<IBitmap> ScreenCapture_X11::CaptureBitmap(const ui::Window* pWindow)
{
    if (pWindow == nullptr) {
        return nullptr;
    }

    // 获取原生窗口指针
    const NativeWindow* pNativeWnd = pWindow->NativeWnd();
    if (pNativeWnd == nullptr) {
        return nullptr;
    }

    // 打开X服务器连接（使用环境变量DISPLAY）
    ::Display* display = ::XOpenDisplay(nullptr);
    if (!display) {
        return nullptr;
    }

    // RAII自动关闭X连接
    struct DisplayCloser {
        Display* d;
        ~DisplayCloser() { 
            if (d) ::XCloseDisplay(d);
        }
    } displayCloser{ display };

    // 获取目标窗口的X11窗口ID（检查是否为无效窗口）
    const ::Window targetWindow = pNativeWnd->GetX11WindowNumber();
    if (targetWindow == BadWindow) {
        return nullptr;
    }

    std::vector<uint8_t> bitmapData;
    int32_t width = 0;
    int32_t height = 0;

    // 捕获屏幕图像（内部已处理X11错误）
    if (!CaptureScreenBitmap(display, targetWindow, bitmapData, width, height)) {
        return nullptr;
    }

    // 验证捕获数据的有效性
    const size_t expectedSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    if (width <= 0 || height <= 0 || bitmapData.size() != expectedSize) {
        return nullptr;
    }

    // 创建IBitmap对象
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory == nullptr) {
        return nullptr;
    }

    std::shared_ptr<IBitmap> spBitmap(pRenderFactory->CreateBitmap());
    if (spBitmap == nullptr) {
        return nullptr;
    }

    // 初始化位图
    if (!spBitmap->Init(width, height, bitmapData.data())) {
        return nullptr;
    }

    return spBitmap;
}

} // namespace ui

#endif //defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)

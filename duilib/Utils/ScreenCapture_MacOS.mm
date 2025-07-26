#include "ScreenCapture.h"
#include "duilib/Core/GlobalManager.h"

#ifdef DUILIB_BUILD_FOR_MACOS
//MacOS
#import <Cocoa/Cocoa.h>

namespace ui
{
// 捕获指定窗口所在屏幕的图像（返回RGBA格式带Alpha通道）
// 参数:
//   targetWindow - 目标窗口的NSWindow指针（若为nullptr则捕获当前屏幕）
//   bitmap       - 输出的RGBA图像数据（每个像素4字节：R, G, B, A）
//   width        - 输出图像的宽度
//   height       - 输出图像的高度
// 返回值:
//   成功返回true，失败返回false
bool CaptureScreenBitmap_MacOS(void* targetWindow, std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height) 
{
    @autoreleasepool {
        // 确定目标窗口所在的屏幕
        NSScreen* targetScreen = nil;
        NSWindow* window = static_cast<NSWindow*>(targetWindow);
        
        if (window) {
            // 从目标窗口获取所在屏幕
            targetScreen = [window screen];
        }
        
        if (!targetScreen) {
            // 若未指定窗口或获取失败，使用主屏幕
            targetScreen = [NSScreen mainScreen];
        }
        
        // 获取屏幕的可见区域（不含菜单栏等）
        NSRect screenRect = [targetScreen visibleFrame];
        width = static_cast<int32_t>(screenRect.size.width);
        height = static_cast<int32_t>(screenRect.size.height);
        
        if (width <= 0 || height <= 0) {
            return false;
        }
        
        // 创建位图上下文（RGBA格式，8位通道，带Alpha）
        CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
        if (!colorSpace) {
            return false;
        }
        
        // 每行字节数（32位对齐）
        size_t bytesPerRow = width * 4;
        // 分配像素内存
        void* pixels = malloc(bytesPerRow * height);
        if (!pixels) {
            CGColorSpaceRelease(colorSpace);
            return false;
        }
        
        // 创建Core Graphics上下文
        CGContextRef context = CGBitmapContextCreate(
            pixels,
            width,
            height,
            8,          // 每个通道8位
            bytesPerRow,
            colorSpace,
            kCGImageAlphaPremultipliedLast  // RGBA格式（Alpha在最后）
        );
        
        CGColorSpaceRelease(colorSpace);
        if (!context) {
            free(pixels);
            return false;
        }
        
        // 捕获屏幕内容到上下文
        CGImageRef screenImage = [targetScreen CGImageForRect:screenRect];
        if (!screenImage) {
            CGContextRelease(context);
            free(pixels);
            return false;
        }
        
        // 绘制图像到上下文（macOS的坐标原点在左下角，需翻转Y轴）
        CGContextDrawImage(context, CGRectMake(0, 0, width, height), screenImage);
        CGImageRelease(screenImage);
        CGContextRelease(context);
        
        // 将像素数据复制到输出向量（已为RGBA格式）
        bitmap.resize(width * height * 4);
        memcpy(bitmap.data(), pixels, width * height * 4);
        
        // 清理内存
        free(pixels);
        return true;
    }
}

std::shared_ptr<IBitmap> ScreenCapture::CaptureBitmap(const Window* pWindow)
{
    if (pWindow == nullptr) {
        return nullptr;
    }
    NSWindow* pNSWindow = (NSWindow*)pWindow->NativeWnd()->GetNSWindow();
    if (pNSWindow == nullptr) {
        return nullptr;
    }
    
    std::vector<uint8_t> bitmap;
    int32_t width = 0;
    int32_t height = 0;
    if (!CaptureScreenBitmap_MacOS(display, targetWindow, std::vector<uint8_t>&bitmap, int32_t & width, int32_t & height)) {
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
} // namespace ui

#endif //DUILIB_BUILD_FOR_MACOS

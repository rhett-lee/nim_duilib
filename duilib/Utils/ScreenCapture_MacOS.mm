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
    return false;
    /** CGDisplayCreateImage在macOS 15.0 以后不可用, 报错提示需要使用ScreenCaptureKit. #239
    @autoreleasepool {
        // 1. 检查屏幕录制权限
        if (@available(macOS 10.15, *)) {
            if (!CGPreflightScreenCaptureAccess()) {
                NSLog(@"请在系统设置 > 安全性与隐私 > 屏幕录制中勾选本程序");
                return false;
            }
        }

        // 2. 确定目标窗口所在屏幕
        NSScreen* targetScreen = nil;
        NSWindow* window = static_cast<NSWindow*>(targetWindow);
        if (window) {
            targetScreen = [window screen];
        }
        if (!targetScreen) {
            targetScreen = [NSScreen mainScreen];
        }

        // 3. 获取完整屏幕区域（替换visibleFrame为frame）
        NSRect screenRect = targetScreen.frame;  // 完整屏幕区域（含菜单栏）
        CGRect cgRect = NSRectToCGRect(screenRect);

        // 4. 获取屏幕ID并捕获完整屏幕
        CGDirectDisplayID displayID = [[targetScreen deviceDescription][@"NSScreenNumber"] unsignedIntValue];
        CGImageRef screenImage = CGDisplayCreateImage(displayID);  // 捕获整个屏幕
        if (!screenImage) {
            NSLog(@"无法捕获屏幕图像，权限可能未生效");
            return false;
        }

        // 5. 裁剪到目标区域（若需要）
        CGImageRef croppedImage = CGImageCreateWithImageInRect(screenImage, cgRect);
        CGImageRelease(screenImage);
        if (!croppedImage) {
            return false;
        }

        // 6. 获取图像尺寸
        width = static_cast<int32_t>(CGImageGetWidth(croppedImage));
        height = static_cast<int32_t>(CGImageGetHeight(croppedImage));
        if (width <= 0 || height <= 0) {
            CGImageRelease(croppedImage);
            return false;
        }

        // 7. 创建RGBA上下文并绘制
        CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
        if (!colorSpace) {
            CGImageRelease(croppedImage);
            return false;
        }

        size_t bytesPerRow = width * 4;
        std::vector<uint8_t> tempBuffer(width * height * 4);
        CGContextRef context = CGBitmapContextCreate(
            tempBuffer.data(), width, height, 8, bytesPerRow,
            colorSpace, kCGImageAlphaPremultipliedLast
        );
        CGColorSpaceRelease(colorSpace);

        if (!context) {
            CGImageRelease(croppedImage);
            return false;
        }

        CGContextDrawImage(context, CGRectMake(0, 0, width, height), croppedImage);
        CGContextRelease(context);
        CGImageRelease(croppedImage);

        // 8. 输出结果
        bitmap.swap(tempBuffer);
        return true;
    }
    */
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
    if (!CaptureScreenBitmap_MacOS(pNSWindow, bitmap, width, height)) {
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

#endif //DUILIB_BUILD_FOR_MACOS

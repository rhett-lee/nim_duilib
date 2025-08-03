#include "CefWindowUtils.h"
#include "duilib/Core/Window.h"

#ifdef DUILIB_BUILD_FOR_MACOS
#include <Cocoa/Cocoa.h>

namespace ui
{
void ReparentNSView(NSView* targetView, NSView* newParent) 
{
    @autoreleasepool {
        // 确保操作在主线程执行
        if (![NSThread isMainThread]) {
            dispatch_sync(dispatch_get_main_queue(), ^{
                [targetView removeFromSuperview];
                [newParent addSubview:targetView];
            });
        } else {
            [targetView removeFromSuperview];
            [newParent addSubview:targetView];
        }
    }
}

void SetNSViewVisibility(NSView* view, bool visible) 
{
    @autoreleasepool {
        if (![NSThread isMainThread]) {
            dispatch_sync(dispatch_get_main_queue(), ^{
                [view setHidden:!visible];
            });
        } else {
            [view setHidden:!visible];
        }
    }
}

void SetNSViewFrame(NSView* view, CGFloat x, CGFloat y, CGFloat w, CGFloat h)
{
    @autoreleasepool {
        NSRect newFrame = NSMakeRect(x, y, w, h);
        if (![NSThread isMainThread]) {
            dispatch_sync(dispatch_get_main_queue(), ^{
                [view setFrame:newFrame];
                [view.superview setNeedsDisplay:YES]; // 触发重绘
            });
        } else {
            [view setFrame:newFrame];
            [view.superview setNeedsDisplay:YES];
        }
    }
}

bool IsValidNSView(NSView* view) 
{
    __block BOOL isValid = NO;
    void (^checkBlock)(void) = ^{
        isValid = (view != nil) &&
                  [view respondsToSelector:@selector(window)] &&
                  [view isKindOfClass:[NSView class]] &&
                  view.window;
    };

    if (![NSThread isMainThread]) {
        dispatch_sync(dispatch_get_main_queue(), checkBlock);
    } else {
        checkBlock();
    }
    return isValid;
}
 
void SetCefWindowPos(CefWindowHandle cefWindow, CefControl* pCefControl)
{
    if ((cefWindow == 0) || (pCefControl == nullptr)) {
        return;
    }
    Window* pWindow = pCefControl->GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    NSView* pCefNSView = (NSView*)cefWindow;
    if (!IsValidNSView(pCefNSView)) {
        return;
    }
    UiRect rc = pCefControl->GetPos();
    UiRect rcWindow;
    pWindow->GetWindowRect(rcWindow);
    //macOS的窗口顶点坐标是左下角（Windows是左上角）
    SetNSViewFrame(pCefNSView, (CGFloat)rc.left, (CGFloat)(rcWindow.Height() - rc.bottom), (CGFloat)rc.Width(), (CGFloat)rc.Height());
}

void SetCefWindowVisible(CefWindowHandle cefWindow, CefControl* pCefControl)
{
    if ((cefWindow == 0) || (pCefControl == nullptr)) {
        return;
    }
    NSView* pCefNSView = (NSView*)cefWindow;
    if (!IsValidNSView(pCefNSView)) {
        return;
    }
    bool bVisible = pCefControl->IsVisible();
    
    SetNSViewVisibility(pCefNSView, bVisible);
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
    NSView* pCefNSView = (NSView*)cefWindow;
    if (!IsValidNSView(pCefNSView)) {
        return;
    }
    NSView* pParentNSView = (NSView*)pWindow->NativeWnd()->GetNSView();
    
    ReparentNSView(pCefNSView, pParentNSView);
}

bool CaptureCefWindowBitmap(CefWindowHandle cefWindow, std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height)
{
    NSView* pCefNSView = (NSView*)cefWindow;
    if (!pCefNSView) {
        return false;
    }
    if (!IsValidNSView(pCefNSView)) {
        return false;
    }
    // 获取视图的边界和尺寸
    NSRect viewBounds = [pCefNSView bounds];
    width = static_cast<int32_t>(NSWidth(viewBounds));
    height = static_cast<int32_t>(NSHeight(viewBounds));
    
    if (width <= 0 || height <= 0) {
        return false;
    }
    
    // 创建位图图像表示
    NSBitmapImageRep* bitmapRep = [[NSBitmapImageRep alloc] 
        initWithBitmapDataPlanes:NULL
                      pixelsWide:width
                      pixelsHigh:height
                   bitsPerSample:8
                 samplesPerPixel:4
                        hasAlpha:YES
                        isPlanar:NO
                  colorSpaceName:NSCalibratedRGBColorSpace
                    bytesPerRow:width * 4
                   bitsPerPixel:32];
    
    if (!bitmapRep) {
        return false;
    }
    
    // 创建图形上下文并绘制视图
    NSGraphicsContext* context = [NSGraphicsContext graphicsContextWithBitmapImageRep:bitmapRep];
    if (!context) {
        [bitmapRep release];
        return false;
    }
    
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:context];
    
    // 绘制视图内容（使用现代方法替代lockFocus/unlockFocus）
    [pCefNSView displayRectIgnoringOpacity:viewBounds inContext:context];
    
    [NSGraphicsContext restoreGraphicsState];
    
    // 获取位图数据
    unsigned char* bitmapData = [bitmapRep bitmapData];
    if (bitmapData) {
        size_t dataSize = width * height * 4;
        bitmap.resize(dataSize);
        memcpy(bitmap.data(), bitmapData, dataSize);
    } else {
        [bitmapRep release];
        return false;
    }
    
    [bitmapRep release];
    return true;
}

void SetCefWindowCursor(CefWindowHandle cefWindow, CefCursorHandle cursor)
{
    if ((cefWindow == nullptr) || (cursor == nullptr)) {
        return;
    }
    
    // 将 CEF 窗口句柄转换为 NSView*
    NSView* cefView = static_cast<NSView*>(cefWindow);
    // 将 CEF 光标句柄转换为 NSCursor*
    NSCursor* nsCursor = static_cast<NSCursor*>(cursor);
    
    // 获取视图所在的窗口
    NSWindow* window = [cefView window];
    if (!window) {
        return;
    }
    
    // 确保在主线程操作 UI 元素
    auto setCursorBlock = ^{        
        // 对于视图范围的光标，可以使用 NSCursor 的 set 方法
         [nsCursor set];
    };
    
    if ([NSThread isMainThread]) {
        setCursorBlock();
    } else {
        dispatch_sync(dispatch_get_main_queue(), setCursorBlock);
    }
}

void RemoveCefWindowFromParent(CefWindowHandle /*cefWindow*/)
{
    //无需实现：如果用代码实现从父View中移除，则CEF在退出时有错误，导致进程无法正常退出
    return;
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_MACOS

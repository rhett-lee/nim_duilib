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

} //namespace ui

#endif //DUILIB_BUILD_FOR_MACOS

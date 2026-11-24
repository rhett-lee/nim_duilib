#include "ClickThrough.h"
#include "duilib/Core/Window.h"

#ifdef DUILIB_BUILD_FOR_MACOS
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>
#include <AppKit/AppKit.h>
#include <CoreFoundation/CoreFoundation.h>
#include "duilib/Utils/LogUtil.h"

namespace ui
{
ClickThrough::ClickThrough()
{
}

ClickThrough::~ClickThrough()
{
}

/** 激活指定的应用
*/
static void MacOS_ActivateApplication(NSRunningApplication* app)
{
    if (app == nil) {
        return;
    }
    if (@available(macOS 14.0, *)) {
        //macOS 14 起使用新的激活接口
        [app activate];
    }
    else {
        //旧系统需要使用该选项才能把应用窗口带到前台
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        [app activateWithOptions:NSApplicationActivateIgnoringOtherApps];
#pragma clang diagnostic pop
    }
}

/** 激活指定窗口编号所属的应用
* @return 找到该窗口并激活其应用时返回true
*/
static bool MacOS_ActivateWindowByNumber(NSInteger windowNumber) 
{
    if (windowNumber == 0) {
        return false;
    }
    
    CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);    
    if (!windowList) {
        return false;
    }
    
    BOOL found = NO;
    pid_t targetPID = 0;
    CFIndex count = CFArrayGetCount(windowList);
    
    for (CFIndex i = 0; i < count; i++) {
        CFDictionaryRef info = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);
        CFNumberRef idRef = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowNumber);
        
        NSInteger currentID;
        if (idRef && CFNumberGetValue(idRef, kCFNumberNSIntegerType, &currentID) && (currentID == windowNumber)) {
            
            CFNumberRef pidRef = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowOwnerPID);
            if (pidRef) {
                CFNumberGetValue(pidRef, kCFNumberIntType, &targetPID);
            }            
            found = YES;
            break;
        }
    }
    CFRelease(windowList);
    
    if (found && targetPID != 0) {
        NSRunningApplication *app = [NSRunningApplication runningApplicationWithProcessIdentifier:targetPID];
        if (app != nil) {
            MacOS_ActivateApplication(app);
            return true;
        }
    }
    return false;
}

bool ClickThrough::ClickThroughWindow(Window* pWindow, const UiPoint& ptMouse)
{
    if (pWindow == nullptr) {
        return false;
    }
    
    // 获取原生窗口句柄
    NSWindow* pNSWindow = (NSWindow*)pWindow->NativeWnd()->GetNSWindow();
    if (pNSWindow == nullptr) {
        return false;
    }
    
    //ptMouse为屏幕坐标(左上角为原点)，先转换为窗口客户端坐标，再转换为Cocoa的屏幕坐标
    UiPoint pt = ptMouse;
    pWindow->ScreenToClient(pt);

    // 当前处理的窗口
    NSWindow* currentWindow = pNSWindow;
    NSView* contentView = [currentWindow contentView];
    if (contentView == nil) {
        return false;
    }
    NSPoint screenPoint = [contentView convertPoint:NSMakePoint(pt.x, pt.y) toView:nil];
    
    //临时让当前窗口忽略鼠标事件，以便获取鼠标下方的窗口：
    //只修改这一项，不修改窗口层级和collectionBehavior，避免引起窗口重新合成
    BOOL originalIgnoresMouseEvents = [currentWindow ignoresMouseEvents];
    [currentWindow setIgnoresMouseEvents:YES];
    NSInteger windowNumber = [NSWindow windowNumberAtPoint:screenPoint
                                belowWindowWithWindowNumber:0];
    [currentWindow setIgnoresMouseEvents:originalIgnoresMouseEvents];

    // 如果点击到其他窗口，则激活该窗口
    if ((windowNumber != 0) && (windowNumber != [currentWindow windowNumber])) {
        bool bActivated = MacOS_ActivateWindowByNumber(windowNumber);
        if (!bActivated) {
            LogUtil::OutputLine(_T("ClickThroughWindow: 激活窗口失败(NSRunningApplication)"));
        }
        return bActivated;
    }
    return false;
}

}//namespace ui

#endif //DUILIB_BUILD_FOR_MACOS

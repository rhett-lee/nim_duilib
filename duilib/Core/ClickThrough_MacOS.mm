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

/** 激活指定窗口编号对应的窗口
* 同一进程的其它窗口直接提升该窗口；其它进程的窗口通过激活其所属应用带到前台
* @return 处理成功时返回true
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
    
    if (!found || (targetPID == 0)) {
        return false;
    }

    if (targetPID == [[NSProcessInfo processInfo] processIdentifier]) {
        //当前进程的其它窗口：激活应用不会把指定的窗口提升到前面，直接提升该窗口
        for (NSWindow* window in [NSApp windows]) {
            if ([window windowNumber] == windowNumber) {
                [window makeKeyAndOrderFront:nil];
                return true;
            }
        }
        return false;
    }

    NSRunningApplication *app = [NSRunningApplication runningApplicationWithProcessIdentifier:targetPID];
    if (app == nil) {
        return false;
    }
    MacOS_ActivateApplication(app);
    return true;
}

bool ClickThrough::ClickThroughWindow(Window* pWindow, const UiPoint& /*ptMouse*/)
{
    if (pWindow == nullptr) {
        return false;
    }
    
    // 获取原生窗口句柄
    NSWindow* pNSWindow = (NSWindow*)pWindow->NativeWnd()->GetNSWindow();
    if (pNSWindow == nullptr) {
        return false;
    }
    
    // 当前处理的窗口
    NSWindow* currentWindow = pNSWindow;

    //使用鼠标的当前位置作为屏幕坐标：windowNumberAtPoint:要求的是Cocoa屏幕坐标
    //(原点在主屏幕左下角)，该坐标与duilib窗口坐标的单位、DPI缩放无关，避免换算误差
    NSPoint screenPoint = [NSEvent mouseLocation];

    //以当前窗口为界，只查询位于当前窗口下方的窗口(当前窗口不参与命中)
    NSInteger currentWindowNumber = [currentWindow windowNumber];
    NSInteger windowNumber = 0;
    if (currentWindowNumber != 0) {
        windowNumber = [NSWindow windowNumberAtPoint:screenPoint
                           belowWindowWithWindowNumber:currentWindowNumber];
    }
    if (windowNumber == 0) {
        //兜底方案：临时让当前窗口忽略鼠标事件后重新查询(例如当前窗口不在窗口服务器的Z序中)
        BOOL originalIgnoresMouseEvents = [currentWindow ignoresMouseEvents];
        [currentWindow setIgnoresMouseEvents:YES];
        windowNumber = [NSWindow windowNumberAtPoint:screenPoint
                           belowWindowWithWindowNumber:0];
        [currentWindow setIgnoresMouseEvents:originalIgnoresMouseEvents];
    }
    if (windowNumber == currentWindowNumber) {
        //未找到当前窗口下方的其它窗口
        windowNumber = 0;
    }

    // 如果点击到其他窗口，则激活该窗口
    if (windowNumber != 0) {
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

#include "ClickThrough.h"
#include "duilib/Core/Window.h"

#ifdef DUILIB_BUILD_FOR_MACOS
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>
#include <AppKit/AppKit.h>
#include <CoreFoundation/CoreFoundation.h>

namespace ui
{
ClickThrough::ClickThrough()
{
}

ClickThrough::~ClickThrough()
{
}

static void MacOS_ActivateWindowByNumber(NSInteger windowNumber) 
{
    if (windowNumber == 0) {
        return;
    }
    
    CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);    
    if (!windowList) {
        return;
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
        [app activateWithOptions:NSApplicationActivateIgnoringOtherApps];
        if (!app.isActive) {
            dispatch_async(dispatch_get_main_queue(), ^{
                [app activateWithOptions:NSApplicationActivateIgnoringOtherApps];
            });
        }
    }
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
    
    // 坐标转换为窗口客户端坐标
    UiPoint pt = ptMouse;
    pWindow->ScreenToClient(pt);

    // 当前处理的窗口
    NSWindow* currentWindow = pNSWindow;
    
    // 保存窗口原始状态以便恢复
    NSWindowLevel originalLevel = [currentWindow level];
    NSWindowCollectionBehavior originalBehavior = [currentWindow collectionBehavior];
    BOOL originalIgnoresMouseEvents = [currentWindow ignoresMouseEvents];
    
    // 临时配置窗口为可穿透状态
    [currentWindow setLevel:NSFloatingWindowLevel];
    [currentWindow setCollectionBehavior:NSWindowCollectionBehaviorCanJoinAllSpaces |
                                     NSWindowCollectionBehaviorIgnoresCycle |
                                     NSWindowCollectionBehaviorStationary];
    [currentWindow setIgnoresMouseEvents:YES];
    
    // 坐标转换: 窗口坐标 -> 屏幕坐标
    NSView* contentView = [currentWindow contentView];
    NSPoint windowPoint = NSMakePoint(pt.x, pt.y);
    NSPoint screenPoint = [contentView convertPoint:windowPoint toView:nil];
    
    // 获取鼠标点击位置下的窗口编号
    NSInteger windowNumber = [NSWindow windowNumberAtPoint:screenPoint
                                belowWindowWithWindowNumber:0];
    
    // 恢复窗口原始状态
    [currentWindow setIgnoresMouseEvents:originalIgnoresMouseEvents];
    [currentWindow setLevel:originalLevel];
    [currentWindow setCollectionBehavior:originalBehavior];

    // 如果点击到其他窗口，则激活该窗口
    if (windowNumber != 0 && windowNumber != [currentWindow windowNumber]) {
        MacOS_ActivateWindowByNumber(windowNumber);
    }

    return true;
}

}//namespace ui

#endif //DUILIB_BUILD_FOR_MACOS

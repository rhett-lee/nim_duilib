#import "SDL_MacOS.h"

#if defined(__APPLE__) && defined(__MACH__)

#import <SDL3/SDL.h>
#import <Cocoa/Cocoa.h>

namespace ui
{

void* GetSDLWindowContentView(SDL_Window* sdlWindow) 
{
    if(sdlWindow == nullptr) {
        return nullptr;
    }
    SDL_PropertiesID propID = ::SDL_GetWindowProperties(sdlWindow);
    NSWindow* pNSWindow = (NSWindow*)::SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
    NSView* pNSView = nullptr;
    if (pNSWindow != nullptr) {
        pNSView = [pNSWindow contentView] ;
    }
    return (void*)pNSView;
}

bool SetFocus_MacOS(void* pNSWindow)
{
    // 检查窗口指针有效性
    NSWindow* window = static_cast<NSWindow*>(pNSWindow);
    if (!window) {
        //NSLog(@"SetFocus_MacOS: 无效的窗口指针");
        return false;
    }
    
    // 获取窗口的contentView
    NSView* targetView = [window contentView];
    if (!targetView) {
        //NSLog(@"SetFocus_MacOS: 窗口没有contentView");
        return false;
    }
    
    // 确保在主线程执行UI操作
    if (![NSThread isMainThread]) {
        //NSLog(@"SetFocus_MacOS: 必须在主线程调用");
        return false;
    }
    
    // 确保目标视图可以接收焦点，如果不能则尝试开启
    if (![targetView acceptsFirstResponder]) {
        //NSLog(@"SetFocus_MacOS: 目标视图默认不接受焦点，尝试开启");
               
        //先清除当前焦点再尝试
        [window makeFirstResponder:nil];
    }
    
    // 切换到目标视图
    BOOL success = [window makeFirstResponder:targetView];
    
    // 刷新窗口以更新焦点状态
    [window update];
    
    if (!success) {
        //NSLog(@"SetFocus_MacOS: 焦点切换失败");
    }
    
    return success;
}

}

#endif

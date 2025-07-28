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
    NSWindow* window = (NSWindow*)pNSWindow;
    if (!window) {
        return false;
    }
    NSView* targetView = nullptr;
    if (window != nullptr) {
        targetView = [window contentView] ;
    }
    if (!targetView) {
        return false;
    }
    
    // 1. 先清除当前焦点（避免CEF等视图拦截）
    [window makeFirstResponder:nil];
    
    // 2. 切换到目标视图
    BOOL success = [window makeFirstResponder:targetView];
    
    // 3. 刷新窗口以更新焦点状态
    [window update];

    //if (success == YES) {
    //    NSLog(@"目标视图接收焦点设置: YES");
    //}
    //else {
    //    NSLog(@"目标视图接收焦点设置: NO");
    //}

    return success == YES;
}

}

#endif

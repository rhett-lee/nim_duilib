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
    
    // 在主线程上执行UI操作
    __block BOOL result = NO;
    dispatch_sync(dispatch_get_main_queue(), ^{
        // 使窗口成为关键窗口并前置显示
        [window makeKeyAndOrderFront:nil];
        
        // 确保窗口是可见的
        [window setIsVisible:YES];
        
        // 激活应用程序
        [[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateIgnoringOtherApps];
        
        result = YES;
    });
    
    return (result == YES) ? true : false;
}

}

#endif

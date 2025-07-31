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

bool SetFocus_MacOS(void* /*pNSWindow*/)
{
    //无需实现：如果用代码实现激活主窗口，则CEF在退出时有错误，导致进程无法正常退出
    return false;
}

}

#endif

#include "SystemUtil.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/Window.h"

#ifdef DUILIB_BUILD_FOR_SDL
#include <SDL3/SDL.h>

namespace ui
{
bool SystemUtil::OpenUrl(const DString& url)
{
    if (url.empty()) {
        return false;
    }
    return SDL_OpenURL(StringUtil::TToUTF8(url).c_str());
}

bool SystemUtil::ShowMessageBox(const Window* pWindow, const DString& content, const DString& title)
{
    SDL_Window* sdlWindow = (SDL_Window*)((pWindow != nullptr) ? pWindow->NativeWnd()->GetWindowHandle() : nullptr);
    SDL_MessageBoxFlags flags = SDL_MESSAGEBOX_INFORMATION;
    return SDL_ShowSimpleMessageBox(flags, StringUtil::TToUTF8(content).c_str(), StringUtil::TToUTF8(title).c_str(), sdlWindow);
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_SDL

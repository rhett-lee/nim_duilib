#include "DragWindowFilter_SDL.h"

#ifdef DUILIB_BUILD_FOR_SDL
#include <SDL3/SDL.h>

namespace ui
{
DragWindowFilter::DragWindowFilter(Window* pOwner, Window* pWindow) :
    m_pOwner(pOwner),
    m_pWindow(pWindow)
{
}

LRESULT DragWindowFilter::FilterMessage(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
    if ((m_pOwner != nullptr) && (m_pWindow != nullptr)) {
        if ((uMsg == SDL_EVENT_MOUSE_MOTION) || (uMsg == SDL_EVENT_MOUSE_BUTTON_DOWN) || (uMsg == SDL_EVENT_MOUSE_BUTTON_UP)) {
            //鼠标事件，转接给父窗口
            SDL_Window* sdlWindow = (SDL_Window*)m_pWindow->GetWindowHandle();
            SDL_Window* sdlOwnerWindow = (SDL_Window*)m_pOwner->GetWindowHandle();
            if ((sdlWindow != nullptr) && (sdlOwnerWindow != nullptr)) {
                bHandled = true;
                SDL_Event sdlEvent = *((const SDL_Event*)wParam);

                int nXPos = 0;
                int nYPos = 0;
                SDL_GetWindowPosition(sdlWindow, &nXPos, &nYPos);

                int nOwnerXPos = 0;
                int nOwnerYPos = 0;
                SDL_GetWindowPosition(sdlOwnerWindow, &nOwnerXPos, &nOwnerYPos);

                if (uMsg == SDL_EVENT_MOUSE_MOTION) {
                    sdlEvent.motion.windowID = SDL_GetWindowID(sdlOwnerWindow);
                    sdlEvent.motion.x = sdlEvent.motion.x + nXPos - nOwnerXPos;
                    sdlEvent.motion.y = sdlEvent.motion.y + nYPos - nOwnerYPos;
                    SDL_PushEvent(&sdlEvent);
                }
                else {
                    sdlEvent.button.windowID = SDL_GetWindowID(sdlOwnerWindow);
                    sdlEvent.button.x = sdlEvent.button.x + nXPos - nOwnerXPos;
                    sdlEvent.button.y = sdlEvent.button.y + nYPos - nOwnerYPos;
                    SDL_PushEvent(&sdlEvent);
                }
            }
        }
    }
    return 0;
}

}

#endif //DUILIB_BUILD_FOR_SDL

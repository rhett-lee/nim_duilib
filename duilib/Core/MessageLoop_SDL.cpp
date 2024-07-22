#include "MessageLoop_SDL.h"

#if defined(DUILIB_BUILD_FOR_SDL)

#include "NativeWindow_SDL.h"
#include <SDL3/SDL.h>

namespace ui
{
MessageLoop_SDL::MessageLoop_SDL()
{
}

MessageLoop_SDL::~MessageLoop_SDL()
{
}

int32_t MessageLoop_SDL::Run()
{
    //初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        SDL_Log("SDL_Init(SDL_INIT_VIDEO) failed: %s", SDL_GetError());
        return -1;
    }

    //进入消息循环
    SDL_bool bKeepGoing = SDL_TRUE;
    SDL_Event sdlEvent;
    memset(&sdlEvent, 0, sizeof(sdlEvent));
    /* run the program until told to stop. */
    while (bKeepGoing) {

        /* run through all pending events until we run out. */
        while (SDL_PollEvent(&sdlEvent)) {
            switch (sdlEvent.type) {
            case SDL_EVENT_QUIT:  /* triggers on last window close and other things. End the program. */
                bKeepGoing = SDL_FALSE;
                break;
            default:
                {
                    //将事件派发到窗口
                    NativeWindow_SDL* pWindow = nullptr;
                    SDL_WindowID id = NativeWindow_SDL::GetWindowIdFromEvent(sdlEvent);
                    if (id != 0) {
                        pWindow = NativeWindow_SDL::GetWindowFromID(id);
                    }
                    if (pWindow != nullptr) {
                        pWindow->OnSDLWindowEvent(sdlEvent);
                    }
                    else {
                        //其他消息，暂不处理
                    }
                }
                break;
            }
        }
    }

    //退出SDL
    SDL_Quit();
    return 0;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_SDL

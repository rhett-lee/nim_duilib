#include "MessageLoop_SDL.h"
#include "duilib/Utils/StringConvert.h"

#if defined(DUILIB_BUILD_FOR_SDL)

#include "NativeWindow_SDL.h"
#include <SDL3/SDL.h>

namespace ui
{
std::unordered_map<uint32_t, SDLUserMessageCallback> MessageLoop_SDL::s_userMsgCallbacks;

MessageLoop_SDL::MessageLoop_SDL()
{
}

MessageLoop_SDL::~MessageLoop_SDL()
{
}

bool MessageLoop_SDL::CheckInitSDL(const DString& videoDriverName)
{
    //初始化SDL
    bool bRet = true;
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (!videoDriverName.empty()) {
            DStringA videoDriverNameA = StringConvert::TToUTF8(videoDriverName);
            if (!videoDriverNameA.empty()) {
                SDL_SetHint(SDL_HINT_VIDEO_DRIVER, videoDriverNameA.c_str());
            }            
        }
        bRet = SDL_Init(SDL_INIT_VIDEO);
        ASSERT_UNUSED_VARIABLE(bRet);

        //处理自定义消息的范围(内部占用32个ID，备用)
        uint32_t nBaseId = SDL_RegisterEvents(32);
        ASSERT_UNUSED_VARIABLE(nBaseId == SDL_EVENT_USER);

        //自定义消息使用情况：
        //1. SDL_EVENT_USER + 0:  MessageLoop_SDL::PostNoneEvent 函数占用
        //2. SDL_EVENT_USER + 1:  duilib\Core\FrameworkThread.cpp WM_USER_DEFINED_MSG 消息占用
        //3. SDL_EVENT_USER + 2:  duilib\Core\TimerManager.cpp WM_USER_DEFINED_TIMER 占用
        //4. SDL_EVENT_USER + 3:  duilib\Core\NativeWindow_SDL.cpp WM_USER_PAINT_MSG 占用
    }
    return bRet;
}

int32_t MessageLoop_SDL::Run()
{
    //初始化SDL
    if (!MessageLoop_SDL::CheckInitSDL()) {
        return -1;
    }
    
    //进入消息循环
    bool bKeepGoing = true;
    SDL_Event sdlEvent;
    memset(&sdlEvent, 0, sizeof(sdlEvent));
    /* run the program until told to stop. */
    while (bKeepGoing) {

        /* run through all pending events until we run out. */
        while (bKeepGoing && SDL_WaitEvent(&sdlEvent)) {
            switch (sdlEvent.type) {
            case SDL_EVENT_QUIT:  /* triggers on last window close and other things. End the program. */
                bKeepGoing = false;
                break;
            default:
                {
                    //将事件派发到窗口
                    NativeWindow_SDL* pWindow = nullptr;
                    SDL_WindowID windowID = NativeWindow_SDL::GetWindowIdFromEvent(sdlEvent);
                    if (windowID != 0) {
                        pWindow = NativeWindow_SDL::GetWindowFromID(windowID);
                    }
                    if (pWindow != nullptr) {
                        pWindow->OnSDLWindowEvent(sdlEvent);
                    }
                    else {
                        //其他消息，除了注册的自定义消息，不处理
                        if ((sdlEvent.type > SDL_EVENT_USER) && (sdlEvent.type < SDL_EVENT_LAST)) {
                            //用户自定义消息
                            OnUserEvent(sdlEvent);
                        }
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

void MessageLoop_SDL::RunDoModal(NativeWindow_SDL& nativeWindow, bool bCloseByEsc, bool bCloseByEnter)
{
    ASSERT(nativeWindow.IsWindow());
    if (!nativeWindow.IsWindow()) {
        return;
    }
    const SDL_WindowID currentWindowId = SDL_GetWindowID((SDL_Window*)nativeWindow.GetWindowHandle());
    ASSERT(currentWindowId != 0);
    if (currentWindowId == 0) {
        return;
    }

    bool bKeepGoing = true;
    SDL_Event sdlEvent;
    memset(&sdlEvent, 0, sizeof(sdlEvent));
    /* run the program until told to stop. */
    while (bKeepGoing) {

        /* run through all pending events until we run out. */
        while (bKeepGoing && SDL_WaitEvent(&sdlEvent)) {
            switch (sdlEvent.type) {
            case SDL_EVENT_QUIT:  /* triggers on last window close and other things. End the program. */
                bKeepGoing = false;
                //重新放入一个Quit消息，让主消息循环也退出，避免该事件丢失
                nativeWindow.PostQuitMsg(0);
                break;
            default:
                {
                    //将事件派发到窗口
                    NativeWindow_SDL* pWindow = nullptr;
                    const SDL_WindowID windowID = NativeWindow_SDL::GetWindowIdFromEvent(sdlEvent);
                    if (windowID != 0) {
                        pWindow = NativeWindow_SDL::GetWindowFromID(windowID);
                    }
                    if (pWindow != nullptr) {
                        pWindow->OnSDLWindowEvent(sdlEvent);
                    }
                    else {
                        //其他消息，除了注册的自定义消息，不处理
                        if ((sdlEvent.type > SDL_EVENT_USER) && (sdlEvent.type < SDL_EVENT_LAST)) {
                            //用户自定义消息
                            OnUserEvent(sdlEvent);
                        }
                    }

                    if ((sdlEvent.type == SDL_EVENT_WINDOW_DESTROYED) && (windowID == currentWindowId)) {
                        //窗口已经退出，退出消息循环
                        bKeepGoing = false;
                    }
                    else if ((bCloseByEsc || bCloseByEnter) && (sdlEvent.type == SDL_EVENT_KEY_DOWN) && (windowID == currentWindowId)) {
                        VirtualKeyCode vkCode = Keycode::GetVirtualKeyCode(sdlEvent.key.key);
                        if (bCloseByEsc && (vkCode == VirtualKeyCode::kVK_ESCAPE)) {
                            //模态对话框，按ESC键时，关闭
                            if (!nativeWindow.IsClosingWnd()) {
                                nativeWindow.CloseWnd(kWindowCloseCancel);
                            }
                        }
                        else if (bCloseByEnter && (vkCode == VirtualKeyCode::kVK_RETURN)) {
                            //模态对话框，按Enter键时，关闭
                            nativeWindow.CloseWnd(kWindowCloseOK);
                        }
                    }
                }
                break;
            }
        }
    }
}

void MessageLoop_SDL::RunUserLoop(bool& bTerminate)
{
    ASSERT(!bTerminate);
    if (bTerminate) {
        return;
    }
    bool bKeepGoing = true;
    SDL_Event sdlEvent;
    memset(&sdlEvent, 0, sizeof(sdlEvent));
    /* run the program until told to stop. */
    while (bKeepGoing) {

        /* run through all pending events until we run out. */
        while (bKeepGoing && SDL_WaitEvent(&sdlEvent)) {
            switch (sdlEvent.type) {
            case SDL_EVENT_QUIT:  /* triggers on last window close and other things. End the program. */
                bKeepGoing = false;
                //重新放入一个Quit消息，让主消息循环也退出，避免该事件丢失
                SDL_Event quitEvent;
                quitEvent.type = SDL_EVENT_QUIT;
                quitEvent.common.timestamp = 0;
                SDL_PushEvent(&quitEvent);
                break;
            default:
                {
                    //将事件派发到窗口
                    NativeWindow_SDL* pWindow = nullptr;
                    const SDL_WindowID windowID = NativeWindow_SDL::GetWindowIdFromEvent(sdlEvent);
                    if (windowID != 0) {
                        pWindow = NativeWindow_SDL::GetWindowFromID(windowID);
                    }
                    if (pWindow != nullptr) {
                        pWindow->OnSDLWindowEvent(sdlEvent);
                    }
                    else {
                        //其他消息，除了注册的自定义消息，不处理
                        if ((sdlEvent.type > SDL_EVENT_USER) && (sdlEvent.type < SDL_EVENT_LAST)) {
                            //用户自定义消息
                            OnUserEvent(sdlEvent);
                        }
                    }

                    if (bTerminate) {
                        //已经标记退出，退出该消息循环
                        bKeepGoing = false;
                    }
                }
                break;
            }
        }
    }
}

void MessageLoop_SDL::RemoveDuplicateMsg(uint32_t msgId)
{
    SDL_FlushEvent(msgId);
}

bool MessageLoop_SDL::PostUserEvent(uint32_t msgId, WPARAM wParam, LPARAM lParam)
{
    ASSERT((msgId > SDL_EVENT_USER) && (msgId < SDL_EVENT_LAST));
    if ((msgId <= SDL_EVENT_USER) || (msgId >= SDL_EVENT_LAST)) {
        return false;
    }
    SDL_Event sdlEvent;
    sdlEvent.type = msgId;
    sdlEvent.common.timestamp = 0;
    sdlEvent.user.reserved = 0;
    sdlEvent.user.timestamp = 0;
    sdlEvent.user.type = msgId;
    sdlEvent.user.code = msgId;
    sdlEvent.user.data1 = (void*)wParam;
    sdlEvent.user.data2 = (void*)lParam;
    sdlEvent.user.windowID = 0;
    bool nRet = SDL_PushEvent(&sdlEvent);
    ASSERT(nRet);
    return nRet;
}

void MessageLoop_SDL::PostNoneEvent()
{
    SDL_Event sdlEvent;
    sdlEvent.type = SDL_EVENT_USER;
    sdlEvent.common.timestamp = 0;
    sdlEvent.user.reserved = 0;
    sdlEvent.user.timestamp = 0;
    sdlEvent.user.type = SDL_EVENT_USER;
    sdlEvent.user.code = SDL_EVENT_USER;
    sdlEvent.user.data1 = 0;
    sdlEvent.user.data2 = 0;
    sdlEvent.user.windowID = 0;
    bool nRet = SDL_PushEvent(&sdlEvent);
    ASSERT_UNUSED_VARIABLE(nRet);
}

void MessageLoop_SDL::AddUserMessageCallback(uint32_t msgId, const SDLUserMessageCallback& callback)
{
    ASSERT((msgId > SDL_EVENT_USER) && (msgId < SDL_EVENT_LAST));
    if ((msgId <= SDL_EVENT_USER) || (msgId >= SDL_EVENT_LAST)) {
        return;
    }
    ASSERT(callback != nullptr);
    if (callback == nullptr) {
        return;
    }
    s_userMsgCallbacks[msgId] = callback;
}

void MessageLoop_SDL::RemoveUserMessageCallback(uint32_t msgId)
{
    auto iter = s_userMsgCallbacks.find(msgId);
    if (iter != s_userMsgCallbacks.end()) {
        s_userMsgCallbacks.erase(iter);
    }
}

void MessageLoop_SDL::OnUserEvent(const SDL_Event& sdlEvent)
{
    if ((sdlEvent.type <= SDL_EVENT_USER) || (sdlEvent.type >= SDL_EVENT_LAST)) {
        return;
    }
    if (sdlEvent.type != sdlEvent.user.type) {
        return;
    }
    if (sdlEvent.type != (uint32_t)sdlEvent.user.code) {
        return;
    }
    if (sdlEvent.user.windowID != 0) {
        return;
    }
    uint32_t msgId = sdlEvent.user.type;
    WPARAM wParam = (WPARAM)sdlEvent.user.data1;
    LPARAM lParam = (LPARAM)sdlEvent.user.data2;

    auto iter = s_userMsgCallbacks.find(msgId);
    if (iter != s_userMsgCallbacks.end()) {
        SDLUserMessageCallback callback = iter->second;
        ASSERT(callback != nullptr);
        if (callback != nullptr) {
            callback(msgId, wParam, lParam);
        }
    }
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_SDL

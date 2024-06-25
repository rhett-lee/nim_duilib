#include "MessageLoop.h"

#ifdef DUILIB_PLATFORM_WIN

namespace ui
{
MessageLoop::MessageLoop()
{
}

MessageLoop::~MessageLoop()
{
}

int32_t MessageLoop::Run()
{
    MSG msg = { 0, };
    BOOL bRet = FALSE;
    while ((bRet = ::GetMessage(&msg, 0, 0, 0)) != 0) {
        if (bRet == -1) {
            // handle the error and possibly exit
            // 忽略这个错误
        }
        else {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }
    return (int32_t)msg.wParam;
}

} // namespace ui

#endif // DUILIB_PLATFORM_WIN

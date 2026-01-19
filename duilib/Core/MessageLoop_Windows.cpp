#include "MessageLoop_Windows.h"

#if defined (DUILIB_BUILD_FOR_WIN)

namespace ui
{
MessageLoop_Windows::MessageLoop_Windows()
{
}

MessageLoop_Windows::~MessageLoop_Windows()
{
}

int32_t MessageLoop_Windows::Run(MessageLoopIdleCallback idleCallback)
{
    if (idleCallback == nullptr) {
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
    else {
        return RunWithIdle(idleCallback);
    }
}

int32_t MessageLoop_Windows::RunWithIdle(MessageLoopIdleCallback idleCallback)
{
    MSG msg = { 0, };
    while (1) {
        BOOL bHasMsg = ::PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
        if (bHasMsg) {
            if (msg.message == WM_QUIT) {
                break;
            }
            // 标准消息处理流程
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        else {
            // 无消息时执行 Idle 处理
            idleCallback();
            if (!::PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
                // 队列中无消息，等待消息（避免CPU空转）
                ::MsgWaitForMultipleObjects(0, nullptr, FALSE, INFINITE, QS_ALLINPUT);
            }
        }
    }
    return (int32_t)msg.wParam;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

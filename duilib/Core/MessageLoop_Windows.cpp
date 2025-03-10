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

int32_t MessageLoop_Windows::Run()
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

#endif // DUILIB_BUILD_FOR_WIN

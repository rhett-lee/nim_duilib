#ifndef UI_CORE_MESSAGE_LOOP_WINDOWS_H_
#define UI_CORE_MESSAGE_LOOP_WINDOWS_H_

#include "duilib/duilib_defs.h"

#if defined (DUILIB_BUILD_FOR_WIN)

namespace ui {

/** 主线程的消息循环
*/
class MessageLoop_Windows
{
public:
    MessageLoop_Windows();
    MessageLoop_Windows(const MessageLoop_Windows& r) = delete;
    MessageLoop_Windows& operator = (const MessageLoop_Windows& r) = delete;
    ~MessageLoop_Windows();

public:
    /** 运行消息循环
    */
    int32_t Run();
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_CORE_MESSAGE_LOOP_WINDOWS_H_

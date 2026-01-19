#ifndef UI_CORE_MESSAGE_LOOP_WINDOWS_H_
#define UI_CORE_MESSAGE_LOOP_WINDOWS_H_

#include "duilib/duilib_defs.h"

#if defined (DUILIB_BUILD_FOR_WIN)
#include <functional>

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
    /** Idle函数的声明
    * @return 返回true表示向消息队列中发送了消息，需要立即处理; 返回false表示未向消息队列中发送消息
    */
    using MessageLoopIdleCallback = std::function<void()>;

public:
    /** 运行消息循环
    */
    int32_t Run(MessageLoopIdleCallback idleCallback);

private:
    /** 支持Idle函数的消息循环
    */
    int32_t RunWithIdle(MessageLoopIdleCallback idleCallback);
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_CORE_MESSAGE_LOOP_WINDOWS_H_

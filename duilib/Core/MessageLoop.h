#ifndef UI_CORE_MESSAGE_LOOP_H_
#define UI_CORE_MESSAGE_LOOP_H_

#include "duilib/duilib_defs.h"

namespace ui {

/** 主线程的消息循环
*/
class MessageLoop
{
public:
    MessageLoop();
    MessageLoop(const MessageLoop& r) = delete;
    MessageLoop& operator = (const MessageLoop& r) = delete;
    ~MessageLoop();

public:
    /** 运行消息循环
    */
    int32_t Run();
};

} // namespace ui

#endif // UI_CORE_MESSAGE_LOOP_H_

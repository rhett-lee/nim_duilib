#ifndef UI_CORE_MESSAGE_LOOP_SDL_H_
#define UI_CORE_MESSAGE_LOOP_SDL_H_

#include "duilib/duilib_defs.h"

#if defined(DUILIB_BUILD_FOR_SDL)

namespace ui {

/** 主线程的消息循环
*/
class MessageLoop_SDL
{
public:
    MessageLoop_SDL();
    MessageLoop_SDL(const MessageLoop_SDL& r) = delete;
    MessageLoop_SDL& operator = (const MessageLoop_SDL& r) = delete;
    ~MessageLoop_SDL();

public:
    /** 运行消息循环
    */
    int32_t Run();
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_SDL

#endif // UI_CORE_MESSAGE_LOOP_SDL_H_

#include "ThreadMessage.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include "MessageLoop_SDL.h"

namespace ui
{
class ThreadMessage::TImpl
{
public:
    /** 消息ID
    */
    uint32_t m_msgId = 0;

    /** 是否已经终止
    */
    bool m_bTerm = false;
};

ThreadMessage::ThreadMessage()
{
    m_impl = new TImpl;
}

ThreadMessage::~ThreadMessage()
{
    Clear();
    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }
}

void ThreadMessage::Initialize(void* /*platformData*/)
{
}

bool ThreadMessage::PostMsg(uint32_t msgId, WPARAM wParam, LPARAM lParam)
{
    if (m_impl->m_bTerm) {
        //已经终止
        return false;
    }
    bool bRet = false;
    ASSERT(msgId == m_impl->m_msgId);
    if (msgId == m_impl->m_msgId) {
        bRet = MessageLoop_SDL::PostUserEvent(msgId, wParam, lParam);
    }
    return bRet;
}

void ThreadMessage::RemoveDuplicateMsg(uint32_t msgId)
{
    //移除队列中多余的消息，避免队列中有大量无用的重复消息，导致无法处理鼠标键盘消息
    ASSERT(msgId == m_impl->m_msgId);
    if (msgId == m_impl->m_msgId) {
        MessageLoop_SDL::RemoveDuplicateMsg(msgId);
    }
}

void ThreadMessage::SetMessageCallback(uint32_t msgId, const ThreadMessageCallback& callback)
{
    if (m_impl->m_msgId != 0) {
        MessageLoop_SDL::RemoveUserMessageCallback(m_impl->m_msgId);
    }
    m_impl->m_msgId = msgId;
    if (m_impl->m_msgId != 0) {
        MessageLoop_SDL::AddUserMessageCallback(m_impl->m_msgId, callback);
    }
}

void ThreadMessage::Clear()
{
    if (m_impl->m_msgId != 0) {
        MessageLoop_SDL::RemoveUserMessageCallback(m_impl->m_msgId);
    }    
    m_impl->m_bTerm = true;
    m_impl->m_msgId = 0;
}

void ThreadMessage::OnUserMessage(uint32_t /*msgId*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_SDL

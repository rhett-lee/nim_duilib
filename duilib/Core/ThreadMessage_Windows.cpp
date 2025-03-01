#include "ThreadMessage.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

namespace ui
{
class ThreadMessage::TImpl
{
public:
    /** 消息窗口函数
    */
    static LRESULT CALLBACK WndProcThunk(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

public:
    /** 消息窗口句柄，用于在UI线程中派发定时器事件
    */
    HWND m_hMessageWnd = nullptr;

    /** 消息回调函数
    */
    ThreadMessageCallback m_callback = nullptr;

    /** 消息ID
    */
    uint32_t m_msgId = 0;

    /** 是否已经终止
    */
    bool m_bTerm = false;
};

LRESULT ThreadMessage::TImpl::WndProcThunk(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message > WM_USER) {
        ThreadMessage* pThis = reinterpret_cast<ThreadMessage*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (pThis != nullptr) {
            pThis->OnUserMessage(message, wparam, lparam);
        }
        return 1;
    }
    return ::DefWindowProcW(hwnd, message, wparam, lparam);
}

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

void ThreadMessage::Initialize(void* platformData)
{
    ASSERT(m_impl->m_hMessageWnd == nullptr);
    if (m_impl->m_hMessageWnd != nullptr) {
        return;
    }
    auto hinst = platformData != nullptr ? (HMODULE)platformData : ::GetModuleHandle(nullptr);
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = ThreadMessage::TImpl::WndProcThunk;
    wc.hInstance = hinst;
    wc.lpszClassName = L"duilib_messaging_window";
    ATOM ret = ::RegisterClassExW(&wc);
    ASSERT_UNUSED_VARIABLE(ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    m_impl->m_hMessageWnd = ::CreateWindowW(wc.lpszClassName, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, hinst, 0);
    if (::IsWindow(m_impl->m_hMessageWnd)) {
        ::SetWindowLongPtr(m_impl->m_hMessageWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
    }
}

bool ThreadMessage::PostMsg(uint32_t msgId, WPARAM wParam, LPARAM lParam)
{
    if (m_impl->m_bTerm) {
        //已经终止
        return false;
    }
    bool bRet = false;
    ASSERT(msgId == m_impl->m_msgId);
    ASSERT(m_impl->m_hMessageWnd != nullptr);
    if ((m_impl->m_hMessageWnd != nullptr) && (msgId == m_impl->m_msgId)) {
        bRet = ::PostMessage(m_impl->m_hMessageWnd, m_impl->m_msgId, wParam, lParam) != FALSE;
    }
    return bRet;
}

void ThreadMessage::RemoveDuplicateMsg(uint32_t msgId)
{
    //移除队列中多余的消息，避免队列中有大量无用的重复消息，导致无法处理鼠标键盘消息
    ASSERT(msgId == m_impl->m_msgId);
    ASSERT(m_impl->m_hMessageWnd != nullptr);
    if ((m_impl->m_hMessageWnd != nullptr) && (msgId == m_impl->m_msgId)) {
        MSG msg;
        while (::PeekMessage(&msg, m_impl->m_hMessageWnd, m_impl->m_msgId, m_impl->m_msgId, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                //检测到退出消息，重新放到消息队列中，避免进程退不出
                ::PostQuitMessage(static_cast<int>(msg.wParam));
                break;
            }
            else {
                ASSERT(msg.message == m_impl->m_msgId);
            }
        }
    }
}

void ThreadMessage::SetMessageCallback(uint32_t msgId, const ThreadMessageCallback& callback)
{
    m_impl->m_msgId = msgId;
    m_impl->m_callback = callback;
}

void ThreadMessage::Clear()
{
    m_impl->m_bTerm = true;
    m_impl->m_msgId = 0;
    m_impl->m_callback = nullptr;
    if (m_impl->m_hMessageWnd != nullptr) {
        ::DestroyWindow(m_impl->m_hMessageWnd);
        m_impl->m_hMessageWnd = nullptr;
    }
}

void ThreadMessage::OnUserMessage(uint32_t msgId, WPARAM wParam, LPARAM lParam)
{
    ASSERT(m_impl->m_msgId == msgId);
    if ((m_impl->m_msgId == msgId) && (m_impl->m_callback != nullptr)) {
        m_impl->m_callback(msgId, wParam, lParam);
    }
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

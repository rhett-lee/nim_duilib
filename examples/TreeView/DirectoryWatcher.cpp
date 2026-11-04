#include "DirectoryWatcher.h"

#if defined (DUILIB_BUILD_FOR_WIN)
    #include <windows.h>
    #include <atomic>
    #include <thread>
    #include <vector>
#endif

namespace
{
#if defined (DUILIB_BUILD_FOR_WIN)
/** 目录内容连续变化时的"安静期"时长（毫秒）：
* 连续变化期间不做刷新，等变化停止后再通知一次，避免频繁刷新界面
*/
const int32_t kChangeDebounceMs = 400;

/** 接收目录变化通知的缓冲区大小（字节）
*/
const uint32_t kNotifyBufferSize = 16 * 1024;
#endif
}

struct DirectoryWatcher::Impl
{
#if defined (DUILIB_BUILD_FOR_WIN)
    /** 被监听的目录句柄（按异步方式打开）
    */
    HANDLE m_hDirectory = nullptr;

    /** 退出事件：通知监听线程退出
    */
    HANDLE m_hStopEvent = nullptr;

    /** 目录变化的 I/O 完成事件
    */
    HANDLE m_hIoEvent = nullptr;

    /** 监听线程
    */
    std::thread m_thread;

    /** 是否请求退出（监听线程会检查该标记）
    */
    std::atomic<bool> m_bStop = false;

    /** 是否正在监听
    */
    std::atomic<bool> m_bWatching = false;

    /** 被监听的目录（用于判断是否重复监听同一个目录）
    */
    DString m_directory;

    /** 目录内容发生变化时的回调函数
    */
    DirectoryWatcher::ChangeCallback m_callback;

    /** 接收目录变化通知的缓冲区
    */
    std::vector<uint8_t> m_notifyBuffer;

    /** 监听线程的处理流程
    */
    void ThreadProc();

    /** 释放监听相关的资源（在监听线程退出后调用）
    */
    void Release();
#endif
};

DirectoryWatcher::DirectoryWatcher()
{
}

DirectoryWatcher::~DirectoryWatcher()
{
    Stop();
}

bool DirectoryWatcher::Start(const ui::FilePath& directory, const ChangeCallback& callback)
{
#if defined (DUILIB_BUILD_FOR_WIN)
    if ((callback == nullptr) || directory.IsEmpty() || !directory.IsExistsDirectory()) {
        Stop();
        return false;
    }
    const DString directoryPath = directory.NativePath();
    if ((m_pImpl != nullptr) && m_pImpl->m_bWatching && (m_pImpl->m_directory == directoryPath)) {
        //当前监听的目录没有变化：不需要重启监听线程
        return true;
    }
    //切换监听的目录：先停止之前的监听
    Stop();
    if (m_pImpl == nullptr) {
        m_pImpl = std::make_unique<Impl>();
    }

    //打开被监听的目录：只需要读取目录内容的权限，并且按异步方式打开（用于Overlapped I/O）
    HANDLE hDirectory = ::CreateFileW(directoryPath.c_str(), FILE_LIST_DIRECTORY,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                      nullptr, OPEN_EXISTING,
                                      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
    if (hDirectory == INVALID_HANDLE_VALUE) {
        return false;
    }
    HANDLE hStopEvent = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
    HANDLE hIoEvent = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if ((hStopEvent == nullptr) || (hIoEvent == nullptr)) {
        ::CloseHandle(hDirectory);
        if (hStopEvent != nullptr) {
            ::CloseHandle(hStopEvent);
        }
        if (hIoEvent != nullptr) {
            ::CloseHandle(hIoEvent);
        }
        return false;
    }

    m_pImpl->m_hDirectory = hDirectory;
    m_pImpl->m_hStopEvent = hStopEvent;
    m_pImpl->m_hIoEvent = hIoEvent;
    m_pImpl->m_directory = directoryPath;
    m_pImpl->m_callback = callback;
    m_pImpl->m_bStop = false;
    m_pImpl->m_bWatching = true;
    Impl* pImpl = m_pImpl.get();
    m_pImpl->m_thread = std::thread([pImpl]() {
        pImpl->ThreadProc();
        });
    return true;
#else
    //SDL 没有提供文件系统变化监听的接口，其他平台不做实现
    (void)directory;
    (void)callback;
    return false;
#endif
}

void DirectoryWatcher::Stop()
{
#if defined (DUILIB_BUILD_FOR_WIN)
    if (m_pImpl == nullptr) {
        return;
    }
    Impl* pImpl = m_pImpl.get();
    if (pImpl->m_thread.joinable()) {
        //通知监听线程退出，并等待其退出（避免回调在对象销毁后被执行）
        pImpl->m_bStop = true;
        pImpl->m_bWatching = false;
        if (pImpl->m_hStopEvent != nullptr) {
            ::SetEvent(pImpl->m_hStopEvent);
        }
        if (pImpl->m_thread.get_id() != std::this_thread::get_id()) {
            pImpl->m_thread.join();
        }
    }
    pImpl->Release();
#endif
}

bool DirectoryWatcher::IsWatching() const
{
#if defined (DUILIB_BUILD_FOR_WIN)
    return (m_pImpl != nullptr) && m_pImpl->m_bWatching;
#else
    return false;
#endif
}

#if defined (DUILIB_BUILD_FOR_WIN)
void DirectoryWatcher::Impl::ThreadProc()
{
    //需要监听的变化类型：文件/目录的新增、删除、改名
    //（不监听文件内容的修改：目录中存在日志类文件被持续写入时，会导致界面频繁刷新）
    const DWORD nWatchFlags = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;
    m_notifyBuffer.resize(kNotifyBufferSize);

    OVERLAPPED overlapped = {};
    overlapped.hEvent = m_hIoEvent;
    while (!m_bStop) {
        ::ResetEvent(m_hIoEvent);
        DWORD nBytes = 0;
        if (!::ReadDirectoryChangesW(m_hDirectory, m_notifyBuffer.data(), kNotifyBufferSize, FALSE,
                                     nWatchFlags, &nBytes, &overlapped, nullptr)) {
            //无法发起监听（比如目录已被删除），结束监听
            break;
        }
        //等待：目录发生变化，或者被要求停止监听
        const HANDLE handles[] = { m_hStopEvent, m_hIoEvent };
        const DWORD nWait = ::WaitForMultipleObjects(2, handles, FALSE, INFINITE);
        if (nWait == WAIT_OBJECT_0) {
            //请求停止：取消本次挂起的读取
            ::CancelIoEx(m_hDirectory, &overlapped);
            break;
        }
        if (nWait != (WAIT_OBJECT_0 + 1)) {
            //等待失败或者I/O出错，结束监听
            break;
        }
        //目录发生了变化：等变化停止后再通知一次（避免连续变化时反复刷新界面）
        if (::WaitForSingleObject(m_hStopEvent, kChangeDebounceMs) == WAIT_OBJECT_0) {
            break;
        }
        const DirectoryWatcher::ChangeCallback callback = m_callback;
        if (callback != nullptr) {
            //切换到UI线程执行回调，UI线程中刷新界面
            ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, [callback]() {
                callback();
                });
        }
    }
    m_bWatching = false;
}

void DirectoryWatcher::Impl::Release()
{
    m_callback = nullptr;
    m_directory.clear();
    if (m_hDirectory != nullptr) {
        ::CloseHandle(m_hDirectory);
        m_hDirectory = nullptr;
    }
    if (m_hStopEvent != nullptr) {
        ::CloseHandle(m_hStopEvent);
        m_hStopEvent = nullptr;
    }
    if (m_hIoEvent != nullptr) {
        ::CloseHandle(m_hIoEvent);
        m_hIoEvent = nullptr;
    }
}
#endif

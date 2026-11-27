#ifndef UI_UTILS_PROCESS_SINGLETON_WINDOWS_H_
#define UI_UTILS_PROCESS_SINGLETON_WINDOWS_H_

#include "ProcessSingletonData.h"
#include "StringConvert.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include <wincrypt.h>
#include <sddl.h>
#include <userenv.h>

namespace ui
{
/** 跨进程单例的实现（通信部分, Windows实现）
*  1. 使用命名互斥体（名称包含当前用户SID）检测是否已有实例在运行；
*  2. 使用命名管道（重叠I/O）把新进程的启动参数传递给已运行的实例。
*/
class DUILIB_API ProcessSingletonImpl : public ProcessSingleton
{
public:
    explicit ProcessSingletonImpl(const std::string& strAppName) :
        ProcessSingleton(strAppName)
    {
        InitializePlatformComponents();
    }

    virtual ~ProcessSingletonImpl() override
    {
        m_bRunning = false;
        CleanupPlatformComponents();
    }

protected:
    ProcessSingletonImpl(const ProcessSingleton&) = delete;
    ProcessSingletonImpl& operator=(const ProcessSingletonImpl&) = delete;

public:
    /** 初始化功能组件：创建命名互斥体和用于退出监听的取消事件
    * @note 创建失败时抛出 std::system_error 等异常，并先清理已创建的资源
    */
    virtual void InitializePlatformComponents() override final
    {
        try {
            SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES) };
            sa.bInheritHandle = FALSE;

            // 设置低完整性级别
            ::ConvertStringSecurityDescriptorToSecurityDescriptorW(L"S:(ML;;NW;;;LW)", SDDL_REVISION_1, &sa.lpSecurityDescriptor, nullptr);
            std::wstring wstrMutexName = GetUserSpecificName();
            m_hMutex = ::CreateMutexW(&sa, FALSE, wstrMutexName.c_str());
            m_dwLastError = ::GetLastError();
            if (!m_hMutex) {
                ::LocalFree(sa.lpSecurityDescriptor);
                throw std::system_error(GetLastError(), std::system_category(), "CreateMutex failed");
            }
            ::LocalFree(sa.lpSecurityDescriptor);

            if (m_hCancelEvent == nullptr) {
                m_hCancelEvent = ::CreateEvent(nullptr, TRUE, FALSE, nullptr); // 手动重置的取消事件                
                ASSERT(m_hCancelEvent != nullptr);
                if (m_hCancelEvent) {
                    ::ResetEvent(m_hCancelEvent);
                }
            }
        }
        catch (const std::exception& /*ex*/) {
            CleanupPlatformComponents();
            throw;
        }
    }

    /** 检查是否有其他进程正在运行
    * @return 返回 true 表示互斥体已存在（已有实例在运行），否则返回 false
    */
    virtual bool PlatformCheckInstance() override final
    {
        return m_dwLastError == ERROR_ALREADY_EXISTS;
    }

    /** 向已经运行的进程发送数据（通过命名管道）
    * @param [in] strData 已经序列化的协议数据
    * @return 返回 true 表示数据全部写入成功，否则返回 false
    */
    virtual bool PlatformSendData(const std::string& strData) override final
    {
        try {
            std::wstring wstrPipeName = L"\\\\.\\pipe\\" + GetUserSpecificName();
            HANDLE hPipe = ::CreateFileW(wstrPipeName.c_str(),
                                        GENERIC_WRITE,
                                        0,
                                        nullptr,
                                        OPEN_EXISTING,
                                        FILE_FLAG_OVERLAPPED,
                                        nullptr);
            if (hPipe == INVALID_HANDLE_VALUE) {
                return false;
            }

            OVERLAPPED overlapped = { 0 };
            overlapped.hEvent = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
            if (overlapped.hEvent == nullptr) {
                ::CloseHandle(hPipe);
                return false;
            }

            DWORD dwWritten = 0;
            bool bSendOk = false;
            if (::WriteFile(hPipe, strData.data(), static_cast<DWORD>(strData.size()), &dwWritten, &overlapped)) {
                bSendOk = (dwWritten == static_cast<DWORD>(strData.size()));
            }
            else if (::GetLastError() == ERROR_IO_PENDING) {
                //等待异步写完成后，需要通过GetOverlappedResult确认实际结果，避免写入失败也返回成功
                ::WaitForSingleObject(overlapped.hEvent, INFINITE);
                if (::GetOverlappedResult(hPipe, &overlapped, &dwWritten, FALSE)) {
                    bSendOk = (dwWritten == static_cast<DWORD>(strData.size()));
                }
            }

            ::CloseHandle(hPipe);
            ::CloseHandle(overlapped.hEvent);
            return bSendOk;
        }
        catch (const std::exception& ex) {
            LogError("Windows send error: " + std::string(ex.what()));
            return false;
        }
    }

    /** 监听其他进程启动的事件：在循环中创建命名管道，接收并解析数据后通过回调通知调用方
    */
    virtual void PlatformListen() override final
    {
        while (m_bRunning) {
            try {
                std::wstring wstrPipeName = L"\\\\.\\pipe\\" + GetUserSpecificName();
                m_hPipe = ::CreateNamedPipeW(wstrPipeName.c_str(),
                                             PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                                             PIPE_TYPE_BYTE | PIPE_WAIT,
                                             PIPE_UNLIMITED_INSTANCES,
                                             ProcessSingletonData::MAX_DATA_SIZE + sizeof(ProcessSingletonData::ProtocolHeader),
                                             ProcessSingletonData::MAX_DATA_SIZE + sizeof(ProcessSingletonData::ProtocolHeader),
                                             0,
                                             nullptr);

                if (m_hPipe == INVALID_HANDLE_VALUE) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }

                OVERLAPPED overlapped = { 0 };
                overlapped.hEvent = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
                if (overlapped.hEvent == nullptr) {
                    ::CloseHandle(m_hPipe);
                    m_hPipe = INVALID_HANDLE_VALUE;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }

                bool bConnected = true;
                if (!::ConnectNamedPipe(m_hPipe, &overlapped)) {
                    const DWORD dwConnectError = ::GetLastError();
                    if (dwConnectError == ERROR_IO_PENDING) {
                        if (!WaitForPipe(m_hPipe, overlapped, m_hCancelEvent)) {
                            bConnected = false;
                        }
                    }
                    else if (dwConnectError == ERROR_PIPE_CONNECTED) {
                        //客户端在ConnectNamedPipe调用之前已经连接上，属于正常情况，不能当作错误
                        bConnected = true;
                    }
                    else {
                        ::CloseHandle(m_hPipe);
                        m_hPipe = INVALID_HANDLE_VALUE;
                        continue;
                    }
                }

                char pBuffer[ProcessSingletonData::MAX_DATA_SIZE + sizeof(ProcessSingletonData::ProtocolHeader)] = { 0 };
                DWORD dwRead = 0;
                bool bReadOk = false;
                if (bConnected && m_bRunning) {
                    if (::ReadFile(m_hPipe, pBuffer, sizeof(pBuffer), &dwRead, &overlapped)) {
                        bReadOk = true;
                    }
                    else if (::GetLastError() == ERROR_IO_PENDING) {
                        //重叠I/O下，ReadFile通常返回ERROR_IO_PENDING，需要等待完成并取回实际读取的字节数。
                        //原实现把这种情况直接当作读取失败，导致新进程发送的参数被静默丢弃
                        bReadOk = WaitForPipe(m_hPipe, overlapped, m_hCancelEvent, &dwRead);
                    }
                }
                if (bReadOk && (dwRead > 0)) {
                    try {
                        auto vecArgs = ProcessSingletonData::DeserializeData(std::string(pBuffer, dwRead));
                        OnAlreadyRunningAppRelaunch(vecArgs);
                    }
                    catch (const std::exception& ex) {
                        LogError("Invalid data received: " + std::string(ex.what()));
                    }
                }

                ::DisconnectNamedPipe(m_hPipe);
                ::CloseHandle(m_hPipe);
                ::CloseHandle(overlapped.hEvent);
                m_hPipe = INVALID_HANDLE_VALUE;
            }
            catch (const std::exception& ex) {
                LogError("Windows listener error: " + std::string(ex.what()));
            }
        }
    }
    
    /** 清理功能组件：通知并等待监听线程退出，然后关闭取消事件和互斥体句柄
    */
    virtual void CleanupPlatformComponents() override final
    {
        m_bRunning = false;
        if (m_hCancelEvent) {
            ::SetEvent(m_hCancelEvent);
        }        
        if (m_thListener.joinable()) {
            m_thListener.join();
        }
        if (m_hCancelEvent != nullptr) {
            ::CloseHandle(m_hCancelEvent);
            m_hCancelEvent = nullptr;
        }
        if (m_hMutex) {
            ::CloseHandle(m_hMutex);
            m_hMutex = nullptr;
        }
    }
    
private:
    /** 等待重叠I/O操作完成（同时监听取消事件）
    * @param [in] hPipe 命名管道句柄
    * @param [in,out] overlapped 重叠I/O结构体（其中 hEvent 为操作完成事件）
    * @param [in] hCancelEvent 取消事件句柄，触发时取消当前I/O操作
    * @param [out] pBytesTransferred 返回实际传输的字节数，可为 nullptr
    * @return 返回 true 表示操作完成且成功，否则返回 false（被取消或出错）
    */
    bool WaitForPipe(HANDLE hPipe, OVERLAPPED& overlapped, HANDLE hCancelEvent, DWORD* pBytesTransferred = nullptr) const
    {
        HANDLE waitHandles[2] = {overlapped.hEvent, hCancelEvent};
        DWORD waitResult = ::WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

        bool bRet = false;
        switch (waitResult) {
            case WAIT_OBJECT_0: // 连接完成或取消
                {
                    DWORD bytesTransferred = 0;
                    BOOL success = ::GetOverlappedResult(hPipe, &overlapped, &bytesTransferred, FALSE);
                    if (!success) {
                        // 操作被取消或其他错误
                        bRet = false;
                    }
                    else {
                        // 操作成功处理
                        bRet = true;
                        if (pBytesTransferred != nullptr) {
                            *pBytesTransferred = bytesTransferred;
                        }
                    }
                    break;
                }
            case WAIT_OBJECT_0 + 1: // 取消事件触发
                {
                    // 取消I/O操作
                    ::CancelIo(hPipe);

                    // 等待重叠操作完成
                    ::WaitForSingleObject(overlapped.hEvent, INFINITE);
                    DWORD error = ::GetLastError();
                    if (error == ERROR_OPERATION_ABORTED) {
                        // 确认已取消
                        bRet = false;
                    }
                    break;
                }
            default:
                // 处理等待错误
                break;
        }
        return bRet;
    }

    /** 获取包含当前用户SID的唯一名称（用于互斥体和管道命名）
    * @return 返回 "Local\\应用名_SID" 形式的名称（应用名从UTF-8转换为UTF-16）
    * @throw std::system_error 或 std::runtime_error 获取用户SID失败时抛出
    */
    std::wstring GetUserSpecificName() 
    {
        HANDLE hToken = nullptr;
        if (!::OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken)) {
            throw std::system_error(GetLastError(), std::system_category(), "OpenProcessToken failed");
        }

        DWORD dwSize = 0;
        SECURITY_INFORMATION siRequested = OWNER_SECURITY_INFORMATION;
        ::GetUserObjectSecurity(hToken, &siRequested, nullptr, 0, &dwSize);
        if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            ::CloseHandle(hToken);
            throw std::system_error(GetLastError(), std::system_category(), "GetUserObjectSecurity failed");
        }

        std::vector<BYTE> vecBuffer(dwSize);
        PSECURITY_DESCRIPTOR pSD = reinterpret_cast<PSECURITY_DESCRIPTOR>(vecBuffer.data());
        siRequested = OWNER_SECURITY_INFORMATION;
        if (!::GetUserObjectSecurity(hToken, &siRequested, pSD, dwSize, &dwSize)) {
            ::CloseHandle(hToken);
            throw std::system_error(GetLastError(), std::system_category(), "GetUserObjectSecurity failed");
        }

        PSID pSid = nullptr;
        BOOL bOwnerDefaulted = FALSE;
        if (!::GetSecurityDescriptorOwner(pSD, &pSid, &bOwnerDefaulted) || pSid == nullptr) {
            ::CloseHandle(hToken);
            throw std::runtime_error("GetSecurityDescriptorOwner failed");
        }

        LPWSTR pszSid = nullptr;
        if (!::ConvertSidToStringSidW(pSid, &pszSid)) {
            ::CloseHandle(hToken);
            throw std::system_error(GetLastError(), std::system_category(), "ConvertSidToStringSid failed");
        }

        std::wstring wstrSid(pszSid);
        ::LocalFree(pszSid);
        ::CloseHandle(hToken);

        //m_strAppName 是UTF-8编码的字符串，需要先转换为UTF-16再拼接，避免非ASCII应用名变成乱码
        return L"Local\\" + StringConvert::UTF8ToWString(m_strAppName) + L"_" + wstrSid;
    }

private:
    // Windows实现
    HANDLE m_hMutex = nullptr;
    HANDLE m_hPipe = INVALID_HANDLE_VALUE;
    DWORD m_dwLastError = 0;
    HANDLE m_hCancelEvent = nullptr;
};

}

#endif //DUILIB_BUILD_FOR_WIN

#endif // UI_UTILS_PROCESS_SINGLETON_WINDOWS_H_

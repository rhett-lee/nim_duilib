#ifndef UI_UTILS_PROCESS_SINGLETON_LINUX_H_
#define UI_UTILS_PROCESS_SINGLETON_LINUX_H_

#include "ProcessSingletonData.h"

#if defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)

#include <sys/file.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pwd.h>

namespace ui
{
/** 跨进程单例的实现（通信部分，Linux实现）
*  1. 使用用户运行时目录下的锁文件（flock）检测是否已有实例在运行；
*  2. 使用 Unix 域套接字（SOCK_STREAM）把新进程的启动参数传递给已运行的实例。
*/
class DUILIB_API ProcessSingletonImpl: public ProcessSingleton
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
    /** 初始化功能组件：创建运行时目录和锁文件，并尝试加排他锁
    * @note 锁已被其他进程占用（已有实例在运行）时直接返回，不抛出异常；其他错误抛出 std::system_error
    */
    virtual void InitializePlatformComponents() override final
    {
        try {
            std::string strRuntimeDir = GetUserRuntimePath();
            CreateDirectoryRecursive(strRuntimeDir);

            std::string strLockFile = strRuntimeDir + "/" + m_strAppName + ".lock";
            m_nLockFile = ::open(strLockFile.c_str(), O_RDWR | O_CREAT, 0600);
            if (m_nLockFile == -1) {
                throw std::system_error(errno, std::system_category(), "Open lock file failed");
            }

            if (::flock(m_nLockFile, LOCK_EX | LOCK_NB) == -1) {
                if (errno == EWOULDBLOCK) {
                    return;
                }
                throw std::system_error(errno, std::system_category(), "File lock failed");
            }

            // 设置文件权限
            ::fchmod(m_nLockFile, 0600);
        }
        catch (const std::exception& ex) {
            CleanupPlatformComponents();
            throw;
        }
    }

    /** 检查是否有其他进程正在运行
    * @return 返回 true 表示锁已被其他进程占用（已有实例在运行），否则返回 false
    */
    virtual bool PlatformCheckInstance() override final
    {
        return errno == EWOULDBLOCK;
    }

    /** 向已经运行的进程发送数据（通过 Unix 域套接字）
    * @param [in] strData 已经序列化的协议数据
    * @return 返回 true 表示数据全部写入成功，否则返回 false（如套接字连接失败）
    */
    virtual bool PlatformSendData(const std::string& strData) override final
    {
        try {
            int nSocket = ::socket(AF_UNIX, SOCK_STREAM, 0);
            if (nSocket == -1) {
                throw std::system_error(errno, std::system_category(), "Socket creation failed");
            }

            sockaddr_un addr = { 0 };
            addr.sun_family = AF_UNIX;
            std::string strSocketPath = GetUserRuntimePath() + "/" + m_strAppName + ".sock";
            ::strncpy(addr.sun_path, strSocketPath.c_str(), sizeof(addr.sun_path) - 1);

            if (::connect(nSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
                ::close(nSocket);
                return false;
            }

            ssize_t nSent = ::write(nSocket, strData.data(), strData.size());
            ::close(nSocket);
            return nSent == static_cast<ssize_t>(strData.size());
        }
        catch (const std::exception& ex) {
            LogError("Linux send error: " + std::string(ex.what()));
            return false;
        }
    }

    /** 监听其他进程启动的事件：创建并绑定 Unix 域套接字，接收并解析数据后通过回调通知调用方
    */
    virtual void PlatformListen() override final
    {
        try {
            m_nSocket = ::socket(AF_UNIX, SOCK_STREAM, 0);
            if (m_nSocket == -1) {
                throw std::system_error(errno, std::system_category(), "Socket creation failed");
            }

            sockaddr_un addr = { 0 };
            addr.sun_family = AF_UNIX;
            std::string strSocketPath = GetUserRuntimePath() + "/" + m_strAppName + ".sock";
            ::strncpy(addr.sun_path, strSocketPath.c_str(), sizeof(addr.sun_path) - 1);

            ::unlink(strSocketPath.c_str());
            if (::bind(m_nSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
                ::close(m_nSocket);
                throw std::system_error(errno, std::system_category(), "Socket bind failed");
            }

            ::fchmod(m_nSocket, 0600);
            if (::listen(m_nSocket, 5) == -1) {
                ::close(m_nSocket);
                throw std::system_error(errno, std::system_category(), "Socket listen failed");
            }

            while (m_bRunning) {
                int nClient = ::accept(m_nSocket, nullptr, nullptr);
                if (nClient == -1) continue;

                char pBuffer[ProcessSingletonData::MAX_DATA_SIZE + sizeof(ProcessSingletonData::ProtocolHeader)] = { 0 };
                ssize_t nRead = ::recv(nClient, pBuffer, sizeof(pBuffer), MSG_NOSIGNAL);
                if (nRead > 0) {
                    try {
                        auto vecArgs = ProcessSingletonData::DeserializeData(std::string(pBuffer, nRead));
                        OnAlreadyRunningAppRelaunch(vecArgs);
                    }
                    catch (const std::exception& ex) {
                        LogError("Invalid data received: " + std::string(ex.what()));
                    }
                }
                ::close(nClient);
            }

            ::close(m_nSocket);
            ::unlink(strSocketPath.c_str());
        }
        catch (const std::exception& ex) {
            LogError("Linux listener error: " + std::string(ex.what()));
        }
    }
    
    /** 清理功能组件：解锁并关闭锁文件、关闭监听套接字，并等待监听线程退出
    */
    virtual void CleanupPlatformComponents() override final
    {
        if (m_nLockFile != -1) {
            ::flock(m_nLockFile, LOCK_UN);
            ::close(m_nLockFile);
            m_nLockFile = -1;
        }
        if (m_nSocket != -1) {
            ::close(m_nSocket);
            m_nSocket = -1;
        }
        if (m_thListener.joinable()) {
            m_thListener.join();
        }
    }

private:
    /** 获取当前用户的运行时目录（优先 $XDG_RUNTIME_DIR，其次 $HOME/.cache，最后回退到 /tmp）
    * @return 返回运行时目录的路径（不保证目录已存在）
    */
    std::string GetUserRuntimePath() 
    {
        const char* pszXdgRuntimeDir = ::getenv("XDG_RUNTIME_DIR");
        if (pszXdgRuntimeDir && *pszXdgRuntimeDir) {
            return std::string(pszXdgRuntimeDir) + "/.cppapp";
        }

        struct passwd* pwd = ::getpwuid(getuid());
        if (pwd && pwd->pw_dir) {
            return std::string(pwd->pw_dir) + "/.cache/.cppapp";
        }

        return "/tmp/.cppapp_" + std::to_string(getuid());
    }
    
    /** 逐级创建目录（权限 0700），已存在的目录会被跳过
    * @param [in] strPath 需要创建的目录路径
    * @throw std::system_error 创建目录失败时抛出
    */
    static void CreateDirectoryRecursive(const std::string& strPath)
    {
        std::string::size_type nPos = 0;
        do {
            nPos = strPath.find('/', nPos + 1);
            std::string strSubDir = strPath.substr(0, nPos);
            if (::mkdir(strSubDir.c_str(), 0700) == -1 && errno != EEXIST) {
                throw std::system_error(errno, std::system_category(), "Create directory failed");
            }
        } while (nPos != std::string::npos);
    }
private:
    // Linux实现
    int m_nLockFile = -1;
    int m_nSocket = -1;
};

}

#endif //DUILIB_BUILD_FOR_LINUX
#endif // UI_UTILS_PROCESS_SINGLETON_LINUX_H_

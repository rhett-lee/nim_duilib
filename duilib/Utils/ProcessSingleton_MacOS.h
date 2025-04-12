#ifndef UI_UTILS_PROCESS_SINGLETON_MACOS_H_
#define UI_UTILS_PROCESS_SINGLETON_MACOS_H_

#include "ProcessSingletonData.h"

#ifdef DUILIB_BUILD_FOR_MACOS

#include <sys/file.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pwd.h>
#include <libproc.h>  // macOS 特有的进程相关头文件

namespace ui
{
/** 跨进程单例的实现（macOS实现）
*/
class UILIB_API ProcessSingletonImpl: public ProcessSingleton
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
    virtual void InitializePlatformComponents() override final
    {
        try {
            std::string strRuntimeDir = GetUserRuntimePath();
            CreateDirectoryRecursive(strRuntimeDir);

            // macOS 需要更严格的临时文件路径
            std::string strLockFile = strRuntimeDir + "/" + m_strAppName + ".lock";
            
            // macOS 需要先 unlink 已存在的文件
            ::unlink(strLockFile.c_str());
            
            m_nLockFile = ::open(strLockFile.c_str(), O_RDWR | O_CREAT, 0600);
            if (m_nLockFile == -1) {
                throw std::system_error(errno, std::system_category(), "Open lock file failed");
            }

            // macOS 推荐使用 fcntl 而非 flock
            struct flock fl;
            fl.l_start = 0;
            fl.l_len = 0;
            fl.l_pid = getpid();
            fl.l_type = F_WRLCK;
            fl.l_whence = SEEK_SET;
            
            if (::fcntl(m_nLockFile, F_SETLK, &fl) == -1) {
                if (errno == EAGAIN || errno == EACCES) {
                    return;  // 已存在实例
                }
                throw std::system_error(errno, std::system_category(), "File lock failed");
            }

            // macOS 需要显式设置文件权限
            ::fchmod(m_nLockFile, 0600);
        }
        catch (const std::exception& ex) {
            CleanupPlatformComponents();
            throw;
        }
    }

    virtual bool PlatformCheckInstance() override final
    {
        return errno == EAGAIN || errno == EACCES;
    }

    virtual bool PlatformSendData(const std::string& strData) override final
    {
        try {
            int nSocket = ::socket(AF_UNIX, SOCK_STREAM, 0);
            if (nSocket == -1) {
                throw std::system_error(errno, std::system_category(), "Socket creation failed");
            }

            sockaddr_un addr = { 0 };
            addr.sun_family = AF_UNIX;
            
            // macOS 对 socket 路径长度限制更严格
            std::string strSocketPath = GetUserRuntimePath() + "/" + m_strAppName + ".sock";
            if (strSocketPath.size() > sizeof(addr.sun_path) - 1) {
                strSocketPath = "/tmp/" + m_strAppName + ".sock";  // 回退到/tmp
            }
            
            ::strlcpy(addr.sun_path, strSocketPath.c_str(), sizeof(addr.sun_path));

            if (::connect(nSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
                ::close(nSocket);
                return false;
            }

            ssize_t nSent = ::write(nSocket, strData.data(), strData.size());
            ::close(nSocket);
            return nSent == static_cast<ssize_t>(strData.size());
        }
        catch (const std::exception& ex) {
            LogError("macOS send error: " + std::string(ex.what()));
            return false;
        }
    }

    virtual void PlatformListen() override final
    {
        try {
            m_nSocket = ::socket(AF_UNIX, SOCK_STREAM, 0);
            if (m_nSocket == -1) {
                throw std::system_error(errno, std::system_category(), "Socket creation failed");
            }

            sockaddr_un addr = { 0 };
            addr.sun_family = AF_UNIX;
            
            // 处理 macOS 的 socket 路径长度限制
            std::string strSocketPath = GetUserRuntimePath() + "/" + m_strAppName + ".sock";
            if (strSocketPath.size() > sizeof(addr.sun_path) - 1) {
                strSocketPath = "/tmp/" + m_strAppName + ".sock";
            }
            
            ::strlcpy(addr.sun_path, strSocketPath.c_str(), sizeof(addr.sun_path));

            // macOS 需要先 unlink 已存在的 socket
            ::unlink(strSocketPath.c_str());
            
            if (::bind(m_nSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
                ::close(m_nSocket);
                throw std::system_error(errno, std::system_category(), "Socket bind failed");
            }

            // macOS 需要显式设置权限
            ::fchmod(m_nSocket, 0600);
            
            if (::listen(m_nSocket, 5) == -1) {
                ::close(m_nSocket);
                throw std::system_error(errno, std::system_category(), "Socket listen failed");
            }

            while (m_bRunning) {
                int nClient = ::accept(m_nSocket, nullptr, nullptr);
                if (nClient == -1) continue;

                char pBuffer[ProcessSingletonData::MAX_DATA_SIZE + sizeof(ProcessSingletonData::ProtocolHeader)] = { 0 };
                ssize_t nRead = ::recv(nClient, pBuffer, sizeof(pBuffer), 0);  // macOS 不需要 MSG_NOSIGNAL
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
            LogError("macOS listener error: " + std::string(ex.what()));
        }
    }
    
    virtual void CleanupPlatformComponents() override final
    {
        if (m_nLockFile != -1) {
            // macOS 使用 fcntl 解锁
            struct flock fl;
            fl.l_start = 0;
            fl.l_len = 0;
            fl.l_pid = getpid();
            fl.l_type = F_UNLCK;
            fl.l_whence = SEEK_SET;
            ::fcntl(m_nLockFile, F_SETLK, &fl);
            
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
    std::string GetUserRuntimePath() 
    {
        // macOS 优先使用 $TMPDIR
        const char* pszTmpDir = ::getenv("TMPDIR");
        if (pszTmpDir && *pszTmpDir) {
            return std::string(pszTmpDir) + ".cppapp_" + m_strAppName;
        }

        // 其次是 $HOME/Library/Caches
        struct passwd* pwd = ::getpwuid(getuid());
        if (pwd && pwd->pw_dir) {
            return std::string(pwd->pw_dir) + "/Library/Caches/.cppapp_" + m_strAppName;
        }

        // 最后回退到 /tmp
        return "/tmp/.cppapp_" + m_strAppName + "_" + std::to_string(getuid());
    }
    
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
    // macOS 实现
    int m_nLockFile = -1;
    int m_nSocket = -1;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_MACOS
#endif // UI_UTILS_PROCESS_SINGLETON_MACOS_H_

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

    virtual bool PlatformCheckInstance() override final
    {
        return errno == EWOULDBLOCK;
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

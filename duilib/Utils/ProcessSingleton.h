#ifndef UI_UTILS_PROCESS_SINGLETON_H_
#define UI_UTILS_PROCESS_SINGLETON_H_

#include "duilib/duilib_defs.h"

#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <memory>

namespace ui
{
/** 跨进程单例控制的回调函数
 * @param [in] argumentList 新启动的进程的启动参数列表（通过跨进程通信传递到已经运行的进程中）
 */
typedef std::function<void (const std::vector<DString>& argumentList)> OnAlreadyRunningAppRelaunchEvent;

/** 跨进程单例的实现
*/
class UILIB_API ProcessSingleton
{
protected:
    explicit ProcessSingleton(const std::string& strAppName);
    ProcessSingleton(const ProcessSingleton&) = delete;
    ProcessSingleton& operator=(const ProcessSingleton&) = delete;

public:
    virtual ~ProcessSingleton();

    /** 创建实例
    * @param strAppName 用以标识一个应用的字符串，标识相同则被视为同一个程序
    */
    static std::unique_ptr<ProcessSingleton> Create(const DString& strAppName);

public:
    /** 是否有其他进程正在运行
    */
    bool IsAnotherInstanceRunning();

    /** 将参数发给已经存在的进程
    * @param vecArgs 需要发送的参数列表，单条数据最大1024字节，数据总长度最大4096字节
    */
    bool SendArgumentsToExistingInstance(const std::vector<std::string>& vecArgs);

    /** 设置重新启动新进程事件的回调函数
    * @param fnCallback 回调函数，用于接收新启动进程的启动参数
    */
    void StartListener(OnAlreadyRunningAppRelaunchEvent fnCallback);

protected:
    /** 初始化功能组件
    */
    virtual void InitializePlatformComponents() = 0;

    /** 清理功能组件
    */
    virtual void CleanupPlatformComponents() = 0;

    /** 检查是否有其他进程正在运行
    */
    virtual bool PlatformCheckInstance() = 0;

    /** 向已经运行的进程发送数据
    */
    virtual bool PlatformSendData(const std::string& strData) = 0;

    /** 监听其他进程启动的事件
    */
    virtual void PlatformListen() = 0;

protected:    
    /** 记录错误日志
    */
    void LogError(const std::string& strMessage);

    /** 有新的进程启动
    */
    void OnAlreadyRunningAppRelaunch(const std::vector<std::string>& args);

protected:
    /** 用以标识一个应用的字符串，标识相同则被视为同一个程序
    */
    std::string m_strAppName;

    /** 进程启动事件的监听线程
    */
    std::thread m_thListener;

    /** 是否正在运行
    */
    std::atomic<bool> m_bRunning;

private:

    /** 重新启动新进程事件的回调函数，用于接收新启动进程的启动参数
    */
    OnAlreadyRunningAppRelaunchEvent m_fnCallback;
};

}

#endif // UI_UTILS_PROCESS_SINGLETON_H_

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

/** 跨进程单例的实现（抽象基类）
*  1. 用应用名称标识一个程序，名称相同的多个进程只允许一个实例运行；
*  2. 新启动的进程可把启动参数发送给已运行的实例（SendArgumentsToExistingInstance）；
*  3. 已运行的实例通过 StartListener() 注册回调，接收后续进程的启动参数；
*  4. 具体的通信方式由各平台的派生类实现（Windows/Linux/macOS）。
*/
class DUILIB_API ProcessSingleton
{
protected:
    explicit ProcessSingleton(const std::string& strAppName);
    ProcessSingleton(const ProcessSingleton&) = delete;
    ProcessSingleton& operator=(const ProcessSingleton&) = delete;

public:
    virtual ~ProcessSingleton();

    /** 创建实例（按当前编译平台创建对应的派生类对象）
    * @param [in] strAppName 用以标识一个应用的字符串，标识相同则被视为同一个程序
    * @return 返回创建的单例对象；strAppName 为空时返回空指针
    * @note 初始化平台组件失败时可能抛出异常
    */
    static std::unique_ptr<ProcessSingleton> Create(const DString& strAppName);

public:
    /** 是否有其他进程正在运行
    * @return 返回 true 表示已有其他实例在运行，false 表示当前是第一个实例
    */
    bool IsAnotherInstanceRunning();

    /** 将参数发给已经存在的进程
    * @param [in] vecArgs 需要发送的参数列表，单条数据最大1024字节，数据总长度最大4096字节
    * @return 返回 true 表示发送成功，否则返回 false（如对方未在监听或数据超长）
    */
    bool SendArgumentsToExistingInstance(const std::vector<std::string>& vecArgs);

    /** 设置重新启动新进程事件的回调函数，并启动监听线程接收其他进程的启动事件
    * @param [in] fnCallback 回调函数，用于接收新启动进程的启动参数
    * @note 只允许调用一次，重复调用时直接返回
    */
    void StartListener(OnAlreadyRunningAppRelaunchEvent fnCallback);

protected:
    /** 初始化功能组件
    */
    virtual void InitializePlatformComponents() = 0;

    /** 清理功能组件
    */
    virtual void CleanupPlatformComponents() = 0;

    /** 检查是否有其他进程正在运行（平台实现）
    * @return 返回 true 表示已有其他实例在运行，否则返回 false
    */
    virtual bool PlatformCheckInstance() = 0;

    /** 向已经运行的进程发送数据（平台实现）
    * @param [in] strData 已经序列化的协议数据
    * @return 返回 true 表示发送成功，否则返回 false
    */
    virtual bool PlatformSendData(const std::string& strData) = 0;

    /** 监听其他进程启动的事件（平台实现，在监听线程中循环执行，直到 m_bRunning 为 false）
    */
    virtual void PlatformListen() = 0;

protected:    
    /** 记录错误日志
    * @param [in] strMessage 错误信息内容
    */
    void LogError(const std::string& strMessage);

    /** 有新的进程启动，解析参数并通过回调通知调用方
    * @param [in] args 新启动进程的启动参数列表
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

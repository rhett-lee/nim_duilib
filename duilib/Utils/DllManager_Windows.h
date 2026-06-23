#ifndef UI_CORE_DLL_MANAGER_H_
#define UI_CORE_DLL_MANAGER_H_

#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include <mutex>
#include <unordered_map>

namespace ui 
{
/** 单LL加载管理器，全局缓存DLL模块，避免重复加载
 *  负责统一托管所有DLL的加载与卸载，相同DLL只会加载一次。
 */
class DUILIB_API DllManager
{
public:
    //! 禁用拷贝构造
    DllManager(const DllManager&) = delete;
    //! 禁用赋值运算符
    DllManager& operator=(const DllManager&) = delete;
    //! 禁用移动构造
    DllManager(DllManager&&) = delete;
    //! 禁用移动赋值
    DllManager& operator=(DllManager&&) = delete;

public:
    /** 获取单例实例
     * @return 单例引用
     */
    static DllManager& Instance();

    /** 加载DLL（内部缓存，不重复加载），加载后可以不立即卸载DLL，如果不立即卸载DLL，则在进程退出时统一释放
     * @param dllPath DLL路径
     * @return HMODULE 模块句柄，加载失败返回nullptr
     */
    HMODULE LoadDll(const DString& dllPath);

private:
    /** 立即卸载DLL，并从缓存中移除(不对外提供此函数，避免误卸载DLL)
     * @param dllPath DLL路径
     * @return bool 成功返回true，失败（未找到）返回false
     */
    bool FreeDll(const DString& dllPath);

private:
    //! 私有构造（单例）
    DllManager();
    //! 析构函数，自动释放所有DLL
    ~DllManager();

private:
    /** 线程安全互斥锁
    */
    mutable std::mutex m_mutex;

    /** DLL缓存：路径 → 模块句柄
    */
    std::unordered_map<DString, HMODULE> m_cache;
};

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN
#endif //UI_CORE_DLL_MANAGER_H_

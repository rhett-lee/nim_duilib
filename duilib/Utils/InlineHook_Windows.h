#ifndef UI_UTILS_INLINE_HOOK_WINDOWS_H_
#define UI_UTILS_INLINE_HOOK_WINDOWS_H_

#include "duilib/duilib_defs.h"

//功能开关（如果不定义该宏，则关闭功能）
#define DUILIB_ENABLE_INLINE_HOOK 1

#ifdef DUILIB_BUILD_FOR_WIN
#include "duilib/duilib_config_windows.h"
#include <vector>
#include <atomic>

namespace ui 
{
/** inline hook的功能封装
*/
class UILIB_API InlineHook
{
public:
    InlineHook();
    ~InlineHook();
    InlineHook(const InlineHook&) = delete;
    InlineHook& operator = (const InlineHook&) = delete;

public:
    //安装hook
    bool Install(void* targetFunc, void* hookFunc);

    //移除hook
    bool Uninstall();

    // 获取跳板函数地址（用于调用原始函数）
    template<typename T>
    T GetTrampoline() const {
        return reinterpret_cast<T>((char*)m_trampoline);
    }

private:
    //安装hook
    bool DoInstall(void);

    //移除hook
    bool DoUninstall();

private:
    /** 生成跳转代码
    */
    void GenerateJump();

    /** 生成跳板函数跳板函数
    */
    bool CreateTrampoline();

    /** 清除内部状态
    */
    void Clear();

private:
    void* m_target;         // 目标函数地址
    void* m_hook;           // 钩子函数地址
    void* m_trampoline;     // 跳板函数地址
    std::vector<uint8_t> m_original;    // 原始指令备份
    std::vector<uint8_t> m_jumpCode;    // 跳转指令
    std::atomic<int32_t> m_installRef;  // 安装状态
    size_t m_replaceLen;    // 需要替换的指令长度
};
    
} //namespace ui 

#endif //DUILIB_BUILD_FOR_WIN

#endif //UI_UTILS_INLINE_HOOK_WINDOWS_H_


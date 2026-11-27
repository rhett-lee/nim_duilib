#ifndef UI_UTILS_INLINE_HOOK_WINDOWS_H_
#define UI_UTILS_INLINE_HOOK_WINDOWS_H_

#include "duilib/duilib_defs.h"

//功能开关（如果不定义该宏，则关闭功能）
//当前实现基于 x86/x64 指令编码（E9 rel32 跳转 + libudis86 反汇编），
//ARM64 的跳转指令与寻址方式完全不同，直接复用会被截断为 32 位地址并写入
//非法机器码，因此 ARM64 上自动关闭该功能（接口保留，调用返回失败）。
#if defined(_M_IX86) || defined(_M_X64)
    #define DUILIB_ENABLE_INLINE_HOOK 1
#endif

#ifdef DUILIB_BUILD_FOR_WIN
#include "duilib/duilib_config_windows.h"
#include <vector>
#include <atomic>

namespace ui 
{
/** inline hook 的功能封装（仅支持 Windows 的 x86/x64 平台）
*  1. Install() 在目标函数入口写入跳转指令，转到 hook 函数执行；
*  2. 通过 GetTrampoline() 取得跳板函数地址，可在 hook 函数内调用原始函数；
*  3. Uninstall() 恢复目标函数的原始指令。
*  说明：ARM64 平台不支持该功能（DUILIB_ENABLE_INLINE_HOOK 未定义），相关接口调用会返回失败。
*/
class InlineHook
{
public:
    InlineHook();
    ~InlineHook();
    InlineHook(const InlineHook&) = delete;
    InlineHook& operator = (const InlineHook&) = delete;

public:
    /** 安装 hook
    * @param [in] targetFunc 被 hook 的目标函数地址，不能为 nullptr
    * @param [in] hookFunc 替换后执行的钩子函数地址，不能为 nullptr
    * @return 返回 true 表示安装成功，否则返回 false（参数为空、平台不支持或写入指令失败）
    * @note 对同一个目标函数重复安装时只增加引用计数，需调用相同次数的 Uninstall() 才会真正移除
    */
    bool Install(void* targetFunc, void* hookFunc);

    /** 移除 hook，恢复目标函数的原始指令
    * @return 返回 true 表示移除成功，否则返回 false
    * @note 引用计数减为 0 时才会真正恢复原始指令；未安装时调用会直接返回失败
    */
    bool Uninstall();

    /** 获取跳板函数地址（用于调用原始函数）
    * @return 返回转换为模板类型 T 的跳板函数地址
    */
    template<typename T>
    T GetTrampoline() const {
        return reinterpret_cast<T>((char*)m_trampoline);
    }

private:
    /** 安装 hook（内部实现）
    * @return 返回 true 表示安装成功，否则返回 false
    */
    bool DoInstall(void);

    /** 移除 hook（内部实现）
    * @return 返回 true 表示移除成功，否则返回 false
    */
    bool DoUninstall();

private:
    /** 生成跳转代码
    */
    void GenerateJump();

    /** 生成跳板函数
    * @return 返回 true 表示生成成功，否则返回 false
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


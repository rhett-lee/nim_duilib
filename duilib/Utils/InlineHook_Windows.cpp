#include "InlineHook_Windows.h"

#ifdef DUILIB_BUILD_FOR_WIN

#ifdef DUILIB_ENABLE_INLINE_HOOK
    #include "duilib/third_party/libudis86/udis86.h"
#endif

namespace ui 
{
InlineHook::InlineHook():
    m_target(nullptr),
    m_hook(nullptr),
    m_trampoline(nullptr),
    m_installRef(0),
    m_replaceLen(0)
{
}

InlineHook::~InlineHook()
{
    Uninstall();
}

#ifndef DUILIB_ENABLE_INLINE_HOOK

bool InlineHook::Uninstall()
{
    return false;
}

bool InlineHook::Install(void* /*targetFunc*/, void* /*hookFunc*/)
{
    return false;
}

void InlineHook::GenerateJump()
{
}

bool InlineHook::CreateTrampoline()
{
    return false;
}

void InlineHook::Clear()
{
}

bool InlineHook::DoInstall()
{
    return false;
}

bool InlineHook::DoUninstall()
{
    return false;
}

#else

void InlineHook::Clear()
{
    if (m_trampoline != nullptr) {
        ::VirtualFree(m_trampoline, 0, MEM_RELEASE);
        m_trampoline = nullptr;
    }
    m_target = nullptr;
    m_hook = nullptr;
    m_installRef = 0;
    m_replaceLen = 0;
    m_original.clear();
    m_jumpCode.clear();
}

bool InlineHook::Install(void* targetFunc, void* hookFunc)
{
    if ((targetFunc == nullptr) || (hookFunc == nullptr)) {
        return false;
    }

    if (m_installRef > 0) {
        ASSERT(m_target == targetFunc);
        ASSERT(m_hook == hookFunc);
        ++m_installRef;
        return true;
    }
    m_target = targetFunc;
    m_hook = hookFunc;

    bool bInstalled = DoInstall();
    if (bInstalled) {
        ++m_installRef;
        ASSERT(m_installRef == 1);
    }
    else {
        Clear();
    }
    return bInstalled;
}

bool InlineHook::Uninstall()
{
    if (m_installRef > 0) {
        --m_installRef;
    }
    if (m_installRef > 0) {
        return true;
    }
    return DoUninstall();
}

// 动态解析指令边界并返回覆盖长度
static size_t CalculateHookLengthWithUdis86(void* target_addr, size_t min_hook_bytes)
{
    ASSERT(min_hook_bytes <= 64);

    ud_t ud_ctx;
    ud_init(&ud_ctx);
#if defined(_M_X64) || defined(_M_AMD64) || defined(_WIN64) || defined(__x86_64__) 
    ud_set_mode(&ud_ctx, 64);
#else
    ud_set_mode(&ud_ctx, 32);
#endif
    ud_set_syntax(&ud_ctx, UD_SYN_INTEL);
    ud_set_input_buffer(&ud_ctx, (uint8_t*)target_addr, 64); // 扫描前64字节

    DWORD old_protect = 0;
    ::VirtualProtect(target_addr, 64, PAGE_EXECUTE_READ, &old_protect);

    size_t total_len = 0;
    while (ud_disassemble(&ud_ctx)) {
        total_len += ud_insn_len(&ud_ctx);
        if (total_len >= min_hook_bytes) {
            break;
        }
    }

    ::VirtualProtect(target_addr, 64, old_protect, &old_protect);
    return total_len;
}

bool InlineHook::DoInstall()
{
    ASSERT((m_target != nullptr) && (m_hook != nullptr));
    if ((m_target == nullptr) || (m_hook == nullptr)) {
        return false;
    }
    //生成跳转指令
    GenerateJump();
    ASSERT(!m_jumpCode.empty());
    if (m_jumpCode.empty()) {
        return false;
    }

    //计算跳板函数的最短长度（必须大于m_jumpCode的长度）
    m_replaceLen = CalculateHookLengthWithUdis86(m_target, m_jumpCode.size());

    ASSERT(m_replaceLen >= m_jumpCode.size());
    if (m_replaceLen < m_jumpCode.size()) {
        return false;
    }

    m_original.resize(m_replaceLen);

    // 备份原始字节
    memcpy(m_original.data(), m_target, m_replaceLen);

    // 创建跳板函数
    if (!CreateTrampoline()) {
        return false;
    }

    // 修改内存保护
    DWORD oldProtect = 0;
    if (!::VirtualProtect(m_target, m_jumpCode.size(), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return false;
    }

    // 写入跳转指令
    memcpy(m_target, m_jumpCode.data(), m_jumpCode.size());

    // 恢复内存保护
    ::VirtualProtect(m_target, m_jumpCode.size(), oldProtect, &oldProtect);
    ::FlushInstructionCache(::GetCurrentProcess(), m_target, m_jumpCode.size());
    return true;
}

bool InlineHook::DoUninstall()
{
    if (m_replaceLen == 0) {
        return false;
    }
    if (m_original.size() != m_replaceLen) {
        return false;
    }
    ASSERT(!m_jumpCode.empty());
    if (m_jumpCode.empty()) {
        return false;
    }

    ASSERT(m_replaceLen >= m_jumpCode.size());
    if (m_replaceLen < m_jumpCode.size()) {
        return false;
    }

    if (m_target == nullptr) {
        return false;
    }

    DWORD oldProtect = 0;
    ::VirtualProtect(m_target, m_jumpCode.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(m_target, m_original.data(), m_jumpCode.size());
    ::VirtualProtect(m_target, m_jumpCode.size(), oldProtect, &oldProtect);
    ::FlushInstructionCache(::GetCurrentProcess(), m_target, m_jumpCode.size());

    Clear();
    return true;
}

void InlineHook::GenerateJump()
{
#ifdef _M_X64
    // mov rax, address; jmp rax
    m_jumpCode = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };
    memcpy(m_jumpCode.data() + 2, &m_hook, sizeof(void*));
#else
    // jmp rel32
    const uint32_t offset = reinterpret_cast<uint32_t>(m_hook) - (reinterpret_cast<uint32_t>(m_target) + 5);
    m_jumpCode = { 0xE9, 0x00, 0x00, 0x00, 0x00 };
    memcpy(m_jumpCode.data() + 1, &offset, sizeof(offset));
#endif
}

bool InlineHook::CreateTrampoline()
{
    //关键实现细节说明(x64)：
    //1. 跳转指令构造
    //(1) 使用mov rax, address + jmp rax组合实现64位地址跳转
    //(2) 共占用14字节（48 B8 [8字节地址] FF E0）
    //2. Trampoline结构
    //   [原始函数的14字节指令]
    //   [mov rax, 原函数+14]
    //   [jmp rax]

    // 分配可执行内存
    m_trampoline = ::VirtualAlloc(nullptr, m_replaceLen + 16,
                                  MEM_COMMIT | MEM_RESERVE,
                                  PAGE_EXECUTE_READWRITE);
    if (!m_trampoline) {
        return false;
    }

    // 复制原始指令
    memcpy(m_trampoline, m_target, m_replaceLen);

    // 生成跳回指令
#ifdef _M_X64
        // mov rax, return_address; jmp rax
    uint8_t jmpBack[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0xFF, 0xE0 };
    void* returnAddr = static_cast<char*>(m_target) + m_replaceLen;
    memcpy(jmpBack + 2, &returnAddr, sizeof(returnAddr));
#else
        // jmp return_address
    uint8_t jmpBack[] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };
    const uint32_t offset = reinterpret_cast<uint32_t>(m_target) + m_replaceLen - (reinterpret_cast<uint32_t>(m_trampoline) + m_replaceLen + 5);
    memcpy(jmpBack + 1, &offset, sizeof(offset));
#endif

    // 写入跳回指令
    memcpy(static_cast<char*>(m_trampoline) + m_replaceLen, &jmpBack[0], sizeof(jmpBack));
    return true;
}

#endif //DUILIB_ENABLE_INLINE_HOOK

} //namespace ui 

#endif //DUILIB_BUILD_FOR_WIN


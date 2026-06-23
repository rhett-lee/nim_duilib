#include "DllManager_Windows.h"

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui
{
DllManager::DllManager()
{
}

DllManager& DllManager::Instance()
{
    static DllManager s_instance;
    return s_instance;
}

DllManager::~DllManager()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    // 程序退出时自动释放所有加载的DLL
    for (auto& pair : m_cache) {
        ::FreeLibrary(pair.second);
    }
    m_cache.clear();
}

HMODULE DllManager::LoadDll(const DString& dllPath)
{
    if (dllPath.empty()) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    // 已缓存，直接返回
    auto it = m_cache.find(dllPath);
    if (it != m_cache.end()) {
        return it->second;
    }

    // 未缓存，执行加载
    HMODULE hMod = ::LoadLibrary(dllPath.c_str());
    if (hMod != nullptr) {
        m_cache[dllPath] = hMod;
    }
    return hMod;
}

bool DllManager::FreeDll(const DString& dllPath)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_cache.find(dllPath);
    if (it == m_cache.end()) {
        return false;
    }

    // 释放并从缓存移除
    ::FreeLibrary(it->second);
    m_cache.erase(it);
    return true;
}

};

#endif //DUILIB_BUILD_FOR_WIN

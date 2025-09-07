#include "WindowManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/GlobalManager.h"
#include <set>

namespace ui 
{
WindowManager::WindowManager()
{
}

WindowManager::~WindowManager()
{
}

void WindowManager::AddWindow(Window* pWindow)
{
    GlobalManager::Instance().AssertUIThread();
    ASSERT(pWindow != nullptr);
    if (pWindow != nullptr) {
        auto iter = std::find(m_windowList.begin(), m_windowList.end(), pWindow);
        if (iter != m_windowList.end()) {
            m_windowList.erase(iter);
        }
        m_windowList.push_back(WindowPtr(pWindow));
    }

#ifdef _DEBUG    
    if (1) {
        //校验窗口ID是否重复
        std::set<DString> windowIdSet;
        for (auto iter = m_windowList.rbegin(); iter != m_windowList.rend(); ++iter) {
            if (iter->get() != nullptr) {
                ASSERT(!iter->get()->GetWindowClassName().empty());
                DString windowId = iter->get()->GetWindowId();
                ASSERT(!windowId.empty());
                if (!windowIdSet.empty()) {
                    ASSERT(windowIdSet.find(windowId) == windowIdSet.end());
                }
                windowIdSet.insert(windowId);
            }
        }
    }
#endif
}

void WindowManager::RemoveWindow(Window* pWindow)
{
    GlobalManager::Instance().AssertUIThread();
    ASSERT(pWindow != nullptr);
    if (pWindow != nullptr) {
        auto iter = std::find(m_windowList.begin(), m_windowList.end(), pWindow);
        if (iter != m_windowList.end()) {
            m_windowList.erase(iter);
        }
    }
}

bool WindowManager::HasWindow(Window* pWindow) const
{
    GlobalManager::Instance().AssertUIThread();
    if (pWindow != nullptr) {
        for (auto iter = m_windowList.begin(); iter != m_windowList.end(); ++iter) {
            if (iter->get() == pWindow) {
                return true;
            }
        }
    }
    return false;
}

bool WindowManager::HasWindowBase(WindowBase* pWindowBase) const
{
    GlobalManager::Instance().AssertUIThread();
    if (pWindowBase != nullptr) {
        for (auto iter = m_windowList.begin(); iter != m_windowList.end(); ++iter) {
            if (iter->get() == pWindowBase) {
                return true;
            }
        }
    }
    return false;
}

std::vector<WindowPtr> WindowManager::GetAllWindowList() const
{
    GlobalManager::Instance().AssertUIThread();
    std::vector<WindowPtr> windowList;
    for (auto iter = m_windowList.begin(); iter != m_windowList.end(); ++iter) {
        if (iter->get() != nullptr) {
            windowList.push_back(*iter);
        }
    }
    return windowList;
}

std::vector<WindowPtr> WindowManager::GetAllWindowList(const DString& windowClassName) const
{
    GlobalManager::Instance().AssertUIThread();
    std::vector<WindowPtr> windowList;
    for (auto iter = m_windowList.begin(); iter != m_windowList.end(); ++iter) {
        if (iter->get() != nullptr) {
            if (windowClassName == iter->get()->GetWindowClassName()) {
                windowList.push_back(*iter);
            }
        }
    }
    return windowList;
}

WindowPtr WindowManager::GetWindowById(const DString& windowId) const
{
    GlobalManager::Instance().AssertUIThread();
    WindowPtr pWindow;
    for (auto iter = m_windowList.begin(); iter != m_windowList.end(); ++iter) {
        if (iter->get() != nullptr) {
            if (windowId == iter->get()->GetWindowId()) {
                pWindow = iter->get();
            }
        }
    }
    return pWindow;
}

void WindowManager::Clear()
{
    m_windowList.clear();
}

} //namespace ui

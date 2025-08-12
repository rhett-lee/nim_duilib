#include "BrowserForm_Windows.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
#include "Windows/BrowserBox_Windows.h"
#include "browser/BrowserManager.h"

namespace
{
    // 注册这个消息，收到这个消息后表示窗口对应的任务栏按钮被系统创建，这时候初始化ITaskbarList4接口
    UINT WM_TASKBARBUTTONCREATED = ::RegisterWindowMessage(TEXT("TaskbarButtonCreated"));
}

BrowserForm_Windows::BrowserForm_Windows()
{
}

BrowserForm_Windows::~BrowserForm_Windows()
{
}

BrowserBox* BrowserForm_Windows::CreateBrowserBox(ui::Window* pWindow, std::string id)
{
    return new BrowserBox_Windows(pWindow, id);
}

LRESULT BrowserForm_Windows::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    if (uMsg == WM_TASKBARBUTTONCREATED) {
        bHandled = true;
        m_taskbarManager.Init(this);

        // 因为窗口刚创建时，浏览器盒子已经创建但是那时还没有收到WM_TASKBARBUTTONCREATED消息，导致RegisterTab函数没有被调用，所以收到消息后重新遍历一下没有被注册的Tab
        for (size_t i = 0; i < m_pBorwserBoxTab->GetItemCount(); ++i) {
            ui::Control *pBoxItem = m_pBorwserBoxTab->GetItemAt(i);
            ASSERT(pBoxItem != nullptr);
            if (pBoxItem == nullptr) {
                continue;
            }

            BrowserBox_Windows* pBrowserBox = dynamic_cast<BrowserBox_Windows*>(pBoxItem);
            if (pBrowserBox == nullptr) {
                continue;
            }

            TaskbarTabItem* pTaskbarItem = pBrowserBox->GetTaskbarItem();
            if (pTaskbarItem != nullptr) {
                m_taskbarManager.RegisterTab(*pTaskbarItem);
            }
        }

        return TRUE;
    }
    return BaseClass::OnWindowMessage(uMsg, wParam, lParam, bHandled);
}

void BrowserForm_Windows::OnCreateNewTabPage(ui::TabCtrlItem* pTabItem, BrowserBox* pBrowserBox)
{
    BaseClass::OnCreateNewTabPage(pTabItem, pBrowserBox);

    BrowserBox_Windows* pBrowserBoxWindows = dynamic_cast<BrowserBox_Windows*>(pBrowserBox);
    if (pBrowserBoxWindows != nullptr) {
        auto pTaskbarItem = pBrowserBoxWindows->GetTaskbarItem();
        if (pTaskbarItem) {
            m_taskbarManager.RegisterTab(*pTaskbarItem);
        }
    }
}

void BrowserForm_Windows::OnCloseTabPage(BrowserBox* pBrowserBox)
{
    BaseClass::OnCloseTabPage(pBrowserBox);

    BrowserBox_Windows* pBrowserBoxWindows = dynamic_cast<BrowserBox_Windows*>(pBrowserBox);
    if (pBrowserBoxWindows != nullptr) {
        auto pTaskbarItem = pBrowserBoxWindows->GetTaskbarItem();
        if (pTaskbarItem) {
            m_taskbarManager.UnregisterTab(*pTaskbarItem);
        }
    }
}

#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

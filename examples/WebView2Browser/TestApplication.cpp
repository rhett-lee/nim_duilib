#include "TestApplication.h"
#include "MainThread.h"

TestApplication::TestApplication()
{
}

TestApplication::~TestApplication()
{
}

TestApplication& TestApplication::Instance()
{
    static TestApplication self;
    return self;
}

int TestApplication::Run(int argc, char** argv)
{
    //创建主线程
    MainThread thread;

    //执行主线程消息循环
    thread.RunMessageLoop();

    //正常退出程序
    return 0;
}

void TestApplication::AddMainWindow(ui::Window* pWindow)
{
    if (pWindow != nullptr) {
        ui::ControlPtrT<ui::Window> pMainWindow(pWindow);
        m_pMainWindows.push_back(pMainWindow);
    }
}

void TestApplication::RemoveMainWindow(ui::Window* pWindow)
{
    auto iter = m_pMainWindows.begin();
    while (iter != m_pMainWindows.end()) {
        if (*iter == pWindow) {
            iter = m_pMainWindows.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

void TestApplication::SetActiveMainWindow(ui::Window* pWindow)
{
    m_pActiveWindow = pWindow;
}

void TestApplication::ActiveMainWindow()
{
    if (m_pActiveWindow != nullptr) {
        m_pActiveWindow->ShowWindow(ui::ShowWindowCommands::kSW_SHOW_NORMAL);
    }
    else {
        for (ui::ControlPtrT<ui::Window> pWindow : m_pMainWindows) {
            if ((pWindow != nullptr) && !pWindow->IsClosingWnd()) {
                pWindow->ShowWindow(ui::ShowWindowCommands::kSW_SHOW_NORMAL);
            }
        }
    }
}

void TestApplication::CloseMainWindow()
{
    for (ui::ControlPtrT<ui::Window> pWindow : m_pMainWindows) {
        if ((pWindow != nullptr) && !pWindow->IsClosingWnd()) {
            pWindow->CloseWnd();
        }
    }
}

void TestApplication::PostQuitMessage()
{
    ui::Window::PostQuitMsg(0);
}

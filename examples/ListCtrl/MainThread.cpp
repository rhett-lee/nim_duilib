#include "MainThread.h"
#include "MainForm.h"

void WorkerThread::Init()
{
    ::OleInitialize(NULL);
    nbase::ThreadManager::RegisterThread(m_threadID);
}

void WorkerThread::Cleanup()
{
    nbase::ThreadManager::UnregisterThread();
    ::OleUninitialize();
}

void MainThread::Init()
{
    ::OleInitialize(NULL);
    nbase::ThreadManager::RegisterThread(kThreadUI);

    //启动工作线程
    m_workerThread.reset(new WorkerThread(kThreadWorker, "WorkerThread"));
    m_workerThread->Start();

    //初始化全局资源, 使用本地文件夹作为资源
    std::wstring resourcePath = nbase::win32::GetCurrentModuleDirectory();
    resourcePath += L"resources\\";
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    //创建主窗口
    MainForm* pWindow = new MainForm();
    pWindow->CreateWnd(NULL, L"ListCtrl控件测试程序", UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
    pWindow->SetIcon(IDI_LISTCTRL);
    pWindow->CenterWindow();
    pWindow->ShowWindow();
}

void MainThread::Cleanup()
{
    ui::GlobalManager::Instance().Shutdown();
    if (m_workerThread != nullptr) {
        m_workerThread->Stop();
        m_workerThread.reset(nullptr);
    }
    SetThreadWasQuitProperly(true);
    nbase::ThreadManager::UnregisterThread();
    ::OleUninitialize();
}

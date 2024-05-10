//MainThread.cpp
#include "MainThread.h"
#include "MainForm.h"

WorkerThread::WorkerThread(ThreadId threadID, const char* name)
    : FrameworkThread(name)
    , m_threadID(threadID)
{
}

WorkerThread::~WorkerThread()
{
}

void WorkerThread::Init()
{
    ::OleInitialize(nullptr);
    nbase::ThreadManager::RegisterThread(m_threadID);
}

void WorkerThread::Cleanup()
{
    nbase::ThreadManager::UnregisterThread();
    ::OleUninitialize();
}

MainThread::MainThread() :
    nbase::FrameworkThread("MainThread")
{
}

MainThread::~MainThread()
{
}

void MainThread::Init()
{
    ::OleInitialize(nullptr);
    nbase::ThreadManager::RegisterThread(kThreadUI);

    //启动工作线程
    m_workerThread.reset(new WorkerThread(kThreadWorker, "WorkerThread"));
    m_workerThread->Start();

    //初始化全局资源, 使用本地文件夹作为资源
    std::wstring resourcePath = nbase::win32::GetCurrentModuleDirectory();
    resourcePath += L"resources\\";
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    //在下面加入启动窗口代码
    //
    //创建一个默认带有阴影的居中窗口
    MainForm* window = new MainForm();
    window->CreateWnd(nullptr, MainForm::kClassName.c_str(), UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
    window->CenterWindow();
    window->ShowWindow();
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

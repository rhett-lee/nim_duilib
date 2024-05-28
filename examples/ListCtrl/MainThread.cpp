#include "MainThread.h"
#include "MainForm.h"

WorkerThread::WorkerThread()
    : FrameworkThread(_T("WorkerThread"), ui::kThreadWorker)
{
}

WorkerThread::~WorkerThread()
{
}

void WorkerThread::OnInit()
{
    ::OleInitialize(NULL);
}

void WorkerThread::OnCleanup()
{
    ::OleUninitialize();
}

MainThread::MainThread() :
    FrameworkThread(_T("MainThread"), ui::kThreadUI)
{
}

MainThread::~MainThread()
{
}

void MainThread::OnInit()
{
    ::OleInitialize(NULL);

    //启动工作线程
    m_workerThread.reset(new WorkerThread);
    m_workerThread->Start();

    //初始化全局资源, 使用本地文件夹作为资源
    std::wstring resourcePath = ui::PathUtil::GetCurrentModuleDirectory();
    resourcePath += L"resources\\";
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    //创建主窗口
    MainForm* pWindow = new MainForm();
    pWindow->CreateWnd(NULL, L"ListCtrl控件测试程序", UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
    pWindow->SetIcon(IDI_LISTCTRL);
    pWindow->CenterWindow();
    pWindow->ShowWindow();
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
    if (m_workerThread != nullptr) {
        m_workerThread->Stop();
        m_workerThread.reset(nullptr);
    }
    ::OleUninitialize();
}

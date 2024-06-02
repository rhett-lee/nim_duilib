#include "MainThread.h"

WorkerThread::WorkerThread():
    FrameworkThread(_T("WorkerThread"), ui::kThreadWorker)
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
    DString resourcePath = ui::PathUtil::GetCurrentModuleDirectory();
    resourcePath += _T("resources\\");
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    ui::ColorPicker* pColorPicker = new ui::ColorPicker;
    pColorPicker->CreateWnd(nullptr, ui::ColorPicker::kClassName, UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
    pColorPicker->CenterWindow();
    pColorPicker->ShowWindow();

    //设置选择前的颜色
    pColorPicker->SetSelectedColor(ui::UiColor(ui::UiColors::White));

    //窗口关闭事件
    pColorPicker->AttachWindowClose([this](const ui::EventArgs& args) {
        //关闭窗口后，退出主线程
        PostQuitMessage(0L);
        return true;
        });
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

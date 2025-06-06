#include "MainThread.h"
#include "ControlForm.h"

MiscThread::MiscThread() :
    FrameworkThread(_T("MiscThread"), ui::kThreadMisc)
{
}

MiscThread::~MiscThread()
{
}

void MiscThread::OnInit()
{
}

void MiscThread::OnCleanup()
{
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
    // 启动杂事处理线程
    m_misc_thread.reset(new MiscThread);
    m_misc_thread->Start();

    //初始化全局资源, 使用本地文件夹作为资源
    ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    resourcePath += _T("resources\\");
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    // 创建一个默认带有阴影的居中窗口
    ControlForm* window = new ControlForm();
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("controls"), true));
    window->PostQuitMsgWhenClosed(true);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
}

void MainThread::OnCleanup()
{
    m_misc_thread->Stop();
    m_misc_thread.reset(nullptr);
    ui::GlobalManager::Instance().Shutdown();
}

#include "MainThread.h"
#include "MainForm.h"

MainThread::MainThread() :
    FrameworkThread(_T("MainThread"), ui::kThreadUI)
{
}

MainThread::~MainThread()
{
}

bool MainThread::OnInit()
{
    //初始化全局资源, 使用本地文件夹作为资源
    ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath();
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    //主窗口：演示调色器的选色回调(AttachSelectColor)与关闭回调(AttachWindowClose)
    MainForm* window = new MainForm();
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("ColorPicker"), true));
    window->PostQuitMsgWhenClosed(true);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
    return true;
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

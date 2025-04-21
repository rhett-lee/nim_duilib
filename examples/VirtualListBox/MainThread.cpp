// VirtualBox.cpp : 定义应用程序的入口点。
//

#include "MainThread.h"
#include "MainForm.h"

MainThread::MainThread() :
    FrameworkThread(_T("MainThread"), ui::kThreadUI)
{
}

MainThread::~MainThread()
{
}

void MainThread::OnInit()
{
    //初始化全局资源, 使用本地文件夹作为资源
    ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    resourcePath += _T("resources\\");
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    // 创建一个默认带有阴影的居中窗口
    MainForm* window = new MainForm();
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("VirtualListBox"), true));
    window->PostQuitMsgWhenClosed(true);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

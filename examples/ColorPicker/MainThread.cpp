#include "MainThread.h"

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

    ui::ColorPicker* pColorPicker = new ui::ColorPicker;
    pColorPicker->CreateWnd(nullptr, ui::WindowCreateParam(_T("ColorPicker"), true));
    pColorPicker->ShowWindow(ui::kSW_SHOW_NORMAL);

    //设置选择前的颜色
    pColorPicker->SetSelectedColor(ui::UiColor(ui::UiColors::White));

    //关闭窗口后，退出主线程
    pColorPicker->PostQuitMsgWhenClosed(true);
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

#include "MainThread.h"

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
    ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath(false);
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    ui::ColorPicker* pColorPicker = new ui::ColorPicker;
    pColorPicker->CreateWnd(nullptr, ui::WindowCreateParam(_T("ColorPicker"), true));
    pColorPicker->ShowWindow(ui::kSW_SHOW_NORMAL);

    //设置选择前的颜色
    if (ui::GlobalManager::Instance().Theme().GetCurrentThemeStyle() == ui::ThemeStyle::kDark) {
        //深色主题
        pColorPicker->SetSelectedColor(ui::UiColor(ui::UiColors::Black));
    }
    else {
        //浅色主题
        pColorPicker->SetSelectedColor(ui::UiColor(ui::UiColors::White));
    }

    //关闭窗口后，退出主线程
    pColorPicker->PostQuitMsgWhenClosed(true);
    return true;
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

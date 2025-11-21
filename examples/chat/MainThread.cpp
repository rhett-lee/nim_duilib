#include "MainThread.h"
#include "ChatForm.h"

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

    // 一个仿微信的布局示例
    ChatForm::ShowCustomWindow(_T("basic_layout"), _T("chat"), _T("wechat.xml"));

    // 一个仿登录窗口的布局示例
    ChatForm::ShowCustomWindow(_T("login"), _T("chat"), _T("login.xml"));
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

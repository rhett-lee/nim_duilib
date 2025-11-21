#include "MainThread.h"
#include "MainForm.h"
#include "duilib/WebView2/WebView2Manager.h"

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

    //初始化WebView2的基本配置
    DString userDataFolder = ui::WebView2Manager::GetInstance().GetDefaultUserDataFolder(_T("WebView2"));
    ui::WebView2Manager::GetInstance().Initialize(userDataFolder);

    //创建一个默认带有阴影的居中窗口
    MainForm* window = new MainForm();
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("WebView2"), true));
    window->PostQuitMsgWhenClosed(true);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
}

void MainThread::OnCleanup()
{
    ui::WebView2Manager::GetInstance().UnInitialize();
    ui::GlobalManager::Instance().Shutdown();    
}

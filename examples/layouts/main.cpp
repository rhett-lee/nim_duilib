// basic.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "main.h"
#include "layouts_form.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 创建主线程
    MainThread thread;

    // 执行主线程循环
    thread.RunOnCurrentThreadWithLoop();

    return 0;
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
    //初始化全局资源, 使用本地文件夹作为资源
    std::wstring resourcePath = ui::PathUtil::GetCurrentModuleDirectory();
    resourcePath += L"resources\\";
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    // 一个仿微信的布局示例
    LayoutsForm::ShowCustomWindow(L"basic_layout", L"layouts", L"wechat.xml");

    // 一个仿登录窗口的布局示例
    // LayoutsForm::ShowCustomWindow(L"login", L"layouts", L"login.xml");
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

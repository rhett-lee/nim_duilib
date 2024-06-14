// render.cpp : Defines the entry point for the application.
//

#include "render.h"
#include "render_form.h"

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
    DString resourcePath = ui::PathUtil::GetCurrentModuleDirectory();
    resourcePath += _T("resources\\");
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    // 创建一个默认带有阴影的居中窗口
    RenderForm* window = new RenderForm();
    ui::WindowCreateParam createParam;
    createParam.m_dwExStyle = WS_EX_LAYERED;
    createParam.m_className = _T("render");
    createParam.m_windowTitle = createParam.m_className;
    window->CreateWnd(nullptr, createParam);
    window->CenterWindow();
    window->ShowWindow();
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

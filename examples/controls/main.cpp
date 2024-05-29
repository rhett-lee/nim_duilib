#include "main.h"
#include "controls_form.h"

//定义应用程序的入口点
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    //开启用鼠标模拟WM_POINTER消息
    //BOOL isOk = EnableMouseInPointer(TRUE);
    //BOOL isOk2 = IsMouseInPointerEnabled();

    MainThread().RunOnCurrentThreadWithLoop();

    return 0;
}

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
    DString resourcePath = ui::PathUtil::GetCurrentModuleDirectory();
    resourcePath += _T("resources\\");
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    // 创建一个默认带有阴影的居中窗口
    ControlForm* window = new ControlForm();
    window->CreateWnd(NULL, ControlForm::kClassName.c_str(), UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
    window->CenterWindow();
    window->ShowWindow();
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();

    m_misc_thread->Stop();
    m_misc_thread.reset(nullptr);
}

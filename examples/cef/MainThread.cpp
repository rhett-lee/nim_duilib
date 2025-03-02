#include "MainThread.h"
#include "CefForm.h"

MainThread::MainThread() :
    FrameworkThread(_T("MainThread"), ui::kThreadUI)
{
}

MainThread::~MainThread()
{
}

const ui::DpiInitParam& MainThread::GetDpiInitParam() const
{
    return m_dpiInitParam;
}

void MainThread::OnInit()
{
    //初始化全局资源, 使用本地文件夹作为资源
    ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    resourcePath += _T("resources\\");
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath), GetDpiInitParam());

    // 创建一个默认带有阴影的居中窗口
    CefForm* window = new CefForm();
    uint32_t dwExStyle = 0;
    if (ui::CefManager::GetInstance()->IsEnableOffScreenRendering()) {
        dwExStyle |= ui::kWS_EX_LAYERED;
    }
    ui::WindowCreateParam createWndParam;
    createWndParam.m_className = _T("cef");
    createWndParam.m_windowTitle = createWndParam.m_className;
    createWndParam.m_dwExStyle = dwExStyle;
    createWndParam.m_bCenterWindow = true;
    window->CreateWnd(nullptr, createWndParam);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

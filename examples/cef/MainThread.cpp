#include "MainThread.h"
#include "CefForm.h"
#include "TestApplication.h"

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

bool MainThread::OnInit()
{
    // 创建一个默认带有阴影的居中窗口
    CefForm* window = new CefForm();
    ui::WindowCreateParam createWndParam;
    createWndParam.m_className = _T("cef");
    createWndParam.m_windowTitle = createWndParam.m_className;
    createWndParam.m_bCenterWindow = true;
    window->CreateWnd(nullptr, createWndParam);
    TestApplication::Instance().SetMainWindow(window);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
    return true;
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

//MainThread.cpp
#include "MainThread.h"
#include "MainForm.h"
#include "resource.h"

WorkerThread::WorkerThread()
    : FrameworkThread(_T("WorkerThread"), ui::kThreadWorker)
{
}

WorkerThread::~WorkerThread()
{
}

void WorkerThread::OnInit()
{
    ::OleInitialize(nullptr);
}

void WorkerThread::OnCleanup()
{
    ::OleUninitialize();
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
    ::OleInitialize(nullptr);

    //启动工作线程
    m_workerThread.reset(new WorkerThread);
    m_workerThread->Start();

    //初始化全局资源
    constexpr ui::ResourceType resType = ui::ResourceType::kResZipFile;
    if (resType == ui::ResourceType::kLocalFiles) {
        //使用本地文件夹作为资源
        DString resourcePath = ui::PathUtil::GetCurrentModuleDirectory();
        resourcePath += _T("resources\\");
        ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));
    }
    else if (resType == ui::ResourceType::kZipFile) {
        //使用本地zip压缩包作为资源（压缩包位于exe相同目录）    
        ui::ZipFileResParam resParam;
        resParam.resourcePath = _T("resources\\");
        resParam.zipFilePath = ui::PathUtil::GetCurrentModuleDirectory();
        resParam.zipFilePath += _T("resources.zip");
        resParam.zipPassword = _T("");
        ui::GlobalManager::Instance().Startup(resParam);
    }
    else if (resType == ui::ResourceType::kResZipFile) {
        //使用exe资源文件中的zip压缩包
        ui::ResZipFileResParam resParam;
        resParam.resourcePath = _T("resources\\");
        resParam.hResModule = nullptr;
        resParam.resourceName = MAKEINTRESOURCE(IDR_THEME);
        resParam.resourceType = _T("THEME");
        resParam.zipPassword = _T("");
        ui::GlobalManager::Instance().Startup(resParam);
    }
    else {
        return;
    }

    //在下面加入启动窗口代码
    //创建一个默认带有阴影的居中窗口
    MainForm* window = new MainForm();
    ui::WindowCreateParam createParam;
    createParam.m_dwExStyle = WS_EX_LAYERED;
    createParam.m_className = _T("MultiLang");
    createParam.m_windowTitle = createParam.m_className;
    window->CreateWnd(nullptr, createParam);
    window->PostQuitMsgWhenClosed(true);
    window->CenterWindow();
    window->ShowWindow();
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
    if (m_workerThread != nullptr) {
        m_workerThread->Stop();
        m_workerThread.reset(nullptr);
    }
    ::OleUninitialize();
}

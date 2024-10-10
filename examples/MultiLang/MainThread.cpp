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
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    ::OleInitialize(nullptr);
#endif
}

void WorkerThread::OnCleanup()
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    ::OleUninitialize();
#endif
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
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    ::OleInitialize(nullptr);
#endif

    //启动工作线程
    m_workerThread.reset(new WorkerThread);
    m_workerThread->Start();

    //初始化全局资源
    constexpr ui::ResourceType resType = ui::ResourceType::kLocalFiles;
    if (resType == ui::ResourceType::kLocalFiles) {
        //使用本地文件夹作为资源
        ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
        resourcePath += _T("resources\\");
        ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));
    }
    else if (resType == ui::ResourceType::kZipFile) {
        //使用本地zip压缩包作为资源（压缩包位于exe相同目录）    
        ui::ZipFileResParam resParam;
        resParam.resourcePath = _T("resources\\");
        resParam.zipFilePath = ui::FilePathUtil::GetCurrentModuleDirectory();
        resParam.zipFilePath += _T("resources.zip");
        resParam.zipPassword = _T("");
        ui::GlobalManager::Instance().Startup(resParam);
    }
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
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
#endif
    else {
        return;
    }

    //在下面加入启动窗口代码
    //创建一个默认带有阴影的居中窗口
    MainForm* window = new MainForm();
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("MultiLang")));
    window->PostQuitMsgWhenClosed(true);
    window->CenterWindow();
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
    if (m_workerThread != nullptr) {
        m_workerThread->Stop();
        m_workerThread.reset(nullptr);
    }
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    ::OleUninitialize();
#endif
}

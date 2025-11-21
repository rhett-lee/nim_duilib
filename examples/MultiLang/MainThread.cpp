//MainThread.cpp
#include "MainThread.h"
#include "MainForm.h"

#if defined (DUILIB_BUILD_FOR_WIN)
    #include "resource.h"
#endif

MainThread::MainThread() :
    FrameworkThread(_T("MainThread"), ui::kThreadUI)
{
}

MainThread::~MainThread()
{
}

void MainThread::OnInit()
{
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
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("MultiLang"), true));
    window->PostQuitMsgWhenClosed(true);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

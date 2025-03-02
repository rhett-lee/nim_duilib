#include "MainThread.h"
#include "browser/MultiBrowserManager.h"

#include <chrono>

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

    uint64_t nTimeMS = std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
    std::string timeStamp = ui::StringUtil::Printf("%I64u", nTimeMS);
    MultiBrowserManager::GetInstance()->CreateBorwserBox(nullptr, timeStamp, _T(""));
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

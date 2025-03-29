#include "MainThread.h"
#include "browser/BrowserManager.h"
#include "browser/BrowserForm.h"
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

    //设置控制主进程单例的回调函数
    ui::CefManager::GetInstance()->SetAlreadyRunningAppRelaunch(UiBind(&MainThread::OnAlreadyRunningAppRelaunch, this, std::placeholders::_1));

    //创建第一个窗口
    std::string id = BrowserManager::GetInstance()->CreateBrowserID();
    BrowserManager::GetInstance()->CreateBorwserBox(nullptr, id, _T(""));
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

void MainThread::OnAlreadyRunningAppRelaunch(const std::vector<DString>& argumentList)
{
    if (ui::GlobalManager::Instance().IsInUIThread()) {
        //CEF 133版本会调用此接口
        BrowserForm* pBrowserForm = BrowserManager::GetInstance()->GetLastActiveBrowserForm();
        if (pBrowserForm != nullptr) {
            pBrowserForm->SetWindowForeground();
            if (!argumentList.empty()) {
                //只处理第一个参数
                DString url = argumentList[0];
                pBrowserForm->OpenLinkUrl(url, false);
            }
        }
    }
    else {
        //转发到UI线程处理
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&MainThread::OnAlreadyRunningAppRelaunch, this, argumentList));
    }
}

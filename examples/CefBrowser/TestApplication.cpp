#include "TestApplication.h"
#include "MainThread.h"

// duilib
#include "duilib/duilib.h"
#include "duilib/duilib_cef.h"
#include <clocale>

TestApplication::TestApplication()
{
}

TestApplication::~TestApplication()
{
}

int TestApplication::Run(int argc, char** argv)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
        HRESULT hr = ::OleInitialize(nullptr);
    if (FAILED(hr)) {
        return 1;
    }
#endif

    // 创建主线程
    MainThread thread;

    //必须在CefManager::Initialize前调用，设置DPI自适应属性，否则会导致显示不正常
    //初始化全局资源, 使用本地文件夹作为资源
    ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    resourcePath += _T("resources\\");
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath), thread.GetDpiInitParam());

    //初始化CEF: 必须在GlobalManager初始化完成之后，因为初始化CEF过程中，会用到GlobalManager
    if (!ui::CefManager::GetInstance()->Initialize(true, _T("cef_browser"), argc, argv, nullptr)) {
        return 1;
    }

    //如果未启用CEF的消息循环，则需要启动一个定时器调用CEF的消息处理函数
    if (!ui::CefManager::GetInstance()->IsMultiThreadedMessageLoop()) {
        ui::CefManager::GetInstance()->ScheduleCefDoMessageLoopWork();
    }

    // 执行主线程循环
    thread.RunOnCurrentThreadWithLoop();

    // 清理 CEF
    ui::CefManager::GetInstance()->UnInitialize();

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    ::OleUninitialize();
#endif

    return 0;
}

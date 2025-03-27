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
    // 将 bin\\cef 目录添加到环境变量，这样可以将所有 CEF 相关文件放到该目录下，方便管理
    // 在项目属性->连接器->输入，延迟加载 libcef.dll
    ui::CefManager::GetInstance()->AddCefDllToPath();

#if defined (DUILIB_BUILD_FOR_WIN)
    _wsetlocale(LC_ALL, L"chs");
#else
    setlocale(LC_ALL, "chs");
#endif

    //#ifdef _DEBUG
    //    AllocConsole();
    //    FILE* fp = nullptr;
    //    freopen_s(&fp, "CONOUT$", "w+t", stdout);
    //    wprintf_s(L"Command:\n%s\n\n", lpCmdLine);
    //#endif

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    HRESULT hr = ::OleInitialize(nullptr);
    if (FAILED(hr)) {
        return 0;
    }
#endif

    // 创建主线程
    MainThread thread;

    //必须在CefManager::Initialize前调用，设置DPI自适应属性，否则会导致显示不正常
    ui::GlobalManager::Instance().Dpi().InitDpiAwareness(thread.GetDpiInitParam());

    // 初始化 CEF
    CefSettings settings;
    ui::FilePath appDataDir = ui::FilePathUtil::GetCurrentModuleDirectory();

    //需要一个可写目录，并且每个程序一个目录
    appDataDir += _T("cef_temp\\multi_browser\\");

#ifdef DUILIB_BUILD_FOR_WIN
    if (!ui::CefManager::GetInstance()->Initialize(appDataDir.ToString(), settings, true, _T("duilib_multi_browser"))) {
        return 0;
    }
#else
    if (!ui::CefManager::GetInstance()->Initialize(appDataDir.ToString(), settings, true, argc, argv)) {
        return 0;
    }
#endif

    // 执行主线程循环
    thread.RunOnCurrentThreadWithLoop();

    // 清理 CEF
    ui::CefManager::GetInstance()->UnInitialize();

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    ::OleUninitialize();
#endif

    return 0;
}

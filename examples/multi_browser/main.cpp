#include "main.h"
#include "browser/multi_browser_manager.h"

#include <clocale>
#include <chrono>

//开启DPI感知功能设置参数
ui::DpiInitParam dpiInitParam;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 将 bin\\cef 目录添加到环境变量，这样可以将所有 CEF 相关文件放到该目录下，方便管理
    // 在项目属性->连接器->输入，延迟加载 nim_libcef.dll
    nim_comp::CefManager::GetInstance()->AddCefDllToPath();

    _wsetlocale(LC_ALL, L"chs");

//#ifdef _DEBUG
//    AllocConsole();
//    FILE* fp = NULL;
//    freopen_s(&fp, "CONOUT$", "w+t", stdout);
//    wprintf_s(L"Command:\n%s\n\n", lpCmdLine);
//#endif

    HRESULT hr = ::OleInitialize(NULL);
    if (FAILED(hr))
        return 0;

    //必须在CefManager::Initialize前调用，设置DPI自适应属性，否则会导致显示不正常
    ui::GlobalManager::Instance().Dpi().InitDpiAwareness(dpiInitParam);

    // 初始化 CEF
    CefSettings settings;
    ui::FilePath appDataDir = ui::FilePathUtil::GetCurrentModuleDirectory();
    appDataDir += _T("cef_temp\\"); //TODO: 可写目录
    if (!nim_comp::CefManager::GetInstance()->Initialize(appDataDir.ToString(), settings, true)) {
        return 0;
    }

    // 创建主线程
    MainThread thread;

    // 执行主线程循环
    thread.RunOnCurrentThreadWithLoop();

    // 清理 CEF
    nim_comp::CefManager::GetInstance()->UnInitialize();

    ::OleUninitialize();

    return 0;
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
    //初始化全局资源, 使用本地文件夹作为资源
    ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    resourcePath += _T("resources\\");
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath), dpiInitParam);

    uint64_t nTimeMS = std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
    std::string timeStamp = ui::StringUtil::Printf("%I64u", nTimeMS);
    MultiBrowserManager::GetInstance()->CreateBorwserBox(NULL, timeStamp, _T(""));
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

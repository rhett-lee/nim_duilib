#include "main.h"
#include "cef_form.h"

//开启DPI感知功能设置参数
ui::DpiInitParam dpiInitParam;

//定义应用程序的入口点
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

    HRESULT hr = ::OleInitialize(NULL);
    if (FAILED(hr))
        return 0;

    //必须在CefManager::Initialize前调用，设置DPI自适应属性，否则会导致显示不正常
    ui::GlobalManager::Instance().Dpi().InitDpiAwareness(dpiInitParam);

    // 初始化 CEF
    CefSettings settings;
    ui::FilePath appDataDir = ui::FilePathUtil::GetCurrentModuleDirectory();
    appDataDir += _T("cef_temp\\");//TODO: 需要一个可写目录
    if (!nim_comp::CefManager::GetInstance()->Initialize(appDataDir.ToString(), settings, kEnableOffsetRender)) {
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

    // 创建一个默认带有阴影的居中窗口
    CefForm* window = new CefForm();
    uint32_t dwExStyle = 0;
    if (nim_comp::CefManager::GetInstance()->IsEnableOffsetRender()) {
        dwExStyle |= ui::kWS_EX_LAYERED;
    }
    ui::WindowCreateParam createWndParam;
    createWndParam.m_className = _T("cef");
    createWndParam.m_windowTitle = createWndParam.m_className;
    createWndParam.m_dwExStyle = dwExStyle;
    window->CreateWnd(nullptr, createWndParam);
    window->CenterWindow();
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}

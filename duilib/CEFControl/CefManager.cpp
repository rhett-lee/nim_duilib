#include "CefManager.h"
#include "duilib/CEFControl/internal/CefClientApp.h"
#include "duilib/CEFControl/internal/CefBrowserHandler.h"
#include "duilib/CEFControl/CefControlNative.h"
#include "duilib/CEFControl/CefControlOffScreen.h"

#ifdef DUILIB_BUILD_FOR_WIN
    #include "CefDragDrop_Windows.h"
#endif

#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/WindowBase.h"

#pragma warning (push)
#pragma warning (disable:4100)
    #include "include/wrapper/cef_closure_task.h"
    #include "include/base/cef_bind.h"
    #include "include/base/cef_callback.h"
#pragma warning (pop)

#if CEF_VERSION_MAJOR <= 109
    //CEF 109版本
    #ifdef _WIN64
        //64位操作系统
        #pragma comment (lib, "duilib/third_party/libcef_win_109/lib/x64/libcef.lib")
        #ifdef _DEBUG
            #pragma comment (lib, "Libs/x64/libcef_dll_wrapper_109_d.lib")
        #else
            #pragma comment (lib, "Libs/x64/libcef_dll_wrapper_109.lib")
        #endif
    #else
        //32位操作系统
        #pragma comment (lib, "duilib/third_party/libcef_win_109/lib/Win32/libcef.lib")
        #ifdef _DEBUG
            #pragma comment (lib, "Libs/libcef_dll_wrapper_109_d.lib")
        #else
            #pragma comment (lib, "Libs/libcef_dll_wrapper_109.lib")
        #endif
    #endif
#else
    //CEF 高版本
    #ifdef _WIN64
        //64位操作系统
        #pragma comment (lib, "duilib/third_party/libcef_win/lib/x64/libcef.lib")
        #ifdef _DEBUG
            #pragma comment (lib, "Libs/x64/libcef_dll_wrapper_d.lib")
        #else
            #pragma comment (lib, "Libs/x64/libcef_dll_wrapper.lib")
        #endif
    #else
        //32位操作系统
        #pragma comment (lib, "duilib/third_party/libcef_win/lib/Win32/libcef.lib")
        #ifdef _DEBUG
            #pragma comment (lib, "Libs/libcef_dll_wrapper_d.lib")
        #else
            #pragma comment (lib, "Libs/libcef_dll_wrapper.lib")
        #endif
    #endif
#endif

namespace ui
{

#ifdef DUILIB_BUILD_FOR_WIN
// 发现一个非常奇葩的bug，离屏渲染+多线程消息循环模式下，在浏览器对象上右击弹出菜单，是无法正常关闭的
// 翻cef源码后发现菜单是用TrackPopupMenu函数创建的，在MSDN资料上查看后发现调用TrackPopupMenu前
// 需要给其父窗口调用SetForegroundWindow。但是在cef源码中没有调用
// 最终翻cef源码后得到的解决方法是在cef的UI线程创建一个窗口，这个窗体的父窗口必须是在主程序UI线程创建的
// 这样操作之后就不会出现菜单无法关闭的bug了，虽然不知道为什么但是bug解决了

// 另外还有个问题：如果不采取这个方法，离屏渲染的页面中，拖拽操作有异常，当拖出数据时，会卡死
//
static void FixContextMenuBug(HWND hwnd)
{
    ::CreateWindowW(L"Static", L"", WS_CHILD, 0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);
    ::PostMessage(hwnd, WM_CLOSE, 0, 0);
}
#endif

//创建CEF控件的回调函数
Control* DuilibCreateCefControl(const DString& className)
{
    Control* pControl = nullptr;
    if (className == _T("CefControl")) {
        if (ui::CefManager::GetInstance()->IsEnableOffScreenRendering()) {
            pControl = new CefControlOffScreen(nullptr);
        }
        else {
            pControl = new CefControlNative(nullptr);
        }
    }
    return pControl;
}

///////////////////////////////////////////////////////////////////////////////////
CefManager::CefManager():
    m_bEnableOffScreenRendering(true),
    m_browserCount(0),
    m_pfnAlreadyRunningAppRelaunch(nullptr)
{
}

CefManager::~CefManager()
{
}

CefManager* CefManager::GetInstance()
{
    static CefManager self;
    return &self;
}

#ifdef DUILIB_BUILD_FOR_WIN
//Windows系统
void CefManager::AddCefDllToPath()
{
    TCHAR path_envirom[4096] = { 0 };
    ::GetEnvironmentVariable(_T("path"), path_envirom, 4096);
    
    ui::FilePath cef_path = ui::FilePathUtil::GetCurrentModuleDirectory();

#if CEF_VERSION_MAJOR <= 109
    //CEF 109版本
    #ifdef _WIN64
        cef_path += _T("libcef_win_109\\x64");
    #else
        cef_path += _T("libcef_win_109\\Win32");
    #endif
#else
    //CEF 高版本
    #ifdef _WIN64
        cef_path += _T("libcef_win\\x64");
    #else
        cef_path += _T("libcef_win\\Win32");
    #endif
#endif

    ui::FilePath cefDllPath = cef_path;
    cefDllPath.NormalizeDirectoryPath();
    cefDllPath += ui::FilePath(L"libcef.dll");

    if (!cef_path.IsExistsDirectory() || !cefDllPath.IsExistsFile()) {
        DStringW errMsg = L"无法加载libcef.dll文件！\n请将libcef.dll等相关的CEF二进制文件和资源文件释放到以下目录：\n";
        errMsg += cef_path.ToStringW();
        ::MessageBoxW(nullptr, errMsg.c_str(), L"错误提示", MB_OK);
        exit(0);
    }
    DString new_envirom(cef_path.NativePath());
    new_envirom.append(_T(";")).append(path_envirom);
    ::SetEnvironmentVariable(_T("path"), new_envirom.c_str());
}

#else
//Linux系统
void CefManager::AddCefDllToPath()
{
}

#endif

// Cef的初始化接口，同时备注了使用各个版本的Cef时遇到的各种坑
// Cef1916版本较稳定，各个功能使用正常，但是某些在debug模式网页打开时会出中断警告（但并不是错误），可能是因为对新html标准支持不够，但是在release模式下正常使用
// Cef2357版本无法使用，当程序处理重定向信息并且重新加载页面后，渲染进程会崩掉
// Cef2526、2623版本对各种新页面都支持，唯一的坑就是debug模式在多线程消息循环开启下，程序退出时会中断，但是release模式正常。
//        (PS:如果开发者不使用负责Cef功能的开发，可以切换到release模式的cef dll文件，这样即使在deubg下也不会报错，修改AddCefDllToPath代码可以切换到release目录)
#ifdef DUILIB_BUILD_FOR_WIN
bool CefManager::Initialize(const DString& app_data_dir, CefSettings& settings, bool bEnableOffScreenRendering /*= true*/)
{
    m_bEnableOffScreenRendering = bEnableOffScreenRendering;
    CefMainArgs main_args(::GetModuleHandle(nullptr));

    CefRefPtr<CefClientApp> app(new CefClientApp);
    
    // 如果是在子进程中调用，会堵塞直到子进程退出，并且exit_code返回大于等于0
    // 如果在Browser进程中调用，则立即返回-1
    int exit_code = CefExecuteProcess(main_args, app.get(), nullptr);
    if (exit_code >= 0) {
        return false;
    }

    GetCefSetting(app_data_dir, settings);

    bool bRet = CefInitialize(main_args, settings, app.get(), nullptr);

    if (IsEnableOffScreenRendering()) {
        HWND hwnd = ::CreateWindowW(L"Static", L"", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
        CefPostTask(TID_UI, base::BindOnce(&FixContextMenuBug, hwnd));
    }
    
    //添加窗口CEF控件的回调函数
    GlobalManager::Instance().AddCreateControlCallback(DuilibCreateCefControl);
    return bRet;
}
#else
//Linux系统
bool CefManager::Initialize(const DString& app_data_dir, CefSettings& settings, bool bEnableOffScreenRendering, int argc, char** argv)
{
    m_bEnableOffScreenRendering = bEnableOffScreenRendering;
    CefMainArgs main_args(argc, argv);

    CefRefPtr<CefClientApp> app(new CefClientApp);

    // 如果是在子进程中调用，会堵塞直到子进程退出，并且exit_code返回大于等于0
    // 如果在Browser进程中调用，则立即返回-1
    int exit_code = CefExecuteProcess(main_args, app.get(), nullptr);
    if (exit_code >= 0) {
        return false;
    }

    GetCefSetting(app_data_dir, settings);

    bool bRet = CefInitialize(main_args, settings, app.get(), nullptr);

    //添加窗口CEF控件的回调函数
    GlobalManager::Instance().AddCreateControlCallback(DuilibCreateCefControl);
    return bRet;
}
#endif

void CefManager::SetAlreadyRunningAppRelaunch(const OnAlreadyRunningAppRelaunchEvent& callback)
{
    m_pfnAlreadyRunningAppRelaunch = callback;
}

OnAlreadyRunningAppRelaunchEvent CefManager::GetAlreadyRunningAppRelaunch() const
{
    return m_pfnAlreadyRunningAppRelaunch;
}

void CefManager::UnInitialize()
{
#ifdef DUILIB_BUILD_FOR_WIN
    CefDragDrop::GetInstance().Clear();
#endif
    CefShutdown();
}

bool CefManager::IsEnableOffScreenRendering() const
{
    return m_bEnableOffScreenRendering;
}

void CefManager::AddBrowserCount()
{
    m_browserCount++;
}

void CefManager::SubBrowserCount()
{
    m_browserCount--;
    ASSERT(m_browserCount >= 0);
}

int CefManager::GetBrowserCount()
{
    return m_browserCount;
}

void CefManager::PostQuitMessage(int32_t nExitCode)
{
    // 当我们需要结束进程时，千万不要直接调用::PostQuitMessage，这是可能还有浏览器对象没有销毁
    // 应该等所有浏览器对象都销毁后再调用::PostQuitMessage
    if (m_browserCount == 0) {
        GlobalManager::Instance().Thread().PostTask(kThreadUI, [nExitCode]() {
            NativeWindow::PostQuitMsg(0);
        });
    }
    else {
        auto cb = [nExitCode]()  {
            CefManager::GetInstance()->PostQuitMessage(nExitCode);
        };
        GlobalManager::Instance().Thread().PostDelayedTask(kThreadUI, cb, 500);
    }
}

void CefManager::GetCefSetting(const DString& app_data_dir, CefSettings& settings)
{
    DString appDataRootDir = app_data_dir;
    if (!appDataRootDir.empty()) {
#ifdef DUILIB_BUILD_FOR_WIN
        if (appDataRootDir[appDataRootDir.size() - 1] == _T('/')) {
            appDataRootDir[appDataRootDir.size() - 1] = _T('\\');
        }
#else
        if (appDataRootDir[appDataRootDir.size() - 1] == _T('\\')) {
            appDataRootDir[appDataRootDir.size() - 1] = _T('/');
        }
#endif
    }

    if (!ui::FilePath(appDataRootDir).IsExistsDirectory()) {
        ui::FilePathUtil::CreateDirectories(appDataRootDir);
    }
    settings.no_sandbox = true;

    // 设置localstorage，不要在路径末尾加"\\"，否则运行时会报错
    CefString(&settings.cache_path) = appDataRootDir + _T("CefLocalStorage");

    // 设置debug log文件位置
    CefString(&settings.log_file) = appDataRootDir + _T("cef.log");

    //设置日志级别
    //settings.log_severity = LOGSEVERITY_VERBOSE;

    // cef2623、2526版本debug模式:在使用multi_threaded_message_loop时退出程序会触发中断
    // 加入disable-extensions参数可以修复这个问题，但是会导致一些页面打开时报错
    // 开启Cef多线程消息循环，兼容nbase库消息循环
    settings.multi_threaded_message_loop = true;

    // 开启离屏渲染
    settings.windowless_rendering_enabled = IsEnableOffScreenRendering();

    //设置默认语言为简体中文
    CefString(&settings.locale) = _T("zh-CN");
}

}

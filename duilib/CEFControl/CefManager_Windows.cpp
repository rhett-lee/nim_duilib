#include "CefManager_Windows.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/ProcessSingleton.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/CEFControl/internal/CefClientApp.h"
#include "duilib/CEFControl/internal/CefBrowserHandler.h"

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/wrapper/cef_closure_task.h"
#include "include/base/cef_bind.h"
#include "include/base/cef_callback.h"
#pragma warning (pop)

namespace ui
{
CefManager_Windows::CefManager_Windows():
    m_pfnAlreadyRunningAppRelaunch(nullptr)
{
}

CefManager_Windows::~CefManager_Windows()
{
}

DString CefManager_Windows::GetCefMoudlePath() const
{
    DString cefMoudlePath = BaseClass::GetCefMoudlePath();
    if (cefMoudlePath.empty()) {
        //使用默认规则
#if CEF_VERSION_MAJOR <= 109
    //CEF 109版本
    #ifdef _WIN64
        cefMoudlePath = _T("libcef_win_109\\x64");
    #else
        cefMoudlePath = _T("libcef_win_109\\Win32");
    #endif
#else
    //CEF 高版本
    #ifdef _WIN64
        cefMoudlePath = _T("libcef_win\\x64");
    #else
        cefMoudlePath = _T("libcef_win\\Win32");
    #endif
#endif
    }
    return cefMoudlePath;
}

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

bool CefManager_Windows::Initialize(bool bEnableOffScreenRendering,
                                    const DString& appName,
                                    int argc,
                                    char** argv,
                                    OnCefSettingsEvent callback)
{
    if (!BaseClass::Initialize(bEnableOffScreenRendering, appName, argc, argv, callback)) {
        return false;
    }

    //把libcef.dll文件的所在路径添加到程序的"path"环境变量中
    AddCefDllToPath();

#if CEF_VERSION_MAJOR <= 109
    //CEF 109版本，控制进程单例
    // 解析命令行参数，识别是否为Browser进程
    if (!appName.empty()) {
        CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
        command_line->InitFromString(::GetCommandLineW());
        if (!command_line->HasSwitch("type")) {
            // Browser进程逻辑
            m_pProcessSingleton = ProcessSingleton::Create(appName);
            if ((m_pProcessSingleton != nullptr) && m_pProcessSingleton->IsAnotherInstanceRunning()) {
                //已经有其他Browser进程在运行, 发送启动参数后，退出
                std::vector<CefString> cmdLineArgv;
                command_line->GetArgv(cmdLineArgv);
                std::vector<std::string> argumentList;
                std::string arg;
                for (size_t i = 1; i < cmdLineArgv.size(); ++i) {
                    arg = StringConvert::WStringToUTF8(cmdLineArgv[i]);
                    if (!arg.empty()) {
                        argumentList.push_back(arg);
                    }
                }
                m_pProcessSingleton->SendArgumentsToExistingInstance(argumentList);
                return false;
            }
        }
    }
#endif

    CefMainArgs main_args(::GetModuleHandle(nullptr));
    CefRefPtr<CefClientApp> app(new CefClientApp);

    // 如果是在子进程中调用，会堵塞直到子进程退出，并且exit_code返回大于等于0
    // 如果在Browser进程中调用，则立即返回-1
    int exit_code = CefExecuteProcess(main_args, app.get(), nullptr);
    if (exit_code >= 0) {
        return false;
    }

    CefSettings settings;
    GetCefSetting(settings);

    bool bRet = CefInitialize(main_args, settings, app.get(), nullptr);
    if (!bRet) {
        return false;
    }

    if (IsEnableOffScreenRendering()) {
        HWND hwnd = ::CreateWindowW(L"Static", L"", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
        CefPostTask(TID_UI, base::BindOnce(&FixContextMenuBug, hwnd));
    }

#if CEF_VERSION_MAJOR <= 109
    //启动单例进程监控
    if (m_pProcessSingleton != nullptr) {
        m_pProcessSingleton->StartListener(OnBrowserAlreadyRunningAppRelaunch);
    }
#endif
    return true;
}

void CefManager_Windows::UnInitialize()
{
#if CEF_VERSION_MAJOR <= 109
    //启动单例进程监控
    if (m_pProcessSingleton != nullptr) {
        m_pProcessSingleton.reset();
    }
#endif
    BaseClass::UnInitialize();
}

void CefManager_Windows::SetAlreadyRunningAppRelaunch(const OnAlreadyRunningAppRelaunchEvent& callback)
{
    m_pfnAlreadyRunningAppRelaunch = callback;
}

OnAlreadyRunningAppRelaunchEvent CefManager_Windows::GetAlreadyRunningAppRelaunch() const
{
    return m_pfnAlreadyRunningAppRelaunch;
}

void CefManager_Windows::AddCefDllToPath()
{
    DString cefMoudlePath = GetCefMoudlePath();
    if (cefMoudlePath.empty()) {
        return;
    }

    TCHAR path_envirom[4096] = { 0 };
    ::GetEnvironmentVariable(_T("path"), path_envirom, 4096);

    FilePath cefDllDir = ui::FilePathUtil::GetCurrentModuleDirectory();
    cefDllDir /= FilePath(cefMoudlePath);
    cefDllDir.NormalizeDirectoryPath();
    FilePath cefDllPath = cefDllDir;
    cefDllPath /= FilePath(L"libcef.dll");
    if (!cefDllDir.IsExistsDirectory() || !cefDllPath.IsExistsFile()) {
        DStringW errMsg = L"无法加载libcef.dll文件！\n请将libcef.dll等相关的CEF二进制文件和资源文件释放到以下目录：\n";
        errMsg += cefDllDir.ToStringW();
        ::MessageBoxW(nullptr, errMsg.c_str(), L"错误提示", MB_OK);
        exit(0);
    }
    DString new_envirom(cefDllDir.NativePath());
    new_envirom.append(_T(";")).append(path_envirom);
    ::SetEnvironmentVariable(_T("path"), new_envirom.c_str());
}

#if CEF_VERSION_MAJOR <= 109
/** 浏览器单例控制回调函数
*/
void CefManager_Windows::OnBrowserAlreadyRunningAppRelaunch(const std::vector<DString>& argumentList)
{
    OnAlreadyRunningAppRelaunchEvent pfnAlreadyRunningAppRelaunch = CefManager::GetInstance()->GetAlreadyRunningAppRelaunch();
    if (pfnAlreadyRunningAppRelaunch != nullptr) {
        pfnAlreadyRunningAppRelaunch(argumentList);
    }
}
#endif

} //namespace ui

#endif //defined (DUILIB_BUILD_FOR_WIN)

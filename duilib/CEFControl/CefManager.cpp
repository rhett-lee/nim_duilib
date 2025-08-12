#include "CefManager.h"
#include "duilib/CEFControl/CefControlNative.h"
#include "duilib/CEFControl/CefControlOffScreen.h"

#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Box.h"

#if defined (DUILIB_BUILD_FOR_WIN)
    #include "CefManager_Windows.h"
#elif defined (DUILIB_BUILD_FOR_LINUX)
    #include "CefManager_Linux.h"
#elif defined (DUILIB_BUILD_FOR_MACOS)
    #include "CefManager_MacOS.h"
#endif

#pragma warning (push)
#pragma warning (disable:4100 4324)
    #include "include/base/cef_callback.h"
    #include "include/base/cef_bind.h"
    #include "include/wrapper/cef_closure_task.h"
#pragma warning (pop)

namespace ui
{
//创建CEF控件的回调函数
static Control* DuilibCreateCefControl(const DString& className)
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

//单位：毫秒
#define CEF_DO_MESSAGE_LOOP_WORK_DELAY_MS 60

///////////////////////////////////////////////////////////////////////////////////
CefManager::CefManager():
    m_logSeverity(LOGSEVERITY_DEFAULT),
    m_browserCount(0),
    m_nCefDoMessageLoopWorkDelayMs(CEF_DO_MESSAGE_LOOP_WORK_DELAY_MS),
    m_nExitCode(0),
    m_bHasCefCachePath(false),
    m_bEnableOffScreenRendering(true),
    m_bCefInit(false),
    m_bCefMessageLoopEmpty(false)
{
#ifdef DUILIB_BUILD_FOR_MACOS
    m_bExiting = false;
#endif
}

CefManager::~CefManager()
{
}

CefManager* CefManager::GetInstance()
{
#if defined (DUILIB_BUILD_FOR_WIN)
    static CefManager_Windows self;
    return &self;
#elif defined (DUILIB_BUILD_FOR_LINUX)
    static CefManager_Linux self;
    return &self;
#elif defined (DUILIB_BUILD_FOR_MACOS)
    static CefManager_MacOS self;
    return &self;
#else
    ASSERT(0);
    return nullptr;
#endif
}

void CefManager::SetCefCachePath(const DString& cefCachePath)
{
    ASSERT(!m_bCefInit);
    m_cefCachePath = cefCachePath;
    m_bHasCefCachePath = true;
}

DString CefManager::GetCefCachePath() const
{
    if (m_bHasCefCachePath) {
        return m_cefCachePath;
    }
    DString defaultCachePath = _T("cef_cache");
    defaultCachePath += FilePath::GetPathSeparatorStr();
    defaultCachePath += m_appName;
    defaultCachePath += FilePath::GetPathSeparatorStr();
    return defaultCachePath;
}

void CefManager::SetCefMoudlePath(const DString& cefMoudlePath)
{
    ASSERT(!m_bCefInit);
    m_cefMoudlePath = cefMoudlePath;
}

DString CefManager::GetCefMoudlePath() const
{
    return m_cefMoudlePath;
}

void CefManager::SetCefLanguage(const DString& lang)
{
    ASSERT(!m_bCefInit);
    m_lang = lang;
}

DString CefManager::GetCefLanguage() const
{
    if (!m_lang.empty()) {
        return m_lang;
    }
    return _T("zh-CN");
}

void CefManager::SetLogSeverity(cef_log_severity_t log_severity)
{
    ASSERT(!m_bCefInit);
    m_logSeverity = log_severity;
}

cef_log_severity_t CefManager::GetLogSeverity() const
{
    return m_logSeverity;
}

bool CefManager::Initialize(bool bEnableOffScreenRendering,
                            const DString& appName,
                            int /*argc*/,
                            char** /*argv*/,
                            OnCefSettingsEvent callback)
{
    ASSERT(!appName.empty());
    ASSERT(!m_bCefInit);
    if (m_bCefInit || appName.empty()) {
        return false;
    }
    m_appName = appName;
    m_cefSettingCallback = callback;
    m_bEnableOffScreenRendering = bEnableOffScreenRendering;

    //添加窗口CEF控件的回调函数
    GlobalManager::Instance().AddCreateControlCallback(DuilibCreateCefControl);
    m_bCefInit = true;
    return true;
}

void CefManager::UnInitialize()
{
    if (m_bCefInit) {
        m_bCefInit = false;
        CefShutdown();
    }
}

bool CefManager::IsCefInited() const
{
    return m_bCefInit;
}

void CefManager::SetAlreadyRunningAppRelaunch(const OnAlreadyRunningAppRelaunchEvent& /*callback*/)
{
}

OnAlreadyRunningAppRelaunchEvent CefManager::GetAlreadyRunningAppRelaunch() const
{
    return nullptr;
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

int32_t CefManager::GetBrowserCount()
{
    return m_browserCount;
}

static void GetCefControlList(Box* pRoot, std::vector<Control*>& cefControlList)
{
    if (pRoot == nullptr) {
        return;
    }
    std::vector<Box*> boxList;
    size_t nItemCount = pRoot->GetItemCount();
    for (size_t nItem = 0; nItem < nItemCount; ++nItem) {
        Control* pControl = pRoot->GetItemAt(nItem);
        if (pControl == nullptr) {
            continue;
        }
        if (dynamic_cast<CefControl*>(pControl) != nullptr) {
            cefControlList.push_back(pControl);
        }
        Box* pBox = dynamic_cast<Box*>(pControl);
        if (pBox != nullptr) {
            boxList.push_back(pBox);
        }
    }

    for (Box* pBox : boxList) {
        GetCefControlList(pBox, cefControlList);
    }
}

void CefManager::ProcessWindowCloseEvent(Window* pWindow)
{
    Box* pRoot = nullptr;
    if (pWindow != nullptr) {
        pRoot = pWindow->GetRoot();
    }
    std::vector<Control*> cefControlList;
    GetCefControlList(pRoot, cefControlList);

    for (Control* pControl : cefControlList) {
        CefControl* pCefControl = dynamic_cast<CefControl*>(pControl);
        if (pCefControl != nullptr) {
            pCefControl->OnHostWindowClosed();
        }
    }
}

void CefManager::PostQuitMessage(int32_t nExitCode)
{
    m_nExitCode = nExitCode;
    bool bForceExit = false;
#ifdef DUILIB_BUILD_FOR_MACOS
    //解决MacOS下，子窗口模式退出时进程残留问题（未调用CefBrowserHandler::OnBeforeClose函数，原因未查明）
    if (!m_bExiting) {
        m_bExiting = true;
        m_exitTime = std::chrono::steady_clock::now();
    }
    else {
        auto waitSeconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_exitTime);
        if (waitSeconds.count() > 15) {
            //等待超过15秒，则强制退出
            bForceExit = true;
        }
    }
#endif

    // 当我们需要结束进程时，千万不要直接调用::PostQuitMessage，这是可能还有浏览器对象没有销毁
    // 应该等所有浏览器对象都销毁后再调用::PostQuitMessage
    if (bForceExit || (m_browserCount <= 0)) {
        if (IsMultiThreadedMessageLoop()) {
            if (m_bCefMessageLoopEmpty) {
                //启用CEF消息循环：退出主线程的消息循环
                GlobalManager::Instance().Thread().PostTask(kThreadUI, [nExitCode]() {
                        NativeWindow::PostQuitMsg(nExitCode);
                    });
            }
            CefPostTask(TID_UI, base::BindOnce([]() {
                //响应后，标记消息队列为空(当CEF消息循环中有待处理的事项时，直接退出主线程消息循环会有偶发崩溃问题)
                CefManager::GetInstance()->m_bCefMessageLoopEmpty = true;
                //未启用CEF消息循环：直接退出主线程的消息循环
                GlobalManager::Instance().Thread().PostTask(kThreadUI, []() {
                    NativeWindow::PostQuitMsg(CefManager::GetInstance()->m_nExitCode);
                    });
                }));
        }
        else {
            //未启用CEF消息循环：直接退出主线程的消息循环
            GlobalManager::Instance().Thread().PostTask(kThreadUI, [nExitCode]() {
                    NativeWindow::PostQuitMsg(nExitCode);
                });
        }
    }
    else {
        auto cb = [nExitCode]()  {
            CefManager::GetInstance()->PostQuitMessage(nExitCode);
        };
        GlobalManager::Instance().Thread().PostDelayedTask(kThreadUI, cb, 200);
    }
}

bool CefManager::IsMultiThreadedMessageLoop() const
{
    return true;
}

void CefManager::GetCefSetting(CefSettings& settings)
{
    DString appDataRootDir = GetCefCachePath();
    if (!appDataRootDir.empty()) {
        FilePath filePath(appDataRootDir);
        filePath.NormalizeDirectoryPath();
        if(!filePath.IsAbsolutePath()) {
            FilePath runPath = FilePathUtil::GetCurrentModuleDirectory();
            runPath /= filePath;
            filePath.Swap(runPath);
            filePath.NormalizeDirectoryPath();
        }
        appDataRootDir = filePath.NativePath();
        if (!appDataRootDir.empty() && !filePath.IsExistsDirectory()) {
            FilePathUtil::CreateDirectories(appDataRootDir);
        }
    }
    settings.no_sandbox = true;

    //设置localstorage，不要在路径末尾加"\\"，否则运行时会报错
    if (!appDataRootDir.empty()) {
        const DString cachePath = appDataRootDir + _T("CefLocalStorage");
        CefString(&settings.cache_path) = cachePath;
        CefString(&settings.root_cache_path) = cachePath;
    }

    //设置日志级别
    settings.log_severity = GetLogSeverity();

    // 设置debug log文件位置
    if (settings.log_severity != cef_log_severity_t::LOGSEVERITY_DISABLE) {
        CefString(&settings.log_file) = appDataRootDir + _T("cef.log");
    }

    // cef2623、2526版本debug模式:在使用multi_threaded_message_loop时退出程序会触发中断
    // 加入disable-extensions参数可以修复这个问题，但是会导致一些页面打开时报错
    // 开启Cef多线程消息循环，兼容nbase库消息循环
    settings.multi_threaded_message_loop = IsMultiThreadedMessageLoop();

    // 开启离屏渲染
    settings.windowless_rendering_enabled = IsEnableOffScreenRendering();

    //设置默认语言为简体中文
    CefString(&settings.locale) = GetCefLanguage();

    //给应用层回调，提供修改的机会
    if (m_cefSettingCallback) {
        bool bMultiThreadedMessageLoop = settings.multi_threaded_message_loop;
        m_cefSettingCallback(settings);

        //恢复不允许修改的值
        settings.multi_threaded_message_loop = bMultiThreadedMessageLoop;
    }

    if (!settings.multi_threaded_message_loop) {
        //需要启用外部调用CEF消息循环模式
        settings.external_message_pump = true;
    }
    else {
        settings.external_message_pump = false;
    }
}

void CefManager::ScheduleCefDoMessageLoopWork()
{
    ASSERT(!IsMultiThreadedMessageLoop());
    if (!IsMultiThreadedMessageLoop()) {
        int32_t delayMs = GetCefDoMessageLoopWorkDelayMs();
        GlobalManager::Instance().Thread().PostRepeatedTask(ui::kThreadUI, []() {
                // 执行单次 CEF 消息处理
                if (ui::CefManager::GetInstance()->IsCefInited()) {
                    CefDoMessageLoopWork();
                }
            }, delayMs);
    }
}

void CefManager::SetCefDoMessageLoopWorkDelayMs(int32_t nCefDoMessageLoopWorkDelayMs)
{
    m_nCefDoMessageLoopWorkDelayMs = nCefDoMessageLoopWorkDelayMs;
    if (m_nCefDoMessageLoopWorkDelayMs < 1) {
        m_nCefDoMessageLoopWorkDelayMs = CEF_DO_MESSAGE_LOOP_WORK_DELAY_MS;
    }
}

int32_t CefManager::GetCefDoMessageLoopWorkDelayMs() const
{
    return m_nCefDoMessageLoopWorkDelayMs;
}

} //namespace ui

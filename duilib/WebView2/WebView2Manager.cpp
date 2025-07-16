#include "WebView2Manager.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

#include "duilib/WebView2/WebView2Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/FilePathUtil.h"
#include <objbase.h>

namespace ui {

//创建WebView控件的回调函数
static Control* DuilibCreateWebView2Control(const DString& className)
{
    Control* pControl = nullptr;
    if (className == _T("WebView2Control")) {
        pControl = new WebView2Control(nullptr);
    }
    return pControl;
}

WebView2Manager::WebView2Manager():
    m_bScriptEnabled(true),
    m_bWebMessageEnabled(true),
    m_bAreDefaultScriptDialogsEnabled(true),
    m_bAreDefaultContextMenusEnabled(true),
    m_bZoomControlEnabled(true),
    m_bAreDevToolsEnabled(true)
{
}

WebView2Manager::~WebView2Manager()
{
}

WebView2Manager& WebView2Manager::GetInstance()
{
    static WebView2Manager self;
    return self;
}

bool WebView2Manager::Initialize(const DString& userDataFolder,
                                 const DString& language,
                                 const DString& userAgent,
                                 const DString& additionalBrowserArguments,
                                 const DString& browserExecutableFolder)
{
    m_userDataFolder = userDataFolder;
    m_language = language;
    m_userAgent = userAgent;
    m_additionalBrowserArguments = additionalBrowserArguments;
    m_browserExecutableFolder = browserExecutableFolder;

    //添加窗口WebView2控件的回调函数
    GlobalManager::Instance().AddCreateControlCallback(DuilibCreateWebView2Control);
    return true;
}

void WebView2Manager::UnInitialize()
{
}

DString WebView2Manager::GetDefaultUserDataFolder(const DString& appName) const
{
    DString defaultCachePath = _T("webview2_cache");
    defaultCachePath += ui::FilePath::GetPathSeparatorStr();
    defaultCachePath += appName;
    defaultCachePath += ui::FilePath::GetPathSeparatorStr();

    FilePath runPath = FilePathUtil::GetCurrentModuleDirectory();
    runPath /= FilePath(defaultCachePath);
    runPath.FormatPathAsDirectory();
    return runPath.ToString();
}

void WebView2Manager::SetBrowserExecutableFolder(const DString& browserExecutableFolder)
{
    m_browserExecutableFolder = browserExecutableFolder;
}

const DString& WebView2Manager::GetBrowserExecutableFolder() const
{
    return m_browserExecutableFolder;
}

void WebView2Manager::SetUserDataFolder(const DString& userDataFolder)
{
    m_userDataFolder = userDataFolder;
}

const DString& WebView2Manager::GetUserDataFolder() const
{
    return m_userDataFolder;
}

void WebView2Manager::SetLanguage(const DString& language)
{
    m_language = language;
}

const DString& WebView2Manager::GetLanguage() const
{
    return m_language;
}

void WebView2Manager::SetUserAgent(const DString& userAgent)
{
    m_userAgent = userAgent;
}

const DString& WebView2Manager::GetUserAgent() const
{
    return m_userAgent;
}

void WebView2Manager::SetAdditionalBrowserArguments(const DString& additionalBrowserArguments)
{
    m_additionalBrowserArguments = additionalBrowserArguments;
}

const DString& WebView2Manager::GetAdditionalBrowserArguments() const
{
    return m_additionalBrowserArguments;
}

void WebView2Manager::SetScriptEnabled(bool bScriptEnabled)
{
    m_bScriptEnabled = bScriptEnabled;
}

bool WebView2Manager::IsScriptEnabled() const
{
    return m_bScriptEnabled;
}

void WebView2Manager::SetWebMessageEnabled(bool bWebMessageEnabled)
{
    m_bWebMessageEnabled = bWebMessageEnabled;
}

bool WebView2Manager::IsWebMessageEnabled() const
{
    return m_bWebMessageEnabled;
}

void WebView2Manager::SetAreDefaultScriptDialogsEnabled(bool bAreDefaultScriptDialogsEnabled)
{
    m_bAreDefaultScriptDialogsEnabled = bAreDefaultScriptDialogsEnabled;
}

bool WebView2Manager::AreDefaultScriptDialogsEnabled() const
{
    return m_bAreDefaultScriptDialogsEnabled;
}

void WebView2Manager::SetAreDefaultContextMenusEnabled(bool bAreDefaultContextMenusEnabled)
{
    m_bAreDefaultContextMenusEnabled = bAreDefaultContextMenusEnabled;
}

bool WebView2Manager::AreDefaultContextMenusEnabled() const
{
    return m_bAreDefaultContextMenusEnabled;
}

void WebView2Manager::SetZoomControlEnabled(bool bZoomControlEnabled)
{
    m_bZoomControlEnabled = bZoomControlEnabled;
}

bool WebView2Manager::IsZoomControlEnabled() const
{
    return m_bZoomControlEnabled;
}

void WebView2Manager::SetAreDevToolsEnabled(bool bAreDevToolsEnabled)
{
    m_bAreDevToolsEnabled = bAreDevToolsEnabled;
}

bool WebView2Manager::AreDevToolsEnabled() const
{
    return m_bAreDevToolsEnabled;
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WEBVIEW2

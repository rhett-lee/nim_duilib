#ifndef UI_WEBVIEW2_WEBVIEW2_MANAGER_H_
#define UI_WEBVIEW2_WEBVIEW2_MANAGER_H_

#include "duilib/Core/UiTypes.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

namespace ui {

/** WebView2控件的公共配置管理
 */
class WebView2Manager
{
public:
    WebView2Manager();
    ~WebView2Manager();
    WebView2Manager(const WebView2Manager&) = delete;
    WebView2Manager& operator=(const WebView2Manager&) = delete;

public:
    /** 单例对象
    */
    static WebView2Manager& GetInstance();

    /** 初始化
    * @param [in] userDataFolder 网页缓存目录
    * @param [in] language 语言：如"zh-CN"、"en-US"等
    * @param [in] userAgent UserAgent字符串
    * @param [in] additionalBrowserArguments 额外浏览器参数
    * @param [in] browserExecutableFolder WebView2模块所在目录
    */
    bool Initialize(const DString& userDataFolder = _T(""),
                    const DString& language = _T(""),
                    const DString& userAgent = _T(""),
                    const DString& additionalBrowserArguments = _T(""),
                    const DString& browserExecutableFolder = _T(""));

    /** 清理
    */
    void UnInitialize();

    /** 获取默认的网页缓存目录
    * @param [in] appName 应用程序名称
    */
    DString GetDefaultUserDataFolder(const DString& appName) const;

public:
    /** WebView2模块所在目录
    */
    void SetBrowserExecutableFolder(const DString& browserExecutableFolder);
    const DString& GetBrowserExecutableFolder() const;

    /** 网页缓存目录
    */
    void SetUserDataFolder(const DString& userDataFolder);
    const DString& GetUserDataFolder() const;

    /** 语言
    */
    void SetLanguage(const DString& language);
    const DString& GetLanguage() const;

    /** UserAgent
    */
    void SetUserAgent(const DString& userAgent);
    const DString& GetUserAgent() const;

    /** 额外的参数
    */
    void SetAdditionalBrowserArguments(const DString& additionalBrowserArguments);
    const DString& GetAdditionalBrowserArguments() const;

    /** 是否启用JavaScript
    */
    void SetScriptEnabled(bool bScriptEnabled);
    bool IsScriptEnabled() const;

    /** 是否启用Web消息
    */
    void SetWebMessageEnabled(bool bWebMessageEnabled);
    bool IsWebMessageEnabled() const;

    /** 是否启用默认脚本对话框
    */
    void SetAreDefaultScriptDialogsEnabled(bool bAreDefaultScriptDialogsEnabled);
    bool AreDefaultScriptDialogsEnabled() const;

    /** 是否允许默认的右键菜单
    */
    void SetAreDefaultContextMenusEnabled(bool bAreDefaultContextMenusEnabled);
    bool AreDefaultContextMenusEnabled() const;

    /** 是否禁用缩放控制
    */
    void SetZoomControlEnabled(bool bZoomControlEnabled);
    bool IsZoomControlEnabled() const;

    /** 是否启用开发者工具
    */
    void SetAreDevToolsEnabled(bool bAreDevToolsEnabled);
    bool AreDevToolsEnabled() const;

public:
    /** WebView2模块所在目录
    */
    DString m_browserExecutableFolder;

    /** 网页缓存目录
    */
    DString m_userDataFolder;

    /** 语言
    */
    DString m_language;

    /** UserAgent
    */
    DString m_userAgent;

    /** 额外的参数
    */
    DString m_additionalBrowserArguments;

    /** 是否启用JavaScript
    */
    bool m_bScriptEnabled;

    /** 是否启用Web消息
    */
    bool m_bWebMessageEnabled;
    
    /** 是否启用默认脚本对话框
    */
    bool m_bAreDefaultScriptDialogsEnabled;

    /** 是否允许默认的右键菜单
    */
    bool m_bAreDefaultContextMenusEnabled;

    /** 是否禁用缩放控制
    */
    bool m_bZoomControlEnabled;

    /** 是否启用开发者工具
    */
    bool m_bAreDevToolsEnabled;
};

} //namespace ui

#endif //DUILIB_BUILD_FOR_WEBVIEW2

#endif //UI_WEBVIEW2_WEBVIEW2_MANAGER_H_

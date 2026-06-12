#include "duilib/Utils/TrayIcon.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Utils/ApiWrapper_Windows.h"

#include <shellapi.h>

namespace ui
{

// 自定义消息ID
#define WM_TRAYICON_MESSAGE (WM_USER + 1024)

/** 托盘图标的Windows实现
*/
class TrayIconImpl : public TrayIcon
{
public:
    TrayIconImpl();
    virtual ~TrayIconImpl() override;

    /** 初始化托盘图标
    * @param [in] pWindow 关联的窗口指针
    * @param [in] iconFilePath 图标文件路径（支持ico格式）
    * @param [in] tooltip 托盘提示文本
    * @return 初始化成功返回true，失败返回false
    */
    bool Initialize(const Window* pWindow, const DString& iconFilePath, const DString& tooltip);

public:
    virtual bool SetIcon(const Window* pWindow, const DString& iconFilePath) override;
    virtual bool SetTooltip(const DString& tooltip) override;
    virtual bool ShowBalloon(const DString& title, const DString& content, uint32_t timeoutMs = 3000) override;
    virtual bool Hide() override;
    virtual bool Show() override;
    virtual bool IsTrayVisible() const override;
    virtual bool Remove() override;
    virtual void* GetTrayHandle() const override;

private:
    /** 加载图标
    * @param [in] iconFilePath 图标文件路径
    * @return 图标句柄，失败返回nullptr
    */
    HICON LoadIconFromFile(const Window* pWindow, const DString& iconFilePath);

    /** 从文件数据加载图标
    * @param [in] fileData 文件数据
    * @param [in] iconFilePath 文件路径
    * @return 图标句柄，失败返回nullptr
    */
    HICON LoadIconFromFileData(const std::vector<uint8_t>& fileData, const DString& iconFilePath);

    /** 更新托盘图标
    * @param [in] dwMessage 消息类型（NIM_ADD, NIM_MODIFY, NIM_DELETE）
    * @return 成功返回true，失败返回false
    */
    bool UpdateTrayIcon(DWORD dwMessage);

    /** 窗口过程函数，用于接收托盘图标消息
    */
    static LRESULT CALLBACK TrayIconWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    /** 消息窗口句柄
    */
    HWND m_hWnd;

    /** 托盘图标ID
    */
    UINT m_uID;

    /** 图标句柄
    */
    HICON m_hIcon;

    /** 提示文本
    */
    DString m_tooltip;

    /** 是否隐藏
    */
    bool m_bHidden;

    /** 鼠标按钮按下标记
    */
    bool m_bLeftButtonDown;
    bool m_bRightButtonDown;
    bool m_bMiddleButtonDown;

    /** 下一个可用的托盘图标ID
    */
    static UINT m_nextID;
};

UINT TrayIconImpl::m_nextID = 1;

TrayIconImpl::TrayIconImpl() :
    m_hWnd(nullptr),
    m_uID(0),
    m_hIcon(nullptr),
    m_bHidden(false),
    m_bLeftButtonDown(false),
    m_bRightButtonDown(false),
    m_bMiddleButtonDown(false)
{
}

TrayIconImpl::~TrayIconImpl()
{
    Remove();
    if (m_hWnd != nullptr) {
        ::DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

//窗口类的名称
#define DUILIB_TRAY_MESSAGE_WINDOW_CLASS L"TrayIconMessageWindow"

bool TrayIconImpl::Initialize(const Window* pWindow, const DString& iconFilePath, const DString& tooltip)
{
    // 创建一个隐藏的消息窗口用于接收托盘消息
    HINSTANCE hInstance = (HINSTANCE)GlobalManager::Instance().GetPlatformData();
    if (hInstance == nullptr) {
        hInstance = ::GetModuleHandle(nullptr);
    }
    
    // 注册窗口类
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = TrayIconWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = DUILIB_TRAY_MESSAGE_WINDOW_CLASS;
    ATOM ret = ::RegisterClassExW(&wc);
    ASSERT_UNUSED_VARIABLE(ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);

    //在模块退出时，注销该ATOM
    static bool bAddAtExitFunction = false;
    if (!bAddAtExitFunction) {
        bAddAtExitFunction = true;
        GlobalManager::Instance().AddAtExitFunction([hInstance]() {
            ::UnregisterClassW(DUILIB_TRAY_MESSAGE_WINDOW_CLASS, hInstance);
            });
    }

    m_hWnd = ::CreateWindowExW(0, wc.lpszClassName, L"", WS_POPUP, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInstance, nullptr);
    if (m_hWnd == nullptr) {
        return false;
    }

    // 将this指针设置到窗口属性中
    ::SetPropW(m_hWnd, L"TrayIconImpl", this);

    m_uID = m_nextID++;
    m_tooltip = tooltip;
    m_bHidden = false;

    // 加载图标
    m_hIcon = LoadIconFromFile(pWindow, iconFilePath);
    if (m_hIcon == nullptr) {
        // 使用默认图标
        m_hIcon = ::LoadIcon(nullptr, IDI_APPLICATION);
    }

    // 添加托盘图标
    return UpdateTrayIcon(NIM_ADD);
}

HICON TrayIconImpl::LoadIconFromFile(const Window* pWindow, const DString& iconFilePath)
{
    if (iconFilePath.empty()) {
        return nullptr;
    }

    FilePath windowResPath;
    FilePath windowXmlPath;
    if (pWindow != nullptr) {
        windowResPath = pWindow->GetResourcePath();
        windowXmlPath = pWindow->GetXmlPath();
    }
    FilePath iconFullPath = GlobalManager::Instance().GetExistsResFullPath(windowResPath, windowXmlPath, FilePath(iconFilePath));
    ASSERT(!iconFullPath.IsEmpty());
    if (iconFullPath.IsEmpty()) {
        return nullptr;
    }

    std::vector<uint8_t> fileData;
    if (GlobalManager::Instance().Zip().IsUseZip() &&
        GlobalManager::Instance().Zip().IsZipResExist(iconFullPath)) {
        //使用压缩包        
        GlobalManager::Instance().Zip().GetZipData(iconFullPath, fileData);
        return LoadIconFromFileData(fileData, iconFullPath.ToString());
    }
    else {
        //使用本地文件
        ASSERT(iconFullPath.IsExistsFile());
        if (!iconFullPath.IsExistsFile()) {
            return nullptr;
        }
        // 尝试从资源路径加载图标
        HICON hIcon = static_cast<HICON>(::LoadImageW(nullptr, StringConvert::TToWString(iconFullPath.ToStringW()).c_str(),
                                                      IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED));
        if (hIcon != nullptr) {
            return hIcon;
        }
        FileUtil::ReadFileData(iconFullPath, fileData);
        return LoadIconFromFileData(fileData, iconFullPath.ToString());
    }
}

HICON TrayIconImpl::LoadIconFromFileData(const std::vector<uint8_t>& fileData, const DString& iconFilePath)
{
    uint32_t uDpiScaleFactor = ui::GlobalManager::Instance().Dpi().GetDisplayScaleFactor();
    HICON hSmallIcon = nullptr;
    CreateIconsFromData(fileData, iconFilePath, uDpiScaleFactor, nullptr, &hSmallIcon);
    return hSmallIcon;
}

bool TrayIconImpl::UpdateTrayIcon(DWORD dwMessage)
{
    if (m_hWnd == nullptr) {
        return false;
    }

    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = m_hWnd;
    nid.uID = m_uID;
    nid.uFlags = NIF_MESSAGE;
    nid.uCallbackMessage = WM_TRAYICON_MESSAGE;

    if (dwMessage == NIM_ADD || dwMessage == NIM_MODIFY) {
        if (m_hIcon != nullptr) {
            nid.uFlags |= NIF_ICON;
            nid.hIcon = m_hIcon;
        }
        if (!m_tooltip.empty()) {
            nid.uFlags |= NIF_TIP;
            StringUtil::StringCopy(nid.szTip, _countof(nid.szTip), StringConvert::TToWString(m_tooltip).c_str());
        }
    }

    BOOL result = ::Shell_NotifyIconW(dwMessage, &nid);
    if (!result && dwMessage == NIM_ADD) {
        // 如果添加失败，尝试先删除再添加
        ::Shell_NotifyIconW(NIM_DELETE, &nid);
        result = ::Shell_NotifyIconW(NIM_ADD, &nid);
    }
    return result == TRUE;
}

bool TrayIconImpl::SetIcon(const Window* pWindow, const DString& iconFilePath)
{
    if (m_hIcon != nullptr) {
        ::DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }

    m_hIcon = LoadIconFromFile(pWindow, iconFilePath);
    if (m_hIcon == nullptr) {
        // 使用默认图标
        m_hIcon = ::LoadIcon(nullptr, IDI_APPLICATION);
    }
    return UpdateTrayIcon(NIM_MODIFY);
}

bool TrayIconImpl::SetTooltip(const DString& tooltip)
{
    m_tooltip = tooltip;
    return UpdateTrayIcon(NIM_MODIFY);
}

bool TrayIconImpl::ShowBalloon(const DString& title, const DString& content, uint32_t timeoutMs)
{
    if (m_hWnd == nullptr || m_bHidden) {
        return false;
    }

    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = m_hWnd;
    nid.uID = m_uID;
    nid.uFlags = NIF_INFO;
    nid.uTimeout = timeoutMs;
    nid.dwInfoFlags = NIIF_INFO;

    if (!title.empty()) {
        StringUtil::StringCopy(nid.szInfoTitle, _countof(nid.szInfoTitle), StringConvert::TToWString(title).c_str());
    }
    if (!content.empty()) {
        StringUtil::StringCopy(nid.szInfo, _countof(nid.szInfo), StringConvert::TToWString(content).c_str());
    }

    return ::Shell_NotifyIconW(NIM_MODIFY, &nid) == TRUE;
}

bool TrayIconImpl::Hide()
{
    if (m_bHidden) {
        return true;
    }
    m_bHidden = true;
    return UpdateTrayIcon(NIM_DELETE);
}

bool TrayIconImpl::Show()
{
    if (!m_bHidden) {
        return true;
    }
    m_bHidden = false;
    return UpdateTrayIcon(NIM_ADD);
}

bool TrayIconImpl::IsTrayVisible() const
{
    return (m_hWnd != nullptr) && !m_bHidden;
}

bool TrayIconImpl::Remove()
{
    if (m_hIcon != nullptr) {
        ::DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
    bool result = UpdateTrayIcon(NIM_DELETE);
    m_bHidden = true;
    return result;
}

void* TrayIconImpl::GetTrayHandle() const
{
    return (void*)m_hWnd;
}

LRESULT CALLBACK TrayIconImpl::TrayIconWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TrayIconImpl* pTrayIcon = reinterpret_cast<TrayIconImpl*>(::GetPropW(hWnd, L"TrayIconImpl"));
    if (pTrayIcon == nullptr) {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    if (uMsg == WM_TRAYICON_MESSAGE) {
        UINT uMouseMsg = LOWORD(lParam);
        POINT pt = { 0 };
        ::GetCursorPos(&pt);

        switch (uMouseMsg) {
        case WM_LBUTTONDOWN:
            // 左键按下，标记状态
            pTrayIcon->m_bLeftButtonDown = true;
            break;
        case WM_LBUTTONUP:
            // 左键弹起，检测是否有按下标记
            if (pTrayIcon->m_bLeftButtonDown) {
                pTrayIcon->m_bLeftButtonDown = false;
                pTrayIcon->NotifyMessage(TrayIconMessageType::kLeftClick, pt.x, pt.y);
            }
            break;
        case WM_LBUTTONDBLCLK:
            // 双击事件（双击时不会收到第二个DOWN消息，直接收到DBLCLK）
            pTrayIcon->m_bLeftButtonDown = false;
            pTrayIcon->NotifyMessage(TrayIconMessageType::kLeftDoubleClick, pt.x, pt.y);
            break;
        case WM_RBUTTONDOWN:
            // 右键按下，标记状态
            pTrayIcon->m_bRightButtonDown = true;
            break;
        case WM_RBUTTONUP:
            // 右键弹起，检测是否有按下标记
            if (pTrayIcon->m_bRightButtonDown) {
                pTrayIcon->m_bRightButtonDown = false;
                pTrayIcon->NotifyMessage(TrayIconMessageType::kRightClick, pt.x, pt.y);
            }
            break;
        case WM_CONTEXTMENU:
            // 上下文菜单消息（有些系统直接发送此消息）
            // 重置右键标记，避免重复触发
            pTrayIcon->m_bRightButtonDown = false;
            pTrayIcon->NotifyMessage(TrayIconMessageType::kRightClick, pt.x, pt.y);
            break;
        case WM_RBUTTONDBLCLK:
            pTrayIcon->m_bRightButtonDown = false;
            pTrayIcon->NotifyMessage(TrayIconMessageType::kRightDoubleClick, pt.x, pt.y);
            break;
        case WM_MBUTTONDOWN:
            // 中键按下，标记状态
            pTrayIcon->m_bMiddleButtonDown = true;
            break;
        case WM_MBUTTONUP:
            // 中键弹起，检测是否有按下标记
            if (pTrayIcon->m_bMiddleButtonDown) {
                pTrayIcon->m_bMiddleButtonDown = false;
                pTrayIcon->NotifyMessage(TrayIconMessageType::kMiddleClick, pt.x, pt.y);
            }
            break;
        case NIN_BALLOONUSERCLICK:
            pTrayIcon->NotifyMessage(TrayIconMessageType::kShowBalloon, pt.x, pt.y);
            break;
        case WM_MOUSEMOVE:
            pTrayIcon->NotifyMessage(TrayIconMessageType::kMouseMove, pt.x, pt.y);
            break;
        default:
            // 其他消息，不处理
            break;
        }
        return 0;
    }

    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// TrayIcon 基类的Create函数，Windows平台实现
std::unique_ptr<TrayIcon> TrayIcon::Create(const Window* pWindow, const DString& iconFilePath, const DString& tooltip)
{
    std::unique_ptr<TrayIconImpl> pTrayIcon = std::make_unique<TrayIconImpl>();
    if (pTrayIcon->Initialize(pWindow, iconFilePath, tooltip)) {
        return pTrayIcon;
    }
    return nullptr;
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN

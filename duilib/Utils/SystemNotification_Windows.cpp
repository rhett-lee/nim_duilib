#include "duilib/Utils/SystemNotification.h"

#if defined(DUILIB_BUILD_FOR_WIN) && !defined(DUILIB_BUILD_FOR_SDL)

#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"

#include <shellapi.h>
#include <map>

namespace ui
{

/** 通知消息的窗口类名称
*/
#define DUILIB_SYSTEM_NOTIFICATION_WINDOW_CLASS L"SystemNotificationMessageWindow"

/** 临时托盘图标的自定义消息ID
*/
#define WM_SYSTEM_NOTIFICATION_MESSAGE (WM_USER + 1100)

namespace
{
/** 通知消息窗口句柄（首次显示通知时创建）
*/
HWND g_notificationWnd = nullptr;

/** 通知ID -> 临时托盘图标ID
*/
std::map<uint32_t, UINT> g_notificationTrayIds;

/** 下一个可用的通知ID
*/
uint32_t g_nextNotificationId = 1;

/** 清理指定通知的临时托盘图标
* @param [in] notificationId 通知ID
* @return 找到并清理返回true，否则返回false
*/
bool CleanupNotification(uint32_t notificationId)
{
    auto it = g_notificationTrayIds.find(notificationId);
    if (it == g_notificationTrayIds.end()) {
        return false;
    }
    if (g_notificationWnd != nullptr) {
        ::KillTimer(g_notificationWnd, notificationId);
    }

    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = g_notificationWnd;
    nid.uID = it->second;
    ::Shell_NotifyIconW(NIM_DELETE, &nid);
    g_notificationTrayIds.erase(it);
    return true;
}

/** 通知消息窗口的过程函数
*/
LRESULT CALLBACK NotificationWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_SYSTEM_NOTIFICATION_MESSAGE:
        {
            //通知被点击、超时或隐藏时，清理临时托盘图标
            UINT uMouseMsg = LOWORD(lParam);
            if ((uMouseMsg == NIN_BALLOONUSERCLICK) ||
                (uMouseMsg == NIN_BALLOONTIMEOUT) ||
                (uMouseMsg == NIN_BALLOONHIDE)) {
                CleanupNotification(static_cast<uint32_t>(wParam));
            }
        }
        return 0;
    case WM_TIMER:
        //达到显示时长后清理临时托盘图标
        CleanupNotification(static_cast<uint32_t>(wParam));
        return 0;
    default:
        break;
    }
    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/** 确保通知消息窗口已创建
*/
bool EnsureNotificationWindow()
{
    if (g_notificationWnd != nullptr) {
        return true;
    }
    HINSTANCE hInstance = ::GetModuleHandleW(nullptr);
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = NotificationWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = DUILIB_SYSTEM_NOTIFICATION_WINDOW_CLASS;
    ::RegisterClassExW(&wc);

    g_notificationWnd = ::CreateWindowExW(0, DUILIB_SYSTEM_NOTIFICATION_WINDOW_CLASS, L"",
                                          WS_POPUP, 0, 0, 0, 0, HWND_MESSAGE,
                                          nullptr, hInstance, nullptr);
    return g_notificationWnd != nullptr;
}

} //namespace

bool SystemNotification::IsSupported()
{
    //Windows 7及以上系统均支持托盘气泡通知
    return true;
}

bool SystemNotification::RequestPermission()
{
    //Windows原生实现无需单独申请通知权限（受系统通知设置控制）
    return true;
}

uint32_t SystemNotification::Show(const DString& title, const DString& content,
                                  SystemNotificationPriority priority, uint32_t timeoutMs)
{
    UNUSED_VARIABLE(priority);
    if (title.empty()) {
        return 0;
    }
    if (!EnsureNotificationWindow()) {
        return 0;
    }

    uint32_t notificationId = g_nextNotificationId++;
    if (notificationId == 0) {
        notificationId = g_nextNotificationId++;
    }
    const UINT trayId = static_cast<UINT>(notificationId);
    const std::wstring titleW = StringConvert::TToWString(title);
    const std::wstring contentW = StringConvert::TToWString(content);

    //1. 创建一个临时托盘图标，用于承载气泡通知
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = g_notificationWnd;
    nid.uID = trayId;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_SYSTEM_NOTIFICATION_MESSAGE;
#if defined(UNICODE) || defined(_UNICODE)
    nid.hIcon = ::LoadIconW(nullptr, IDI_APPLICATION);
#else
    //#define IDI_APPLICATION     MAKEINTRESOURCE(32512)
    nid.hIcon = ::LoadIconW(nullptr, MAKEINTRESOURCEW(32512));
#endif
    StringUtil::StringCopy(nid.szTip, _countof(nid.szTip), titleW.c_str());

    BOOL bRet = ::Shell_NotifyIconW(NIM_ADD, &nid);
    if (!bRet) {
        //资源管理器重启等情况下可能添加失败，先删除再重试
        ::Shell_NotifyIconW(NIM_DELETE, &nid);
        bRet = ::Shell_NotifyIconW(NIM_ADD, &nid);
    }
    if (!bRet) {
        return 0;
    }
    g_notificationTrayIds[notificationId] = trayId;

    //2. 通过气泡通知显示内容
    NOTIFYICONDATAW balloon = { 0 };
    balloon.cbSize = sizeof(NOTIFYICONDATAW);
    balloon.hWnd = g_notificationWnd;
    balloon.uID = trayId;
    balloon.uFlags = NIF_INFO;
    balloon.uTimeout = timeoutMs;
    balloon.dwInfoFlags = NIIF_INFO;
    StringUtil::StringCopy(balloon.szInfoTitle, _countof(balloon.szInfoTitle), titleW.c_str());
    StringUtil::StringCopy(balloon.szInfo, _countof(balloon.szInfo), contentW.c_str());

    if (::Shell_NotifyIconW(NIM_MODIFY, &balloon) != TRUE) {
        CleanupNotification(notificationId);
        return 0;
    }

    //3. 显示时长结束后自动清理临时托盘图标
    ::SetTimer(g_notificationWnd, notificationId, (timeoutMs == 0) ? 5000 : timeoutMs, nullptr);
    return notificationId;
}

bool SystemNotification::Remove(uint32_t notificationId)
{
    if (notificationId == 0) {
        return false;
    }
    return CleanupNotification(notificationId);
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN && !DUILIB_BUILD_FOR_SDL

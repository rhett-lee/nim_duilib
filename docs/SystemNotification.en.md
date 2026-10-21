English | [简体中文](SystemNotification.md)

## Native System Notification (SystemNotification)

`SystemNotification` is the framework's **system-level notification** interface. It delivers messages to the
operating system's notification center / notification area, so they remain visible even when the user switches
to another application. It is independent of [ToastWnd](Toast.en.md):

* ToastWnd is an in-app owner-drawn notification that only appears near the application window;
* SystemNotification uses the operating system notification channel, does not depend on a window, and requires no XML skin.

`SystemNotification` exposes fire-and-forget static methods. Calls return immediately and the system is responsible
for displaying and dismissing the notification. This class does not provide notification click callbacks.

* Header: [duilib/Utils/SystemNotification.h](../duilib/Utils/SystemNotification.h)
* Demo: the "System Notification / Notify Test" button on the main window of `examples/controls`

### 1. Platform Implementations

| Build / platform | Implementation | Notes |
| :--- | :--- | :--- |
| SDL builds (Windows / macOS / Linux / FreeBSD) | SDL system notification API (`SDL_ShowNotification`) | Requires SDL 3.6.0 or later; the SDL source used by this repository already contains the API |
| Windows builds without SDL | `Shell_NotifyIcon` tray balloon | Supports Windows 7 and later; a temporary tray icon is created while the notification is shown, and is cleaned up on timeout or click |

Platform limitations:

* macOS: the application must run inside a `.app` bundle, otherwise notifications cannot be shown;
* Linux / FreeBSD: an `org.freedesktop.Notifications` notification daemon (D-Bus) is required; desktop environments normally provide one;
* Windows 10 and later: notifications may be folded into the notification center; timing and appearance are controlled by system policy;
* Focus Assist / Do Not Disturb may suppress notifications; this class cannot bypass it.

### 2. Interface

| Method | Description |
| :--- | :--- |
| `static bool IsSupported()` | Whether the current platform supports system notifications |
| `static bool RequestPermission()` | Requests notification permission (needed on some platforms before the first notification; safe to call repeatedly) |
| `static uint32_t Show(title, content, priority, timeoutMs)` | Shows a notification and returns its ID (> 0); returns 0 on failure |
| `static bool Remove(notificationId)` | Removes a shown notification; returns false if it does not exist or the platform does not support removal |

`Show` parameters:

| Parameter | Description |
| :--- | :--- |
| `title` | Notification title; must not be empty |
| `content` | Notification body; may be empty |
| `priority` | Notification priority, defaults to `kNormal`; only effective in the SDL implementation |
| `timeoutMs` | Display duration in milliseconds, defaults to 5000; the Windows native implementation uses it to clean up the temporary tray icon, while actual display time is decided by the system |

### 3. Notification Priority (SystemNotificationPriority)

| Value | Description |
| :--- | :--- |
| `kLow` | Low priority |
| `kNormal` | Normal priority (default) |
| `kHigh` | High priority |
| `kCritical` | Highest priority; may override the system's Do Not Disturb mode depending on the platform |

### 4. Basic Usage

```cpp
#include "duilib/duilib.h"

void ShowNativeNotification(ui::Window* pWindow)
{
    if (!ui::SystemNotification::IsSupported()) {
        // The current platform (or SDL version) does not support system notifications.
        ui::ToastWnd::Show(pWindow, _T("System notifications are not supported on this platform"));
        return;
    }

    // Some platforms require permission before the first notification.
    ui::SystemNotification::RequestPermission();

    uint32_t notificationId = ui::SystemNotification::Show(
        _T("nim_duilib"),
        _T("This is a native system notification"),
        ui::SystemNotificationPriority::kNormal,
        5000);
    if (notificationId == 0) {
        ui::ToastWnd::Show(pWindow, _T("Failed to show the system notification"));
    }
}
```

To withdraw a notification early, keep the ID returned by `Show` and call `Remove`:

```cpp
uint32_t notificationId = ui::SystemNotification::Show(_T("Downloading"), _T("The file is downloading..."));
// ...
ui::SystemNotification::Remove(notificationId);
```

### 5. Notes

1. **The Windows native implementation must be called on the UI thread**: it relies on a hidden window's message loop, and timeout cleanup is performed in `WM_TIMER`;
2. **Notification ID lifetime**: the Windows native implementation can only remove notifications created by this process that are still shown; the behavior of `Remove` in the SDL implementation depends on the platform notification center;
3. **No click callback**: this class does not register notification click events; if interaction is needed, ask the user to return to the application in the notification text;
4. **Choosing between ToastWnd and SystemNotification**: use ToastWnd for short in-app hints; use SystemNotification for messages that must remain visible after the user switches away (download completion, background task results, new IM messages, and so on).

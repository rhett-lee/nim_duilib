简体中文 | [English](SystemNotification.en.md)

## 系统原生通知（SystemNotification）

`SystemNotification` 是框架提供的**系统级通知**接口，用于把消息送到操作系统的通知中心 / 通知区域，
即使用户切到其他应用也能看到。它与 [ToastWnd](Toast.md) 相互独立：

* ToastWnd 是应用内自绘的提示框，只在应用窗口附近显示；
* SystemNotification 走操作系统通知通道，不依赖窗口，也不需要额外的 XML 皮肤。

`SystemNotification` 是 fire-and-forget 的静态接口，调用后立即返回，由系统负责显示与销毁；
本类不提供通知点击回调。

* 关联头文件：[duilib/Utils/SystemNotification.h](../duilib/Utils/SystemNotification.h)
* 完整演示：`examples/controls` 主界面的「System Notification / Notify Test」按钮

### 1. 平台实现

| 构建方式 / 平台 | 实现 | 说明 |
| :--- | :--- | :--- |
| SDL 构建（Windows / macOS / Linux / FreeBSD） | SDL 系统通知 API（`SDL_ShowNotification`） | 依赖 SDL 3.6.0 及以上版本；当前仓库使用的 SDL 源码已包含该 API |
| 不使用 SDL 的 Windows 构建 | `Shell_NotifyIcon` 托盘气泡通知 | 兼容 Windows 7 及以上；显示期间会创建一个临时托盘图标，超时或点击后自动清理 |

平台限制：

* macOS：程序必须运行在 `.app` 包内，否则通知无法显示；
* Linux / FreeBSD：需要 `org.freedesktop.Notifications` 通知守护进程（D-Bus），桌面环境通常自带；
* Windows 10 及以上：系统可能把通知折叠到通知中心，显示时长与样式由系统策略决定；
* 系统"专注助手 / 勿扰模式"可能拦截通知，本类无法绕过。

### 2. 接口说明

| 接口 | 说明 |
| :--- | :--- |
| `static bool IsSupported()` | 当前平台是否支持系统通知 |
| `static bool RequestPermission()` | 请求通知权限（部分平台首次显示前需要，可重复调用，无副作用） |
| `static uint32_t Show(title, content, priority, timeoutMs)` | 显示通知，返回通知 ID（大于 0）；失败返回 0 |
| `static bool Remove(notificationId)` | 移除已显示的通知；通知不存在或平台不支持移除时返回 false |

`Show` 的参数：

| 参数 | 说明 |
| :--- | :--- |
| `title` | 通知标题，不能为空 |
| `content` | 通知内容，可为空 |
| `priority` | 通知优先级，默认 `kNormal`；仅 SDL 实现生效 |
| `timeoutMs` | 显示时长（毫秒），默认 5000；仅 Windows 原生实现用于清理临时托盘图标，实际显示时长由系统决定 |

### 3. 通知优先级（SystemNotificationPriority）

| 枚举值 | 说明 |
| :--- | :--- |
| `kLow` | 低优先级 |
| `kNormal` | 普通优先级，默认值 |
| `kHigh` | 高优先级 |
| `kCritical` | 最高优先级，可能覆盖系统的"勿扰模式"（取决于平台实现） |

### 4. 基本用法

```cpp
#include "duilib/duilib.h"

void ShowNativeNotification(ui::Window* pWindow)
{
    if (!ui::SystemNotification::IsSupported()) {
        //当前平台（或当前 SDL 版本）不支持系统通知
        ui::ToastWnd::Show(pWindow, _T("当前平台不支持系统通知"));
        return;
    }

    //部分平台首次显示前需要申请权限
    ui::SystemNotification::RequestPermission();

    uint32_t notificationId = ui::SystemNotification::Show(
        _T("nim_duilib"),
        _T("这是一条系统原生通知"),
        ui::SystemNotificationPriority::kNormal,
        5000);
    if (notificationId == 0) {
        ui::ToastWnd::Show(pWindow, _T("系统通知发送失败"));
    }
}
```

需要提前撤回通知时，可保存 `Show` 返回的通知 ID 并调用 `Remove`：

```cpp
uint32_t notificationId = ui::SystemNotification::Show(_T("下载中"), _T("文件正在下载..."));
//...
ui::SystemNotification::Remove(notificationId);
```

### 5. 注意事项

1. **Windows 原生实现需在 UI 线程调用**：它依赖隐藏窗口的消息循环，超时清理通过 `WM_TIMER` 完成；
2. **通知 ID 的生命周期**：Windows 原生实现只能移除本进程创建、且仍在显示的通知；
   SDL 实现的 `Remove` 行为取决于平台通知中心；
3. **不支持点击回调**：本类不注册通知点击事件，需要交互时请在通知内容中提示用户回到应用；
4. **与 ToastWnd 的选择**：应用内短提示用 ToastWnd；需要用户切出应用后仍能看到的消息（下载完成、
   后台任务结果、IM 新消息等）用 SystemNotification。

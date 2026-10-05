简体中文 | [English](Toast.en.md)

## 非模态通知框（ToastWnd）

`ToastWnd` 是框架自带的自绘皮肤非模态通知框（Toast），用于向用户展示短时提示信息
（如"保存成功"、"操作失败"等）。与模态的 [MessageBoxWnd](MessageBox.md) 不同，
Toast 弹出后**不阻塞父窗口、不抢占焦点**，停留指定时间后自动消失，并支持淡入淡出、
多条堆叠、点击关闭与鼠标悬停暂停等交互。

* 关联头文件：[duilib/Utils/ToastWnd.h](../duilib/Utils/ToastWnd.h)
* 皮肤文件：`bin/resources/themes/default/public/toast/toast.xml`（皮肤目录 `public/toast`）
* 图标资源：同目录下的 `toast_info.svg`、`toast_success.svg`、`toast_warning.svg`、`toast_error.svg`
* 完整演示：`examples/controls`（ToastForm）

### 1. 接口说明

只能通过静态接口 `Show` 使用，fire-and-forget：调用后立即返回，窗口对象在关闭后由框架
自动销毁，调用方不需要也不能自己管理生命周期：

```cpp
static void Show(ui::Window* pParentWindow,
                 const DString& text,
                 ToastType type = kTypeInfo,
                 int32_t nDurationMs = 3000,
                 ToastPosition position = kPosTop,
                 bool bTextId = false);
```

| 参数 | 说明 |
| :--- | :--- |
| pParentWindow | 父窗口，通知显示在其客户区附近；显示期间父窗口仍可正常操作。可为 nullptr，此时按显示器工作区定位 |
| text | 通知内容，支持 `\n` 显式换行，长文本自动换行，窗口高度随文本自适应（最高 200 DIP） |
| type | 通知类型（决定左侧图标），见下表 |
| nDurationMs | 自动关闭的停留时长（毫秒），默认 3000；**传 0 表示不自动关闭**，只能由用户点击关闭 |
| position | 显示位置，见下表 |
| bTextId | 为 true 时 text 按多语言 ID 解析，切换语言后实时刷新 |

### 2. 通知类型（ToastType）

| 枚举值 | 说明 |
| :--- | :--- |
| `kTypeInfo` | 信息（蓝色 i），默认类型 |
| `kTypeSuccess` | 成功（绿色对勾） |
| `kTypeWarning` | 警告（黄色 !） |
| `kTypeError` | 错误（红色 X） |

### 3. 显示位置（ToastPosition）

位置相对父窗口客户区计算；pParentWindow 为 nullptr 时相对显示器工作区。
同一位置上的多条通知沿垂直方向堆叠（见第 7 节）：

| 枚举值 | 说明 |
| :--- | :--- |
| `kPosTop` | 顶部居中，默认位置，新通知依次向下排列 |
| `kPosCenter` | 垂直水平居中 |
| `kPosBottom` | 底部居中 |
| `kPosTopRight` | 右上角，新通知依次向下排列 |
| `kPosBottomRight` | 右下角（类系统通知），新通知依次向上排列 |

通知距父窗口客户区边缘的边距为 24 DIP（`kAnchorMargin`/`kRightMargin`），
最终位置会被夹取在显示器工作区内，避免超出屏幕。

### 4. 基本用法

```cpp
#include "duilib/duilib.h"

//保存成功后给一条 3 秒的成功提示（默认顶部居中）
ui::ToastWnd::Show(this, _T("保存成功"), ui::ToastWnd::kTypeSuccess);

//网络请求失败：错误图标、停留 5 秒
ui::ToastWnd::Show(this, _T("网络连接失败，请稍后重试。"),
                   ui::ToastWnd::kTypeError, 5000);

//右下角的长时提示
ui::ToastWnd::Show(this, _T("文件已下载完成"),
                   ui::ToastWnd::kTypeInfo, 4000,
                   ui::ToastWnd::kPosBottomRight);
```

### 5. 停留时长、悬停暂停与点击关闭

* **自动关闭**：通知停留 `nDurationMs` 毫秒后自动播放淡出动画并关闭。
  计时通过 UI 线程的 `ThreadManager::PostDelayedTask` 延迟任务实现，
  不依赖窗口定时器，窗口意外销毁时任务可通过任务 ID 取消。
* **0 = 不自动关闭**：`nDurationMs` 传 0 时通知一直停留，只能由用户点击关闭，
  适合需要用户明确知晓的重要信息。
* **鼠标悬停暂停**：鼠标移入通知区域时暂停倒计时；移开后按**剩余时间**继续。
  剩余时间以 `steady_clock` 毫秒时间戳在暂停瞬间计算，多次移入移出不会重置总时长。
* **点击立即关闭**：点击通知条任意位置立即播放淡出动画关闭（无需等倒计时结束）。
  点击与悬停事件绑定在 XML 根容器上，详见第 9 节的实现说明。

### 6. 多语言支持

最后一个参数 `bTextId` 传 true 时，`text` 按多语言 ID 传入（内部调用
`Label::SetTextId`），框架负责解析为当前语言文本，并在语言切换后实时刷新：

```cpp
ui::ToastWnd::Show(this,
    _T("STRID_TOAST_DEMO_INFO"),
    ui::ToastWnd::kTypeInfo,
    3000,
    ui::ToastWnd::kPosTop,
    true);
```

### 7. 多条堆叠与数量上限

* 所有活动中的通知保存在静态列表 `s_toasts` 中（仅在 UI 线程访问）。
* 只对**父窗口相同、位置相同**的通知进行堆叠：例如顶部居中和右下角的通知各自独立排列，
  互不影响。每条新通知排在前一条之后，间距为 12 DIP（`kToastGap`）。
* 某条通知关闭（超时或点击）后，会先从列表摘除，其余通知通过位移动画**平滑上移补齐**。
* 同一时刻最多显示 5 条通知（`kMaxToastCount`）。超出上限时，最早出现的一条
  自动先走退场流程，再放入新通知。

### 8. 动画效果

动画使用框架的 `AnimationPlayer`（与 TabBox 等控件的动画机制一致），
动画类型为 `AnimationType::kAnimationNone`，由回调自行驱动窗口属性：

| 动画 | 时长 | 缓动 | 效果 |
| :--- | :--- | :--- | :--- |
| 入场 | 200ms | `EaseOutCubic` | 透明度 0→255 淡入，同时从目标位置下方 12 DIP 处向上滑入 |
| 退场 | 180ms | `EaseInCubic` | 透明度 255→0 淡出，结束后调用 `CloseWnd` 关闭窗口 |
| 补齐 | 200ms | `EaseOutCubic` | 其他通知关闭后，剩余通知沿 Y 轴平滑移动到新位置 |

窗口本身是分层窗口（`kWS_EX_LAYERED`），淡入淡出通过 `SetLayeredWindowAlpha` 实现；
动画回调统一通过 `GetWeakFlag()` 做生命周期保护，窗口已销毁时不再访问成员。
正在播入场动画的通知若遇到其他通知关闭触发重排，会直接吸附到目标位置，
避免入场动画与位移动画互相覆盖。

### 9. 实现要点与皮肤结构

窗口样式为 `kWS_POPUP | kWS_EX_TOPMOST | kWS_EX_LAYERED | kWS_EX_NOACTIVATE`：
置顶、分层、**不激活**（不抢焦点）；没有父窗口时额外加 `kWS_EX_TOOLWINDOW`，
避免出现在任务栏和 Alt+Tab 列表中。皮肤根节点开启了窗口阴影（`shadow_attached="true"`）。

皮肤 `toast.xml` 的关键结构：

* 根 HBox 为通知条本体：固定宽度 360 DIP、高度 auto（`max_height="200"`），
  圆角 8px，复用语义色 `bg_tooltip`（背景）、`text_tooltip`（文字）、
  `border_window`（描边），未引入新的颜色名。
* 内部使用 HBox 排列图标（`name="toast_icon"`）与文本
  （`name="toast_text"`，`multi_line="true"`）。
* 内部容器及图标、文本均设置 `mouse_enabled="false"`、`tab_stop="false"`，
  保证鼠标消息统一命中最外层通知条。

在该框架下实现此类交互窗口时，有三点容易踩坑，使用方自行开发类似控件时需注意：

1. **普通容器不会派发 `kEventClick`**：`kEventClick` 只有 Button、ComboButton、
   ListBoxItem 等控件会产生。让整个通知条可点击，应在容器上使用
   `AttachButtonUp`（监听 `kEventMouseButtonUp`），而不是 `AttachClick`。
2. **有阴影的窗口，事件要绑在 `GetXmlRoot()` 而不是 `GetRoot()` 上**：
   `GetRoot()` 返回的是框架注入的阴影容器 ShadowBox，它被显式设置为
   `SetMouseEnabled(false)`，收不到任何鼠标消息；XML 中实际可见的根容器要用
   `GetXmlRoot()` 获取。注意尺寸测量仍应使用 `GetRoot()`：其 `EstimateSize`
   返回值包含阴影，`GetPadding()` 即四周阴影边距。
3. **鼠标完全移出窗口时收不到控件级 `kEventMouseLeave`**：控件的 leave 事件只在
   同一窗口内从一个控件移动到另一个控件时派发；鼠标直接移出小窗口边界时，
   框架的 `Window::OnMouseLeaveMsg`（WM_MOUSELEAVE）不会向控件转发该事件。
   `ToastWnd` 重写了 `OnMouseLeaveMsg`，在其中调用 `ResumeAutoCloseTimer()`
   恢复倒计时，再调用基类实现。

此外，通知窗口作为父窗口的 owned window 创建，父窗口关闭时会随之一并销毁；
`OnFinalMessage` 中负责把自身从静态列表摘除并重排其余通知，随后非模态窗口基类
自动 `delete this`。

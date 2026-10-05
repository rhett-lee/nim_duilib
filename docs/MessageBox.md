简体中文 | [English](MessageBox.en.md)

## 模态消息框（MessageBoxWnd）

`MessageBoxWnd` 是框架自带的自绘皮肤模态消息框，使用 `DoModal` 模态显示，阻塞父窗口，
并返回用户点击的按钮。接口风格与 Win32 `MessageBox` 类似，支持常用的按钮组合、标准图标、
多语言与自定义按钮文字。

* 关联头文件：[duilib/Utils/MessageBoxWnd.h](../duilib/Utils/MessageBoxWnd.h)
* 皮肤文件：`bin/resources/themes/default/public/messagebox/messagebox.xml`（皮肤目录 `public/messagebox`）
* 图标资源：同目录下的 `msg_info.svg`、`msg_warning.svg`、`msg_error.svg`、`msg_question.svg`
* 完整演示：`examples/controls`（MessageBoxForm）

### 1. 接口说明

只能通过静态接口 `Show` 使用，不需要（也不能）直接创建对象：

```cpp
static int32_t Show(ui::Window* pParentWindow,
                    const DString& text,
                    const DString& title,
                    uint32_t buttonFlags = kButtonsOK,
                    IconType iconType = kIconNone,
                    const ButtonText* pButtonText = nullptr,
                    bool bTextId = false);
```

| 参数 | 说明 |
| :--- | :--- |
| pParentWindow | 父窗口，模态期间不可操作，可为 nullptr |
| text | 消息内容，支持 `\n` 换行，长文本自动换行，窗口高度随文本自适应 |
| title | 窗口标题 |
| buttonFlags | 按钮组合，见下表 |
| iconType | 图标类型，见下表 |
| pButtonText | 自定义按钮文字，为 nullptr 时使用框架默认文字 |
| bTextId | 为 true 时，text、title、按钮文字均按多语言 ID 解析（切换语言后实时生效） |

返回值为用户选择的按钮（见返回值表）；窗口创建失败时返回 -1。

### 2. 按钮组合（ButtonFlags）

| 枚举值 | 含有的按钮 |
| :--- | :--- |
| `kButtonsOK` | 确定 |
| `kButtonsOKCancel` | 确定、取消 |
| `kButtonsYesNo` | 是、否 |
| `kButtonsYesNoCancel` | 是、否、取消 |
| `kButtonsRetryCancel` | 重试、取消 |

按钮的显隐由上述标志位按位组合控制（`kButtonOK`/`kButtonCancel`/`kButtonYes`/`kButtonNo`/`kButtonRetry`）。

### 3. 图标类型（IconType）

| 枚举值 | 说明 |
| :--- | :--- |
| `kIconNone` | 无图标 |
| `kIconInfo` | 信息（蓝色 i） |
| `kIconWarning` | 警告（黄色 !） |
| `kIconError` | 错误（红色 X） |
| `kIconQuestion` | 疑问（蓝色 ?） |

### 4. 返回值（Result）

取值与 Win32 `MessageBox` 的 `ID*` 返回值一致：

| 枚举值 | 数值 | 触发方式 |
| :--- | :--- | :--- |
| `kResultOK` | 1 | 点击"确定"按钮 |
| `kResultCancel` | 2 | 点击"取消"按钮、按 ESC 键、点击标题栏关闭按钮 |
| `kResultRetry` | 4 | 点击"重试"按钮 |
| `kResultYes` | 6 | 点击"是"按钮 |
| `kResultNo` | 7 | 点击"否"按钮 |

说明：按 Enter 键时触发**当前焦点按钮**，默认焦点是按钮组合中的第一个肯定性按钮
（优先"确定"，其次"是"，最后"重试"），因此"是/否"组合直接按 Enter 返回 `kResultYes(6)`。
ESC 键与标题栏关闭按钮统一返回 `kResultCancel(2)`；即使当前组合没有"取消"按钮，
也不会映射为默认按钮的返回值，避免用户放弃选择时触发肯定性动作。

### 5. 基本用法

```cpp
#include "duilib/Utils/MessageBoxWnd.h"

int32_t nResult = ui::MessageBoxWnd::Show(this,
    _T("确定要删除该文件吗？删除后不可恢复。"),
    _T("删除确认"),
    ui::MessageBoxWnd::kButtonsYesNoCancel,
    ui::MessageBoxWnd::kIconQuestion);
if (nResult == ui::MessageBoxWnd::kResultYes) {
    //执行删除操作
}
```

### 6. 多语言支持

最后一个参数 `bTextId` 传 true 时，`text`、`title` 以及按钮文字都按多语言 ID 传入，
框架负责解析为当前语言的文本，并在语言切换后实时刷新；此时任务栏显示的原生窗口标题
同样是译文，而不是 ID 字符串：

```cpp
int32_t nResult = ui::MessageBoxWnd::Show(this,
    _T("STRID_DELETE_CONFIRM_TEXT"),
    _T("STRID_DELETE_CONFIRM_TITLE"),
    ui::MessageBoxWnd::kButtonsOKCancel,
    ui::MessageBoxWnd::kIconWarning,
    nullptr, true);
```

默认按钮文字对应的多语言 ID 为 `STRID_MSGBOX_OK`、`STRID_MSGBOX_CANCEL`、
`STRID_MSGBOX_YES`、`STRID_MSGBOX_NO`、`STRID_MSGBOX_RETRY`。

### 7. 自定义按钮文字

通过 `ButtonText` 结构体可以自定义每一个按钮的文字；某一项留空时，该项使用框架默认文字。
`bTextId` 为 true 时，结构体成员同样填写多语言 ID：

```cpp
ui::MessageBoxWnd::ButtonText buttonText;
buttonText.yes    = _T("STRID_CUSTOM_SAVE");     //例如"保存"
buttonText.no     = _T("STRID_CUSTOM_DISCARD");  //例如"不保存"
buttonText.cancel = _T("STRID_CUSTOM_CANCEL");   //例如"取消"
ui::MessageBoxWnd::Show(this,
    _T("STRID_SAVE_CHANGED_TEXT"), _T("STRID_SAVE_CHANGED_TITLE"),
    ui::MessageBoxWnd::kButtonsYesNoCancel,
    ui::MessageBoxWnd::kIconQuestion,
    &buttonText, true);
```

### 8. 键盘操作

* **TAB / Shift+TAB**：在功能按钮之间正向/反向循环切换焦点；标题栏关闭按钮不参与 TAB 循环。
* **Enter**：触发当前拥有焦点的按钮（按钮对 `kEventKeyDown` + `kVK_RETURN`/`kVK_SPACE` 响应 click）。
* **ESC**：取消，返回 `kResultCancel(2)`。
* **焦点矩形**：窗口初始弹出时不显示按钮的焦点虚线框；用户首次按 TAB 进行键盘导航后，
  才由 `MessageBoxWnd::OnKeyDownMsg` 调用 `SetShowFocusedRect(true)` 开启，避免界面一弹出就显示焦点环。
  该属性的 XML 名称为 `show_focused_rect`（别名 `show_focus_rect`），详见 [Control.md](Control.md)。

### 9. 模态对话框的键盘消息机制（Windows 平台）

`DoModal` 在 Windows 平台通过 `DialogBoxIndirectParam` 创建模态对话框，其消息循环在派发消息前
会先调用 `IsDialogMessage`。该 API 会吞掉两类按键：

* **TAB 键**：被转换为原生子控件之间的焦点切换。自绘窗口没有原生子控件，
  于是 TAB 键到不了窗口过程，自绘按钮的键盘导航会失效。
* **Enter 键**：被转换为 `WM_COMMAND` + `IDOK`，窗口过程收不到 `WM_KEYDOWN`，
  因此回车无法触发当前焦点按钮，只会走 IDOK 的默认关闭路径。

框架在 `duilib/Core/NativeWindow_Windows.cpp` 的 `IsDialogMessageDuiLib` 内联 hook 中处理：

* **TAB 键全局放行**（不区分窗口），放行后由 `Window::OnKeyDownMsg` 的
  `SetNextTabControl` 完成焦点切换，所有模态窗口都支持 TAB 导航。
* **Enter 键仅对显式注册的窗口放行**，按窗口句柄登记，避免改变普通模态对话框
  "按回车走默认 IDOK"的行为。注册接口为：

```cpp
//duilib/Core/NativeWindow_Windows.h（仅 Windows 平台）
static void NativeWindow_Windows::SetEnterKeyPassthrough(HWND hWnd, bool bPassthrough);
```

`MessageBoxWnd` 已在内部完成注册（`OnInitWindow` 中注册、`OnPreCloseWindow` 中注销），
使用方无需关心。自定义模态窗口如果也需要"回车触发当前焦点按钮"，必须同样成对注册/注销；
窗口关闭时若不注销，该句柄被系统复用后会影响后来的模态对话框：

```cpp
void MyModalWnd::OnInitWindow()
{
    BaseClass::OnInitWindow();
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    ui::NativeWindow_Windows::SetEnterKeyPassthrough(NativeWnd()->GetHWND(), true);
#endif
}

void MyModalWnd::OnPreCloseWindow()
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    ui::NativeWindow_Windows::SetEnterKeyPassthrough(NativeWnd()->GetHWND(), false);
#endif
    BaseClass::OnPreCloseWindow();
}
```

补充：`WindowBase::DoModal` 的后两个参数控制默认关窗行为，
签名为 `DoModal(pParentWindow, createParam, bCloseByEsc = true, bCloseByEnter = false)`，
即默认 ESC 关窗、Enter 不关窗；`MessageBoxWnd` 内部以 `(true, true)` 调用，两者都允许关窗。
模态返回值中 ESC/`IDCANCEL` 对应 `kWindowCloseCancel(2)`，Enter/`IDOK` 对应 `kWindowCloseOK(1)`。

# 已核实的陷阱与历史问题

<!-- verify:allow-missing kEventResize kEventChecked kEventTextChange kEventSelChange kEventValueChange kEventVisibleChange kEventStateChange -->
<!-- 上面这些名字是刻意举例的"错误写法"，verify_docs.py 不应把它们当成文档漂移。 -->

本文记录的都是**实际验证过**的坑，按"症状"分类。排查诡异问题时先查这里，
每条都标注了源码依据，可据此复核结论是否仍然成立。

## 一、静默失败（不报错，但界面不对）

这类问题最难排查——框架既不报错也不警告。

### 1. `class` 属性写在后面 → 整个样式被静默忽略

`class` 必须写在所有属性最前面。写在中间或末尾时，控件没有任何样式，且无任何提示。

```xml
<!-- ❌ 错：class 在后面，样式失效 -->
<Button name="ok" text="确定" class="btn_global_blue_80x30"/>
<!-- ✅ 对 -->
<Button class="btn_global_blue_80x30" name="ok" text="确定"/>
```

### 2. 引用不存在的 Class / 颜色 / 字体 → 无样式，不报错

写 XML 前先核对：

```bash
grep -o '<Class name="[^"]*"'  bin/resources/themes/default/global.xml
grep -o '<Font id="[^"]*"'     bin/resources/themes/default/global.xml
grep -o '<ThemeColor name="[^"]*"' bin/resources/themes/color_light/global.xml
```

### 3. 打包漏了 `themes/color_light/` 与 `themes/color_dark/` → 整屏没有颜色

语义色（`bg_window_main`、`text_default` …）的色值全部定义在这两个目录的 `global.xml` 里，
`themes/default/global.xml` 只有 `<Alias>` 映射。`GlobalManager::Startup()`
（`GlobalManager.cpp:518-524`）会按系统深浅色自动加载其中一个。只打包 `default/global.xml` + `public/`
发布出去的程序会**整屏没有颜色**。

### 4. 硬编码色值 → 深色主题失效

`bg_window_main` 浅色是 `#FFF4F4F4`、深色是另一个值。写死 `#FFF4F4F4` 后切到深色主题不会变。
一律写语义色名。

### 5. 用旧标题栏名 → 仍能工作，但走的是 fallback

`WindowImplBase::GetBtnWindowByName`（`WinImplBase.cpp:439-458`）**先按新名查找，找不到才用旧名**
fallback。所以旧名 `minbtn`/`closebtn` 能跑，但会多一次失败的 `FindControl`，且不符合当前约定。
新代码一律用 `btn_window_*`。

## 二、编译错误

### 6. `args.pSender` → 编译不过

`EventArgs::pSender` 是 **private** 成员。取发送者必须用 `args.GetSender()`（返回 `nullptr`
表示控件已销毁）。依据 `duilib/Core/EventArgs.h`。

### 7. `FrameworkThread::OnInit()` 写成 `void` → 编译不过

实际返回 **`bool`**（`FrameworkThread.h:122`），实现末尾要 `return true;`。
参考写法见 `examples/basic/MainThread.cpp:17`。

### 8. 事件枚举名用错 → 编译不过

勾选/文本/选择/值/可见性/状态/尺寸这几类都是**过去式**：

| 容易写错 | 正确 |
|---------|------|
| `kEventChecked` | `kEventCheck` |
| `kEventTextChange` | `kEventTextChanged` |
| `kEventSelChange` | `kEventSelChanged` |
| `kEventValueChange` | `kEventValueChanged` |
| `kEventVisibleChange` | `kEventVisibleChanged` |
| `kEventStateChange` | `kEventStateChanged` |
| `kEventResize`（**不存在**） | `kEventSizeChanged` |

权威清单在 `duilib/duilib_defs.h` 的 `enum EventType`。

### 9. `EventArgs` 成员类型记错

`wParam`/`lParam` 是 `WPARAM`/`LPARAM`（不是 `size_t`）；`vkCode` 是 `VirtualKeyCode` 枚举
（不是 `uint16_t`）；`modifierKey` 是 `uint32_t`；`eventData` 是 `int32_t`（不是 `int64_t`）。
取下标时显式转型：`static_cast<size_t>(args.wParam)`。

### 10. 改了头文件模板类只重编示例 → 运行时 0xC0000005 崩溃

`PanelTemplate`、`BoxTemplate` 等模板类的实例化发生在 `duilib/Core/WindowBuilder.cpp`
的控件工厂表里（`new PanelVBox(pWindow)` 等），编译进了 `duilib.lib`。
给这些类新增成员变量（如 `m_titleId`）会改变对象内存布局。

只重编示例、不重编 `duilib` target 时，示例用新布局读写对象，但链接的旧 `duilib.lib`
仍按旧布局 `new` 对象 → 偏移错位 → 越界访问 → 窗口创建期崩溃（`0xC0000005` / `0xC000041D`），
且编译零错误，极难定位。

**修复**：先重建库，再重建示例：

```bash
cmake --build build/build_temp/msvc/duilib --config Release --target duilib
cmake --build build/build_temp/msvc/panel   --config Release --target panel
```

判断某个头文件改动是否需要 Rebuild 库：`grep -rn "<类名>" duilib/*.cpp duilib/**/*.cpp`，
有命中即说明库内有实例化，必须 Rebuild。

## 三、已修复的历史问题（不要按旧说法处理）

### 11. `<Include>` 作为 `<Window>` 唯一子节点导致空白窗口 —— 已修复

早期版本：`<Include>` 是窗口根 `<Window>` 下第一个非样式子节点时，`ParseXmlNodeChildren` 用
`continue` 跳过了 `pReturn = pControl` 赋值，`CreateControls` 返回 `nullptr`，
窗口被静默创建成完全空白（不可见、无控件、不报错）。

**已在 2026-05-21 提交 `62c56e491` 修复**。现在 `<Include>` 分支（`WindowBuilder.cpp:1233-1240`）
会自行给 `pReturn` 赋值，下面的写法正常可用：

```xml
<Window size="900,32" layered_window="true">
    <Include src="status_bar.xml"/>
</Window>
```

**仍然推荐**用 Box 包裹一层，但理由变为"便于后续叠加控件、保持结构一致"，不再是"避免崩溃"。

判定文档是否被后续提交推翻的方法：

```bash
git log --oneline -5 -- <文档路径>
git log --oneline -5 -- <源码路径>
git merge-base --is-ancestor <代码提交> <文档提交> && echo "文档写于修复之后"
```

### 12. 示例资源里的旧标题栏名 —— 已全部迁移

`chat/login.xml`、`controls/about.xml`、`rich_edit/find.xml`、`rich_edit/replace.xml`
曾使用 `minbtn`/`closebtn`，现均已改为 `btn_window_min`/`btn_window_close`。
源码中 `WinImplBase.cpp:9-14` 的旧名宏**故意保留**，作为第三方项目的兼容 fallback，不要删除。

改名安全的前提是窗口类必须继承 `WindowImplBase`（按钮行为由基类按名字自动接管）。
本仓库四个窗口均满足：`AboutForm`、`FindForm`、`ReplaceForm`，以及 `chat/login.xml`
——它由 `ChatForm::ShowCustomWindow`（`examples/chat/ChatForm.cpp:29-35`）加载，
方法体内 `new` 的正是 `ChatForm`，而 `ChatForm : WindowImplBase`。

## 四、命名与别名

### 13. `list_item` 与 `listitem`

真名是 `list_item`，`listitem` 是通过 `<Alias>` 定义的旧名，两者等价。新代码写 `list_item`。

### 14. 字体 ID：`system_<样式>_<字号>`

样式段 regular/bold/underline/italic/strikeout/fullstyle，字号只有 12/14/16/18/20/22 六档
（下划线/斜体/删除线同样各有六档，不是只有 12）。
**不存在 `arial_*` 系列**（已从 global.xml 移除）。默认字体是 `system_regular_14`。

### 15. `WinImplBase.h` 文件名不带 Window 前缀

类名是 `ui::WindowImplBase`，但文件是 `duilib/Utils/WinImplBase.h`。按类名猜文件名会找不到。

### 16. 语言文件在 `bin/resources/lang/`，不在 `themes/default/lang/`

## 四点五、CMake 变量设置的时序陷阱

**`option()` 声明过的变量永远"已定义"**，所以 `if(NOT DEFINED X)` 在 `option(X ...)` 之后判断恒为假。
`duilib_common.cmake:29` 就是 `option(DUILIB_SKIA_LIB_SUBPATH "Skia lib sub path" OFF)`，
想给 Skia 库子目录设默认值，**必须写在 `include(duilib_common.cmake)` 之前**（即 `# 包含公共实现代码` 之前），
否则默认值永远设不进去，会退化成按 `${编译器}.${架构}.${构建类型}` 拼接的规则路径。

Windows 下默认规则与 `build/msvc_build.bat:52-56` 一致：`llvm.<CPU_ARCH>.release`
（`CPU_ARCH` 取 `cmake -A` 的目标架构，`-A Win32` 时为 `x86`，缺省 `x64`）。
`msvc_build.bat` 会用 `-DDUILIB_SKIA_LIB_SUBPATH=llvm.x64.release` 覆盖，所以工程里的默认值
只在单独 configure 时生效——这正是"仅当未定义时才定义"的意义。

## 五、易混淆的目录与大小写

- `examples/RichEdit` 是大写 R（skin 目录却是小写 `rich_edit`），按小写去 grep 源码会落空。
- WorkBuddy 工作区（`C:\Users\lee\WorkBuddy\nim_duilib`）与 nim_duilib 源码仓库
  （`C:\develop\nim_duilib`）是**两个不同目录**，改源码要去后者。

## 六、文档漂移警戒

本项目曾出现过 26 处文档与代码不一致，根源是文档分散且长期未与源码同步。
**写任何不确定的名称前先核对源码**（权威源清单见 SKILL.md 的"核对源码"一节）。
改动本 skill 的 `references/` 后，运行：

```bash
python scripts/verify_docs.py --repo <nim_duilib仓库根目录>
```

它会自动比对文档里出现的 `DUI_CTR_*`、`kEvent*`、类名、字体 ID、颜色名与源码是否一致。

## 七、模态对话框（DoModal）的键盘消息

### 17. DoModal 窗口里 TAB 切不了焦点、Enter 永远触发默认按钮

`DoModal` 在 Windows 平台用 `DialogBoxIndirectParam` 创建模态对话框，其消息循环在派发前
先调用 `IsDialogMessage`。该 API 会**吞掉两类键**：

- **`VK_TAB`**：被转换为原生子控件之间的焦点切换。自绘窗口没有原生子控件，
  于是 TAB 键根本到不了窗口过程，自绘按钮的 TAB 导航失效。
- **`VK_RETURN`**：被转换为 `WM_COMMAND` + `IDOK`，进入 `__DialogProc` 的 WM_COMMAND 分支
  （`duilib/Core/NativeWindow_Windows.cpp`），窗口过程收不到 `WM_KEYDOWN`，
  因此无论焦点在哪个按钮上，回车都只走 `m_bCloseByEnter`/IDOK 的默认关闭路径，
  无法触发当前焦点按钮的 click（`Button::HandleEvent` 对 `kEventKeyDown` +
  `kVK_RETURN`/`kVK_SPACE` 才会 `Activate`）。

框架在 `IsDialogMessageDuiLib`（同名文件，内联 hook）中的处理：

- **TAB：全局放行**（不区分窗口）。TAB 对自绘窗口只有副作用，放行后由
  `Window::OnKeyDownMsg → SetNextTabControl` 完成焦点切换，任何 DoModal 窗口都受益。
- **Enter：仅对显式注册的窗口放行**。放行是按窗口句柄登记的：

```cpp
// duilib/Core/NativeWindow_Windows.h（仅 Windows 平台）
static void NativeWindow_Windows::SetEnterKeyPassthrough(HWND hWnd, bool bPassthrough);
```

需要"回车触发当前焦点按钮"的模态窗口（框架自带的 `MessageBoxWnd` 已这样做）：
`OnInitWindow` 中注册、`OnPreCloseWindow` 中注销——必须注销，否则窗口句柄被系统复用后
会影响后来的普通模态对话框：

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

不要把 Enter 改成全局放行：未注册的普通模态对话框依赖默认的
`Enter → WM_COMMAND IDOK` 行为（例如"回车直接关闭对话框"的交互），全局放行会改变它们的行为。

### 18. `DoModal` 默认不响应回车关闭，ESC 默认关闭

`WindowBase::DoModal(pParentWindow, createParam, bCloseByEsc=true, bCloseByEnter=false)`
后两个参数默认 **ESC 关窗、Enter 不关窗**。只有显式传 `bCloseByEnter=true` 时，
`__DialogProc` 收到 IDOK 才会 `CloseWnd(kWindowCloseOK)`。
模态返回值经 `m_closeParam` 传递，ESC/`IDCANCEL` 对应 `kWindowCloseCancel(2)`，
Enter/`IDOK` 对应 `kWindowCloseOK(1)`。

## 八、非模态通知窗口（ToastWnd）开发要点

### 19. 做"整条可点击的小窗口"时三个必踩坑（Toast 实战总结）

1. **普通容器不派发 `kEventClick`**：`kEventClick` 只有 Button/ComboButton/
   ListBoxItem 等控件会 SendEvent，Box/HBox/VBox 上 `AttachClick` 永远不触发。
   需要"点击容器任意位置"时用 `AttachButtonUp`（监听 `kEventMouseButtonUp`）。
2. **有阴影的窗口，鼠标事件绑 `GetXmlRoot()` 而不是 `GetRoot()`**：
   `Shadow::DoAttachShadow` 注入的 ShadowBox 被显式 `SetMouseEnabled(false)`，
   绑在 `GetRoot()` 上收不到任何鼠标消息。事件绑 XML 实际根容器 `GetXmlRoot()`；
   但尺寸测量仍用 `GetRoot()`（`EstimateSize` 返回值含阴影，
   `GetPadding()` 即四周阴影边距，窗口定位时要减去 left/top）。
   内层容器及非交互子控件也要 `mouse_enabled="false"`，避免命中内层。
3. **鼠标完全移出窗口时收不到控件级 `kEventMouseLeave`**：
   `Window::OnMouseLeaveMsg`（WM_MOUSELEAVE）只处理 tooltip，不向
   `m_pEventHover` 派发 leave；控件的 leave 只在同窗口内控件切换时派发。
   小窗口需要"移出即恢复"的逻辑时，重写窗口级
   `virtual LRESULT OnMouseLeaveMsg(const NativeMsg& nativeMsg, bool& bHandled)`，
   在其中处理后再调基类（控件级 AttachMouseLeave 可同时保留，Resume 要幂等）。

配套要点：非模态小窗口样式用
`kWS_POPUP|kWS_EX_TOPMOST|kWS_EX_LAYERED|kWS_EX_NOACTIVATE`（不抢焦点），
无父窗口时加 `kWS_EX_TOOLWINDOW`；倒计时不要用窗口定时器，用
`GlobalManager::Instance().Thread().PostDelayedTask(kThreadUI, ...)`
+ CancelTask，剩余时间用 steady_clock 时间戳计算；事件回调直接传裸 lambda，
不要用 UiBind 包装（UiBind 只用于 Thread().PostXxxTask）；动画枚举写
`AnimationType::kAnimationNone`，缓动名是 `EaseOutCubic`/`EaseInCubic`。


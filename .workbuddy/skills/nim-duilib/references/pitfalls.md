# 已核实的陷阱与历史问题

<!-- verify:allow-missing kEventResize kEventChecked kEventTextChange kEventSelChange kEventValueChange kEventVisibleChange kEventStateChange bk_main_wnd_title -->
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

## 三、已修复的历史问题（不要按旧说法处理）

### 10. `<Include>` 作为 `<Window>` 唯一子节点导致空白窗口 —— 已修复

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

### 11. 示例资源里的旧标题栏名 —— 已全部迁移

`chat/login.xml`、`controls/about.xml`、`rich_edit/find.xml`、`rich_edit/replace.xml`
曾使用 `minbtn`/`closebtn`，现均已改为 `btn_window_min`/`btn_window_close`。
源码中 `WinImplBase.cpp:9-14` 的旧名宏**故意保留**，作为第三方项目的兼容 fallback，不要删除。

改名安全的前提是窗口类必须继承 `WindowImplBase`（按钮行为由基类按名字自动接管）。
本仓库四个窗口均满足：`AboutForm`、`FindForm`、`ReplaceForm`，以及 `chat/login.xml`
——它由 `ChatForm::ShowCustomWindow`（`examples/chat/ChatForm.cpp:29-35`）加载，
方法体内 `new` 的正是 `ChatForm`，而 `ChatForm : WindowImplBase`。

## 四、命名与别名

### 12. `list_item` 与 `listitem`

真名是 `list_item`，`listitem` 是通过 `<Alias>` 定义的旧名，两者等价。新代码写 `list_item`。

### 13. 字体 ID：`system_<样式>_<字号>`

样式段 regular/bold/underline/italic/strikeout/fullstyle，字号只有 12/14/16/18/20/22 六档
（下划线/斜体/删除线同样各有六档，不是只有 12）。
**不存在 `arial_*` 系列**（已从 global.xml 移除）。默认字体是 `system_regular_14`。

### 14. `WinImplBase.h` 文件名不带 Window 前缀

类名是 `ui::WindowImplBase`，但文件是 `duilib/Utils/WinImplBase.h`。按类名猜文件名会找不到。

### 15. 语言文件在 `bin/resources/lang/`，不在 `themes/default/lang/`

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

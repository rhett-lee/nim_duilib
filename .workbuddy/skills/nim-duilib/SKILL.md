---
name: nim-duilib
description: nim_duilib C++ 桌面 UI 框架开发助手。This skill should be used when working with the nim_duilib / duilib C++ framework — creating a window or form class, writing or modifying XML skin layouts, adding or configuring controls (Button / RichEdit / ListBox / TreeView / Combo / Progress ...), binding events (XML inline `<Event>` or C++ Attach* callbacks), customizing themes, colors, fonts or global.xml Classes, or packaging and deploying resources (local folder / ZIP / embedded EXE). It also applies when diagnosing why a window renders blank, why a control silently has no style, or why a color or font reference fails.
agent_created: true
---

# nim_duilib 开发助手

nim_duilib 是一个 XML 驱动皮肤 + C++ 逻辑的跨平台桌面 UI 框架（C++20）。窗口外观由 XML 布局描述，
行为由继承 `ui::WindowImplBase` 的 C++ 类实现，二者通过控件 `name` 关联。

## 定位源码根目录

动手前先确认 nim_duilib 仓库根目录（含 `duilib/`、`examples/`、`bin/resources/` 的那个目录）。
下文所有相对路径均相对于它。若当前工作目录就在其中，直接使用相对路径即可。

## 按任务载入参考文档

按需读取，不要一次全读——只有当前任务相关的才载入上下文。

| 任务 | 载入 |
|------|------|
| 新建窗口 / 窗体 / 对话框 | `references/create-window.md` |
| 设计或修改 XML 布局、选容器、写尺寸 | `references/xml-layout.md` |
| 添加控件、写控件属性、动态加数据 | `references/add-control.md` |
| 事件绑定（XML 内联或 C++ Attach）、查事件名 | `references/event-handler.md` |
| 主题、颜色、字体、global.xml 的 Class | `references/theme.md` |
| 资源打包、发布、ZIP、单文件 EXE | `references/resource-pack.md` |
| 查某个 API / 属性 / 容器的完整定义 | `references/api-reference.md` |
| 排查诡异问题、确认某个坑是否还存在 | `references/pitfalls.md` |

`references/api-reference.md` 是最大的一份（约 600 行完整速查表），**只在确实需要查表时才读**，
其余场景优先读对应的专题文档。

## 六条必守约束

违反这些不会报错，只会静默出问题，排查成本极高。

1. **`class` 属性必须写在所有属性最前面。** 写在后面则整个 class 的样式被忽略。
   `<Button class="btn_global_blue_80x30" name="ok" text="确定"/>` 正确；反过来则控件无任何样式。

2. **标题栏与按钮必须用新名。** 容器 `window_title_bar`，按钮 `btn_window_min` / `btn_window_max` /
   `btn_window_restore` / `btn_window_close` / `btn_window_fullscreen`。
   旧名（`window_caption_bar`、`minbtn`、`closebtn` …）只是兼容 fallback，新代码不要用。
   依据 `duilib/duilib_defs.h:145-152` 与 `duilib/Utils/WinImplBase.cpp:9-14`。

3. **颜色一律写语义色名，不要写死色值。** 语义色的真实色值定义在
   `bin/resources/themes/color_light/global.xml` 与 `color_dark/global.xml`，
   `themes/default/global.xml` 里只剩旧名→新名的 `<Alias>` 映射。写死色值会导致深色主题失效。

4. **取事件发送者用 `args.GetSender()`，不是 `args.pSender`。** `pSender` 是 private 成员，直接访问编译不过。
   `GetSender()` 返回 `nullptr` 表示控件已销毁，应提前返回。

5. **`FrameworkThread::OnInit()` 返回 `bool`，不是 `void`。** 末尾要 `return true;`。

6. **资源初始化用 `ui::GlobalManager::GetResourceRootPath(false)`**，不要手工拼
   `GetCurrentModuleDirectory() + "resources\\"`——后者硬编码反斜杠，在 Linux/macOS 上路径分隔符错误。

## 核对源码，别信记忆

框架的 API 名、属性名、色值、字体 ID 会随版本变动，凭记忆写极易出错（本项目就曾出现 26 处文档与代码
不一致）。写任何不确定的名称前，先按下面的权威源核对：

| 要确认的东西 | 权威源 |
|------|------|
| XML 节点名（容器/控件） | `duilib/duilib_defs.h` 的 `DUI_CTR_*` 宏 |
| 控件支持的属性名 | 该控件 `.h/.cpp` 里 `strName == _T("...")` 分支 |
| 事件枚举名 | `duilib/duilib_defs.h` 的 `enum EventType` |
| XML 内联事件的 type 字符串 | `duilib/Core/EventArgs.cpp` 的 `InitEventStringMap` |
| 主题色真实取值 | `bin/resources/themes/color_light/global.xml` 的 `<ThemeColor>` |
| 字体 ID | `bin/resources/themes/default/global.xml` 的 `<Font id=...>` |
| Class 样式名 | 同上文件的 `<Class name=...>` |
| C++ 类声明 | `grep -rn "class DUILIB_API <Name>" duilib/` |

**自动核对**：改动或生成文档后，运行 `scripts/verify_docs.py`，它会把 `references/` 里出现的所有
`DUI_CTR_*`、`kEvent*`、类名、字体 ID、颜色名对照源码逐一验证，报告不一致项：

```bash
python scripts/verify_docs.py --repo <nim_duilib仓库根目录>
```

## 编译前自检：只编译不链接

完整构建需要 duilib 库与 Skia（Skia 源码通常在仓库外，全量构建耗时数小时）。

> **首次拉取源码后必须先做一次依赖库预编译**，否则找不到库：
> Windows 依次运行 `build\build_duilib_all_in_one.bat`（下载并编译 Skia，耗时数小时）
> 与 `build\msvc_build.bat`（cmake/MSVC 编译第三方库 + duilib + 示例）；
> 非 Windows 运行 `build/build_duilib_all_in_one.sh`（自动分流到 linux/macos/freebsd/msys2 脚本）。
> 详见仓库根目录的 `WORKBUDDY.md`。

只想验证自己写的 C++ 是否用对了 API 时，用 MSVC 的**语法检查模式** `/Zs`（不生成 obj、不链接）：

```bash
# Git Bash 下必须关掉路径转换，否则 /I 参数会被当成源文件
export MSYS_NO_PATHCONV=1
VC="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/<版本>"
SDK="C:/Program Files (x86)/Windows Kits/10/Include/<SDK版本>"
"$VC/bin/Hostx64/x64/cl.exe" -nologo -std:c++20 -utf-8 -EHsc -Zs -W3 \
    -DWIN32 -D_WINDOWS -DUNICODE -D_UNICODE \
    -I<仓库根> -I. -I"$VC/include" \
    -I"$SDK/um" -I"$SDK/shared" -I"$SDK/ucrt" -I"$SDK/winrt" \
    YourForm.cpp
```

`-utf-8` 不能省：源含中文注释时否则报 C4819。退出码 0 即语法与模板实例化均通过
（`UiBind` 这类模板错误在这一步就能暴露）。

## 常用路径约定

- 主题资源根：`bin/resources/themes/default/`
- 全局样式：`bin/resources/themes/default/global.xml`
- 窗口 XML：`bin/resources/themes/default/<skin_folder>/<skin_file>.xml`
- 公共图片：`bin/resources/themes/default/public/`
- 字体文件：`bin/resources/fonts/`
- 语言文件：`bin/resources/lang/`（**不在** `themes/default/lang/`）
- 主题色：`bin/resources/themes/color_light|color_dark/global.xml`
- 示例源码：`examples/<example_name>/`（注意目录名大小写，如 `examples/RichEdit`）

## 新建窗口的推荐流程

1. 复制 `assets/templates/` 下的四件套（`CMakeLists.txt`、`MyForm.h`、`MyForm.cpp`、`my_form.xml`）到目标位置。
   `CMakeLists.txt` 里的 `<project_name>` 要换成工程目录名。
2. 按窗口名批量替换 `MyForm` → `<FormName>`、`my_skin` → 皮肤目录、`my_form.xml` → 布局文件名。
3. 按 `references/create-window.md` 补充主线程创建代码、把源文件加入构建。
4. 需要具体控件或事件时，再载入对应专题文档。

直接复制模板比照着文档重写可靠得多——模板里的骨架是已核对过的可用状态。

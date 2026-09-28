# 用 AI 写 C++ 桌面 UI：TRAE + nim_duilib 实战指南

> 还在为记不住 UI 框架的 API 名而反复翻文档发愁？还在因为文档写错了一个函数名导致编译失败、排查半天？这篇文章教你用 TRAE 的 AI 辅助能力，在 nim_duilib 框架下快速开发桌面应用——从建窗口到绑事件，全程 AI 生成、自动校验、语法预检，把重复劳动交给工具，把精力留给业务。

---

## 为什么是 nim_duilib + TRAE？

**nim_duilib** 是一个 XML 驱动皮肤 + C++ 逻辑的跨平台桌面 UI 框架（C++20），基于 Skia 渲染引擎，支持 Windows / Linux / macOS / FreeBSD。窗口外观用 XML 描述，行为用继承 `ui::WindowImplBase` 的 C++ 类实现，二者通过控件 `name` 关联——写起来像 Web 前端，跑起来是原生性能。

**TRAE** 是一款 AI 驱动的 IDE，支持项目级 Skill（技能包）。你可以给项目配一个 Skill，让 AI 自动理解这个框架的 API 约定、命名规则、常见坑，在对话中直接生成正确的代码。

两者结合的效果是：**你说"帮我建一个带按钮的窗口"，AI 就能生成全部文件——C++ 头文件、实现文件、XML 布局、CMakeLists.txt，甚至帮你跑语法检查和文档校验。**

---

## 一、环境准备：第一次拉源码后必须做的事

nim_duilib 依赖 Skia 渲染引擎和若干第三方库（zlib、libpng、cximage、libwebp 等），新克隆的仓库里没有编译好的库，直接开工程会报错。**只需要做一次。**

### Windows（两步）

```bat
cd /d <仓库根>\build

REM 第 1 步：拉取 Skia 源码并编译（需要联网，耗时较长）
build_duilib_all_in_one.bat

REM 第 2 步：用 cmake/MSVC 编译依赖库 + duilib + 示例
msvc_build.bat
```

### Linux / macOS / FreeBSD（一步）

```bash
cd <仓库根>/build
chmod +x build_duilib_all_in_one.sh
./build_duilib_all_in_one.sh
```

脚本会按平台自动分流。完成后，`lib/` 目录下就有编译好的库了。

> **小贴士**：Skia 源码在仓库外且体积巨大，日常开发不需要全量链接构建。后面会讲怎么用 `/Zs` 语法检查模式快速验证代码，不生成 obj、不链接，几秒就能确认 API 用法对不对。

---

## 二、给项目装上 nim_duilib 专用 Skill

TRAE 的 Skill 是一组指导 AI 行为的指令文件，放在项目的 `.trae/skills/` 目录下。装好之后，你在对话中提到建窗口、改 XML、加控件、绑事件等关键词，AI 就会自动加载对应的参考文档来辅助生成代码。

### 目录结构

```
.trae/skills/nim-duilib/
├── SKILL.md                  # 入口：路由表 + 六条必守约束 + 权威源对照表
├── references/               # 按需加载的专题文档（不要一次全读）
│   ├── create-window.md      #   新建窗口（四件套 + 主线程）
│   ├── xml-layout.md         #   XML 布局、容器选择、尺寸写法
│   ├── add-control.md        #   控件属性、动态增删数据
│   ├── event-handler.md      #   事件绑定（XML 内联 / C++ Attach）
│   ├── theme.md              #   主题、语义色、字体、global.xml Class
│   ├── resource-pack.md      #   资源打包：目录 / ZIP / 嵌入 EXE
│   ├── api-reference.md      #   约 600 行完整速查表（查表时才读）
│   └── pitfalls.md           #   15 个已知坑及是否仍存在
├── assets/templates/         # 已核对可用的骨架文件
│   ├── CMakeLists.txt
│   ├── MyForm.h
│   ├── MyForm.cpp
│   └── my_form.xml
└── scripts/verify_docs.py    # 文档与源码漂移校验
```

### SKILL.md 的 frontmatter

TRAE 的 Skill 入口文件头部用 YAML frontmatter 描述触发条件：

```yaml
---
name: nim-duilib
description: nim_duilib C++ desktop UI framework guide. Use when creating windows,
  XML layouts, adding controls, binding events, theming, or packaging resources.
  Also for diagnosing render or style issues.
---
```

`description` 是 AI 路由器唯一看到的触发文本，所以要把关键动词放进去。注意几个 TRAE 的硬规则：

- `name` 必须与目录名一致，kebab-case
- `description` 不能出现 `<` 或 `>`（会和 YAML 冲突）
- 不能出现 `: `（冒号+空格，会强制加引号）
- 尽量控制在 200 字符以内

装好 Skill 后，在 TRAE 里打开这个项目，直接在对话里说"帮我建一个窗口"，AI 就会自动触发。

---

## 三、实战：5 分钟做一个带按钮的窗口

下面用真实对话场景演示。假设我们要做一个简单窗口：标题栏 + 一个文字标签 + 一个按钮，点击按钮后标签文字改变。

### 第 1 步：告诉 AI 你要什么

在 TRAE 对话中输入：

> 帮我开发一个简单的应用程序，界面中包含一个按钮。

AI 识别到"开发应用程序"和"按钮"关键词，自动触发 `nim-duilib` Skill，加载 `references/create-window.md` 和模板文件。

### 第 2 步：AI 生成的文件

AI 会从 `assets/templates/` 复制四件套骨架，然后批量替换标识符。生成的项目结构：

```
examples/simple_button/
├── CMakeLists.txt           # 构建配置（含 Skia 库路径自动检测）
├── MainForm.h               # 窗口类声明
├── MainForm.cpp             # 窗口类实现（按钮点击事件）
├── MainThread.h / .cpp      # 主线程（资源初始化 + 窗口创建）
├── TestApplication.h / .cpp  # 应用入口
├── main_windows.cpp         # Windows 入口（wWinMain）
├── resource.h / targetver.h / .rc / .ico  # Windows 资源
└── (对应 XML 布局在 bin/resources/themes/default/simple_button/ 下)
```

**XML 布局**（`simple_button.xml`）：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Window size="400,200" min_size="240,100"
        caption="0,0,0,36" use_system_caption="false"
        snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" shadow_attached="true"
        layered_window="true" alpha="255" size_box="4,4,4,4"
        icon="public/caption/logo.ico">
    <VBox bkcolor="bg_window_main">
        <!-- 标题栏 -->
        <HBox name="window_title_bar" width="stretch" height="36" bkcolor="bg_titlebar">
            <Control width="18" height="18" bkimage="public/caption/logo.svg" valign="center" margin="8"/>
            <Label text="Simple Button Demo" valign="center" margin="8" mouse_enabled="false"/>
            <Control />
            <Button class="btn_wnd_min_11" height="32" width="40" name="btn_window_min" margin="0,2,0,2"/>
            <Box height="stretch" width="40" margin="0,2,0,2">
                <Button class="btn_wnd_max_11" height="32" width="stretch" name="btn_window_max"/>
                <Button class="btn_wnd_restore_11" height="32" width="stretch" name="btn_window_restore" visible="false"/>
            </Box>
            <Button class="btn_wnd_close_11" height="stretch" width="40" name="btn_window_close" margin="0,0,0,2"/>
        </HBox>
        <!-- 内容区域 -->
        <Box padding="20,20,20,20" bkcolor="bg_container">
            <VBox valign="center" halign="center" child_margin_y="20">
                <Label name="lbl_msg" text="Click the button!"
                       font="system_regular_14"
                       normal_text_color="text_default"
                       text_align="hcenter,vcenter"/>
                <Button class="btn_global_blue_80x30" name="btn_click" text="Click Me" width="120" height="36"/>
            </VBox>
        </Box>
    </VBox>
</Window>
```

**C++ 实现**（`MainForm.cpp`）：

```cpp
#include "MainForm.h"

void MainForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    // 查找按钮并绑定点击事件
    if (auto* btn = dynamic_cast<ui::Button*>(FindControl(_T("btn_click")))) {
        btn->AttachClick([this](const ui::EventArgs& args) {
            ui::Control* pSender = args.GetSender();
            if (pSender == nullptr) {
                return true;
            }
            // 点击按钮后改变标签文字
            if (auto* label = dynamic_cast<ui::Label*>(FindControl(_T("lbl_msg")))) {
                label->SetText(_T("Button clicked!"));
            }
            return true;
        });
    }
}
```

**主线程**（`MainThread.cpp`）：

```cpp
bool MainThread::OnInit()
{
    // 初始化全局资源（使用本地文件夹，不要手工拼路径）
    ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath(false);
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    // 创建并显示主窗口
    MainForm* window = new MainForm();
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("Simple Button Demo"), true));
    window->PostQuitMsgWhenClosed(true);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);

    return true;  // 返回 bool，不是 void！
}
```

### 第 3 步：自动校验

AI 生成完代码后，可以跑两个自动检查：

**文档漂移校验**——确认代码里出现的所有 API 名、控件名、颜色名、字体 ID 都真实存在于源码：

```bash
python .trae/skills/nim-duilib/scripts/verify_docs.py --repo . \
    --extra examples/simple_button bin/resources/themes/default/simple_button
```

输出示例：

```
扫描文档 22 个，源码索引：121 个宏 / 96 个事件 / 2408 个类名 / 42 个字体 / 350 个色名
结果：全部命中，未发现文档漂移。
```

**语法检查**（MSVC `/Zs` 模式，只编译不链接，几秒出结果）：

```bash
cl.exe -nologo -std:c++20 -utf-8 -EHsc -Zs -W3 \
    -DWIN32 -D_WINDOWS -DUNICODE -D_UNICODE \
    -I<仓库根> -I<源码目录> \
    -I<MSVC include> -I<SDK include> \
    MainForm.cpp
```

退出码 0 就是通过。`UiBind` 这类模板错误在这一步就能暴露，不需要等全量构建。

---

## 四、六条必守约束——违反了不会报错，只会静默出问题

这是 nim_duilib 开发中代价最高的六类错误。AI Skill 里内置了这六条约束，生成代码时会自动遵守，但理解原理更重要。

### 1. `class` 属性必须写在所有属性最前面

```xml
<!-- 正确：class 在最前 -->
<Button class="btn_global_blue_80x30" name="ok" text="确定"/>

<!-- 错误：class 在后面，整个 class 的样式被忽略 -->
<Button name="ok" text="确定" class="btn_global_blue_80x30"/>
```

原因：框架的 XML 解析器在遇到 `class` 属性时立刻应用样式，后面的属性会覆盖 class 里的同名项。如果 `class` 写在后面，先设的属性又被 class 覆盖回去，等于白写。

### 2. 标题栏与按钮必须用新名

```xml
<!-- 正确 -->
<HBox name="window_title_bar">...</HBox>
<Button name="btn_window_min"/>
<Button name="btn_window_max"/>
<Button name="btn_window_restore"/>
<Button name="btn_window_close"/>

<!-- 错误：旧名只是兼容 fallback，新代码不要用 -->
<HBox name="window_caption_bar">...</HBox>
<Button name="minbtn"/>
<Button name="closebtn"/>
```

依据：`duilib/duilib_defs.h` 第 145–152 行。

### 3. 颜色一律写语义色名，不要写死色值

```xml
<!-- 正确：语义色名，深色主题自动切换 -->
<VBox bkcolor="bg_window_main">
<Label normal_text_color="text_default"/>

<!-- 错误：写死色值，深色主题失效 -->
<VBox bkcolor="#FFFFFFFF">
```

语义色的真实色值定义在 `bin/resources/themes/color_light/global.xml` 和 `color_dark/global.xml`。`themes/default/global.xml` 里只剩旧名→新名的 `<Alias>` 映射。打包时必须带上 `color_light/` 和 `color_dark/`，否则整屏无色。

### 4. 取事件发送者用 `args.GetSender()`

```cpp
// 正确
ui::Control* pSender = args.GetSender();
if (pSender == nullptr) return true;  // 控件已销毁

// 错误：pSender 是 private，编译不过
ui::Control* pSender = args.pSender;
```

`GetSender()` 返回 `nullptr` 表示控件已销毁（比如在异步回调中），务必判空提前返回。

### 5. `FrameworkThread::OnInit()` 返回 `bool`

```cpp
// 正确
bool MainThread::OnInit()
{
    // ... 初始化代码 ...
    return true;
}

// 错误：返回 void，框架不知道初始化是否成功
void MainThread::OnInit()
{
    // ...
}
```

### 6. 资源初始化用 `GetResourceRootPath(false)`

```cpp
// 正确：自动处理平台差异
ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath(false);
ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

// 错误：硬编码反斜杠，Linux/macOS 上路径分隔符错误
std::wstring path = GetCurrentModuleDirectory() + L"\\resources\\";
```

---

## 五、CMakeLists.txt 里的一个隐藏陷阱

如果你用 CMake 构建，`DUILIB_SKIA_LIB_SUBPATH` 变量的设置位置至关重要。它必须写在 `include(duilib_common.cmake)` **之前**：

```cmake
# Skia 库子目录名（仅 Windows 平台）
# 规则与 build/msvc_build.bat 保持一致：llvm.<CPU_ARCH>.release
# 必须放在 include(duilib_common.cmake) 之前！
if(WIN32 AND NOT DEFINED DUILIB_SKIA_LIB_SUBPATH)
    set(CPU_ARCH "x64")
    if(CMAKE_GENERATOR_PLATFORM)
        if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
            set(CPU_ARCH "x86")
        else()
            set(CPU_ARCH "${CMAKE_GENERATOR_PLATFORM}")
        endif()
    endif()
    set(DUILIB_SKIA_LIB_SUBPATH "llvm.${CPU_ARCH}.release")
endif()

# 包含公共实现代码（在这之后）
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_common.cmake")
```

原因：`duilib_common.cmake` 内部用 `option()` 声明这个变量。`option()` 的语义是"如果变量未定义才设置默认值"——但 `option()` 执行后变量就必然已定义了（值为 `OFF`）。所以如果你把 `set()` 写在 `include()` 后面，`NOT DEFINED` 判断永远为假，你的设置永远不会生效。

Skill 的模板已经按正确顺序写好了，照抄即可。

---

## 六、日常开发流程总结

| 你想做的事 | 操作 |
|------------|------|
| 新建窗口 | 复制模板四件套 → 批量替换标识符 → 补主线程代码 |
| 加控件 | XML 里写 `<Button>`/`<Label>` 等，`class` 属性放最前 |
| 绑事件 | C++ 里 `FindControl` + `AttachClick` 等，用 `GetSender()` |
| 改主题 | 编辑 `global.xml` 的 `<ThemeColor>`/`<Font>`/`<Class>` |
| 打包发布 | 把 `bin/resources/` 打成 ZIP 或嵌入 EXE |
| 验证语法 | `cl.exe /Zs` 只编译不链接，几秒出结果 |
| 校验文档 | `python verify_docs.py --repo . --extra <你的代码目录>` |

### 权威源对照表——写不确定的名字前先查

| 要确认 | 权威源 |
|--------|--------|
| XML 节点名 | `duilib/duilib_defs.h` 的 `DUI_CTR_*` 宏 |
| 控件属性名 | 该控件 `.h/.cpp` 里 `strName == _T("...")` 分支 |
| 事件枚举名 | `duilib/duilib_defs.h` 的 `enum EventType` |
| XML 内联事件 type 字符串 | `duilib/Core/EventArgs.cpp` 的 `InitEventStringMap` |
| 语义色真实取值 | `bin/resources/themes/color_light/global.xml` 的 `<ThemeColor>` |
| 字体 ID | `bin/resources/themes/default/global.xml` 的 `<Font id=...>` |
| Class 样式名 | 同上文件的 `<Class name=...>` |
| C++ 类声明 | `grep -rn "class DUILIB_API <Name>" duilib/` |

---

## 七、为什么这套方案有效？

本项目历史上出现过 26 处文档与源码不一致——错误的事件枚举名、不存在的字体 ID、写死的错误色值……靠人工审计成本很高。TRAE Skill 方案从三个层面解决这个问题：

1. **模板优于记忆**：AI 不凭记忆写代码，而是从已核对的模板复制再改，降低出错概率。

2. **自动校验**：`verify_docs.py` 把文档中出现的所有 `DUI_CTR_*`、`kEvent*`、类名、字体 ID、颜色名对照源码逐一验证。生成完代码跑一次就知道有没有漂移。

3. **语法预检**：`/Zs` 模式做完整语义分析和模板实例化，不需要链接 Skia 就能确认 API 用法对不对。`UiBind` 这类模板错误在这一步就能暴露。

三者形成闭环：AI 生成 → 文档校验 → 语法检查，在写代码的当下就拦截错误，而不是等到全量构建时才发现。

---

## 小结

- **nim_duilib** 是 XML 驱动皮肤 + C++ 逻辑的跨平台桌面 UI 框架，写起来像 Web 前端，跑起来是原生性能。
- **TRAE Skill** 让 AI 理解框架的 API 约定和命名规则，对话即生成代码。
- **六条必守约束**是框架的暗坑，违反不会报错只会静默出问题，Skill 内置了这些约束。
- **模板 + 校验 + 语法预检**三重保障，把文档与代码不一致的问题消灭在生成阶段。

如果你也在做 C++ 桌面 UI 开发，不妨试试这套方案。Skill 文件随仓库走，团队成员拉取即生效，AI 辅助开发的门槛很低。

---

*本文涉及的全部代码和 Skill 文件可在 nim_duilib 仓库的 `.trae/skills/nim-duilib/` 和 `examples/simple_button/` 下找到。*

## 资源链接
nim_duilib界面库的代码库，请点击访问：[nim_duilib](https://github.com/rhett-lee/nim_duilib)     
nim_duilib 是一款基于C++开发的跨平台界面库，源于经典的 duilib 界面库并进行了深度优化与功能扩展，支持Windows/Linux/macOS/FreeBSD平台，支持的Linux系统包括OpenEuler、OpenKylin、UbuntuKylin、统信UOS、中科方德、Ubuntu、Fedora、Debian等，专注于简化桌面应用的高效开发。其设计融合了DirectUI理念，通过XML描述界面布局，实现视觉与逻辑的分离，显著提升开发灵活性与维护性。

# 用 WorkBuddy 开发 nim_duilib 应用：把踩过的坑固化成 AI 的肌肉记忆

> 一个 C++ 桌面 UI 框架，121 个控件节点、96 种事件、42 个字体 ID、350 个语义色名。  
> 靠人脑记住它们不现实，靠通用大模型现编则会写出能编译、但界面全废的代码。  
> 这篇文章记录我们如何把 nim_duilib 的项目知识做成 WorkBuddy 的 Skill，  
> 让 AI 第一次接手就能产出可直接编译的窗口代码。

---

## 一、先说结论

我们在 nim_duilib 上做的不是「用 AI 写代码」，而是**用 AI 写代码之前，先把项目的领域知识喂给它**：

1. 写一份项目入口文档（本文对应 `WORKBUDDY.md`），说清环境、目录、构建方式、硬约束；
2. 把框架知识拆成一个 Skill（入口 + 8 份按需加载的专题参考 + 一套可复制的模板四件套）；
3. 配一个**文档漂移自检脚本**，让 AI 每次改完文档都能自己验证有没有写错名字。

结果：让 AI 从零写一个「带标题栏和一个计数按钮」的窗口，四个 C++ 源文件一次通过编译，  
XML 布局里引用的控件、类名、语义色、字体 ID 全部命中源码，没有一个需要人工返工。

---

## 二、nim_duilib 是什么，难在哪

[nim\_duilib](https://github.com/rhett-lee/nim_duilib) 是基于 Skia 的跨平台 C++ UI 框架，  
支持 Windows 7/10/11、Linux、macOS 12+、FreeBSD，采用 **XML 描述界面 + C++ 写逻辑** 的模式：

```xml
<Button class="btn_global_blue_80x30" name="btn_ok" text="确定" width="80" height="32"/>
```

```cpp
ui::Button* btn = dynamic_cast<ui::Button*>(FindControl(_T("btn_ok")));
btn->AttachClick([this](const ui::EventArgs& args) { /* ... */ return true; });
```

它很好用，但有三道门槛：

**门槛一：编译链很重。** Skia 的源码在仓库外（与 nim_duilib 同级目录），一次完整编译要数小时，  
还要配 LLVM/Clang。日常开发不可能反复全量构建。

**门槛二：API 面太广。** 光是 XML 节点就有 121 个 `DUI_CTR_*` 宏，事件枚举 96 个，  
字体 ID 42 个，语义色名 350 个，C++ 类名与函数模板索引下来 2400+ 个。  
靠记忆写，出错是常态。

**门槛三：错误大多是「静默失效」。** 这类框架最难受的不是编译报错，而是：  
`class` 属性写错位置 → 控件没样式；颜色写死色值 → 深色主题下界面全黑；  
标题栏按钮用旧名 → 最小化/关闭按钮点了没反应。**代码能编译，界面是废的。**

第三点正是通用大模型最容易翻车的地方——它没有项目知识，只能按「看起来合理」的方式写。

---

## 三、环境准备：第一次拉源码必须做两件事

仓库里**没有**编译好的依赖库，直接开工程会找不到库。第一次（以及升级 Skia、切换架构时）执行一次即可。

### Windows：两步

```bat
cd /d <仓库根>\build

REM 第 1 步：下载依赖库源码并编译（主要是 Skia，耗时数小时）
build_duilib_all_in_one.bat

REM 第 2 步：用 cmake/MSVC 编译第三方库 + duilib + 各示例
msvc_build.bat
```

- 第 1 步会 `git clone` 三份源码（`nim_duilib`、`skia_compile`、`google/skia`），  
  把 Skia 检出到固定 commit 并打补丁，然后编译 `llvm.x64.debug/release`、`llvm.x86.debug/release` 四套配置，  
  最后用 `devenv` 构建 `build\examples.sln`。前置条件是 `git`、`python3`、`C:\LLVM`（Clang）、VS 2022+。  
  可选参数 `/MD` 切到动态运行时库（默认 `/MT` 静态），**选定后中途不要混用**。
- 第 2 步用 cmake 依次构建第三方库（`zlib`、`libpng`、`cximage`、`libwebp`、`libcef`）、`duilib`，  
  再构建 `examples` 下的各示例程序。要求 cmake ≥ 3.21（用 VS2026 则需 4.2）。

### 非 Windows：一步

```bash
cd <仓库根>/build
chmod +x build_duilib_all_in_one.sh
./build_duilib_all_in_one.sh      # MSYS2 下可加 -sdl
```

脚本按 `uname` 自动分流到 `linux_build.sh` / `macos_build.sh` / `freebsd_build.sh` / `msys2_build.sh`。  
**非 Windows 没有对应的第二步。**

产物落在 `lib/` 与 `bin/`；`build/build_temp`、`build/.vs` 是临时目录，可随时清理。

---

## 四、给 AI 装上项目记忆：入口文档 + Skill

### 4.1 入口文档 `WORKBUDDY.md`

放在仓库根目录，与 `CLAUDE.md` 平级。它的作用不是教程，而是**给 AI 看的项目说明书**：预编译步骤、  
Skill 位置、开发流程、必守约束、验证手段、路径约定、权威源对照表。

其中最有价值的一张表是「写不确定的名字前，先查这里」：

| 要确认的东西             | 权威源                                                |
| ------------------ | -------------------------------------------------- |
| XML 节点名            | `duilib/duilib_defs.h` 的 `DUI_CTR_*`               |
| 控件支持的属性名           | 该控件 `.h/.cpp` 里的 `strName == _T("...")` 分支         |
| 事件枚举名              | `duilib/duilib_defs.h` 的 `enum EventType`          |
| XML 内联事件的 type 字符串 | `duilib/Core/EventArgs.cpp` 的 `InitEventStringMap` |
| 主题色真实取值            | `bin/resources/themes/color_light/global.xml`      |
| 字体 ID / Class 样式名  | `bin/resources/themes/default/global.xml`          |

### 4.2 Skill：一份入口 + 八份专题 + 一套模板

```
.workbuddy/skills/nim-duilib/
├── SKILL.md                  # 入口：路由表 + 六条必守约束 + 权威源对照表
├── references/               # 按需加载，不要一次全读
│   ├── create-window.md      # 新建窗口（四件套 + 主线程）
│   ├── xml-layout.md         # 布局、容器选择、尺寸写法
│   ├── add-control.md        # 控件属性、动态增删数据
│   ├── event-handler.md      # 事件绑定（XML 内联 / C++ Attach）
│   ├── theme.md              # 主题、语义色、字体、Class
│   ├── resource-pack.md      # 打包：目录 / ZIP / 嵌入 EXE
│   ├── api-reference.md      # 约 600 行速查表（只在查表时才读）
│   └── pitfalls.md           # 已知坑及是否已修复
├── assets/templates/         # CMakeLists.txt + MyForm.h/.cpp + my_form.xml
└── scripts/verify_docs.py    # 文档与源码漂移校验
```

设计上有两个关键点：

- **单 Skill + 分层参考**，而不是按功能拆成 6 个 Skill。AI 靠语义匹配触发 Skill，  
  多个描述相近的 Skill 会互相抢触发；分成入口 + `references/` 后，平时只占极少上下文，  
  用到哪份读哪份（这就是 Skill 的三级加载机制）。
- **模板必须存在**。让 AI「照文档写」和「复制已验证的骨架再改」，后者可靠得多。  
  模板里的 CMakeLists、窗口类、XML 都是已经核对过、能跑通的状态。

同一个 Skill 我们放了两份：项目级 `.workbuddy/skills/nim-duilib/`（随仓库走）  
和用户级 `~/.workbuddy/skills/nim-duilib/`（跨项目可用），改动后互相同步：

```bash
cp -r <仓库根>/.workbuddy/skills/nim-duilib/. ~/.workbuddy/skills/nim-duilib/
```

---

## 五、实战：一句话需求到一个能编译的窗口

需求：「写一个窗口，带标题栏、一个 Label 和一个按钮，点按钮显示点击次数。」

AI 拿到这个需求后的动作是：加载 `SKILL.md` → 读 `create-window.md` → 复制 `assets/templates/` 四件套 →  
批量替换类名与皮肤名 → 补主线程与入口 → 编译自检。

**完整工程结构**（`examples/hello_button/`）：

```
examples/hello_button/
├── CMakeLists.txt        # 工程定义（四件套之一，含 Skia 子目录默认规则）
├── HelloForm.h           # 窗口类声明
├── HelloForm.cpp         # 窗口类实现（查找控件 + 绑事件 + 点击计数）
├── MainThread.h          # 主线程声明
├── MainThread.cpp        # 主线程实现（初始化资源 + 创建窗口）
├── TestApplication.h     # 应用入口声明
├── TestApplication.cpp   # 应用入口实现（启动消息循环）
└── main_windows.cpp      # Windows 平台 wWinMain 入口
bin/resources/themes/default/hello_button/
└── hello_button.xml      # 界面布局（与代码通过控件 name 关联）
```

下面把每个文件完整列出。它们都经过 `cl.exe /Zs` 语法检查（4 个 `.cpp` 全部退出码 0），
XML 里引用的控件、`class`、语义色名、字体 ID 也都通过漂移校验，可以直接拷贝到仓库里编译运行。

**① 窗口类 `HelloForm.h`**

```cpp
#ifndef EXAMPLES_HELLO_FORM_H_
#define EXAMPLES_HELLO_FORM_H_

#include "duilib/duilib.h"

// 基类 ui::WindowImplBase 的声明在 duilib/Utils/WinImplBase.h（文件名不带 Window 前缀）
class HelloForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    HelloForm();
    virtual ~HelloForm() override;

    // 返回皮肤目录名：bin/resources/themes/default/<此处返回值>/
    virtual DString GetSkinFolder() override;
    // 返回布局文件名：bin/resources/themes/default/<skin_folder>/<此处返回值>
    virtual DString GetSkinFile() override;

    // 窗口创建完成、控件已就绪后调用，在这里查找控件并绑定事件
    virtual void OnInitWindow() override;

private:
    int32_t m_clickCount = 0;   // 按钮点击次数
};

#endif // EXAMPLES_HELLO_FORM_H_
```

**② 窗口类实现 `HelloForm.cpp`**

```cpp
#include "HelloForm.h"

HelloForm::HelloForm()
{
}

HelloForm::~HelloForm()
{
}

DString HelloForm::GetSkinFolder()
{
    return _T("hello_button");
}

DString HelloForm::GetSkinFile()
{
    return _T("hello_button.xml");
}

void HelloForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    // 查找控件务必 dynamic_cast 并判空：引用不存在的 name 不会报错，只会拿到 nullptr
    ui::Label* label = dynamic_cast<ui::Label*>(FindControl(_T("label_tip")));

    if (auto* btn = dynamic_cast<ui::Button*>(FindControl(_T("btn_click")))) {
        // 用 C++ lambda 绑事件；也可在 XML 里写 <Event type="click" ...> 内联绑定
        btn->AttachClick([this, label](const ui::EventArgs& args) {
            // 取发送者控件用 GetSender()，不是 pSender（pSender 是 private）
            if (args.GetSender() == nullptr) {
                return true;
            }
            ++m_clickCount;
            if (label != nullptr) {
                // 数字转文本用 StringUtil::Printf，避免手写 DString 拼接出错
                label->SetText(ui::StringUtil::Printf(_T("你已点击 %d 次"), m_clickCount));
            }
            return true;   // 返回 true 表示事件已处理
        });
    }
}
```

**③ 界面布局 `hello_button.xml`**

放在 `bin/resources/themes/default/hello_button/hello_button.xml`，与代码通过控件 `name` 关联：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Window size="480,320" min_size="240,100"
        caption="0,0,0,36" use_system_caption="false"
        snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" shadow_attached="true"
        layered_window="true" alpha="255" size_box="4,4,4,4"
        icon="public/caption/logo.ico">
    <VBox bkcolor="bg_window_main">
        <!-- 标题栏：容器 name 必须是 window_title_bar，按钮必须用 btn_window_* 新名 -->
        <HBox name="window_title_bar" width="stretch" height="36" bkcolor="bg_titlebar">
            <Control width="18" height="18" bkimage="public/caption/logo.svg" valign="center" margin="8"/>
            <Label text="Hello nim_duilib" valign="center" margin="8" mouse_enabled="false"/>
            <Control />
            <Button class="btn_wnd_min_11"   height="32" width="40" name="btn_window_min"   margin="0,2,0,2"/>
            <!-- 最大化/还原按钮成对出现：btn_window_restore 默认隐藏，框架在最大化时自动切换显隐 -->
            <Box height="32" width="40" margin="0,2,0,2">
                <Button class="btn_wnd_max_11"     height="32" width="stretch" name="btn_window_max"/>
                <Button class="btn_wnd_restore_11" height="32" width="stretch" name="btn_window_restore" visible="false"/>
            </Box>
            <Button class="btn_wnd_close_11" height="stretch" width="40" name="btn_window_close" margin="0,0,0,2"/>
        </HBox>

        <!-- 内容区域 -->
        <Box padding="16,16,16,16">
            <VBox halign="center" valign="center">
                <!-- name=label_tip 供 C++ 在点击时更新文本 -->
                <Label name="label_tip" text="点一下下面的按钮" font="system_regular_14"
                       normal_text_color="text_default" text_align="hcenter"/>
                <HBox height="auto" halign="center" margin="0,20,0,0">
                    <Button class="btn_global_blue_80x30" name="btn_click" text="点我" width="80" height="32"/>
                </HBox>
            </VBox>
        </Box>
    </VBox>
</Window>
```

> 注意 `<VBox bkcolor="bg_window_main">` 与 `<HBox ... bkcolor="bg_titlebar">` 用的是**语义色名**，
> 不是写死色值——这样浅色/深色主题会自动切换。`bg_window_main`、`bg_titlebar`、`text_default` 都是
> 框架内置语义色（`color_light`/`color_dark` 两套取值）。

**④ 主线程 `MainThread.h` / `MainThread.cpp`**

负责初始化全局资源（`GetResourceRootPath(false)` 按平台自动定位 resources 目录）并创建窗口：

```cpp
// MainThread.h
class MainThread : public ui::FrameworkThread
{
public:
    MainThread();
    virtual ~MainThread() override;
private:
    virtual bool OnInit() override;      // 返回 bool，不是 void
    virtual void OnCleanup() override;
};

// MainThread.cpp
#include "MainThread.h"
#include "HelloForm.h"

MainThread::MainThread() : FrameworkThread(_T("MainThread"), ui::kThreadUI) {}

bool MainThread::OnInit()
{
    ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath(false);
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    // 窗口由框架负责析构，用 new 创建即可，不要手动 delete
    HelloForm* window = new HelloForm();
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("hello_button"), true));
    window->PostQuitMsgWhenClosed(true);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
    return true;   // 初始化成功才进入消息循环
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}
```

**⑤ 应用入口 `TestApplication.h` / `TestApplication.cpp` / `main_windows.cpp`**

```cpp
// TestApplication.h
class TestApplication
{
public:
    TestApplication();
    ~TestApplication();
    void Run();
};

// TestApplication.cpp
#include "TestApplication.h"
#include "MainThread.h"

void TestApplication::Run()
{
    MainThread thread;
    thread.RunMessageLoop();
}

// main_windows.cpp（Windows 平台入口，MinGW / MSVC 统一走 wWinMain）
#if defined (_WIN32) || defined (_WIN64)
#include "duilib/duilib_config_windows.h"
#include "TestApplication.h"

#if defined (__MINGW32__) || defined (__MINGW64__)
int APIENTRY WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
#else
int APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
#endif
{
    TestApplication app;
    app.Run();
    return 0;
}
#endif
```

**⑥ 工程文件 `CMakeLists.txt`**

四件套之一，关键是 `DUILIB_SKIA_LIB_SUBPATH` 的默认值必须写在 `include(duilib_common.cmake)` 之前
（原因见第六节）：

```cmake
cmake_minimum_required(VERSION 3.18)
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

project(hello_button CXX)                      # 需与 examples/ 下的目录名一致

get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)

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

include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_common.cmake")
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_bin.cmake")
```

整个过程不需要人去翻 `docs/Control.md` 查属性名，也不需要回忆按钮该叫什么——
窗口类、XML、主线程、入口、CMake 都已经由 Skill 提供并验证，AI 直接取用即可。

---

## 六、验证三板斧：不做全量构建也能放心

Skia 在仓库外、全量构建要几小时，所以日常用这三层验证，成本都在秒级到分钟级。

**第一层：文档漂移自检。** 改完文档或生成新代码后跑一遍，脚本会把文档里出现的  
`DUI_CTR_*`、`kEvent*`、类名、字体 ID、语义色名逐一对照源码：

```bash
python .workbuddy/skills/nim-duilib/scripts/verify_docs.py --repo <仓库根>
python .workbuddy/skills/nim-duilib/scripts/verify_docs.py --repo <仓库根> --extra <自己生成的文档或目录>
```

输出形如：`扫描文档 13 个，源码索引：121 个宏 / 96 个事件 / 2408 个类名 / 42 个字体 / 350 个色名，
未发现文档漂移`。它第一次运行就抓出了源文档里一个真实错误：`bg_statusbar` 这个色名在三个  
`global.xml` 里都不存在。<!-- verify:allow-missing bg_statusbar -->

**第二层：cmake configure。** 只配置不构建，即可验证 `CMakeLists.txt` 是否写对，几秒钟出结果。

**第三层：MSVC 语法检查模式 `/Zs`。** 不生成 obj、不链接，但会完成语义分析与模板实例化——  
`UiBind`、`AttachClick`、`GetSender()` 这类用法错误在这一步就会暴露：

```bash
export MSYS_NO_PATHCONV=1     # Git Bash 必加，否则 /I 被当成源文件（D9024）
VC="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/<版本>"
SDK="C:/Program Files (x86)/Windows Kits/10/Include/<SDK版本>"
"$VC/bin/Hostx64/x64/cl.exe" -nologo -std:c++20 -utf-8 -EHsc -Zs -W3 \
    -DWIN32 -D_WINDOWS -DUNICODE -D_UNICODE \
    -I<仓库根> -I. -I"$VC/include" \
    -I"$SDK/um" -I"$SDK/shared" -I"$SDK/ucrt" -I"$SDK/winrt" \
    HelloForm.cpp
```

`-utf-8` 不能省（源码有中文注释时否则报 C4819）。退出码 0 即通过。

> 顺带说一个 CMake 的时序坑：工程里给 `DUILIB_SKIA_LIB_SUBPATH` 设默认值时，  
> 必须写在 `include(duilib_common.cmake)` **之前**。因为那个脚本用 `option()` 声明了同名变量，  
> `option()` 执行后变量必然已定义，此后再判断「是否未定义」永远为假。

---

## 七、六条「不报错但静默失效」的硬约束

这六条我们写进了 `SKILL.md` 的最显眼位置，因为它们违反后编译器一声不吭：

1. **`class` 属性必须写在所有属性最前面**，写在后面则整个 class 的样式被忽略。
2. **标题栏与按钮必须用新名**：容器 `window_title_bar`，按钮 `btn_window_min/max/restore/close/fullscreen`。  
   旧名（`minbtn`、`closebtn`…）只是兼容 fallback，新代码不要用。
3. **颜色一律写语义色名**，不要写死色值。真实色值在  
   `bin/resources/themes/color_light/global.xml` 与 `color_dark/global.xml` 的 `<ThemeColor>` 里，  
   随系统深浅色自动切换；`themes/default/global.xml` 只剩旧名→新名的 `<Alias>` 映射。  
   **打包发布时必须带上 `color_light/` 与 `color_dark/` 两个目录，否则整屏没有颜色。**
4. **取事件发送者用 `args.GetSender()`**，不是 `args.pSender`（后者是 private，编译不过）；  
   返回 `nullptr` 表示控件已销毁，应提前返回。
5. **`FrameworkThread::OnInit()` 返回 `bool`**，不是 `void`，末尾要 `return true;`。
6. **资源初始化用 `ui::GlobalManager::GetResourceRootPath(false)`**，不要手工拼 `+ "\\resources\\"`——  
   硬编码反斜杠在 Linux/macOS 上路径就是错的。

---

## 八、把这套方法搬到你自己的项目

nim_duilib 只是例子，这套做法对任何「API 面广、错误静默、文档会过期」的框架都适用：

1. **先写入口文档，再谈让 AI 写代码。** 内容聚焦于：怎么构建、目录在哪、哪些事绝对不能做、  
   不确定的东西去哪查。不要写成长篇教程——AI 只会读它需要的部分。
2. **按任务切分参考资料，不要塞进一个文件。** 入口保持精简，用一张路由表指向专题文档，  
   让 AI 按需加载。
3. **提供可复制的模板。** 模板比文档可靠，因为它是被验证过的状态。
4. **一定要有自动校验。** 人写的文档会漂，AI 写的文档也会漂。  
   一个能对照源码检查名字的脚本，价值远大于十条注意事项。  
   我们正是在做校验时发现：项目原有文档与代码之间存在 26 处不一致——包括写错基类、  
   事件枚举用了不存在的名字、字体 ID 引用了早已删除的系列。这些问题靠肉眼审是审不出来的。
5. **AI 也是使用者，也是维护者。** 让它改完文档后自己跑一遍校验，把「防漂移」变成流程的一部分。

---

## 九、结语

大模型不缺通用编程能力，缺的是**你这个项目的具体事实**：按钮该叫什么名字、颜色该引用哪个变量、  
哪个返回值其实是 `bool`。把这些事实结构化地放进 Skill，配上一套自动校验，  
AI 就从「会写 C++ 的陌生人」变成「懂这个框架的同事」。

本文涉及的三份产物都在 nim_duilib 仓库里可以直接看到：  
仓库根目录的 `WORKBUDDY.md`（WorkBuddy 使用指南）、  
`.workbuddy/skills/nim-duilib/`（Skill 本体）、  
`scripts/verify_docs.py`（漂移校验脚本）。

如果你也在维护一个有历史包袱的 C++ 项目，不妨从「写一份入口文档 + 建一个校验脚本」开始，  
成本比想象中低，收益比想象中大。

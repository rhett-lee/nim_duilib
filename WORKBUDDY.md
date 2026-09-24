# nim_duilib - WorkBuddy 使用指南

本文面向使用 **WorkBuddy** 开发 nim_duilib 的场景，说明环境准备、专用 Skill 的用法、日常开发流程
与验证手段。内容以仓库源码为准；与 `CLAUDE.md`（Claude Code 用）信息同源，两份需保持一致。

## 1. 三分钟速览

| 事项 | 位置 / 命令 |
|------|-------------|
| 首次拉取源码后的依赖库预编译 | `build\build_duilib_all_in_one.bat` → `build\msvc_build.bat`（Windows） |
| WorkBuddy 专用 Skill | `.workbuddy\skills\nim-duilib\`（项目级）、`~/.workbuddy/skills/nim-duilib\`（用户级） |
| 新建窗口骨架 | Skill 的 `assets/templates/`：CMakeLists.txt + MyForm.h + MyForm.cpp + my_form.xml |
| 改文档后自检 | `python .workbuddy/skills/nim-duilib/scripts/verify_docs.py --repo .` |
| 只验证 C++ 语法（不链接） | `cl.exe /Zs ...`（见第 6 节） |

---

## 2. 环境准备：首次获取源码后必须做一次 lib 预编译

新克隆的仓库里**没有**编译好的依赖库（Skia、zlib、libpng、cximage、libwebp 等），直接开工程会报
找不到库。**只需要在第一次（以及升级 Skia / 切换架构时）做一次**。

### 2.1 Windows：两步

```bat
cd /d <仓库根>\build

REM 第 1 步：拉取依赖库源码并编译（主要是 Skia）
build_duilib_all_in_one.bat

REM 第 2 步：用 cmake/MSVC 编译依赖库 + duilib + 各示例
msvc_build.bat
```

**第 1 步 `build_duilib_all_in_one.bat`**（需要联网）：
- 前置软件：`git`、`python3`、`C:\LLVM`（clang/clang++，Skia 用它编译）、Visual Studio 2022 或更高。
  缺任一项脚本会直接退出并提示。
- 依次 `git clone/pull`：`nim_duilib`、`skia_compile`（补丁与工具）、`google/skia`。
- 编译 Skia 四种配置：`llvm.x64.debug`、`llvm.x64.release`、`llvm.x86.debug`、`llvm.x86.release`。
- 最后用 `devenv` 编译 `build\examples.sln`（Debug|x64）。
- 可选参数：`/MD` 切到动态运行时库（默认 `/MT` 静态）。运行库选择会影响后续所有工程，**中途不要混用**。
- 网络失败会自动重试（每 10 秒一次），不用人工干预。

**第 2 步 `msvc_build.bat`**（仅 Windows）：
- 前置：`cmake`（最低 v3.21；用 VS2026 则需 v4.2）、VS2022+。Skia 必须已编译好——脚本会检查
  `<仓库根>\..\skia\out\llvm.<arch>.release` 是否存在，不存在则提示先跑第 1 步。
- 依次 cmake 构建：第三方库 `zlib,libpng,cximage,libwebp,libcef\libcef_win` → `duilib` →
  `examples` 下的各示例程序。
- 默认 Release；架构取 VS 目标架构（缺省 x64），并据此设置 `DUILIB_SKIA_LIB_SUBPATH=llvm.<arch>.release`。

### 2.2 非 Windows：一步

```bash
cd <仓库根>/build
chmod +x build_duilib_all_in_one.sh
./build_duilib_all_in_one.sh        # MSYS2 下可加 -sdl 启用 SDL
```

脚本按 `uname` 自动分流：Linux → `linux_build.sh`，macOS → `macos_build.sh`，
FreeBSD → `freebsd_build.sh`，MSYS2(Windows) → `msys2_build.sh`；同样会先下载并编译 Skia。
**没有** `msvc_build.bat` 对应的第二步，非 Windows 不需要单独跑。

### 2.3 产物与目录

| 产物 | 路径 |
|------|------|
| lib 文件 | `<仓库根>/lib`（`lib/x64`、`lib/Release` 等） |
| bin / 资源 | `<仓库根>/bin`，资源在 `bin/resources/` |
| 编译临时目录（可清理） | `<仓库根>/build/build_temp`、`build/.vs` |
| Skia 源码与产物 | `<仓库根>/../skia`（**在仓库外**，与 nim_duilib 同级） |

> Skia 在仓库外且体积巨大，因此**全量链接构建通常不在日常开发中做**，
> 日常用第 6 节的 `/Zs` 语法检查或单示例 cmake 构建来替代。

---

## 3. WorkBuddy 专用 Skill：`nim-duilib`

### 3.1 部署位置（两份，内容相同）

| 级别 | 路径 | 生效范围 |
|------|------|----------|
| 项目级 | `<仓库根>\.workbuddy\skills\nim-duilib\` | 仅本仓库 |
| 用户级 | `C:\Users\<用户名>\.workbuddy\skills\nim-duilib\` | 所有项目 |

改动 Skill 后必须双向同步，否则两边行为会分叉：

```bash
cp -r <仓库根>/.workbuddy/skills/nim-duilib/. ~/.workbuddy/skills/nim-duilib/
```

### 3.2 目录结构

```
.workbuddy/skills/nim-duilib/
├── SKILL.md                  # 入口：路由表 + 六条必守约束 + 权威源对照表
├── references/               # 按需加载，不要一次全读
│   ├── create-window.md      # 新建窗口（四件套 + 主线程）
│   ├── xml-layout.md         # XML 布局、容器选择、尺寸写法
│   ├── add-control.md        # 控件属性、动态增删数据
│   ├── event-handler.md      # 事件绑定（XML 内联 / C++ Attach）
│   ├── theme.md              # 主题、语义色、字体、global.xml Class
│   ├── resource-pack.md      # 资源打包：目录 / ZIP / 嵌入 EXE
│   ├── api-reference.md      # 约 600 行完整速查表（确实要查表时才读）
│   └── pitfalls.md           # 15 个已知坑及是否仍存在
├── assets/templates/         # 已核对可用的骨架：CMakeLists.txt / MyForm.h / MyForm.cpp / my_form.xml
└── scripts/verify_docs.py    # 文档与源码漂移校验
```

### 3.3 触发与加载

WorkBuddy 按语义自动触发：提到建窗口/改 XML/加控件/绑事件/主题配色/打包发布/窗口空白/样式失效等
即会加载 `SKILL.md`，再按任务类型**只读**对应的 `references/*.md`。
`api-reference.md` 最大，只在需要查表时才载入。

### 3.4 文档自检脚本

改完 Skill 文档或生成的新文档后跑一次，比对其中出现的 `DUI_CTR_*`、`kEvent*`、类名、字体 ID、
颜色名是否与源码一致：

```bash
python .workbuddy/skills/nim-duilib/scripts/verify_docs.py --repo <仓库根>
python .workbuddy/skills/nim-duilib/scripts/verify_docs.py --repo <仓库根> --extra <自己生成的文档.md>
```

确实需要引用源码中不存在的名字时，在文档里加一行 HTML 注释走白名单：
`<!-- verify:allow-missing 名字1 名字2 -->`。

---

## 4. 日常开发流程

### 4.1 新建窗口：四件套 + 主线程

1. 从 `assets/templates/` 复制 `CMakeLists.txt`、`MyForm.h`、`MyForm.cpp`、`my_form.xml`；
   XML 放到 `bin/resources/themes/default/<skin_folder>/<skin_file>.xml`。
2. 批量替换：`MyForm` → 窗体类名，`my_skin` → 皮肤目录，`my_form.xml` → 布局文件名；
   `CMakeLists.txt` 里的 `project(<name>)` 改成工程目录名。
3. 补主线程与入口（`MainThread` 继承 `ui::FrameworkThread`，`TestApplication` + `main_windows.cpp`，
   可参考 `examples/hello_button/`）。
4. 把新示例加入构建：`examples.sln` 需**手工**添加工程；若希望 `build\msvc_build.bat` 一并编译，
   把工程名加进脚本里的 `DUILIB_PROGRAMS` 列表。

`CMakeLists.txt` 中 `DUILIB_SKIA_LIB_SUBPATH` 的默认设置必须写在
`include(duilib_common.cmake)` **之前**——该脚本用 `option()` 声明此变量，include 之后它必然已定义，
再判断"是否未定义"永远为假。模板已按此规则写好，照抄即可。

### 4.2 关键代码模式

```cpp
// 资源初始化：自动处理平台差异，不要手工拼 "\\resources\\"
ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath(false);
ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

// 创建窗口（框架负责析构，用 new，不要 delete）
MainForm* window = new MainForm();
window->CreateWnd(nullptr, ui::WindowCreateParam(_T("WindowTitle"), true));
window->PostQuitMsgWhenClosed(true);
window->ShowWindow(ui::kSW_SHOW_NORMAL);

// 查找控件 + 绑定事件
ui::Button* btn = dynamic_cast<ui::Button*>(FindControl(_T("btn_name")));
btn->AttachClick([this](const ui::EventArgs& args) { return true; });
```

---

## 5. 六条必守约束

违反这些不会报错，只会静默出问题，排查成本极高。

1. **`class` 属性写在所有属性最前面**（写在后面整个 class 的样式被忽略）。
2. **标题栏与按钮用新名**：容器 `window_title_bar`；按钮 `btn_window_min` / `btn_window_max` /
   `btn_window_restore` / `btn_window_close` / `btn_window_fullscreen`。旧名（`minbtn`、`closebtn` …）
   只是 fallback，新代码不要用（依据 `duilib/duilib_defs.h:145-152`）。
3. **颜色写语义色名，不写死色值**。真实色值在
   `bin/resources/themes/color_light/global.xml` 与 `color_dark/global.xml` 的 `<ThemeColor>`；
   `themes/default/global.xml` 只剩旧名→新名的 `<Alias>`。写死色值会让深色主题失效。
   **打包时必须带上 `themes/color_light/` 与 `themes/color_dark/`，否则整屏无色。**
4. **取事件发送者用 `args.GetSender()`**，不是 `args.pSender`（后者是 private，编译不过）；
   返回 `nullptr` 表示控件已销毁，应提前返回。
5. **`FrameworkThread::OnInit()` 返回 `bool`**（不是 void），末尾 `return true;`。
6. **资源初始化用 `ui::GlobalManager::GetResourceRootPath(false)`**，不要手工拼路径分隔符。

其他编码规范：字符串用 `DString` + `_T("...")`；`FindControl` 后 `dynamic_cast`；
事件回调返回 `true` 表示已处理；XML 属性值内嵌引号用单引号 `'` 或花括号 `{}`。

---

## 6. 构建与验证

### 6.1 只编译不链接（推荐日常验证）

完整构建需要 duilib 库与 Skia。只想验证 API 用法时，用 MSVC 语法检查模式 `/Zs`
（不生成 obj、不链接，但做完整语义分析与模板实例化）：

```bash
export MSYS_NO_PATHCONV=1        # Git Bash 必加，否则 /I 被当成源文件（D9024）
VC="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/<版本>"
SDK="C:/Program Files (x86)/Windows Kits/10/Include/<SDK版本>"
"$VC/bin/Hostx64/x64/cl.exe" -nologo -std:c++20 -utf-8 -EHsc -Zs -W3 \
    -DWIN32 -D_WINDOWS -DUNICODE -D_UNICODE \
    -I<仓库根> -I. -I"$VC/include" \
    -I"$SDK/um" -I"$SDK/shared" -I"$SDK/ucrt" -I"$SDK/winrt" \
    YourForm.cpp
```

`-utf-8` 不能省（源含中文注释否则 C4819）。退出码 0 即通过，`UiBind` 这类模板错误在这一步就会暴露。

### 6.2 单示例 cmake 构建

以 `examples/hello_button` 为例：

```bash
cmake -S examples/hello_button -B build/build_temp/hello_button -G "Visual Studio 17 2022" -A x64
cmake --build build/build_temp/hello_button --config Release
```

`-A Win32` 时 Skia 子目录自动取 `llvm.x86.release`；也可用 `-DDUILIB_SKIA_LIB_SUBPATH=...` 覆盖。

### 6.3 Visual Studio

打开 `build\examples.sln`（含库 + 示例）或 `build\duilib.sln`（仅库），选 Debug|x64 / Release|x64。
不需要 CEF 时用 `*_no_cef.sln`。

---

## 7. 路径与文档索引

**常用路径**

| 用途 | 路径 |
|------|------|
| 主题资源根 | `bin/resources/themes/default/` |
| 全局样式 / Class / 字体 ID | `bin/resources/themes/default/global.xml` |
| 语义色真实取值 | `bin/resources/themes/color_light\|color_dark/global.xml` |
| 窗口 XML | `bin/resources/themes/default/<skin_folder>/<skin_file>.xml` |
| 公共图片 | `bin/resources/themes/default/public/` |
| 字体文件 | `bin/resources/fonts/` |
| 语言文件 | `bin/resources/lang/`（**不在** `themes/default/lang/`） |
| 示例源码 | `examples/<示例名>/`（注意大小写，如 `examples/RichEdit`） |

**仓库文档**：`docs/Summary.md`（索引）、`Getting-Started.md`（入门）、`Control.md`、`Box.md`、
`Global.md`、`Window.md`、`Theme.md`、`ThemeColor.md`、`Events.md`、`XmlEvents.md`、`XmlNode.md`、
`Examples.md`、`build/build.md`（构建脚本对照表）、`docs/CEF.md`（CEF 模块）。

**权威源（写不确定的名字前先查这里）**

| 要确认 | 权威源 |
|--------|--------|
| XML 节点名 | `duilib/duilib_defs.h` 的 `DUI_CTR_*` |
| 控件属性名 | 该控件 `.h/.cpp` 里 `strName == _T("...")` 分支（`Progress.cpp` 用的是 `srName`） |
| 事件枚举名 | `duilib/duilib_defs.h` 的 `enum EventType` |
| XML 内联事件 type 字符串 | `duilib/Core/EventArgs.cpp` 的 `InitEventStringMap` |
| C++ 类声明 | `grep -rn "class DUILIB_API <Name>" duilib/` |

---

## 8. 与 CLAUDE.md / `.claude` 的关系

| 目录 | 使用者 | 形态 |
|------|--------|------|
| `CLAUDE.md` + `.claude/docs` + `.claude/skills/*.md` | Claude Code | 扁平 md，靠注册脚本部署 |
| `WORKBUDDY.md`（本文）+ `.workbuddy/skills/nim-duilib/` | WorkBuddy | `SKILL.md` + `references/`，语义触发、三级加载 |

两边描述的是同一套框架事实，**修一边就要同步另一边**，否则会重新出现文档与代码不一致
（本项目历史上出现过 26 处）。`C++20`、`WindowImplBase` 位于 `duilib/Utils/WinImplBase.h`、
`OnInit()` 返回 `bool`、`pSender` 为 private 等都属于易错点，改动时重点复核。

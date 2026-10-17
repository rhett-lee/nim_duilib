
# nim_duilib 开发执行规划（Development Plan）

> 本文档是**可跟踪执行**的开发规划，与其它规划类文档的分工见下表。
> 基线日期：**2026-10-17**（A-5 已完成）　｜　覆盖平台：**Windows / macOS / Linux / FreeBSD**
> 更新规则：每完成一项把 `- [ ]` 改为 `- [x]` 并填写「完成日期」；新增项追加到对应小节末尾，不要重排既有编号。

## 与其它文档的关系

| 文档 | 定位 | 与本文档的关系 |
| :--- | :--- | :--- |
| `docs/misc/Project-Audit.md` | 一次性审计报告（现状 + 建议） | **本文档的上游输入**。审计已提出但未进入本文档跟踪的条目，视为「已记录、不排期」 |
| `docs/misc/Control-Roadmap.md` | 控件规划清单 1–20 项 | 已完成的 1–17 项不再重复；其 18–20 项（无障碍 / 原生通知 / 应用内更新）并入本文档 D、E 组跟踪 |
| `docs/misc/跨平台开发相关事项.md` | 跨平台历史决策记录 | 已完成的条目标记为存量；本文档只跟踪**新增**跨平台工作 |
| `docs/misc/ListCtrl-Roadmap.md` / `ListCtrl-Performance.md` | ListCtrl 专项 | ListCtrl 内的单元格/性能议题仍由这两份文档负责，本文只跟踪其**结论对全局的影响**（见 B-4） |

若与上述文档冲突，以本文档的「执行顺序」为准，但**结论性描述**应同步回写原文档。

---

## 零、现状快照

| 维度 | 现状数据 | 判断 |
| :--- | :--- | :--- |
| 源码规模 | 602 个第一方文件，12 个模块，3529 次提交 | — |
| 控件数量 | `duilib/Control/` 159 个文件，Roadmap 1–17 项已全部落地 | 数量充足，**重心应从"补控件"转向"补地基"** |
| 示例程序 | 24 个，是目前**唯一**的质量验证手段 | 无自动化 |
| 单元测试 | **0 个**（无 gtest/catch2/doctest，无 `tests/`） | 🔴 最大短板 |
| CI | **0 条**（`.github/` 下只有 issue/PR 模板，无 `workflows/`） | 🔴 |
| 打包发布 | **0 条**（无 CPack、无 `install()`、无发版文档） | 🔴 |
| 文档一致性 | 属性名 493/493 命中、事件名 110/110 命中；**A-5 已修完**：`verify_docs.py --extra docs` 退出码 0、1162 条相对链接零失效 | 🟢 |
| `ASSERT` 一致性 | Windows Release **仍激活**（无 `_DEBUG` 包裹），三平台 Release **完全消失** | 🔴 跨平台硬伤 |
| 无障碍 | 全库零实现（仅 CEF 子控件自带） | 🔴 空白领域 |
| 跨平台架构 | SDL3 主轴覆盖窗口/输入/渲染/剪贴板/托盘/文件对话框全链路 | 🟢 设计扎实，风险集中在细节 |

---

## 一、执行总览

### 优先级定义

| 级别 | 含义 | 判据 |
| :--- | :--- | :--- |
| **P0** | 阻塞级 | 存在"编译不过"的风险，或后续任何大改动都没有安全网 |
| **P1** | 高优先级 | 跨平台行为不一致 / 明显的功能缺口 / 影响库的可交付性 |
| **P2** | 中优先级 | 能力补齐与体验完善，缺失不致命 |
| **P3** | 低优先级 | 长期演进项，或需要先完成前置架构改造 |

### 里程碑

```
M0 止血（约 3.5 人天）      ← 消除编译阻塞与埋雷式设计
   P0 组全部

M1 立网（约 8-10 人天）     ← 有了回归能力，才敢做后面所有事
   B 组（测试 + CI + 文档校验）

M2 补刚需（约 15-20 人天）  ← 控件与跨平台一致性
   C 组 + D 组 + E 组低垂果实

M3 架构（约 25-35 人天）    ← 决定库天花板的地基改造
   F 组（ListCtrl DataTemplate / 无障碍基础 / RTL 决策）

M4 生态（远期）             ← 打包发布、系统级能力
   G 组 + H 组
```

> **核心建议**：不要跳过 M1 直接做 M3。ListCtrl DataTemplate、无障碍适配、RichEdit 跨平台对齐都属于"改动面大、回归成本高"的架构级改造，在零测试零 CI 的现状下做这类改造的风险远高于收益。

### 依赖关系

```
        ┌─────────────────────────────┐
        │ M0 止血（4 项，可并行）        │
        └──────────────┬──────────────┘
                       ▼
        ┌─────────────────────────────┐
        │ M1 立网（测试框架 + CI）       │  ← 必须先于一切架构改造
        └──────────────┬──────────────┘
                       ▼
        ┌─────────────────────────────┐
        │ M2 补刚需                     │
        │  ├ C 跨平台一致性（IME/DPI）   │
        │  └ E 控件低垂果实             │
        └──────────────┬──────────────┘
                       ▼
        ┌─────────────────────────────┐
        │ M3 架构                       │
        │  F-1 ListCtrl DataTemplate    │←─┐
        │  F-2 无障碍基础属性           │  │ 有序依赖：
        │  F-3 RTL（需先决策）           │  │ F-2 → 平台 UIA 适配
        └─────────────────────────────┘  │ F-1 → TreeView 模板/Chart 增量
                                         │ F-3 → 全部 Layout 适配
                       ▼
        ┌─────────────────────────────┐
        │ M4 生态：打包发布 / 系统通知   │
        └─────────────────────────────┘
```

---

## 二、P0 组 — 阻塞级：消除编译阻塞与埋雷式设计

> 这一组全部是「现在不做，将来某次无关改动就会炸」的项，总量约 2.5 人天。

### A-1 `DUILIB_BUILD_FOR_CEF` 在 FreeBSD 上被定义但源码已排除 🔴

- **现状**：`duilib/duilib_config.h:74` 对**所有非 Windows 平台**无条件定义 `DUILIB_BUILD_FOR_CEF 1`，但 `duilib/CMakeLists.txt:68-73, 92-99` 在 FreeBSD 上排除了 CEF 的 include 路径与全部源码。
- **当前只是靠巧合没炸**：`duilib/duilib.cpp:4-7` 有单点防护 `#if !defined(__FreeBSD__)` 屏蔽了 `duilib_cef.h`。任何人新增一处 `#include "duilib/CEFControl/CefManager.h"`（该头在 `DUILIB_BUILD_FOR_CEF` 下会 `#include "include/cef_app.h"`）就会立即编译失败。
- **动作**：把 `duilib_config.h:74` 改为仅对 Windows/Linux/macOS 定义。
- **验收**：在 FreeBSD 上 `#define DUILIB_BUILD_FOR_CEF` 不再出现；全仓 grep 确认无其它位置依赖 FreeBSD 的 CEF 宏。
- **工作量**：0.5 人天　**依赖**：无

### A-2 CEF 层 4 个文件缺 FreeBSD 守卫 🟡

- **现状**：`CefManager.cpp:13-19, 72-84` 的三平台分支无 FreeBSD 落点（`GetInstance()` 会落入 `#else ASSERT(0); return nullptr`）；`CefClientApp.cpp:9,21,28,56` 与 `CefManager_Linux.cpp:3` 在 FreeBSD 下编译为空 TU → 链接期 undefined symbol。
- **参照**：`CefControlOffScreen.cpp:153` 与 `CefControlNative.cpp:101` 已正确写了 `|| defined (DUILIB_BUILD_FOR_FREEBSD)`，说明 FreeBSD CEF 曾被规划但未完成。
- **动作**：4 个文件补齐 FreeBSD 条件分支。
- **验收**：FreeBSD 构建开启 CEF 时无空 TU 警告、无 undefined symbol。
- **工作量**：1 人天　**依赖**：A-1

### A-3 SDL3 未锁定版本，全平台共同暴露面 🟡

- **现状**：`build/build_duilib_all_in_one.sh:196` 直接 `git clone` SDL3 主干，而同文件 `:228` 对 Skia 做了 `git checkout <commit>` 锁定。SDL3 上游一次 API 变动会同时击穿四个平台。
- **动作**：为 SDL 补 `git checkout <tag>`，并在 `WORKBUDDY.md` / `build/README.md` 记录当前锁定版本。
- **验收**：从零执行 `build_duilib_all_in_one.sh` 时 SDL 落到指定 commit。
- **工作量**：0.5 人天　**依赖**：无

### A-4 Skia 构建脚本 Linux 分支缺 `skia_enable_fontmgr_fontconfig=true` 🟡

- **现状**：`build_duilib_all_in_one.sh:431` FreeBSD 分支显式传入该开关，`:433` Linux 分支未传。而字体枚举依赖 `duilib/RenderSkia/FontMgr_Skia.cpp:212` 的 `SkFontMgr_New_FontConfig`。若上游默认值变化，Linux 会**静默返回空字体列表**（`FontManager.cpp:546` 无告警）。
- **动作**：Linux 分支补齐该开关；在 `FontManager.cpp` 字体列表为空时输出一条 WARN 日志。
- **验收**：Linux 上 `GetFontNameList` 返回非空；空列表时有可检索的日志。
- **工作量**：0.5 人天　**依赖**：无

### A-5 文档硬伤（5 处路径 / 2 处名称 / 22 个死链 / 1 处历史残留）✅ 已完成

- **原审计结论**（其中 1 条经复核为误判，已修正）：
  | 位置 | 问题 | 处理 |
  | :--- | :--- | :--- |
  | `docs/Summary.md` / `.en.md` | `ControlDragable.h` 路径错写为 `duilib/Control/`，实为 `duilib/Core/` | ✅ 改为 `duilib/Core/`（各 4 处） |
  | `docs/Summary.md:222` / `.en.md:224` | 引用了不存在的 `RichEdit_SDL.h` | ✅ 改为 `RichEdit2.h`，并注明 Windows 另有 `RichEditCtrl_Windows.h` / `RichEditHost_Windows.h` |
  | `docs/Summary.md` / `.en.md` | 引用了不存在的 `WindowDropTarget.h`（已按平台拆分） | ✅ 改为并列引用 `WindowDropTarget_SDL.h` + `WindowDropTarget_Windows.h` |
  | `docs/WindowShadow.md` / `.en.md` | `../../bin/...` 多了一级，文档在 `docs/` 下 | ✅ 改为 `../bin/...`（各 2 处） |
  | `docs/Summary.md` / `.en.md` | 基类写作 `nbase::SupportWeakCallback`，全仓已无 `nbase` | ✅ 改为 `SupportWeakCallback`（`PlaceHolder.h:15` 确认为虚继承基类） |
  | `docs/Menu.md:29` / `.en.md:31` | 样式类 `slider_green` 不存在 | ✅ 改为 `slider_horizontal_green`（`global.xml:151` 有 Alias） |
  | `README.md` / `.en.md` 4 个表格 | 「参考文档（本地文件）」列 22 处指向仓库外 `skia_compile` 的相对链接 | ✅ 整列删除（`skia_compile` 是仓库外独立仓库，任何只 clone 本库的环境都是死链，且与相邻「网络链接」列内容完全重复） |
  | ~~`docs/Global.md` 字体 ID `roboto_mono_12` 不存在~~ | ❌ **原审计误判** | 该 ID 是文档演示 `FontFile` 自定义字体时**由用户自行定义**的 ID（`Global.md:76` 的 `<Font id=... name="Roboto Mono">`），字体文件 `bin/resources/fonts/RobotoMono-*.ttf` 确实存在。已改为白名单 |
- **追加修复**（原清单未列，全量扫链接时发现）：
  | 位置 | 问题 | 处理 |
  | :--- | :--- | :--- |
  | `docs/misc/*.md`（6 个文件） | 35 处 `file:` 协议的本机绝对路径链接 —— 只在作者本机可点 | ✅ 改为按文件深度计算的相对路径 |
  | `docs/misc/Control-Roadmap.*.md` | 跨目录引用 `docs/` 下文档时缺 `../` 前缀 | ✅ 11+11 处补前缀 |
  | `docs/misc/ListCtrl-Roadmap.*.md`、`Project-Audit.md` | 在 `misc/` 下引用同目录文档时多套了一层 `misc/`；另有指向仓库根的层级错位 | ✅ 共 13 处修正 |
- **豁免名单**（`scripts/verify_allowlist.txt`，均为示例占位符/通配写法/未落地规划项，非漂移）：
  `Checkbox`、`Buttons`（`tooltiptext="ui::..."` 双语文案占位符）、`roboto_mono_12`（自定义字体 ID 示例）、
  `border_dash_style`（属性名而非色名）、`color_aquatic`/`color_desert`/`color_dusk`/`color_night_sky`/`color_xxx`/`color_mybrand`/`color_blue_deep`（自定义主题举例）、
  `bg_window_dark`/`bg_window_light`/`color_splitline_`（通配写法）、`color_progress`（实际为 `bg_progress`）、
  `bg_selected_inactive`/`bg_text_highlight`（ThemeColor §7.2「建议补充」章节，尚未落地）、
  `btn_global_*_80x30_normal`（示例中自定义的 Class 名）、
  `kEventThemeChanged`/`kEventLanguageChanged`（E 组待实现项）。
  名单按 `<glob> :: 名字1 名字2` 分文件登记，**不写入 markdown 正文**（HTML 注释会破坏文档结构）。
- **验收结果**（2026-10-17）：
  | 检查项 | 修前 | 修后 |
  | :--- | :--- | :--- |
  | `verify_docs.py --repo . --extra docs` | 52 处疑似漂移，退出码 1 | **全部命中，退出码 0** |
  | 全仓 markdown 相对链接可达性 | 失效 46 条 | **1162 条全部可达，失效 0** |
  | 改动文件换行符 | — | 23 个文件全部纯 CRLF，无 LF-only 混入 |
- **工作量**：1 人天（实际 1 人天）　**依赖**：无

> **执行提示**：`verify_docs.py` 默认只扫 skill 自带 13 份文档（绿灯是假绿灯），必须加 `--extra docs` 才扫得到 70 份。
> A-5 已完成，下一步把它接进 CI（见 B-2），否则后续文档漂移仍会复发。

---

## 三、P1 组 — 立网：测试、CI、打包

### B-1 引入单元测试框架 🟢 最高优先级

- **现状**：全仓零测试目录、零测试框架、零覆盖率工具。24 个示例程序是唯一回归手段。而 `ListCtrl.cpp`（176KB，单元格选择 + 就地编辑状态机）、`Calendar.cpp`（1400 行，三级导航）、`RichEdit2.cpp`（160KB，纯自绘）三大状态机**零覆盖**，且都在近期做过大改。
- **`verify_screenshots/` 是空目录**（0 文件、未纳入 git、未在 `.gitignore`）——它不是视觉回归测试，是一个会误导新开发者的空壳。
- **动作**（分三步）：
  1. 引入 GoogleTest + CTest，先只测**纯函数/纯逻辑模块**（`Ui*` 类型运算、`StringUtil`、`FilePathUtil`、`ColorConverter`、`AttributeUtil`、`EasingFunctions`）——这些无窗口依赖，成本最低；
  2. 为 `ListCtrl` 数据层、`Calendar` 日期换算、`RichEdit2` 文本排版补逻辑单测；
  3. 借助 Skia CPU 后端支持离屏渲染的特性，做"加载 XML → 离屏渲染 → 与基线 PNG 比对"的截图回归（`Project-Audit.md` C2 已提出，此处落到具体阶段）。
- **验收**：`ctest` 可运行；首批 ≥200 个用例覆盖上述纯逻辑模块；三块大状态机各有 ≥20 个用例；`verify_screenshots/` 要么变成真实回归目录，要么删除。
- **工作量**：3–4 周（框架搭建 3 天 + 首批用例 2 周 + CI 集成 3 天）
- **依赖**：M0 全部完成

### B-2 CI 流水线（四平台构建矩阵）🟢

- **现状**：`.github/` 下只有 `ISSUE_TEMPLATE/` 与 `pull_request_template.md`，**无 `workflows/` 目录**；gitlab / travis / jenkins / azure / appveyor / drone 全部不存在。
- **动作**：
  1. Windows（MSVC）、Linux（GCC + Clang）、macOS（Clang）三平台构建矩阵；
  2. FreeBSD 用 `cross` 镜像或定期任务，先只做 configure + 编译 smoke；
  3. 接入 B-1 的 `ctest`；
  4. 接入 `verify_docs.py --extra docs` 做文档回归。
- **验收**：任一平台编译失败即阻断 PR；文档漂移能在 CI 里被捕获。
- **工作量**：1–1.5 周　**依赖**：B-1（可并行启动，测试接入部分后置）

### B-3 补齐发布与打包流程 🟡

- **现状**：零 `CPack`、零 `install()` 规则、零 NSIS/WiX/DEB/DMG 配置。`build/README.md`(510 行) 只讲如何编译，**没有任何一节讲如何打包与发版**。
- **动作**：
  1. 补 `install(TARGETS / FILES / DIRECTORY)` 规则，资源目录（`bin/resources/**`）必须完整安装——注意 `themes/color_light/` 与 `themes/color_dark/` 两套都要带上，缺一套深浅色切换会失效；
  2. CPack：Windows（NSIS/ZIP）、macOS（DragNDrop）、Linux（TGZ/deb）、FreeBSD（TGZ）；
  3. 编写 `docs/Release.md`，覆盖四平台的产物、版本号、签名与公证流程。
- **验收**：四平台各能一条命令产出可分发包；`docs/Release.md` 有可执行的命令序列。
- **工作量**：4–6 天　**依赖**：无（可与 B 组并行）

### B-4 分级日志 + 文件输出 + 崩溃 dump 🟡

- **现状**：`duilib/Utils/LogUtil.h` 只有 `Output` / `OutputLine` 两个方法，**无级别枚举、无文件输出**；Windows 走 `OutputDebugString`（`LogUtil.cpp:38`），**Release 版用户永远看不到**；`SetUnhandledExceptionFilter` / `MiniDumpWriteDump` / `backtrace` 全库 0 命中。
- **影响**：用户报"界面卡死/崩溃"时库侧零诊断信息。提交 `98d89248`（ColorPicker 启动即卡死）这类问题只能靠人工复现定位。
- **动作**：
  1. `LogUtil` 增加级别（Debug/Info/Warn/Error）+ 滚动文件输出；把 `cmake/duilib_common.cmake:26` 的 `DUILIB_LOG` 从编译期开关升级为**运行期级别**开关；
  2. Windows 用 `MiniDumpWriteDump`，Linux/macOS/FreeBSD 用 `backtrace()`；
  3. 全库 14 处 `catch (...)` 吞噬式异常处理**逐个补日志**（当前 16 个 catch 中 14 个是 `catch(...)` 且完全不记录）；
  4. 统一异常规范：库内不 `throw`（改返回值/错误码）。当前 `throw` 35 处 / `try` 16 处，至少 19 处 throw 会穿透到应用层。
- **验收**：Release 版出问题能从日志文件定位；崩溃有 dump。
- **工作量**：1–1.5 周　**依赖**：A-5（先把日志需求写清文档）

---

## 四、P2 组 — 跨平台一致性与质量

### C-1 `ASSERT` 跨平台行为不一致 🔴 架构级硬伤

- **现状**（`duilib/duilib_config.h:110-142`，已逐平台核实）：

  | 平台 | Debug | Release |
  | :--- | :--- | :--- |
  | Windows | `_ASSERTE` 激活 | 🔴 **仍然激活**（缺 `#ifdef _DEBUG` 包裹）→ 发行版用户会看到断言弹窗 |
  | Linux / macOS / FreeBSD | `assert` 激活 | `((void)0)` **完全消失** → 1970 处前置校验形同虚设 |

- **影响**：同一个跨平台库，Windows Release 会弹断言框、其他三平台 Release 完全没有校验。这不是风格问题，是**跨平台一致性的根本缺陷**，且影响 1970 处断言。
- **动作**（二选一，推荐 A）：
  - **方案 A（推荐）**：引入双宏区分语义——`DUILIB_ASSERT`（恒定生效的"不可违背前提"）+ `DUILIB_VERIFY`（Release 生效的"运行期校验"），然后全量审阅 1970 处断言分类；
  - **方案 B（低成本）**：Windows 侧加 `#ifdef _DEBUG` 包裹与其余三平台对齐，其余不动。缺点是 Windows Release 仍无校验。
- **验收**：四平台 Release 行为一致；`docs/Control.md` 增加一节说明两个宏的用法差异。
- **工作量**：方案 A 3–5 天 / 方案 B 0.5 天　**依赖**：B-4（先有日志，校验失败才有地方记录）

### C-2 IME 组合输入在非 Windows 平台显著退化 🔴

- **现状**：
  - `NativeWindow_SDL.cpp:2758-2773` 调 `SDL_SetTextInputArea(m_sdlWindow, nullptr, 0)` —— **候选框位置不跟随光标**（Windows 侧 `RichEdit2.cpp:3524-3534` 会用 `GetCaretPos` 算 `CFS_POINT`）；
  - `RichEdit2.cpp:3505-3543` 的 `OnImeStartComposition` / `OnImeEndComposition` 整段被 `#if WIN && !SDL` 包住，非 Windows 下 `m_bIsComposition` **恒为 false**，无法区分拼音/假名/组合态；
  - 未处理 `SDL_EVENT_TEXT_INPUT_EDITING`（只处理 `SDL_EVENT_TEXT_INPUT`）；
  - `RichEdit2.cpp:3434-3436` 从不调 `SetImeOpenStatus` 的面积设置。
- **影响**：这是**中文用户可感知的最严重退化项**——在 macOS/Linux/FreeBSD 上用中文输入法时候选框位置不对。
- **动作**：补 `SDL_EVENT_TEXT_INPUT_EDITING` 处理与组合态跟踪；`RichEdit` 光标变化时向 NativeWindow 传 `SDL_SetTextInputArea` 的实际矩形。
- **验收**：三平台中文输入候选框跟随光标；组合过程中不触发文本变更事件。
- **工作量**：3–5 人天　**依赖**：M1（有回归能力后再动输入链路）

### C-3 主题 / 语言切换无变更广播 🟡

- **现状**：`ColorManager::SetColorThemeDarkMode`（`ColorManager.cpp:93-96`）**只设一个 flag，不触发任何失效重绘**，调用方必须自己 `Invalidate()` 全窗口树。语言切换同理：`LangManager` 换表后应用层要自行遍历所有控件重新应用文本，框架无通知事件。
- **影响**：切主题/语言后出现"部分控件不刷新"的 bug，且每新增一个控件都要在应用层再写一遍刷新代码，极易漏。
- **动作**：引入 `kEventThemeChanged` / `kEventLanguageChanged`，由 `Window` 统一广播并自动 `Invalidate` 全树。
- **验收**：切主题/语言后无需应用层写任何刷新代码，全部控件同步更新。
- **工作量**：1 周　**依赖**：B-1

### C-4 Linux/FreeBSD 无系统阴影 🟡

- **现状**：`NativeWindow_SDL.cpp:3331-3332` 对 Linux/FreeBSD 返回 `false` → 无系统阴影，降级为自绘 SVG（`Shadow.cpp:584-601` 自动降级逻辑已实现，视觉略有差异）。
- **文档准确性**：`docs/WindowShadow.md:99-103` 的表格与代码一致，**没有夸大**，无需改文档。
- **动作**：评估接入 libdecor（X11/Wayland 社区方案），或明确接受自绘降级并在 README 标注。
- **验收**：Linux 下阴影视觉可接受，或文档明确说明降级。
- **工作量**：5–8 人天（libdecor 集成风险高）　**依赖**：无　**优先级建议**：先做评估，不一定做

### C-5 非 Windows 无系统气泡通知 🟡

- **现状**：`TrayIcon_SDL.cpp:248-256` 的 `ShowBalloon` 恒 `return false`（注释说明 SDL3 托盘 API 不支持）；`Hide()` 也被标注为"只能销毁后重建"。
- **动作**：macOS 走 `UNUserNotificationCenter`，Linux/FreeBSD 走 D-Bus `org.freedesktop.Notifications`。
- **验收**：三平台托盘气泡可弹出。
- **工作量**：2 人天　**依赖**：无

### C-6 文件对话框参数部分失效 🟢

- **现状**：`FileDialog_SDL.cpp` 的 `nFileTypeIndex` / `defaultExt` / `defaultLocation` 三个参数在签名中被注释掉（即**未生效**）；`:333` 有代码内 TODO 标注 SDL3 多选返回值有 bug。
- **动作**：补齐三个参数；多选 bug 等 SDL3 上游修复或自己做校正。
- **验收**：打开对话框时初始目录与默认扩展名正确。
- **工作量**：1–2 人天　**依赖**：A-3（SDL 版本锁定）

### C-7 `DUILIB_SRC_MACOS` 平台判定脆弱 🟢

- **现状**：`duilib/CMakeLists.txt:45, 102` 用自设变量 `DUILIB_SRC_MACOS` 而非 `CMAKE_SYSTEM_NAME STREQUAL "Darwin"` 判断；若将来新增平台（如 OpenBSD），会误删 macOS 的 `.mm` 文件。另外 `:106` 的 `list(REMOVE_ITEM CEF_CONTROL_SRC ...)` 在 FreeBSD 下变量未定义，**静默无操作无报错**。
- **动作**：改用 CMake 原生判断；对未定义变量加保护。
- **验收**：构建日志中不再有静默跳过。
- **工作量**：0.5 人天　**依赖**：无

---

## 五、P2 组 — 控件补齐

> 已核实**确实缺失**（类名与 `DUI_CTR_*` 宏均全仓无命中）的控件，按投入产出排序。

### E 组：低垂果实（合计 < 1500 行，建议优先）

| ID | 项 | 现状依据 | 动作 | 工作量 |
| :--- | :--- | :--- | :--- | :--- |
| **E-1** | **StatusBar 分格索引 API** | `StatusBar.h` 无 `SetStatusText(格,文本)` / `GetStatusText`，只能业务层 `FindControl` 找子 Label；且 size grip 头文件自认"仅作视觉提示，不改变窗口可调整大小行为"= **装饰性死控件** | 补 4–6 个方法；明确 size grip 是否要接真实拖拽 | 极低 |
| **E-2** | **NavigationView 属性文档** | `docs/Control.md` 中 `NavigationView` 出现 0 次——**唯一已实现但主文档无章节的控件** | 补中英双章节 + `Summary` / `XmlNode` / `Events` 四处 | 极低（纯文档） |
| **E-3** | **通用 Dialog 控件** | 只有 `MessageBoxWnd` 五种固定按钮，无法放任意内容。**地基已齐**（`Mask` 遮罩 + `Flyout` 浮层），只差组装 | 新增 `Dialog` 基类，可放任意内容 + 自定义底部按钮区 + `DoModal` | 低（约 600 行） |
| **E-4** | **Pagination 分页器** | 全仓无命中。报表/表格类软件刚需，当前全靠业务层手写按钮组 | HBox + 若干 Button 组合 | 低（约 250 行） |
| **E-5** | **ScrollBar 键盘导航** | `ScrollBar` 无 PageUp/PageDown/方向键/Home/End 处理（对比 `Slider.h:215 HandleKeyDown` 有完整键盘支持）；`ScrollBar.cpp:50` 还有 TODO"DPI 自适应参数，需要确认一下" | 补键盘处理 + 修 DPI TODO | 低 |

### D 组：中优先级

| ID | 项 | 现状依据 | 工作量 |
| :--- | :--- | :--- | :--- |
| **D-1** | **RangeSlider 双滑块** | `Slider.h` 仅单值 `SetValue`；`Project-Audit.md` B3 已登记待办。底子好（有刻度、标签、click-to-jump、键盘支持） | 低（约 400 行改动） |
| **D-2** | **富内容 Tooltip** | `ToolTip.h` 仅 `ShowToolTip` 单方法**纯文本**，Windows 侧还是系统原生实现；`Flyout` 地基已就绪未封装（`Project-Audit.md` B8） | 低（约 200 行） |
| **D-3** | **SpinBox 支持小数与格式化** | `SpinBox.h:47,51,57` 全为 `int64_t`，不支持小数/千分位/前后缀（对比 Qt `QDoubleSpinBox`） | 低 |
| **D-4** | **TreeView 节点拖拽重排** | `TreeView.cpp` 中 `drag`/`drop` **零命中**。树控件头号缺失（文件树/权限树/组织树通用需求）。另 `TreeView` 无排序（`sort` 零命中）、继承 `ListBox` 非 `VirtualListBox` 无虚拟化 | 中（约 700 行） |
| **D-5** | **TimePicker 时间选择弹层** | `DateTime.h:117` 自认 `kDateCalendar`「不支持选择时间」，时间只能靠 spin 按钮逐格调。日程/工单/预约场景刚需。另无 min/max 日期约束、无禁用日期、无周起始日配置 | 中（约 800 行） |
| **D-6** | **Stepper / Steps 向导步骤条** | `Timeline` 语义不符（时间轴 ≠ 流程状态机）；`Project-Audit.md` B4 登记待办 | 低-中（约 400 行） |
| **D-7** | **ToolBar 工具项抽象** | `ToolBar` 只是"会收纳的 HBox"，**无项管理 API**（无 `AddTool`/`SetToolChecked`），checkable 工具项只能塞 CheckBox，分隔符靠 `Line` 手工拼 | 中（约 500 行） |
| **D-8** | **ColorPicker 补 Alpha 通道 + HSV/Hex 数值输入** | 现有 4 种形态 + `ColorSlider` 已很全，但**无透明度选择**（WinUI/Qt 均有），无数值输入面板。`ScreenCapture` 已存在可做吸管 | 中（约 400 行） |
| **D-9** | **CascadingSelect 级联选择器** | 无"父变更自动重置/过滤子项"语义；`Combo` 的 Tree 形态只能手工模拟两级 | 中（约 600 行） |
| **D-10** | **WrapPanel 不等宽换行布局** | 有等宽 `HTileBox`/`VTileBox`，缺按内容宽度换行。`TagInput` 内部已自行实现（需求已被验证） | 中（约 400 行） |
| **D-11** | **NavigationView / Breadcrumb / Rating / Carousel** | Breadcrumb 全仓无命中（`AddressBar` 是浏览器地址栏，语义不同）；Rating 全仓无命中（B5 待办）；Carousel 无命中（B7 待办） | 各低 |
| **D-12** | **原生系统通知** | `Control-Roadmap.md` 第 19 项；Toast 仅应用内自绘。macOS 通知中心 / Windows 桌面通知 / Linux D-Bus | 中高 |

### 现有控件的补强点（不新增控件，只补 API）

| ID | 控件 | 缺口 | 备注 |
| :--- | :--- | :--- | :--- |
| **E-6** | `ListCtrl` | ① 无列汇总行 / 分组行（只有 `nSortGroup` 排序分组）；② 排序用 `std::sort` 非 `stable_sort`，同值行会乱序，同组文件夹/文件可能交错；③ `ListCtrl.h:741` 自认单元格合并"尚未实现"，归并逻辑已写好但缺数据入口 | 后两项成本低、收益明确 |
| **E-7** | `TabCtrl` | **无 `Add`/`Insert`/`Remove` 的程序化管理 API** —— 一个标签栏控件不能增删标签，必须绕道操作 `TabBox`。另无垂直标签、无固定标签、无溢出滚动 | 最大 API 缺陷 |
| **E-8** | `Menu` / `MenuItem` | `MenuItem` 几乎没有自己的 API：**无 `SetCheck`/`SetEnabled`/`SetIcon`/`SetShortcutText`/`SetRadio`**，勾选/禁用/图标/快捷键在专业菜单里是标配。`Menu.cpp:11` TODO"静态对象集中管理"；`Menu.cpp:147` 遗留 2020 年代码注释 | 成本低收益高 |
| **E-9** | `DateTime` | `DateTime.h:35-46` 两处专门为"被复用为 ListCtrl 编辑器"打的补丁（`ResetCalendarFlyoutState`/`CloseCalendarFlyout`）+ 成员 `m_bSuppressCalendarReopen` —— **抽象缺失导致实现细节外泄**；`DateTimeWnd` 与 `kDateCalendar` 两套编辑路径并存 | 架构债 |
| **E-10** | `SearchBox` | 仅 150 行；**无防抖**（每次按键都触发 `kEventTextChanged`，搜索场景必须业务层自己加定时器）；无历史/建议（那是 `AutoComplete`，两个控件未整合）；`SearchBox.cpp:22` 自认"这些属性需要转发给内部编辑框；OnInit 之前子控件尚未创建，先缓存"= 初始化顺序债 | 中低 |
| **E-11** | `Chart` | ① **只有全量 `SetSeries`，无增量更新 API** → 实时监控场景基本不可用；② 无导出 PNG/SVG；③ 无坐标轴格式化回调（对比 `Slider` 有 `TickLabelCallback`）；④ `Chart.cpp` 93KB 单文件 | 中（约 600 行） |

---

## 六、P3 组 — 架构改造（需 M1 安全网就位后再动）

### F-1 ListCtrl DataTemplate 机制（决定 ListCtrl 天花板）

- **现状**：列的显示形态由 `ListCtrlSubItemData` 的**硬编码布尔位**决定（`nProgress` / `bRichText` / `bImageId` / `bShowCheckBox`）。要加"第 5 种形态"必须改 `ListCtrlDefs.h` + `ListCtrlData.h/.cpp` + `ListCtrlReportView.cpp` + `ListCtrlSubItem.cpp` **四个文件**，无法通过子类或回调扩展。
- **影响**：这是一个**架构级缺口**——同类需求（TreeView 节点模板、卡片内嵌自定义内容、报表列扩展）都会撞到同一堵墙。
- **动作**：设计声明式模板 / Selector 机制，让列的呈现形态可由 XML 或回调定义。
- **工作量**：高（架构级重构）
- **依赖**：**必须先有 B-1 测试安全网**；完成后 F-1 才能解锁 TreeView 节点模板、Sparkline 内嵌等下游工作

### F-2 无障碍基础（Roadmap 第 18 项，企业/政务合规刚需）

- **现状**：全库 `accessib` / `a11y` / `aria` / `UIA` / `screen reader` / `role=` / `tabindex` **全部零命中**。仅有最基础的 `SetTabStop`（`Control.h:716`）。无焦点遍历顺序管理、无屏幕阅读器播报、无高对比度模式。
- **分阶段**：
  1. **基础属性**：XML 层加 `accessibility` 属性（`role` / `label` / `description`）+ 焦点遍历顺序管理 + 对骨架屏等装饰性元素标记为对辅助技术隐藏；
  2. **Windows UI Automation 适配**；
  3. macOS AX / Linux AT-SPI。
- **工作量**：基础 1–2 周 / 平台适配 4–6 周
- **依赖**：阶段 2 依赖阶段 1；建议 F-1 之后启动

### F-3 RTL 布局支持（需先决策）

- **现状**：`RTL` / `bidi` / `LayoutDirection` / `mirrored` 全库**零命中**，`duilib/Layout/` 下 13 种布局引擎全部硬编码 `left`/`right`/`top`/`bottom` 物理方向。希伯来语/阿拉伯语界面无法正确呈现。
- **决策点**：这是一个"要么做要么明确不做"的决策。做的话需要给 `UiMargin`/`UiRect`/`UiPoint` 加 `swapLeftRight()`，并在 Layout 层统一镜像 —— 涉及 13 种布局引擎全部适配。
- **工作量**：2–3 周　**依赖**：**先决策**，建议 M4 之前给出结论

### F-4 RichEdit 跨平台能力对齐

- **现状**：`RichEdit`（Windows 原生 EM_RECT）vs `RichEdit2`（纯自绘跨平台）双实现并存。**同一份 XML 在 Windows 有富文本、在 Linux/macOS 退化为纯文本**。`RichEdit2` 无表格、无项目符号/编号列表、无缩进、无行距段距、无对齐、无上下标、无字符样式、无拼写检查、无 HTML/Markdown 导入导出（`RichText.h` 是**只读**渲染器，不能反向序列化）。`RichEdit_Windows.cpp` 有 6 处"不支持文件拖放操作"。
- **影响**：这是"同代码不同表现"的一致性底线问题。
- **工作量**：高　**依赖**：C-2（IME 先修）+ B-1

---

## 七、P3 组 — 工程化与生态（M4）

### G 组：打包与发布

见 **B-3**（已列入 P1 组，因其是"库可交付性"的底线）。

### H 组：其它工程化欠债

| ID | 项 | 现状 | 工作量 |
| :--- | :--- | :--- | :--- |
| **H-1** | `.clang-format` + `.clang-tidy` 落地 | 两者均不存在，仅有 `.editorconfig`(13 行) 与 `duilib.ruleset`。命名双轨制（1479 个 `m_` 前缀 vs 27 个无前缀）、属性别名混乱（`text_id`/`textid`/`tooltiptextid` 三种写法并存于 `Control.cpp:453`）无人拦截 | 3–5 天 |
| **H-2** | 图片解压炸弹防护 | 全库无 `maxWidth`/`maxHeight`/像素总量校验，PNG/JPEG/APNG/WEBP/GIF/SVG/ICO 全走无上限路径。类比 `ZipManager.cpp:132` 直接 `resize(uncompressed_size)` 亦无上限 | 3–5 天 |
| **H-3** | `docs/misc` 过时文档归档 | `跨平台界面库基础组件设计.md` 仍描述 GTK/Cairo 技术栈（项目实际是 SDL3+Skia，**误导性最强**）；`ListCtrl修改.md` / `ListCtrl修改2.md` 记录的工作已全部落地；`nim_duilib 代码深度分析.md` 数据过期（83→85 头文件）。建议移入 `docs/misc/archive/` 并加过时标注 | 1–2 天 |
| **H-4** | AI Skill 目录收敛 | `.trae/` 与 `.workbuddy/` 各维护一套同用途 reference，**9 份中 5 份已分叉**（`api-reference` 1398 vs 1347 行、`pitfalls` 344 vs 290 行），同一天修改却内容不同 = 无单一权威源。AI 开发时会得到相互矛盾的技术指导 | 2–3 天 |
| **H-5** | 公共 PCH | 无 `pch.h`/`stdafx.h`；`duilib.h` 聚合 110+ 头文件，600+ TU 全量重解析，Windows MSVC 编译时间偏长 | 1–2 天 |
| **H-6** | `ListCtrl-Performance.md` 行号漂移 | 该文档方法论最扎实，但行号已失效（`global.xml` L725→L726；`SkTextBox.cpp` CountLines L122→L172/604）。建议改为引用函数名 + grep 锚点 | 2–3 天 |
| **H-7** | README 双语同步标记 | `README.en.md:3` 有 `> Last synced: 2026-09-29`，中文版无对应标记 | 10 分钟 |
| **H-8** | 异常处理规范 | `throw` 35 处 / `try` 16 块，至少 19 处 throw 穿透所有 try；16 个 catch 中 **14 个是 `catch(...)` 吞噬式**且无日志。与 B-4 合并治理 | 5–8 天 |
| **H-9** | `LangManager` 多语言共存 | 仅单张扁平表（`LangManager.h:75`），一次只能激活一种语言；无语言列表枚举、无 fallback 链、无复数形式 | 1 周 |
| **H-10** | 性能测试体系 | `PerformanceUtil`/`PerformanceUtilFast`（`PerformanceUtil.h:25,39`）设计良好且已有 64 处埋点，但无 Google Benchmark、无性能回归阈值、除 ListCtrl 外无性能文档 | 1–1.5 周 |

---

## 八、明确不做的事项

记录下来避免反复讨论：

| 项 | 决定 | 理由 |
| :--- | :--- | :--- |
| Docking 可停靠布局 / MDI | **不做** | `Control-Roadmap.md` 第三节已明确"成本极高，不建议做" |
| TreeGrid / 内嵌树 / 分组折叠 / L3 列筛选 | **按需再评估** | `Project-Audit.md` B9 已明确 ListCtrl 规划中取消；若出现企业报表需求需重新评估 |
| Pivot 透视表 | **不做** | 属 BI 而非 UI 库职责 |
| FreeBSD + CEF | **不做** | Chromium 不官方支持 FreeBSD；`freebsd_build.sh:77` 已注释掉 `cef`/`CefBrowser`，`README.md:562` 已声明。**但 A-1/A-2 要把宏与源码的矛盾清干净** |
| Windows 独占工具（`InlineHook`/`ApiWrapper`/`DllManager`/`DiskUtils`/`libudis86`） | **不跨平台化** | 功能本身即 Windows 专属，非缺陷 |

---

## 九、跟踪表

> 勾选规则：完成并验证后把未勾选框改为已勾选（`[ ]` → `[x]`），并在「完成日期」填日期。**每完成一项请同步更新本文档顶部的「基线日期」。**

### P0 — 阻塞级（目标：3.5 人天）

- [ ] **A-1** FreeBSD CEF 宏矛盾　`duilib_config.h:74`　｜ 0.5d ｜ 完成日期：______
- [ ] **A-2** CEF 层 4 文件 FreeBSD 守卫　｜ 1d ｜ 依赖 A-1 ｜ 完成日期：______
- [ ] **A-3** SDL3 版本锁定　`build_duilib_all_in_one.sh:196`　｜ 0.5d ｜ 完成日期：______
- [ ] **A-4** Linux Skia fontconfig 开关 + 空字体告警　｜ 0.5d ｜ 完成日期：______
- [x] **A-5** 文档硬伤 8 类修正　｜ 1d ｜ 完成日期：2026-10-17

### P1 — 立网（目标：6–8 周）

- [ ] **B-1** 引入 GoogleTest + CTest（分三步）　｜ 3–4 周 ｜ 完成日期：______
  - [ ] B-1.1 纯逻辑模块单测（`Ui*` / `StringUtil` / `FilePathUtil` / `ColorConverter` / `AttributeUtil` / `EasingFunctions`）≥200 用例
  - [ ] B-1.2 三块大状态机逻辑单测（ListCtrl / Calendar / RichEdit2）各 ≥20 用例
  - [ ] B-1.3 离屏渲染截图回归（`verify_screenshots/` 二选一：做成真回归 或 删除）
- [ ] **B-2** CI 四平台构建矩阵 + 测试 + 文档校验　｜ 1–1.5 周 ｜ 完成日期：______
- [ ] **B-3** 发布与打包流程（install 规则 + CPack + `docs/Release.md`）　｜ 4–6 天 ｜ 完成日期：______
- [ ] **B-4** 分级日志 + 文件输出 + 崩溃 dump + 异常规范　｜ 1–1.5 周 ｜ 完成日期：______

### P2 — 一致性与控件（目标：25–35 人天）

- [ ] **C-1** `ASSERT` 跨平台行为统一（推荐方案 A 双宏）　｜ 3–5 天 ｜ 依赖 B-4 ｜ 完成日期：______
- [ ] **C-2** IME 组合输入非 Windows 平台补齐　｜ 3–5 天 ｜ 依赖 M1 ｜ 完成日期：______
- [ ] **C-3** 主题 / 语言切换变更广播　｜ 1 周 ｜ 依赖 B-1 ｜ 完成日期：______
- [ ] **C-4** Linux/FreeBSD 系统阴影（先评估是否做）　｜ 5–8 天 ｜ 完成日期：______
- [ ] **C-5** 非 Windows 托盘气泡通知　｜ 2 天 ｜ 完成日期：______
- [ ] **C-6** 文件对话框参数补齐　｜ 1–2 天 ｜ 依赖 A-3 ｜ 完成日期：______
- [ ] **C-7** CMake 平台判定健壮化　｜ 0.5 天 ｜ 完成日期：______
- [ ] **E-1** StatusBar 分格索引 API　｜ 极低 ｜ 完成日期：______
- [ ] **E-2** NavigationView 属性文档（中英 4 处）　｜ 极低 ｜ 完成日期：______
- [ ] **E-3** 通用 Dialog 控件　｜ 约 600 行 ｜ 完成日期：______
- [ ] **E-4** Pagination 分页器　｜ 约 250 行 ｜ 完成日期：______
- [ ] **E-5** ScrollBar 键盘导航 + 修 DPI TODO　｜ 低 ｜ 完成日期：______
- [ ] **E-6** ListCtrl 排序稳定性 + 汇总行 + 单元格合并入口　｜ 低 ｜ 完成日期：______
- [ ] **E-7** TabCtrl 增删标签 API　｜ 低 ｜ 完成日期：______
- [ ] **E-8** MenuItem 勾选/禁用/图标/快捷键 API　｜ 低 ｜ 完成日期：______
- [ ] **D-1** RangeSlider 双滑块　｜ 约 400 行 ｜ 完成日期：______
- [ ] **D-2** 富内容 Tooltip　｜ 约 200 行 ｜ 完成日期：______
- [ ] **D-3** SpinBox 小数与格式化　｜ 低 ｜ 完成日期：______
- [ ] **D-4** TreeView 节点拖拽重排　｜ 约 700 行 ｜ 完成日期：______
- [ ] **D-5** TimePicker 弹层 + DateTime 日期约束　｜ 约 800 行 ｜ 完成日期：______
- [ ] **D-6** Stepper 向导步骤条　｜ 约 400 行 ｜ 完成日期：______
- [ ] **D-7** ToolBar 工具项抽象　｜ 约 500 行 ｜ 完成日期：______
- [ ] **D-8** ColorPicker Alpha 通道 + 数值输入　｜ 约 400 行 ｜ 完成日期：______
- [ ] **D-9** CascadingSelect 级联选择器　｜ 约 600 行 ｜ 完成日期：______
- [ ] **D-10** WrapPanel 不等宽换行布局　｜ 约 400 行 ｜ 完成日期：______
- [ ] **D-11** Breadcrumb / Rating / Carousel　｜ 各低 ｜ 完成日期：______
- [ ] **D-12** 原生系统通知三平台通道　｜ 中高 ｜ 完成日期：______
- [ ] **E-9** DateTime 编辑器抽象泄漏修正　｜ 中 ｜ 完成日期：______
- [ ] **E-10** SearchBox 防抖 + 与 AutoComplete 整合　｜ 中低 ｜ 完成日期：______
- [ ] **E-11** Chart 增量数据 API + 导出 + 轴格式化　｜ 约 600 行 ｜ 完成日期：______

### P3 — 架构与生态（目标：60–90 人天）

- [ ] **F-1** ListCtrl DataTemplate 机制　｜ 高 ｜ 依赖 B-1 ｜ 完成日期：______
- [ ] **F-2** 无障碍基础（阶段 1 属性标注 + 焦点管理）　｜ 1–2 周 ｜ 完成日期：______
  - [ ] F-2.1 `accessibility` 属性（role/label/description）+ 焦点遍历顺序
  - [ ] F-2.2 Windows UI Automation 适配
  - [ ] F-2.3 macOS AX / Linux AT-SPI
- [ ] **F-3** RTL 布局（**先做决策**）　｜ 2–3 周 ｜ 完成日期：______
- [ ] **F-4** RichEdit 跨平台能力对齐　｜ 高 ｜ 依赖 C-2 + B-1 ｜ 完成日期：______
- [ ] **H-1** `.clang-format` + `.clang-tidy`　｜ 3–5 天 ｜ 完成日期：______
- [ ] **H-2** 图片/解压炸弹防护　｜ 3–5 天 ｜ 完成日期：______
- [ ] **H-3** `docs/misc` 过时文档归档　｜ 1–2 天 ｜ 完成日期：______
- [ ] **H-4** AI Skill 目录单一化　｜ 2–3 天 ｜ 完成日期：______
- [ ] **H-5** 公共 PCH　｜ 1–2 天 ｜ 完成日期：______
- [ ] **H-6** `ListCtrl-Performance.md` 行号改函数名锚点　｜ 2–3 天 ｜ 完成日期：______
- [ ] **H-7** README 双语同步标记　｜ 10 分钟 ｜ 完成日期：______
- [ ] **H-8** 异常处理规范落地　｜ 5–8 天 ｜ 与 B-4 合并 ｜ 完成日期：______
- [ ] **H-9** `LangManager` 多语言共存　｜ 1 周 ｜ 完成日期：______
- [ ] **H-10** 性能测试体系（Google Benchmark）　｜ 1–1.5 周 ｜ 完成日期：______

---

## 附录 A：审计方法与可信度说明

本规划的每条结论均基于实际命令输出（`grep` / `find` / `sed` / `python` 交叉比对），未依赖推测。关键结论的核实方式：

| 结论 | 核实方式 |
| :--- | :--- |
| `ASSERT` 平台不一致 | 直接读 `duilib/duilib_config.h:105-145`，逐平台确认宏定义 |
| 零测试 / 零 CI | `ls -d .github/workflows`、`find -iname "*test*"`（排除 third_party）、全仓 grep 测试框架名 |
| `verify_screenshots/` 是空目录 | `find verify_screenshots -type f \| wc -l` → 0；`git ls-files` 无跟踪 |
| 控件缺失 | 逐个对 `duilib/Control/` 全目录 grep 类名 + `duilib_defs.h` 的 `DUI_CTR_*` 宏，双重确认 |
| FreeBSD CEF 矛盾 | 读 `duilib_config.h:74` + `duilib/CMakeLists.txt:68-73,92-99` + `duilib/duilib.cpp:4-7` 三处交叉 |
| TreeView 无拖拽 | `grep -ci "drag\|drop" TreeView.cpp` → 0 |
| SpinBox 仅 int64 | 读 `SpinBox.h:47,51,57` |
| 无障碍全缺失 | `grep -rniE "accessib\|a11y\|aria"` 全仓，唯一命中是 CEF 子控件自带 |
| 文档属性/事件名一致 | 建 24728 个源码标识符索引，与 `Control.md` 的 493 个属性名、`Events.md` 的 110 个事件名做双向 `comm` 比对 |

**已知需要复核的结论**：
- 各平台（非 Windows）的实测运行结论目前**无记录**，文档层面的能力矩阵来自源码分析。首次接 CI 时应把三平台实际编译/冒烟结果回写本文档。
- `docs/misc/ListCtrl单元格交互与结构升级设计方案.md`(82KB) 与 `ListCtrl-Roadmap.md`(37KB) 之间的范围重叠度未核查，两份若有冲突需以 `ListCtrl-Roadmap.md` 为准。

## 附录 B：单个新控件的落地成本（实测）

供估算参考。以第 13–17 项实测为准，最小实现仅 114–349 行：

| # | 必改文件 | 说明 |
| :--- | :--- | :--- |
| 1 | `duilib/Control/<Name>.h` + `.cpp` | 覆写 `GetType` / `SetAttribute` / `Paint*` / 交互虚函数 |
| 2 | `duilib/duilib_defs.h` | 加 `#define DUI_CTR_<NAME> (_T("<Name>"))` |
| 3 | `duilib/Core/WindowBuilder.cpp` | 在 `createControlMap` 加一行 lambda —— **只靠 `AddCreateControlCallback` 不够**（回调在控件自身构造里才注册，首个控件永远创建不了） |
| 4 | `duilib/duilib.h` | 加 `#include` |
| 5 | `duilib/CMakeLists.txt` **+** `duilib/duilib.vcxproj[.filters]` | 双构建系统都要改 |
| 6 | `themes/default/global.xml` **+** `color_light/global.xml` **+** `color_dark/global.xml` | 三处资源手工同步 |
| 7 | `docs/Control.md` / `Summary.md` / `XmlNode.md` / `Events.md` **+ 各自 `.en.md`** | **8 份文档**，最容易漏 |
| 8 | `examples/controls/<Name>Form.cpp/.h` + 示例 XML + 示例构建文件 | 示例 |

> **注**：`examples/*.cpp` 在全局命名空间，`UiRect` / `UiPointF` / `DrawStringParam` / `IRender` / `TEXT_*` 必须写全 `ui::` 前缀；`GetUiColor` / `GetIFontById` 是 Control 成员函数而非 `ui::` 自由函数。

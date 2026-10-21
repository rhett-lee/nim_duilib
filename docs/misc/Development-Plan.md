
# nim_duilib 开发执行规划（Development Plan）

> 本文档是**可跟踪执行**的开发规划，与其它规划类文档的分工见下表。
> 基线日期：**2026-10-20**（已完成 A-1、A-2、A-4、A-5、B-3、C-1、C-2、C-6、C-7；A-3 与 B-1~B-4 暂缓，C-4 明确不处理）
> 更新规则：每完成一项把 `- [ ]` 改为 `- [x]` 并填写「完成日期」；新增项追加到对应小节末尾，不要重排既有编号。

## 与其它文档的关系

| 文档 | 定位 | 与本文档的关系 |
| :--- | :--- | :--- |
| `docs/misc/Project-Audit.md` | 一次性审计报告（现状 + 建议） | **本文档的上游输入**。审计已提出但未进入本文档跟踪的条目，视为「已记录、不排期」 |
| `docs/misc/CMake-Improvement-Plan.md` | CMake 构建脚本改进计划（CM-1 ~ CM-22） | **本文档的下游专项计划**：全仓 CMake 脚本的缺陷与改进项由该文档逐项跟踪（不重复登记到本文档），完成情况回写该文档的跟踪清单 |
| `docs/misc/Control-Roadmap.md` | 控件规划清单 1–20 项 | 已完成的 1–17 项不再重复；其 18–20 项（无障碍 / 原生通知 / 应用内更新）并入本文档 D、E 组跟踪 |
| `docs/misc/跨平台开发相关事项.md` | 跨平台历史决策记录 | 已完成的条目标记为存量；本文档只跟踪**新增**跨平台工作 |
| `docs/misc/ListCtrl-Roadmap.md` / `ListCtrl-Performance.md` | ListCtrl 专项 | ListCtrl 内的单元格/性能议题仍由这两份文档负责，本文只跟踪其**结论对全局的影响**（见 B-4） |

若与上述文档冲突，以本文档的「执行顺序」为准，但**结论性描述**应同步回写原文档。

---

## 零、现状快照

| 维度 | 现状数据 | 判断 |
| :--- | :--- | :--- |
| 源码规模 | 626 个第一方 `.h/.cpp/.mm`（其中 `duilib/` 618 个），3550 次提交 | — |
| 控件数量 | `duilib/Control/` 171 个文件；Roadmap 1–17 项已全部落地，2026-10-17~19 又新增 8 个控件（StatusBar / ToolBar / RangeSlider / Rating / Carousel / Pagination / Stepper / TreeGrid） | 数量充足，**重心应从"补控件"转向"补地基"** |
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

### A-1 `DUILIB_BUILD_FOR_CEF` 在 FreeBSD 上被定义但源码已排除 ✅ 已完成（2026-10-20）

- **原审计结论**：`duilib/duilib_config.h:74` 对**所有非 Windows 平台**无条件定义
  `DUILIB_BUILD_FOR_CEF1`，但 `duilib/CMakeLists.txt:68-73, 92-99` 在 FreeBSD 上排除了 CEF 的
  include 路径与全部源码。此前只靠 `duilib/duilib.cpp:4-7` 的单点防护
  `#if !defined(__FreeBSD__)` 侥幸没炸——任何人新增一处
  `#include "duilib/CEFControl/CefManager.h"`（该头在 `DUILIB_BUILD_FOR_CEF` 下会
  `#include "include/cef_app.h"`）就会立即编译失败。
- **已完成的改动**（`duilib/duilib_config.h`，非 Windows 分支）：
  | 项 | 改动 |
  | :--- | :--- |
  | 原 | `#define DUILIB_BUILD_FOR_CEF    1`（对 Linux / macOS / FreeBSD 一律定义） |
  | 现 | 改为 `#if defined (DUILIB_BUILD_FOR_LINUX) \|\| defined (DUILIB_BUILD_FOR_MACOS)` 包裹，
  并补注释说明 Chromium 官方不提供 FreeBSD 构建 |
  **Windows 分支未动**（第 46/50 行两处定义由 `#ifndef DUILIB_CEF` 保护，语义不变） |
- **连带解决（超出原计划）**：原 A-2 描述的「FreeBSD 下编译为空 TU → 链接期 undefined symbol」，
  根因就是本宏被误定义。A-1 落地后 `duilib/CEFControl/` 下**全部 23 个源文件的外层
  `#ifdef DUILIB_BUILD_FOR_CEF` 同时失效**，整个 CEF 层在 FreeBSD 上自动塌缩为空 TU，
  不再产生任何符号。已写脚本逐文件核对守卫覆盖范围（23/23 完整包裹）。
- **验收**：读磁盘真实文本求值新增的 `#if` 块，四个平台结论
  Windows=不定义 / Linux=定义 / macOS=定义 / **FreeBSD=不定义**，与预期一致；
  `duilib_config.h` 预处理指令配平（22 / 22）；Windows Debug x64 编译 `EXITCODE=0`。
- **工作量**：0.5 人天　**依赖**：无

### A-2 CEF 层 4 个文件缺 FreeBSD 守卫 ✅ 已完成（2026-10-20，与 A-1 一并处理）

- **原审计结论**：`CefManager.cpp:13-19, 72-84` 的三平台分支无 FreeBSD 落点
  （`GetInstance()` 会落入 `#else ASSERT(0); return nullptr`）；`CefClientApp.cpp` 与
  `CefManager_Linux.cpp` 在 FreeBSD 下编译为空 TU → 链接期 undefined symbol。
  审计同时指出 `CefControlOffScreen.cpp:153` 与 `CefControlNative.cpp:101` 已写了
  `|| defined (DUILIB_BUILD_FOR_FREEBSD)`，说明 FreeBSD CEF 曾被规划但未完成。
- **处理原则变更**：任务下发时明确「**FreeBSD 平台不支持 CEF**」，因此**不做**「补齐 FreeBSD 分支」，
  改为清除「FreeBSD 曾被支持」的残留痕迹。下列 `|| defined (DUILIB_BUILD_FOR_FREEBSD)`
  正是该残留，与新原则冲突：
  | 文件 | 原判定 | 现判定 |
  | :--- | :--- | :--- |
  | `duilib/CEFControl/CefWindowUtils_Linux.cpp` | `(LINUX \|\| FREEBSD) && CEF` | `LINUX && CEF` |
  | `duilib/CEFControl/CefControlNative.cpp` | `LINUX \|\| FREEBSD` | `LINUX` |
  | `duilib/CEFControl/CefControlOffScreen.cpp` | `LINUX \|\| FREEBSD` | `LINUX` |
  | `duilib/CEFControl/CefManager.cpp` | 三分支 + 裸 `#else ASSERT(0)` | 补注释说明该分支不可达 |
  **范围严格限定在 `CEFControl/` 目录**——`duilib/Core` / `Utils` / `Control` 下另有 27 处
  `LINUX || FREEBSD` 判定，属 SDL / X11 / Wayland 平台适配，**与 CEF 无关，全部保留不动**。
- **为什么不需要给 `CefManager.cpp` 补 FreeBSD 实现**：`GetInstance()` 所在的整个文件已被
  外层 `#ifdef DUILIB_BUILD_FOR_CEF` 包裹，A-1 之后该文件在 FreeBSD 上根本不参与编译，
  `#else` 分支不可达。补实现等于凭空引入一个永不执行的分支。
- **验收**：`grep -rn DUILIB_BUILD_FOR_FREEBSD duilib/CEFControl/` 结果为空；
  非 CEF 层 27 处判定计数不变；5 个改动文件 CRLF 无混入；
  `duilib.vcxproj` Debug x64 编译 `EXITCODE=0`。
- **工作量**：1 人天　**依赖**：A-1

### A-3 SDL3 未锁定版本，全平台共同暴露面 ⏸ 暂缓（2026-10-20 决策）

- **原审计结论（仍然成立，保留备查）**：`build/build_duilib_all_in_one.sh:196` 直接
  `git clone https://github.com/libsdl-org/SDL.git` 拉主干，重复执行时走 `:198` 的 `git pull`，
  全程无 `git checkout <tag>`。而同文件对 Skia 做了 `git checkout <commit>` 锁定 ——
  SDL3 上游一次 API 变动会同时击穿 Windows / Linux / macOS / FreeBSD 四个平台。
- **决策：暂不处理**（2026-10-20，用户明确要求）。当前 SDL3 主干在四平台上均可正常编译，
  属于「已知风险但当前无实际损失」，不占用本轮修复名额。
- **重新评估的触发条件**（命中任一条即应重启本项）：
  | 触发条件 | 说明 |
  | :--- | :--- |
  | SDL3 上游发布 breaking change | SDL3 仍处于 3.0 正式版前的活跃迭代期，
API 变动频率高 |
  | 任一平台出现与 SDL 相关的编译失败 | 先确认是否为上游 commit 引入
  | 同一份代码在不同机器上编译行为不一致 | `git pull` 导致各机器 commit 不同步 |
  | 需要对外发布稳定版本 | 发布前必须锁定，否则无法复现构建 |
- **临时缓解手段**（无需改脚本）：若日后 SDL 拉取导致编译失败，
  在 `SDL/` 目录执行 `git log -1` 确认 commit 后回退到可用版本即可，
  例如 `git -C SDL checkout <上一个可用 commit>`。
- **工作量**：0.5 人天　**依赖**：无

### A-4 Skia 构建脚本 Linux 分支缺 `skia_enable_fontmgr_fontconfig=true` ✅ 已完成（2026-10-19）

- **原审计结论**（复核后**修正了风险等级**，但结论与动作不变）：
  `build/build_duilib_all_in_one.sh:431` FreeBSD 分支显式传入该开关，Linux 分支未传。字体枚举依赖 `duilib/RenderSkia/FontMgr_Skia.cpp:212` 的 `SkFontMgr_New_FontConfig`；一旦该后端未启用，Linux 会**静默返回空字体列表**（`FontManager.cpp` 无告警），表现为"所有文本都不显示"且无任何报错，极易误判为布局或主题色问题。
- **复核补充（对本地 `skia` 源码求值，审计未展开的一层）**：
  `gn/skia.gni:173` 中 `skia_enable_fontmgr_fontconfig = skia_use_freetype && skia_use_fontconfig`，而 `skia_use_fontconfig = is_linux`（`:58`）、`skia_use_freetype = is_android || is_linux || is_canvaskit`（`:61`）。**即当前上游默认值在 Linux 上恰好为 `true`，此前的 Linux 构建实际能正常枚举字体**——所以这不是"当前已坏"的缺陷，而是**隐式依赖上游默认值**的脆弱性（上游若把 `skia_use_fontconfig` 改为跟随发行版探测或收紧条件即会失效）。显式声明的价值在于把"碰巧对"变成"确定对"，并与 FreeBSD 分支保持一致的显式风格。
- **已完成的改动**：
  | 文件 | 改动 |
  | :--- | :--- |
  | `build/build_duilib_all_in_one.sh` | clang 路径在 FreeBSD 与 `else` 之间插入显式 `Linux` 分支；gcc 路径同样加 Linux 判断（该路径原先连 FreeBSD 都没判断）。两条 Linux 分支均传 `skia_enable_fontmgr_fontconfig=true skia_use_freetype=true`。**macOS 与 MSYS2 仍走原 `else`，不带该开关**（macOS 用 CoreText、Windows 用 DirectWrite，加了反而错） |
  | `duilib/Core/FontManager.cpp` | `GetFontNameList` 在列表为空时输出 `[WARN] FontManager::GetFontNameList: font list is empty...`，指向具体构建开关 |
  | `build/README.md` / `build/README.en.md` | 新增 Q6「Linux 下所有文本都不显示」，给出排查关键字 |
- **验收**：`bash -n` 语法检查通过；用桩函数替换 `gn` 实测四个平台（Linux / FreeBSD / Darwin / MINGW64）走 clang 与 gcc 两条路径，确认仅 Linux 与 FreeBSD 带上 fontconfig、macOS 与 MSYS2 未被污染；`duilib.vcxproj` Debug x64 编译通过。
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
- **豁免名单**（`scripts/verify_allowlist.txt`）：
  C4 后已把其中的**真实漂移公开修复**，不再用豁免掩盖：`border_dash_style` 属 XML 属性名，已纳入 `verify_docs.py` 的 `EXCLUDE_IDENTS`；
  `roboto_mono_12` 是文档示例内自行定义的字体 ID，已让脚本接受"同文档内定义"的字体引用；
  `tooltiptext` 里带 `ui::` 前缀的假类名占位符、以及 `Theme*.md` 中的虚构主题目录 / `Class` 名 / 色名，均已按源码实际名改写
  （如 `bg_window_main`、`bg_window_card`、`border_split_level*`、`bg_progress`、`color_blue_dark`）。
  `verify_docs.py` 的类名索引原先只收集 `class`/`struct`，遗漏 `enum`，已补上（`EasingFunctionType`、`ListCtrlSubItemSortFlag` 等真实枚举不再误报）。
  ThemeColor §7.2「建议补充的颜色」表亦按源码实况改写为「落地情况」：13 项中 10 项现已落地（部分按状态拆分为 `bg_scrollbar_normal` / `bg_scrollbar_thumb_hovered` 等），仅 2 项仍缺失。
  仍保留豁免的只有真正**尚未落地**的名字：`bg_selected_inactive`/`bg_text_highlight`（ThemeColor §7.3「仍缺失颜色」章节）、
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

### B-1 引入单元测试框架 ⏸ 暂缓（2026-10-20 决策）

- **原审计结论（仍然成立，保留备查）**：全仓零测试目录、零测试框架、零覆盖率工具，
  24 个示例程序是唯一回归手段；`verify_screenshots/` 是空目录（0 文件、未纳入 git），
  既不是视觉回归测试也没被 `.gitignore` 收编，属遗留物。原计划分三步引入 GoogleTest + CTest，
  工作量 3–4 周。
- **决策：暂不处理**（2026-10-20，用户明确要求）。核心理由：**引入后长期维护成本很高**——
  测试框架一旦落地就需持续跟进（用例随重构失效、新平台需补齐、CI 需维护），
  对当前以人工回归 + 24 个示例程序验证为主的开发节奏而言性价比不足。
- **重新评估的触发条件**（命中任一条即应重启本项）：
  | 触发条件 | 说明 |
  | :--- | :--- |
  | 出现过一次「改动 A 却在平台 B 出问题」且难以定位 | 测试安全网的核心价值场景已经发生 |
  | 开始实施 F-1 / F-4 等架构级重构 | 这些改动波及面大，人工回归不再可靠 |
  | 对外发布并需要长期维护承诺 | 回归手段将从"人工抽查"升级为硬要求 |
  | 团队规模扩大、多人并行开发 | 回归成本随人数线性上升，自测会失效 |
- **当前替代方案（零维护成本）**：沿用 24 个示例程序作为回归入口——
  `build/msvc_build.bat` 一次编译全部示例，任一编译失败即阻断，这是目前唯一的自动化守门；
  文档侧由 `verify_docs.py --repo . --extra docs` 兜底（本轮已用它扫 85 份文档零漂移）。
- **工作量**：3–4 周　**依赖**：M0 全部完成

### B-2 CI 流水线（四平台构建矩阵）⏸ 暂缓（2026-10-20 决策）

- **原审计结论（仍然成立，保留备查）**：`.github/` 下只有 `ISSUE_TEMPLATE/` 与
  `pull_request_template.md`，**无 `workflows/`**，即四平台编译失败无法在 PR 阶段被拦截。
  原计划建 Windows / Linux / macOS 三平台矩阵 + FreeBSD smoke，1–1.5 周。
- **决策：暂不处理**（2026-10-20，用户明确要求）。核心理由：**引入后长期维护成本很高**——
  CI 矩阵需要持续维护构建环境（工具链升级、系统镜像变更、四平台依赖差异），
  且仓库当前无 commit 频率压力，尚不足以摊平这份运维成本。
- **重新评估的触发条件**（命中任一条即应重启本项）：
  | 触发条件 | 说明 |
  | :--- | :--- |
  | 出现「本地能编过、某平台编不过」且反复发生 | CI 的核心价值场景已经发生 |
  | 贡献者增多，不再能靠单一维护者手工验证四平台 | 验证能力成为瓶颈 |
  | 开始对外发布稳定版| 四平台构建可复现性成为硬要求 |
  | 现有 24 个示例的编译耗时已明显影响日常迭代 | 有了缓存与并行收益，CI 才划算 |
- **当前替代方案（零维护成本）**：本地一次编译全部示例
  （`build/msvc_build.bat` 覆盖 zlib/libpng/libwebp → duilib → examples 全量），
  任何编译错误都会当场暴露；文档漂移由 `verify_docs.py` 本地执行覆盖。
- **风险与对冲**：无 CI 意味着**编译错误由人工发现**，漏测平台存在风险。
  本项目当前的个人开发节奏下可接受；若日后出现跨平台编译事故，优先补 B-2 的最小子集
  （仅 Windows + Linux 两平台 build smoke），而非一步到位做完整四平台矩阵。
- **工作量**：1–1.5 周　**依赖**：无（原依赖 B-1，可并行；B-1 暂缓后本项亦暂缓）

### B-3 补齐发布与打包流程 ✅ 已完成（2026-10-20，暂缓提交）

- **原审计结论**（复核后**方案整体更换**，问题描述仍然成立）：零 `CPack`、零 `install()` 规则、
  零 NSIS/WiX/DEB/DMG 配置；`build/README.md` 只讲如何编译，没有一节讲如何打包与发版。
- **复核：原方案的三步在本仓库都走不通**，原因不是遗漏而是架构不匹配：
  1. **补 `install()` 规则 —— 无处可补**。本仓库**没有根 `CMakeLists.txt`**，
     `duilib/`、`examples/*/`、`duilib/third_party/*/` 各自独立 `project()`，
     由 `msvc_build.bat`、`linux_build.sh` 等脚本逐个 configure + build。
     `install()` 只能挂在单个项目里，没有任何单一 CMake 项目能覆盖全部产物。
  2. **CPack —— 需要顶层项目才能挂载**，同上。更关键的是 Windows 有两条构建路径
     （`build/*.sln` 共 36 个 `.vcxproj` 的 MSBuild 路径、`msvc_build.bat` 的 CMake 路径），
     产物混在同一 `lib/`、`bin/` 下，CPack 产物对 MSBuild 用户是空的。
  3. `docs/Release.md` —— 唯一按原计划完成的一项。
  因此改为「**独立打包脚本**」路线：与构建体系解耦，按文件名白名单收集，
  两条构建路径的产物都能收，也不需要新增顶层工程。
- **详细工作记录**（成果清单、技术决策依据、踩过的坑、已知缺口与后续跟进）：
  [发布打包工作记录](Release-Packaging-Notes.md) / [Release Packaging Work Record](Release-Packaging-Notes.en.md)。
- **已完成的改动**：
  | 文件 | 改动 |
  | :--- | :--- |
  | `VERSION`（新增） | 版本号单一事实源，四段式 `1.0.2026.0416`。包名、包内 `VERSION`、
  `MANIFEST.txt` 全部由它派生 |
  | `build/pack.py`（新增） | 纯标准库、跨平台。不参与编译，只收集产物 |
  | `docs/Release.md` / `docs/Release.en.md`（新增） | 打包流程、产物结构、版本规则、签名与公证、四平台差异 |
  | `.gitignore` | 新增 `/dist` |
  | `docs/Summary.md` / `Summary.en.md` | 索引登记 `Release.md` |
  | `build/README.md` / `README.en.md` | 新增「发布打包 / Release packaging」章节 |
- **产物契约**（`pack.py` 强制）：
  | 项 | 规则 | 依据 |
  | :--- | :--- | :--- |
  | 头文件路径 | 必须是 `include/duilib/xxx.h`，**不可压平** | 下游写法是 `#include "duilib/duilib.h"`（`examples/basic/MainForm.h:5`），压平则下游全部编译失败 |
  | `RenderSkia/` | 不对外 | `duilib.h` 不引用它，是渲染后端实现细节 |
  | `third_party` 转发 | zlib / libpng / giflib / Microsoft.Web.WebView2 | 有 5 个对外头文件直接 include 了它们（`Core/ZipStreamIO.h`、`Image/APngDecoder.h`、`Image/FrameSequence_gif.h`、`WebView2/*.h`），不转发下游编译失败 |
  | zlib / libpng | 整目录转发头文件，不逐文件筛选 | 存在传递依赖（`pngpriv.h` → `pnglibconf.h`） |
  | Debug 包 | 由 `--config Debug` 选择，包名加 `-debug` | 配置只由目录表达（CM-5 D-2），Debug 库带 `/MTd`，只能给 Debug 下游 |
  | CEF 开发包 | 默认不打，需 `--with-cef`（可选 `--cef-version 109`） | wrapper 单个 80–100 MB，运行库仍需外置 |
  | `bin/resources.zip` | 一律排除 | 仅 748 字节的空壳 |
- **`--with-examples` 的资源范围差异**（本项最容易出错处）：
  - 关闭：仅保留 `resources/themes/default/public/` 与 `global.xml`；
    排除 22 个示例私有主题目录（`controls`、`render`、`chat`、`layout`、`tree_view` 等）。
    `global.xml` **必须保留** —— `GlobalManager.cpp:440` 在本地文件模式下找不到它
    会直接 `return false`，框架初始化失败。
  - 打开：整个 `resources/`，无排除项（实测 389/389 文件，与仓库一致）。
- **Debug 库判定踩过的坑**：Debug 来源有**两处**，只看文件名会漏。
  CMake 路径是 `duilib_d.lib`（`_d` 后缀），MSBuild 路径是 `lib/Debug/duilib.lib`
  （**无后缀**，只靠目录名区分，见 `msvc/PropertySheets/DuilibSettings.props`）。
  首版只认 `_d`，结果把 232MB 的 Debug 库当成 Release 打进了包，
  压缩后体积从 4.8MB 涨到 45.8MB。现按「后缀 + 目录名」双条件判定。
  > **P4（2026-10-21）已取代该判定**：库布局统一为目录区分配置，
  > `_d` 判定与 `--allow-debug-libs` 已删除，改用 `--config {Release,Debug}`；
  > 同时补入 Skia / WebView2 / JPEG / SDL / CEF 链接库。详见
  > [CM-5 P4 打包方案](CM-5-P4-Pack-Plan.md)。
- **验收实测**：
  - **下游编译验证**（本项的关键验收）：解压产物后，在包外写 `probe.cpp` 只加
    `/Iinclude`，`#include "duilib/duilib.h"` 编译 **EXITCODE=0**；
    再验 4 个依赖 third_party 的头文件（`ZipStreamIO.h` / `APngDecoder.h` /
    `FrameSequence_gif.h` / `WebView2Control.h`）同样 **EXITCODE=0**。
    这项证明「包外独立工程可编译」，比检查文件数量有效得多。
  - 资源排除精确性：关闭示例时包内 `themes/default/` 下只有 `global.xml` 与 `public/`；
    打开时 389 个文件与仓库完全一致。
  - `SHA256SUMS` 独立复算 587/587 全部一致；zip 与 tgz 两种格式包结构均正确。
- **未做的部分（有意为之）**：
  - **签名与公证未自动化**。本项目产出纯静态库（`add_library` 未指定 `SHARED`，
    `msvc/dllRes/dllmain.cpp` 只在 `.vcxproj` 中编译），SDK 包本身无需签名；
    文档只给手工流程。自动化签名等同B-2 的 CI —— 需要证书管理，是长期维护负担。
  - **只实测了 Windows**。脚本本身跨平台（纯标准库，四平台共用一份代码），
    但其它平台的库文件名后缀与静态库格式（`.a` vs `.lib`）差异
    可能需要按实际构建结果调整 `LIB_BASENAMES`，文档中已标注「未在本机验证」。
- **已知缺口**：`msvc/dllRes/version.h` 的 `DUILIB_VER_*` 宏与 `VERSION` 文件
  目前**两处独立维护**，且该宏只被 `.vcxproj` 使用、CMake 侧读不到。
  升级版本须同时改两处。后续应让 `version.h` 改为从 `VERSION` 派生。
- **原计划中已放弃的子项**：NSIS/WiX（安装程序）与 DEB/dmg（系统包）不做——
  静态库 SDK 用 zip / tgz 足够，制作安装程序对当前阶段是过度设计。
- **工作量**：4–6 天　**依赖**：无（可与 B 组并行）

### B-4 分级日志 + 文件输出 + 崩溃 dump ⏸ 暂缓（2026-10-20 决策）

- **原审计结论（仍然成立，保留备查）**：项目已有 `duilib/Utils/LogUtil.h` 的
  `LogUtil::OutputLine`（本轮 A-4 的字体告警即用它），但**无日志分级、无文件输出、
  无崩溃 dump**，排查线上问题只能靠复现。原计划 1–1.5 周。
- **决策：暂不处理**（2026-10-20，用户明确要求）。核心理由是**投入产出比不足**：
  完整的分级日志 + 文件轮转 + 跨平台崩溃 dump 是需要长期打磨的基础设施
  （日志体积与性能开销、用户隐私合规、dump 文件分发流程），
  而当前阶段的问题基本都能在开发期复现，不需要事后取证能力。
- **重新评估的触发条件**（命中任一条即应重启本项）：
  | 触发条件 | 说明 |
  | :--- | :--- |
  | 开始收到无法本地复现的用户侧问题报告 | 需要事后取证能力，这是本项的核心价值 |
  | C-1 采用方案 A（引入 `DUILIB_ASSERT` 恒定生效） | Release 下校验开始生效，失败需要落地位置 |
  | H-8 异常处理规范落地 | 二者原计划合并推进 |
- **对下游的影响（已化解，非阻塞）**：C-1 原本标注「依赖 B-4（先有日志，校验失败才有地方记录）」。
  但 C-1 自带**方案 B：仅给 Windows 侧加 `#ifdef _DEBUG` 包裹，0.5 天、不依赖日志**，
  可独立消除「Windows Release 弹断言框」这一最严重的症状。
  因此 C-1 改为「依赖：无（可选依赖 B-4）」——**B-4 暂缓不会卡死 C-1**。
- **工作量**：1–1.5 周　**依赖**：无

## 四、P2 组 — 跨平台一致性与质量

### C-1 `ASSERT` 跨平台行为不一致 ✅ 已完成（2026-10-20，采用方案 B）

- **现状**（`duilib/duilib_config.h:113-157`，已逐平台核实）：

  | 平台 | Debug | Release |
  | :--- | :--- | :--- |
  | Windows | `_ASSERTE` 激活 | ✅ **已修复**（2026-10-20 加 `#ifdef _DEBUG` 包裹）→ 与其余三平台一致 |
  | Linux / macOS / FreeBSD | `assert` 激活 | `((void)0)` **完全消失** → 2904 处前置校验形同虚设（保持现状） |

- **影响**：同一个跨平台库，Windows Release 会弹断言框、其他三平台 Release 完全没有校验。这不是风格问题，是**跨平台一致性的根本缺陷**，且影响 2904 处断言调用。
- **动作**（二选一，推荐 A）：
  - **方案 A（推荐，未采纳）**：引入双宏区分语义——`DUILIB_ASSERT`（恒定生效的"不可违背前提"）+ `DUILIB_VERIFY`（Release 生效的"运行期校验"），然后全量审阅 2904 处断言分类；
  - **方案 B（低成本，✅ 本次采纳）**：Windows 侧加 `#ifdef _DEBUG` 包裹与其余三平台对齐，其余不动。缺点是 Windows Release 仍无校验——这与其余三平台一致，属可接受的既有现状。
- **验收**：✅ 四平台 Release 行为一致（Debug 生效 / Release `((void)(0))`）；`docs/Control.md` / `.en.md` 已增加「`ASSERT` 的生效范围」一节。
- **工作量**：方案 A 3–5 天 / 方案 B 0.5 天　**依赖**：无（B-4 已于 2026-10-20 暂缓，方案 B 不需要日志，独立消除了「Windows Release 弹断言框」这一最严重症状）
- **实测**（2026-10-20）：
  - 判别器验证：`ASSERT(未声明标识符)` 在 Debug 下报 `C2065`（证明 `_ASSERTE` 确实引用表达式）、Release 下编译通过（证明 `((void)(0))` 完全不引用），两个方向都被证实；
  - 真实源码验证：断言最密集的 11 个 `.cpp`（`Window.cpp` / `Control.cpp` / `ListCtrl.cpp` / `RichEdit2.cpp` 等，合计约 900 处断言）在 Release 与 Debug 双配置下**全部编译通过，零错误零警告**，未出现「变量仅在断言中被使用」的新告警；
  - 静态排查：全仓 2904 处断言调用中，裸赋值 0 处、`++` / `--` 0 处、状态修改调用 0 处、出现在 `if` 条件中 0 处 —— 不存在依赖 Release 侧求值的写法。

### C-2 IME 组合输入在非 Windows 平台显著退化 ✅ 已完成（2026-10-20）

- **改前的现状**（已逐条核实）：
  - **组合态完全缺失**：`RichEdit2::OnImeStartComposition` / `OnImeEndComposition` 整段被 `#if WIN && !SDL` 包住，非 Windows 下 `m_bIsComposition` 恒为 `false`，无法区分拼音/假名/组合态（`RichEdit2.cpp:3505-3543`）；
  - **SDL 侧不派发 IME 事件**：主分发 `switch` 只有 `SDL_EVENT_TEXT_INPUT`（相当于 `WM_CHAR`），**没有处理 `SDL_EVENT_TEXT_EDITING`**，框架的 `Window::OnImeStartCompositionMsg` 等入口在 SDL 下从不被调用；
  - **候选框位置**：候选框定位其实已在 `RichEdit2::ShowCaret()` 里实现（`RichEdit2.cpp:2092-2124` 会算光标矩形并调 `SetTextInputArea`），但它只在 `m_bTextInputMode` 下生效、与组合态无关，且组合过程中光标被输入系统接管后没有新的触发点；
  - **组合时仍绘制文本光标**：`PaintCaret` 的 `#if WIN && !SDL` 分支让 SDL 侧忽略了 `m_bIsComposition`，组合期间光标与候选框文字重叠（`RichEdit2.cpp:2208-2210`）。
- **影响**：这是**中文用户可感知的最严重退化项**——在 macOS/Linux/FreeBSD 上用中文输入法时候选框位置不对。
- **已完成的改动**：
  - `NativeWindow_SDL.cpp` 主分发新增 `case SDL_EVENT_TEXT_EDITING`，用 `m_bImeComposing` 状态位把单一事件还原成三态——`text` 非空且未在组合 → `kEventImeStartComposition`；`text` 非空且已在组合 → `kEventImeComposition`；`text` 为 `nullptr` 或空串 → `kEventImeEndComposition`（仅在确实处于组合态时派发，避免重复触发）；
  - `NativeWindow_SDL.h` 新增 `m_bImeComposing` 成员，在构造、失焦（`SDL_EVENT_WINDOW_FOCUS_LOST`）与 `SetImeOpenStatus(false)` 三处复位，防止焦点切换后卡在组合态；
  - `RichEdit2` 新增 `OnImeComposition` 覆写，并在 SDL 分支下于组合开始 / 内容变化 / 结束时通过 `ShowCaret()` 同步候选框位置（复用既有的光标矩形计算，不重复实现坐标换算）；
  - `PaintCaret` 的平台分支去掉，四平台统一为「组合期间不绘制文本光标」，与 Windows 侧一致；
  - **组合态复位要通知控件侧**：`SDL_EVENT_WINDOW_FOCUS_LOST` 与 `SetImeOpenStatus(false)` 两处复位时，除了清 `m_bImeComposing`，还要派发 `EndComposition`；`RichEdit2::OnKillFocus` 也补了 `m_bIsComposition = false` 兜底 —— 否则失焦时组合态残留，重新获得焦点后 `PaintCaret` 因 `m_bIsComposition` 为真而永不绘制文本光标。
- **验收**：✅ 候选框位置由 `ShowCaret()` 统一计算并随组合过程刷新；✅ 组合过程不触发文本变更事件（`SDL_EVENT_TEXT_EDITING` 只更新组合态与候选框位置，真正插入字符仍由组合完成后的 `SDL_EVENT_TEXT_INPUT` 触发，与 Windows 侧 `WM_IME_COMPOSITION` 语义一致）；✅ 四平台组合期间均不绘制文本光标。
- **工作量**：实际约 1 人天（原估 3–5 人天）　**依赖**：M1（原标依赖回归能力，本次通过逻辑探针 + 编译验证替代）
- **实测**（2026-10-20，SDL3 3.5.0 部署后重验）：
  - **SDL 事件语义已对源码核实**（`C:\develop\SDL`）：`SDL_SendEditingText` 遇 `text == nullptr` 直接 return（`src/events/SDL_keyboard.c:825-827`），**组合结束是靠 `SDL_SendEditingText("", 0, 0)` 表达的** —— Windows 侧见 `SDL_windowskeyboard.c:947` 的 `IME_SendClearComposition`，且提交时先发空 EDITING 再发 TEXT_INPUT（`:1121-1122`）；Wayland（`:3035-3037`）、X11（`:616`）、Cocoa（`unmarkText`）行为一致。本实现的判据 `text 为空串或 nullptr` 与之吻合；
  - **编译验证**：`/DDUILIB_SDL=1`（触发 `duilib_config.h` 定义 `DUILIB_BUILD_FOR_SDL`）下，**11 个 SDL 路径源文件 × Release/Debug 双配置 = 22 次编译全部通过**，`DUILIB_SDL=0` 下 5 个原生源文件也通过（两种配置共存无冲突）；
  - **链接验证**：SDL 路径目标文件归档后与真实 `SDL3-static.lib` 链接成 DLL，**LINK_EXITCODE=0、零未解析符号**（仅 LNK4001 无导出表，属预期）；
  - **预处理核对**：`/EP` 输出中确认三个 IME 派发入口与 `bComposing` 判断确实被编译，不是被 `#ifdef` 跳过；
  - **组合态状态机探针**：11 个用例全部通过（含组合态外的空事件不误触发 End、失焦复位后重新输入是 Start）。
  - **仍需人工验证**：「候选框实际跟随光标」这一视觉验收点无法自动化，需在 macOS/Linux 上用中文输入法实测。

### C-3 主题 / 语言切换无变更广播 🟡

- **现状**：`ColorManager::SetColorThemeDarkMode`（`ColorManager.cpp:93-96`）**只设一个 flag，不触发任何失效重绘**，调用方必须自己 `Invalidate()` 全窗口树。语言切换同理：`LangManager` 换表后应用层要自行遍历所有控件重新应用文本，框架无通知事件。
- **影响**：切主题/语言后出现"部分控件不刷新"的 bug，且每新增一个控件都要在应用层再写一遍刷新代码，极易漏。
- **动作**：引入 `kEventThemeChanged` / `kEventLanguageChanged`，由 `Window` 统一广播并自动 `Invalidate` 全树。
- **验收**：切主题/语言后无需应用层写任何刷新代码，全部控件同步更新。
- **工作量**：1 周　**依赖**：无（B-1 已暂缓；主题 / 语言切换的广播行为可用 24 个示例人工验证）

### C-4 Linux/FreeBSD 无系统阴影 ⏸ 不处理（2026-10-20 决策）

- **现状**：`NativeWindow_SDL.cpp:3331-3332` 对 Linux/FreeBSD 返回 `false` → 无系统阴影，降级为自绘 SVG（`Shadow.cpp:584-601` 自动降级逻辑已实现，视觉略有差异）。
- **文档准确性**：`docs/WindowShadow.md:99-103` 的表格与代码一致，**没有夸大**，无需改文档。
- **决策：不处理，保持现状**（2026-10-20，用户明确要求）。理由：自绘 SVG 降级路径已可用、差异可接受；文档已如实标注；libdecor 属 X11/Wayland 社区方案，集成需引入新外部依赖并覆盖各合成器差异，风险高（原估 5–8 人天）而收益仅限视觉效果，与当前优先级（编译埋雷 / 测试网 / 打包）不匹配。
- **重新评估的触发条件**（命中任一条即重新评估本项）：
  | 触发条件 | 说明 |
  | :--- | :--- |
  | Linux/FreeBSD 用户明确反馈"无阴影不可接受" | 视觉差异升级为实际需求 |
  | libdecor 成为 X11/Wayland 事实标准且被主流发行版默认启用 | 外部风险与集成成本大幅下降 |
  | 需要对外承诺"跨平台视觉像素级一致" | 自绘降级不再可接受 |
- **保持现状的行为**（无需任何代码改动）：Linux/FreeBSD 下自动走自绘 SVG 阴影，Windows / macOS 仍用系统阴影。
- **工作量**：0（本次决策不实施；若日后重启用原估 5–8 人天）　**依赖**：无

### C-5 非 Windows 无系统气泡通知 🟡

- **现状**：`TrayIcon_SDL.cpp:248-256` 的 `ShowBalloon` 恒 `return false`（注释说明 SDL3 托盘 API 不支持）；`Hide()` 也被标注为"只能销毁后重建"。
- **动作**：macOS 走 `UNUserNotificationCenter`，Linux/FreeBSD 走 D-Bus `org.freedesktop.Notifications`。
- **验收**：三平台托盘气泡可弹出。
- **工作量**：2 人天　**依赖**：无

### C-6 文件对话框参数部分失效 ✅ 已完成（2026-10-20）

- **改前的现状**（已逐条核实）：
  - `FileDialog_SDL.cpp` 的 `BrowseForFile` / `BrowseForFiles` 把 `nFileTypeIndex` / `defaultExt` / `defaultLocation` 三个形参写成 `/*...*/`（未命名，**完全未生效**）；`BrowseForFile` 还把 `fileName` 误当作 SDL 的 `default_location` 使用；
  - `BrowseForFolders` 的 `allow_many` 恒为 `false`（拷贝错误），"多选目录"退化为单选 —— Windows 原生后端用的是 `FOS_ALLOWMULTISELECT`；
  - **全仓库没有任何调用方传入 `defaultLocation`**（`PropertyGridFileProperty` 与 `RichEdit` 示例都只用到前 6 个参数），参数即使补齐也无观测途径；
  - SDL3 的能力边界（对照 `C:\develop\SDL` 源码）：`SDL_Show*Dialog` 只有 `default_location` 一个位置参数（其 Windows 实现按最后一个 `/` 或 `\` 拆成 `SetFolder` + `SetFileName`，**以分隔符结尾则只当目录**），**没有预选筛选器索引的接口**，其 Windows 实现也**从未调用** `IFileDialog::SetDefaultExtension`。
- **动作（已完成）**：
  - 新增文件内静态辅助函数：`DlgFileFilter`、`ConvertFileTypes`（合并 `BrowseForFile` / `BrowseForFiles` 中重复的筛选器转换代码）、`GetDefaultExtension`（`"*.txt"`→`txt`、`"doc;docx"`→`doc`、`"*.*"`→空）、`GetDefaultLocation`（目录补齐结尾分隔符后与文件名拼接）；
  - `BrowseForFile`：`defaultLocation` + `fileName` 组合成 `default_location`；`defaultExt` 为空且 `nFileTypeIndex` 有效时用该文件类型的扩展名作为默认扩展名，并预填到文件名（`abc` → `abc.txt`）；保存对话框返回路径无扩展名时**追加默认扩展名**（等价于 Windows 的 `SetDefaultExtension`）；
  - `BrowseForFiles`：形参恢复命名、`defaultLocation` 生效、删除恒空的死代码；
  - `BrowseForFolders`：`allow_many` 修正为 `true`；两个目录函数统一用 `GetDefaultLocation` 规范化目录；
  - 调用侧启用：`PropertyGridFileProperty::OnBrowseButtonClicked` 由当前属性值推导 `fileName` / `defaultLocation`；`examples/RichEdit/MainForm.cpp` 打开/另存为传入当前文件所在目录；
  - `FileDialog.h` 与 `FileDialog_Windows.cpp` **未改动**（签名不变、Windows 原生后端行为不变、未改 SDL 源码）。
- **上游核对**（SDL 3.5.0）：改前位于 `FileDialog_SDL.cpp:333` 的代码内 TODO「SDL 3.0 多选返回值不对」在当前 SDL 版本**已不成立** —— 多选改走 `IFileOpenDialog::GetResults`，返回以 NULL 结尾的完整路径数组；该 TODO 已替换为准确注释（现位于 `FileDialog_SDL.cpp:394-396`），**不写额外兼容代码**。
- **验收**：初始目录与默认文件名/扩展名由传入参数决定；保存对话框返回路径的扩展名被校正。⚠️ **原生模态对话框的交互无法自动化（duilib 自绘 UI、无 UI Automation 提供程序），需人工核对**，核对清单见 [C-6 spec](specs/C-6-FileDialog-SDL-Params/spec.md) 的 T7/T8。
- **已知能力差异**（SDL3 无对应接口，已记入代码注释与 spec）：筛选器下拉的**预选索引**在 SDL 后端仍无效（`nFileTypeIndex` 只用于推导默认扩展名）；`defaultLocation` 若指向不存在的目录，SDL 会回退到旧版 COM32 对话框（不会卡死，返回值扩展名校正仍生效）。
- **工作量**：实际约 0.5 人天（原估 1–2 人天）　**依赖**：无（A-3 暂缓不影响本项；SDL3 无法预选筛选器索引属平台能力限制，不构成 A-3 的触发条件）
- **实测**（2026-10-20）：
  - Debug x64 增量编译 `build/examples.sln`：**0 error / 0 warning**，全部示例链接成功；`FileDialog_SDL.obj`、`PropertyGrid.obj` 与 `bin/*_d.exe` 时间戳已更新（改动进入产物）；
  - grep 核对：三个 `/*...*/` 形式的被注释形参 **0 处**残留、`BrowseForFolders` 的 `allow_many = true` 已生效、过期 TODO **0 处**；
  - **仍需人工验证**：对话框的初始目录 / 默认文件名 / 保存返回值扩展名、多选目录（`BrowseForFiles` / `BrowseForFolders` 当前无示例调用点）。

### C-7 `DUILIB_SRC_MACOS` 平台判定脆弱 ✅ 已完成（2026-10-20）

- **改前的现状**：
  1. `duilib/CMakeLists.txt` 用**自设变量** `DUILIB_SRC_MACOS`（仅在 `Darwin` 分支被 `set`）代替 CMake 原生的 `CMAKE_SYSTEM_NAME STREQUAL "Darwin"` 判定，平台的"真值"藏在变量里，新增 / 调整平台分支时容易断链；
  2. `list(REMOVE_ITEM CEF_CONTROL_SRC ...)` 在 FreeBSD 下对**未定义变量**执行 —— CMake 对未定义变量静默无操作，**无任何日志**；
  3. 同类隐患（本次一并处理）：`CEF_CONTROL_INTERNAL_WINDOWS_SRC`（Linux/macOS 下未定义）、`SKIA_WGL_SRCS` / `UDIS_HDRS` / `UDIS_SRCS`（非 Windows 下未定义）被 `LIBRARY_SOURCES` 直接引用，同样是静默空追加；Windows 专有源文件的 `aux_source_directory` 还散落在 CEF 块之外。
- **动作（已完成）**：
  1. 删除 `set(DUILIB_SRC_MACOS 1)`，改用 `CMAKE_SYSTEM_NAME STREQUAL "Darwin"`；全仓 `DUILIB_SRC_MACOS` 现为 **0 处**（代码）；
  2. CEF 源文件的收集、macOS 专有 `.mm` 的移除、Windows 专有源文件的收集，**收敛进同一个** `if(NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")` 块 —— 这些语句所用到的变量在该块内必然已定义；
  3. `CEF_CONTROL_INTERNAL_WINDOWS_SRC` / `SKIA_WGL_SRCS` / `UDIS_HDRS` / `UDIS_SRCS` 移入各自的 `if(CMAKE_SYSTEM_NAME STREQUAL "Windows")` 分支，非 Windows 平台不再引用未定义变量；
  4. 平台分支补齐 `STATUS` 日志：`CEF support enabled/disabled (...)`、`CEF macOS-only sources removed (... is not Darwin)`、`macOS-only sources removed (... is not Darwin)`。
- **验收结果**：
  - 验收项「构建日志中不再有静默跳过」：本机 CMake 配置（Windows）`exit 0`，日志包含 `CEF support enabled (Windows)` 与两条 `... is not Darwin` 移除日志，分支全部显式可见；
  - 额外的 `cmake --warn-uninitialized` 全量配置：除既有的可选缓存变量 `DUILIB_MD`（`CMakeLists.txt:10` 的 `-DDUILIB_MD` 开关，属正常写法）外，**0 条未初始化变量警告**；
  - Debug x64 编译（`build/duilib.sln`）：**0 error / 0 warning**（注：仓库 `build/*.sln` 是手写 `.vcxproj` 工程、不经过 CMake；本项改动影响的是 CMake 构建路径，即 `build/msvc_build.bat`、`build/linux_build.sh`、`build/macos_build.sh`、`build/freebsd_build.sh`）；
  - ⚠️ macOS / Linux / FreeBSD 分支本机**无法实机构建**（缺对应编译器，交叉配置无法通过编译器检测），本次以「变量定义域审查 + 分支日志」覆盖，需在对应平台构建时按上述日志核对。
- **工作量**：实际约 0.25 人天（原估 0.5 天）　**依赖**：无

---

## 五、P2 组 — 控件补齐

> 已核实**确实缺失**（类名与 `DUI_CTR_*` 宏均全仓无命中）的控件，按投入产出排序。

### E 组：低垂果实（合计 < 1500 行，建议优先）

| ID | 项 | 现状依据 | 动作 | 工作量 |
| :--- | :--- | :--- | :--- | :--- |
| **E-1** | **StatusBar 分格索引 API** ✅ 已完成（2026-10-18） | `StatusBar.h` 无 `SetStatusText(格,文本)` / `GetStatusText`，只能业务层 `FindControl` 找子 Label；且 size grip 头文件自认"仅作视觉提示，不改变窗口可调整大小行为"= **装饰性死控件** | 补 4–6 个方法；明确 size grip 是否要接真实拖拽 | 极低 |
| **E-2** | **NavigationView 属性文档** ✅ 已完成（2026-10-18） | `docs/Control.md` 中 `NavigationView` 出现 0 次——**唯一已实现但主文档无章节的控件** | 补中英双章节 + `Summary` / `XmlNode` / `Events` 四处 | 极低（纯文档） |
| **E-3** | **通用 Dialog 控件** | 只有 `MessageBoxWnd` 五种固定按钮，无法放任意内容。**地基已齐**（`Mask` 遮罩 + `Flyout` 浮层），只差组装 | 新增 `Dialog` 基类，可放任意内容 + 自定义底部按钮区 + `DoModal` | 低（约 600 行） |
| **E-4** | **Pagination 分页器** ✅ 已完成（2026-10-19） | 全仓无命中。报表/表格类软件刚需，当前全靠业务层手写按钮组 | HBox + 若干 Button 组合 | 低（约 250 行） |
| **E-5** | **ScrollBar 键盘导航** ✅ 已完成（2026-10-18 核实：现有代码已支持，无需开发） | `ScrollBar` 无 PageUp/PageDown/方向键/Home/End 处理（对比 `Slider.h:215 HandleKeyDown` 有完整键盘支持）；`ScrollBar.cpp:50` 还有 TODO"DPI 自适应参数，需要确认一下" | 补键盘处理 + 修 DPI TODO | 低 |

#### E-1 实施记录（2026-10-18 完成）

**决策：size grip 接真实拖拽，默认开启。** 未采用"控件内自建拖拽循环 + `MoveWindow`"方案——
那样会绕过系统的模态缩放循环，丢失贴靠布局、最小/最大尺寸约束（`WM_GETMINMAXINFO` 在 `SetWindowPos` 路径上不生效）
与多显示器边界处理。改为**放大窗口的 `size_box`**：`StatusBar` 在 `SetPos` 时判断自身是否停靠在窗口客户区右下角
（允许 1 逻辑像素布局误差），是则把 `size_box` 的 `right`/`bottom` 扩大到覆盖手柄（仅放大、不缩小，保留用户原值并可在离开右下角时还原）；
Windows 侧沿用 `WM_NCHITTEST`，SDL 侧沿用命中测试回调，两端都零平台代码改动。新增属性 `size_grip_resize`（默认 `true`）可关掉该行为。

**分格索引 API（6 个方法）**：`GetPanelCount` / `GetPanel` / `GetPanelIndex` / `FindPanel` / `SetPanelText` / `GetPanelText`。
文本读写通过 `LabelOwner` 接口（`LabelImpl.h`）做 `dynamic_cast`，Label / RichEdit 等文本类面板通用，非文本面板（Progress 等）返回 `false` 而非崩溃。
索引只统计**可见**面板，与视觉分格顺序一致；增删子控件或改可见性后重新编号（已在头文件与 `docs/Control.md` 双语说明）。

**已知取舍**：`size_box` 是窗口级属性，放大后窗口底边一带也会成为可拉伸区域（标准行为，Qt 的 size grip 同理）；
若状态栏右下角放的是可交互控件（按钮等），其右侧约 12 逻辑像素会被缩放区占用。
**平台前提**：Windows 默认带 `WS_THICKFRAME`，直接生效；SDL 等平台只有 XML 里声明了非零 `size_box` 才会把窗口建成可缩放的，
因此演示窗口 `statusbar.xml` 已补 `size_box="0,0,4,4"`。若要在运行期动态开启窗口可缩放，需要给 `WindowBase` 加
`SetWindowResizable`（涉及 `INativeWindow` 接口 + Windows/SDL 两份实现），未在本次范围内做。

**验证**：`cl /Zs` 语法检查 `duilib/Control/StatusBar.cpp` 与 `examples/controls/StatusBarForm.cpp` 均退出码 0；
文档（`docs/Control.md` / `.en.md` / `Summary.md` / `.en.md` / 三处 skill / `.claude` 参考）已同步；
`examples/controls/statusbar.xml` 增补两个演示按钮，语言文件中英各加 2 条 STRID。
**未做运行期验证**（本机无法完整链接 Skia），拖拽行为需在真机确认。

#### E-2 实施记录（2026-10-18 完成）

**内容**：`docs/Control.md` / `Control.en.md` 各新增 `NavigationView`（12 个属性）与 `NavigationViewItem`（17 个属性）两节，
含属性表、直接子节点分流规则（NavigationViewItem / TabBox / 其它控件三类）、序号约定（仅统计 item 形态可选项，
不含 header/separator 与设置项）、键盘导航（Tab 聚焦、上下方向键循环、回车/空格激活）、XML 示例、C++ 示例与事件表。
属性名与默认值逐条对照 `NavigationView.cpp` 的 `SetAttribute` 与构造函数核实（`pane_width`/`compact_pane_width`/`icon_size`
仅接受正值；三种形态固定高度 36/32/9 DIP）；10 个语义色名已在 color_light + color_dark 双主题 grep 验证。

**`Summary` / `XmlNode` / `Events` 三处（各中英）原有条目已逐条核对无需改动**：4 个事件的 wParam/lParam 语义
与 `duilib_defs.h:435-438`、`EventArgs.cpp:121-124` 的实现一致；`Summary` 两行（NavigationView / NavigationViewItem）
与 `XmlNode` 两行的头文件链接均有效。

**附加清理**：`Control.md` / `Control.en.md` 顶部各有一条历史遗留的 HTML 豁免注释（`border_dash_style` /
`EasingFunctionType`），已删除——这两个名字登记在 `verify_allowlist.txt` 的 `docs/Control.md docs/Control.en.md` 行，
注释属冗余，且不符合"md 正文不插 HTML 注释"的约定。

**验证**：`verify_docs.py --extra docs` 89 份文档零漂移；章节内 XML 示例通过 ElementTree 解析；
改动文件全部保持 CRLF；markdown 行尾硬换行空格零丢失。

#### E-5 实施记录（2026-10-18 完成）

**结论：现有代码已支持，无需开发。** E-5 的前提找错了类——键盘导航不在 `ScrollBar` 控件里，而在其宿主容器
`ScrollBox::HandleEvent`（`duilib/Box/ScrollBox.cpp:452-523`）中完整实现：竖向滚动条可滚动时响应
↑/↓（`LineUp`/`LineDown`，步长 `vscroll_unit`，默认 30 像素）、PageUp/PageDown（翻页，步长为可视区高度）、
Home/End（`HomeUp`/`EndDown`，跳到首/末）；仅当竖向不可滚动时，横向滚动条响应同样的按键
（`LineLeft`/`LineRight`、`PageLeft`/`PageRight`、`HomeLeft`/`EndRight`）。
ScrollBar 是悬浮子控件、不参与焦点链，键盘由容器统一处理是合理架构；与 `Slider` 的差异源于
Slider 是单个可聚焦控件、ScrollBox 是容器，二者不构成同类对比。

**事件冒泡机制**：容器内的子控件获得焦点时，未处理的 `kEventKeyDown` 经 `Control::HandleEvent`
（`Control.cpp:2886-2889`）沿父链冒泡到 ScrollBox，因此无需给容器单独设置焦点，
焦点在滚动区域内任意子控件上时按键均生效。

**TODO 清理**：`ScrollBar.cpp:50` 的 "TODO: DPI自适应参数，需要确认一下" 已删除。该处是构造函数里的
`SetFixedWidth(UiFixedInt(DEFAULT_SCROLLBAR_SIZE), false, false)`，第三参 `bNeedDpiScale=false`
表示 12px 默认宽度不随 DPI 缩放——经确认无问题。

**文档标注**：`Box.md` / `Box.en.md` 的 ScrollBox 节新增"键盘导航 / Keyboard navigation"小节
（按键映射表、冒泡机制、竖横优先级、步长说明）；`Control.md` / `Control.en.md` 的 ScrollBar 节
补充交叉引用说明（键盘由所属 ScrollBox 实现，不在 ScrollBar 自身）。

#### E-6 实施记录（2026-10-18 完成 ②③；① 保持开放）

**② 排序稳定性（已修复）**：`ListCtrlData::SortStorageData`（`duilib/Control/ListCtrlData.cpp`）
原用 `std::sort`，且降序是"升序排完再 `std::reverse` 整体反转"——后者即使换成 `stable_sort`
也会把同值行的相对顺序一并颠倒。现改为：

1. 两个比较分支（自定义 `pfnCompareFunc` / 默认 `SortDataCompareFunc`）均改用 `std::stable_sort`；
2. 降序改为**转置比较**（交换 a/b 位置后按同一升序语义排），同值元素在升降序两个方向都保持原始相对顺序；
3. 无单元格数据（`pStorage` 为 nullptr）的行仍统一视为最小值：升序在最前、降序在最后（与原行为一致）；
4. 新增文件内辅助函数 `StableSortDisplayData`（匿名命名空间，迭代器 + 比较器模板），
   稳定性与转置逻辑集中一处，两个分支只贡献各自的比较语义。

`ListCtrl.h` / `ListCtrlData.h` 的排序接口文档已补稳定性说明。验证：`cl /Zs` 通过；
另写独立测试程序复刻该算法跑 11 项断言（同值升降序稳定性、空数据边界、2000 元素大规模验证、
旧路径 bug 复现对比）全部通过。

**③ 单元格合并（核实后结论：不实施，仅清理误导注释）**：该项前提有误，两处核实：

1. **归并逻辑并非"已写好"**：全仓无任何 span / merge 代码（`grep -rni "merge\|span"` 于
   `duilib/Control/` 零命中）；代码里唯一的"归并"是 `GetSelectedCellRanges` 把**选中单元格**
   合并为矩形区域（用于 `kEventCellSelectionChanged` 事件上报与复制导出），已完整实现，
   与"单元格合并（跨行/跨列 span）"是两回事。
2. **单元格合并（原设计方案 C4）已在评审中取消**：见 `docs/misc/ListCtrl-Roadmap.md`
   （"单元格合并 ｜ 已取消（不实施）"）与设计方案 §3 C4 节【已取消（不实施）】标注——
   理由是需表头与行的布局模型整体升级、改动面过大；设计方案还明确"不再新增
   `sub_item_row_span` / `sub_item_column_span`"（即数据入口也不加）。

因此未实施合并功能（避免推翻既定评审结论），仅清理两处会误导后续维护者的注释：
`ListCtrl.h` 的 `GetSelectedCells` 文档（原写"跳过被合并覆盖的单元格，后续合并功能落地后生效"，
改为说明合并已评审决定不实施、不存在被覆盖的单元格）；`ListCtrl.cpp` 的 `BuildSelectionText`
内部注释（原写"C4 实施后按合并锚点格展开"，改为说明已取消、按单元格逐一取文本）。
`docs/Control.md` / `.en.md` 中对应的两处"跳过被合并覆盖的单元格"描述同步删除。

**① 列汇总行 / 分组行（保持开放，本轮未做）**：属布局模型新增能力，需单独设计与评审。

#### E-7 实施记录（2026-10-18 完成：垂直标签 / 固定标签 / 溢出滚动）

**范围澄清（用户指令）**：原条目的"无 `Add`/`Insert`/`Remove` 程序化管理 API"经用户确认这些接口在 `TabCtrl` 中**已存在**，
不开发；本轮只做其余三项。

**① 垂直标签**：新增 `TabCtrlOrientation` 枚举与 `SetTabCtrlOrientation` / `GetTabCtrlOrientation`，
XML 属性 `tab_ctrl_orientation`（`horz` / `vert`）。方向切换经 `Box::ResetLayout(new VLayout() / new HLayout())`
+ `FreeLayout` 完成（`OnInit` 与运行期均可切）；`PaintStateColors` 的底部分割线在垂直时改为右侧竖线，
`PaintTabItemSelected` 的选中轮廓 / 竖线在垂直时贴右边缘（`rc.right -= outlineWidth/2` 防止 1 像素溢出）。

**② 固定标签**：`TabCtrlItem::SetFixed(bool)`（XML 属性 `fixed`），实现为 `SetEnableDragOrder(!bFixed)`——
`ControlDragableT` 原生支持跳过非 draggable 项（拖拽时与作为放置点时均跳过），无需自写拦截。
锚定约束写在三处：`AddItem`（固定项插到固定块末尾 `GetFixedItemCount()`）、`AddItemAt`、`SetItemIndex`——
因 `Box::AddItemAt` 经 `DoAddItemAt` 不回调 `SetItemIndex`，只改一处会漏。固定项的关闭按钮经
`UpdateCloseButtonVisible` 统一收口强制隐藏。

**③ 溢出滚动**：`SetEnableOverflowScroll` / `IsEnableOverflowScroll`（XML `overflow_scroll`，默认 `true`）。
`OnInit` 末尾按方向启用对应滚动条并设 `SetScrollBarFloat(true)` 悬浮条（不占布局空间，仅内容溢出时 `IsValid()`）；
拖拽靠近边缘的自动滚动由 `ControlDragableT` 内置。`global.xml` 已有 `vscrollbar` / `hscrollbar` 皮肤，无需新增。
运行期重设方向时一并重配滚动条并 `Invalidate()`。

**验证**：`controls_d.exe` 构建 EXITCODE=0（`duilib_d.lib` 同批产出），启动冒烟 SMOKE_OK；
`test.xml` 新增垂直标签栏（首页/收藏 fixed）与 12 项水平溢出两个演示卡，XML 解析 OK。
**真机视觉 / 交互验证待用户确认**（垂直选中轮廓、固定项拖拽拦截、溢出滚动交互）。

#### 2026-10-17~19 提交对照（不在原跟踪表编号内的变更）

原跟踪表只列了带编号的规划项，实际同期还有一批**未挂编号**的变更落地，一并登记以便追溯：

| 提交 | 日期 | 类型 | 内容 |
| :--- | :--- | :--- | :--- |
| `ce567279` | 10-17 | 新控件 | `StatusBar`（继承 `HBox`，子控件即状态面板 + 可选右下角尺寸手柄） |
| `25a534f1` | 10-17 | 新控件 | `ToolBar`（继承 `HBox`，宽度不足自动收纳尾部子控件并弹出"更多"浮层） |
| `001352c5` | 10-17 | API 增强 | `Progress` / `Slider` / `ColorPicker` 新增 `SetValueNoEvent`：设值但不触发 `kEventValueChanged`，用于"一个事件源驱动多个控件同步显示"避免回调循环 |
| `98d89248` | 10-17 | Bug 修复 | `ColorPicker` 界面启动即卡死 |
| `df2ac755` / `62385554` | 10-17 | Bug 修复 | `ListCtrl` 日期/数字类型编辑细节（修复日历浮窗弹出位置错位） |
| `d0281bd4` / `ccf5e714` | 10-17 | 性能 | `ListCtrl::RefreshSubItem` 重写为真正的按单元格刷新 |
| `51363f88` / `d0458009` | 10-18 | Bug 修复 | `Flyout` / `ToastWnd` 弹出后关闭时抢焦点 |
| `0d987cb6` | 10-18 | API 增强 | `Chart` 实时监控数据（对应 E-11 ①③） |
| `7794515a` | 10-19 | Bug 修复 | `Slider` 鼠标 hover 状态下刻度线不绘制 |
| `2595e34d` | 10-19 | 视觉 | 进度条图标重新设计 |
| `6ca1d303` | 10-19 | Bug 修复 | `ControlMovableT` 的 `move_pos_draggable_border` 属性无效（issue #479） |
| `e76376cf` / `46e92b35` / `269c0171` / `65962133` / `2421d21f` / `27b9a497` | 10-18~19 | 新控件 | `Rating` / `Carousel`+`Pagination` / `Stepper`（含图标完善）/ `RangeSlider` / `TreeGrid` |
| `009fb5ce` | 10-19 | 文档 | 修复文档漂移（A-5 的延续维护） |

> **规律**：10-17~10-19 的产出集中在「新增控件 + 已有控件的交互/Bug 修复」，**规划表里的 P0（A 组）与 P1（B 组）
> 零进展** —— 与「不要跳过 M1 直接做 M3」的原始建议相反，实际执行是从 M2 控件侧切入的。
> 文档校验（B-2 的手工版）目前靠人工跑 `verify_docs.py`，尚未进 CI。

### D 组：中优先级

| ID | 项 | 现状依据 | 工作量 |
| :--- | :--- | :--- | :--- |
| **D-1** | **RangeSlider 双滑块** | `Slider.h` 仅单值 `SetValue`；`Project-Audit.md` B3 已登记待办。底子好（有刻度、标签、click-to-jump、键盘支持） | 低（约 400 行改动） |
| **D-2** | **富内容 Tooltip** | `ToolTip.h` 仅 `ShowToolTip` 单方法**纯文本**，Windows 侧还是系统原生实现；`Flyout` 地基已就绪未封装（`Project-Audit.md` B8） | 低（约 200 行） |
| **D-3** | **SpinBox 支持小数与格式化** | `SpinBox.h:47,51,57` 全为 `int64_t`，不支持小数/千分位/前后缀（对比 Qt `QDoubleSpinBox`） | 低 |
| **D-4** | **TreeView 节点拖拽重排** | `TreeView.cpp` 中 `drag`/`drop` **零命中**。树控件头号缺失（文件树/权限树/组织树通用需求）。另 `TreeView` 无排序（`sort` 零命中）、继承 `ListBox` 非 `VirtualListBox` 无虚拟化 | 中（约 700 行） |
| **D-5** | **TimePicker 时间选择弹层** | `DateTime.h:117` 自认 `kDateCalendar`「不支持选择时间」，时间只能靠 spin 按钮逐格调。日程/工单/预约场景刚需。另无 min/max 日期约束、无禁用日期、无周起始日配置 | 中（约 800 行） |
| **D-6** | **Stepper / Steps 向导步骤条** | `Timeline` 语义不符（时间轴 ≠ 流程状态机）；`Project-Audit.md` B4 登记待办 | 低-中（约 400 行） |
| **D-7** | **ToolBar 工具项抽象** | `ToolBar`（10-17 新增，继承 `HBox`）已具备溢出收纳与回调，但**仍无项管理 API**（无 `AddTool`/`SetToolChecked`/`RemoveTool`），checkable 工具项只能塞 CheckBox，分隔符靠 `Line` 手工拼 | 中（约 500 行） |
| **D-8** | **ColorPicker 补 Alpha 通道 + HSV/Hex 数值输入** | 现有 4 种形态 + `ColorSlider` 已很全，但**无透明度选择**（WinUI/Qt 均有），无数值输入面板。`ScreenCapture` 已存在可做吸管 | 中（约 400 行） |
| **D-9** | **CascadingSelect 级联选择器** | 无"父变更自动重置/过滤子项"语义；`Combo` 的 Tree 形态只能手工模拟两级 | 中（约 600 行） |
| **D-10** | **WrapPanel 不等宽换行布局** | 有等宽 `HTileBox`/`VTileBox`，缺按内容宽度换行。`TagInput` 内部已自行实现（需求已被验证） | 中（约 400 行） |
| **D-11** | **NavigationView / Breadcrumb / Rating / Carousel** | NavigationView（E-2 已补文档）、Rating（B5 完成，提交 `e76376cf`，见 `duilib/Control/Rating.h`）、Carousel（B7 完成，提交 `46e92b35`，见 `duilib/Control/Carousel.h`）均已落地；**Breadcrumb 仍全仓无命中**（`AddressBar` 是浏览器地址栏，语义不同） | 低（仅剩 Breadcrumb） |
| **D-12** | **原生系统通知** | `Control-Roadmap.md` 第 19 项；Toast 仅应用内自绘。macOS 通知中心 / Windows 桌面通知 / Linux D-Bus | 中高 |
| **D-13** | **TreeGrid 树表格** ✅ 已完成（2026-10-19，提交 `27b9a497`） | 原审计 B9 的落地方式：**不改 `ListCtrl`，新增独立控件** `duilib/Control/TreeGrid.h/.cpp`（继承 `VBox`，固定表头 + 自带滚动条的列表体，表头与数据行复用同一列宽数组）；`tree_column`+`indent` 层级缩进、`multi_select` 多选、`check_mode`+`check_cascade` 父子三态级联。**ListCtrl 内嵌树 / 分组折叠 / L3 列筛选仍不做**（见第八节） | 已完成 |

### 现有控件的补强点（不新增控件，只补 API）

| ID | 控件 | 缺口 | 备注 |
| :--- | :--- | :--- | :--- |
| **E-6** | `ListCtrl` ✅ ②③已完成（2026-10-18）；① 保持开放（未做） | ① 无列汇总行 / 分组行（只有 `nSortGroup` 排序分组）；② 排序用 `std::sort` 非 `stable_sort`，同值行会乱序，同组文件夹/文件可能交错；③ `ListCtrl.h:741` 自认单元格合并"尚未实现"，归并逻辑已写好但缺数据入口 | 后两项成本低、收益明确 |
| **E-7** | `TabCtrl` ✅ 已完成（2026-10-18）：垂直标签 / 固定标签 / 溢出滚动 | 原条目称"无 `Add`/`Insert`/`Remove` 的程序化管理 API"——经用户核实这些接口**已存在，不开发**；实际缺口为：另无垂直标签、无固定标签、无溢出滚动 | 三项均已落地；真机视觉/交互待确认 |
| **E-8** | `Menu` / `MenuItem` ⚠️ **仅完成注释清理（2026-10-18），API 未做** | `MenuItem` 至今**无 `SetCheck`/`SetEnabled`/`SetIcon`/`SetShortcutText`/`SetRadio`**（`Menu.h:260-317` 逐条核实，公开方法只有子菜单增删查与 `Activate`），勾选/禁用/图标/快捷键在专业菜单里是标配。当前勾选只能靠内嵌 `CheckBox` 手工拼（见 `bin/resources/themes/default/controls/menu/settings_menu.xml:69-73`）。提交 `79307ecd` 只清理了 `Menu.cpp:11` 的 TODO 与 `Menu.h` 的遗留注释，**未新增任何 API** | 成本低收益高 |
| **E-9** | `DateTime` 不处理（架构债，留待重构） | `DateTime.h:35-46` 两处专门为"被复用为 ListCtrl 编辑器"打的补丁（`ResetCalendarFlyoutState`/`CloseCalendarFlyout`）+ 成员 `m_bSuppressCalendarReopen` —— **抽象缺失导致实现细节外泄**；`DateTimeWnd` 与 `kDateCalendar` 两套编辑路径并存 | 本轮不处理 |
| **E-10** | `SearchBox` ✅ 已完成（2026-10-18）：防抖 + 建议/历史下拉整合 | 原"无防抖"已补 `debounce_interval`（默认 0 立即触发，>0 输入停顿后才发 `kEventTextChanged`）；"初始化顺序债"实为 `SetAttribute` 缓存 + `OnInit` 转发，已正确处理，无需改动；"无历史/建议"已整合——`SearchBox` 复用 `AutoCompleteFlyout` 实现下拉联想：建议数据源（`suggestions`/SetCandidates）+ 搜索历史（提交时自动 MRU 记录，`history_enabled`/`max_history` 可配），支持 prefix/contains 匹配、键盘上下/回车/Esc 导航、复用 `kEventAutoCompleteSelect` 事件 | 中低 |
| **E-11** | `Chart` ✅ ①③已完成（2026-10-18，提交 `0d987cb6`）；②④不做（用户决定） | ① 增量更新已落地：`AddDataPoint` / `InsertDataPoint` / `SetDataPoint` / `RemoveDataPoint` / `ClearSeries` / `RemoveSeries` / `SetMaxDataPoints`（超出上限自动丢弃最旧点）；③ `SetXAxisLabelFormatter` / `SetYAxisLabelFormatter`（类比 `Slider` 的 `TickLabelCallback`）。② 无导出 PNG/SVG；④ `Chart.cpp` 约 2600 行单文件 | ①③已落地；②④不做 |

---

## 六、P3 组 — 架构改造（需 M1 安全网就位后再动）

### F-1 ListCtrl DataTemplate 机制（决定 ListCtrl 天花板）

- **现状**：列的显示形态由 `ListCtrlSubItemData` 的**硬编码布尔位**决定（`nProgress` / `bRichText` / `bImageId` / `bShowCheckBox`）。要加"第 5 种形态"必须改 `ListCtrlDefs.h` + `ListCtrlData.h/.cpp` + `ListCtrlReportView.cpp` + `ListCtrlSubItem.cpp` **四个文件**，无法通过子类或回调扩展。
- **影响**：这是一个**架构级缺口**——同类需求（TreeView 节点模板、卡片内嵌自定义内容、报表列扩展）都会撞到同一堵墙。
- **动作**：设计声明式模板 / Selector 机制，让列的呈现形态可由 XML 或回调定义。
- **工作量**：高（架构级重构）
- **依赖**：~~必须先有 B-1 测试安全网~~ → **B-1 已于 2026-10-20 暂缓**。若仍要推进 F-1，改为「小步拆分 + 每步回归 24 个示例」的方式控制风险；完成后 F-1 才能解锁 TreeView 节点模板、Sparkline 内嵌等下游工作

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
- **工作量**：高　**依赖**：C-2（IME 先修）；B-1 已暂缓，不再作为前置

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
| TreeGrid / 内嵌树 / 分组折叠 / L3 列筛选 | **独立控件已落地，内嵌路线仍不做** | `Project-Audit.md` B9：已新增独立控件 `TreeGrid`（不改动 `ListCtrl`）；**ListCtrl 内嵌树 / 分组折叠 / L3 列筛选仍不开发**（改动面过大） |
| Pivot 透视表 | **不做** | 属 BI 而非 UI 库职责 |
| FreeBSD + CEF | **不做** | Chromium 不官方支持 FreeBSD；`freebsd_build.sh:77` 已注释掉 `cef`/`CefBrowser`，`README.md:562` 已声明。**但 A-1/A-2 要把宏与源码的矛盾清干净** |
| Windows 独占工具（`InlineHook`/`ApiWrapper`/`DllManager`/`DiskUtils`/`libudis86`） | **不跨平台化** | 功能本身即 Windows 专属，非缺陷 |
| Linux/FreeBSD 系统阴影（**C-4**） | **不做，保持现状** | 2026-10-20 决策：`Shadow.cpp` 的自绘 SVG 降级路径已可用、`docs/WindowShadow.md` 已如实标注；libdecor 集成需引入新外部依赖且覆盖各合成器差异，风险高（原估 5–8 人天）而收益仅限视觉效果。重新评估的触发条件见 C-4 小节 |

---

## 九、跟踪表

> 勾选规则：完成并验证后把未勾选框改为已勾选（`[ ]` → `[x]`），并在「完成日期」填日期。**每完成一项请同步更新本文档顶部的「基线日期」。**

### P0 — 阻塞级（目标：3.5 人天）

A 组 5 项中已完成 4 项（A-1、A-2、A-4、A-5）；A-3（SDL3 版本锁定）经 2026-10-20 评估后**主动暂缓**，当前 SDL3 主干四平台编译正常，触发条件见 A-3 小节

- [x] **A-1** FreeBSD CEF 宏矛盾　`duilib_config.h`　｜ 0.5d ｜ 完成日期：2026-10-20
- [x] **A-2** CEF 层 4 文件 FreeBSD 守卫　｜ 1d ｜ 依赖 A-1 ｜ 完成日期：2026-10-20
- [~] **A-3** SDL3 版本锁定　`build/build_duilib_all_in_one.sh:196`　｜ 0.5d ｜ 状态：⏸ 暂缓（2026-10-20 决策，触发条件见 A-3 小节）
- [x] **A-4** Linux Skia fontconfig 开关 + 空字体告警　｜ 0.5d ｜ 完成日期：2026-10-19（复核修正风险等级：上游默认值在 Linux 上本就为 `true`，本次是把隐式依赖改为显式声明）
- [x] **A-5** 文档硬伤 8 类修正　｜ 1d ｜ 完成日期：2026-10-17

### P1 — 立网（目标：6–8 周）

- [~] **B-1** 引入 GoogleTest + CTest（分三步）　｜ 3–4 周 ｜ 状态：⏸ 暂缓（2026-10-20 决策，触发条件见 B-1 小节）
  - [~] B-1.1 纯逻辑模块单测（`Ui*` / `StringUtil` / `FilePathUtil` / `ColorConverter` / `AttributeUtil` / `EasingFunctions`）≥200 用例　｜ ⏸ 随 B-1 暂缓
  - [~] B-1.2 三块大状态机逻辑单测（ListCtrl / Calendar / RichEdit2）各 ≥20 用例　｜ ⏸ 随 B-1 暂缓
  - [~] B-1.3 离屏渲染截图回归（`verify_screenshots/` 二选一：做成真回归 或 删除）　｜ ⏸ 随 B-1 暂缓
- [~] **B-2** CI 四平台构建矩阵 + 测试 + 文档校验　｜ 1–1.5 周 ｜ 状态：⏸ 暂缓（2026-10-20 决策，触发条件见 B-2 小节）
- [x] **B-3** 发布与打包流程（install 规则 + CPack + `docs/Release.md`）　｜ 4–6 天 ｜ 完成日期：2026-10-20（`build/pack.py` + `VERSION` + `docs/Release.md`）｜ ⏸ 改动暂留在工作区未提交（2026-10-20 决策），提交清单与后续待办见 [发布打包工作记录](Release-Packaging-Notes.md)
- [~] **B-4** 分级日志 + 文件输出 + 崩溃 dump + 异常规范　｜ 1–1.5 周 ｜ 状态：⏸ 暂缓（2026-10-20 决策，触发条件见 B-4 小节）

### P2 — 一致性与控件（目标：25–35 人天）

> **实际进展（截至 2026-10-20）**：C 组（跨平台一致性）**7 项中完成 4 项**（C-1 `ASSERT` 统一，方案 B；C-2 IME 组合输入补齐；C-6 文件对话框参数补齐；C-7 CMake 平台判定健壮化），另有 C-4 经决策**明确不处理（保持现状）**；D / E 组（控件）**已完成 13 项**，其中 10 项集中在 10-17~10-19 四天内。当前瓶颈仍是 A 组（编译埋雷）与 B 组（测试 / CI / 打包）—— 控件越加越多，回归网还是零。

- [x] **C-1** `ASSERT` 跨平台行为统一　｜ 0.5 天（方案 B）｜ 依赖：无　｜ 完成日期：2026-10-20（Windows 侧加 `#ifdef _DEBUG` 包裹，方案 A 双宏未采纳）
- [x] **C-2** IME 组合输入非 Windows 平台补齐　｜ 约 1 人天（原估 3–5）｜ 依赖：无（原标 M1，本次以逻辑探针替代）｜ 完成日期：2026-10-20
- [ ] **C-3** 主题 / 语言切换变更广播　｜ 1 周 ｜ 依赖：无（B-1 暂缓，改为人工验证广播行为）｜ 完成日期：______
- [~] **C-4** Linux/FreeBSD 系统阴影　｜ 状态：⏸ **不处理，保持现状**（2026-10-20 决策，理由与重新评估触发条件见 C-4 小节；原估 5–8 天）
- [ ] **C-5** 非 Windows 托盘气泡通知　｜ 2 天 ｜ 完成日期：______
- [x] **C-6** 文件对话框参数补齐　｜ 实际约 0.5 天（原估 1–2 天）｜ 依赖：无（A-3 暂缓）｜ 完成日期：2026-10-20 ｜ ⚠️ 原生对话框的交互需**人工核对**（见 C-6 小节，清单见 [C-6 spec](specs/C-6-FileDialog-SDL-Params/spec.md)）
- [x] **C-7** CMake 平台判定健壮化　｜ 实际约 0.25 天（原估 0.5 天）｜ 依赖：无 ｜ 完成日期：2026-10-20 ｜ ⚠️ macOS / Linux / FreeBSD 分支本机无法实机构建，需在对应平台按分支日志核对（见 C-7 小节）
- [x] **E-1** StatusBar 分格索引 API　｜ 极低 ｜ 完成日期：2026-10-18
- [x] **E-2** NavigationView 属性文档（中英 4 处）　｜ 极低 ｜ 完成日期：2026-10-18
- [ ] **E-3** 通用 Dialog 控件　｜ 约 600 行 ｜ 完成日期：______
- [x] **E-4** Pagination 分页器　｜ 约 250 行 ｜ 完成日期：2026-10-19
- [x] **E-5** ScrollBar 键盘导航 + 修 DPI TODO　｜ 低 ｜ 完成日期：2026-10-18（核实：现有代码已支持，无需开发）
- [x] **E-6** ListCtrl ②排序稳定性 + ③合并注释清理　｜ 低 ｜ 完成日期：2026-10-18（① 汇总行/分组行保持开放，未做）
- [x] **E-7** TabCtrl 垂直标签 / 固定标签 / 溢出滚动（增删 API 经核实已存在，不开发）　｜ 低 ｜ 完成日期：2026-10-18
- [ ] **E-8** MenuItem 勾选/单选/图标/快捷键 API　｜ 低 ｜ 完成日期：______（2026-10-18 仅清理了 `Menu.cpp:11` TODO 与 `Menu.h` 遗留注释，**API 一项未做**，见 E-8 行）
- [x] **D-1** RangeSlider 双滑块　｜ 约 400 行 ｜ 完成日期：2026-10-18
- [ ] **D-2** 富内容 Tooltip　｜ 约 200 行 ｜ 完成日期：______
- [ ] **D-3** SpinBox 小数与格式化　｜ 低 ｜ 完成日期：______
- [ ] **D-4** TreeView 节点拖拽重排　｜ 约 700 行 ｜ 完成日期：______
- [ ] **D-5** TimePicker 弹层 + DateTime 日期约束　｜ 约 800 行 ｜ 完成日期：______
- [x] **D-6** Stepper 向导步骤条　｜ 约 400 行 ｜ 完成日期：2026-10-19
- [ ] **D-7** ToolBar 工具项抽象　｜ 约 500 行 ｜ 完成日期：______
- [ ] **D-8** ColorPicker Alpha 通道 + 数值输入　｜ 约 400 行 ｜ 完成日期：______
- [ ] **D-9** CascadingSelect 级联选择器　｜ 约 600 行 ｜ 完成日期：______
- [ ] **D-10** WrapPanel 不等宽换行布局　｜ 约 400 行 ｜ 完成日期：______
- [x] **D-11** Carousel 图片轮播（提交 `46e92b35`，B7；同批完成 Pagination，见 E-4）　｜ 低 ｜ 完成日期：2026-10-19
  - [ ] D-11.1 Breadcrumb 面包屑导航（全仓无命中，**唯一剩余子项**）
- [ ] **D-12** 原生系统通知三平台通道　｜ 中高 ｜ 完成日期：______
- [x] **D-13** TreeGrid 树表格独立控件（B9，提交 `27b9a497`）　｜ 中 ｜ 完成日期：2026-10-19（ListCtrl 内嵌树 / 分组折叠 / L3 列筛选仍不做）
- [ ] **E-9** DateTime 编辑器抽象泄漏修正（不处理，架构债留待重构）　｜ 中 ｜ 完成日期：______
- [x] **E-10** SearchBox 防抖 + 建议/历史下拉整合（已完成 2026-10-18）　｜ 中低 ｜ 完成日期：2026-10-18
- [x] **E-11** Chart ①增量数据 API + ③坐标轴格式化回调（提交 `0d987cb6`）　｜ 约 600 行 ｜ 完成日期：2026-10-18（②导出 PNG/SVG、④拆分单文件，经用户决定不做）

### P3 — 架构与生态（目标：60–90 人天）

- [ ] **F-1** ListCtrl DataTemplate 机制　｜ 高 ｜ 依赖：无（B-1 暂缓；架构级重构建议改为「小步拆分 + 逐版回归示例」推进）｜ 完成日期：______
- [ ] **F-2** 无障碍基础（阶段 1 属性标注 + 焦点管理）　｜ 1–2 周 ｜ 完成日期：______
  - [ ] F-2.1 `accessibility` 属性（role/label/description）+ 焦点遍历顺序
  - [ ] F-2.2 Windows UI Automation 适配
  - [ ] F-2.3 macOS AX / Linux AT-SPI
- [ ] **F-3** RTL 布局（**先做决策**）　｜ 2–3 周 ｜ 完成日期：______
- [ ] **F-4** RichEdit 跨平台能力对齐　｜ 高 ｜ 依赖：C-2（B-1 暂缓）｜ 完成日期：______
- [ ] **H-1** `.clang-format` + `.clang-tidy`　｜ 3–5 天 ｜ 完成日期：______
- [ ] **H-2** 图片/解压炸弹防护　｜ 3–5 天 ｜ 完成日期：______
- [ ] **H-3** `docs/misc` 过时文档归档　｜ 1–2 天 ｜ 完成日期：______
- [ ] **H-4** AI Skill 目录单一化　｜ 2–3 天 ｜ 完成日期：______
- [ ] **H-5** 公共 PCH　｜ 1–2 天 ｜ 完成日期：______
- [ ] **H-6** `ListCtrl-Performance.md` 行号改函数名锚点　｜ 2–3 天 ｜ 完成日期：______
- [ ] **H-7** README 双语同步标记　｜ 10 分钟 ｜ 完成日期：______
- [ ] **H-8** 异常处理规范落地　｜ 5–8 天 ｜ 依赖：无（B-4 暂缓，两者不再合并）｜ 完成日期：______
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

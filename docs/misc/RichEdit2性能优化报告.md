# RichEdit2 文本绘制性能优化报告

本文针对 `RichEdit2`（自绘实现的编辑框控件）的文本绘制流程，完成性能基线采集、绘制流程与瓶颈分析、实现优化以及优化前后的对比评估。

| 关注点 | 主要实现文件 |
| --- | --- |
| 编辑框绘制入口 | `duilib/Control/RichEdit2.cpp`（`RichEdit2::Paint`） |
| 文本排版与行缓存 | `duilib/Control/RichEditData.cpp`（`CheckCalcTextRects` / `CalcTextRectsFull` / `CalcTextRectsPart`） |
| 格式化文本绘制 | `duilib/RenderSkia/DrawRichText.cpp`（`CreateDrawRichTextCache` / `DrawRichTextCacheData`） |
| 文本断行与字形测量 | `duilib/RenderSkia/DrawSkiaText.cpp`（`BreakText` / `MeasureText`） |
| 字体与回退字体管理 | `duilib/Core/FontManager.cpp`（`FallbackFontMgrImpl::CreateFallbackFont`） |

## 1. 结论摘要

1. 基线数据（Release，中文+英文与"中文+英文+大量 emoji"两类文本）：
   - 小文档稳态绘制：无 emoji **166.2 µs/帧**，含 emoji **423.7 µs/帧**；
   - 大文档重新排版：无 emoji **2570.9 µs/帧**，含 emoji **16785.9 µs/帧**（约为前者的 6.5 倍）。
2. 稳态绘制（缓存有效）的耗时**与可见字符数量近似成正比**（约 0.10~0.14 µs/字符），
   与文档总长度基本无关；含 emoji 文本更慢的主要原因是同样的行数里字符数多 3 倍以上（每行约 88 个 emoji），
   而不是"字体回退查询"本身（关闭回退后稳态耗时并未下降）。
3. 重新排版的耗时**与文档总字符数成正比**：含 emoji 时约为 0.164 µs/字符，无 emoji 时约为 0.081 µs/字符，
   即回退字符使单位字符的排版成本翻倍。`BreakText` 中的"回退字体解析 + 逐字形测量"是排版阶段的主要额外开销。
4. 实施 3 项优化：
   - `FontManager` 增加"按字符"的回退字体缓存，避免每次排版都扫描回退字体列表；
   - `DrawSkiaText::BreakText` 改为"先解析回退字体 → 同字体分组批量测宽 + 字形宽度缓存 → 再计算断点"，
     大幅减少 Skia 的 `getWidthsBounds` 调用次数；
   - `DrawRichText::DrawRichTextCacheData` 增加可视区域快速剔除，跳过视区外片段的坐标换算与对齐计算。
5. 优化后（12 轮交替、丢弃 2 轮冷启动、取中位数）：
   - `relayout_emoji_large` **16785.9 → 14375.8 µs（-14.4%）**，`relayout_emoji_small` **1030.6 → 867.4 µs（-15.8%）**；
   - `steady_emoji_xlarge` **561.0 → 517.7 µs（-7.7%）**，`scroll_zhen_large` **182.5 → 171.1 µs（-6.3%）**；
   - 中英文本与空文本场景在噪声带内（-5%~+3%），无回退文本路径无性能退化；关闭回退的对照实验中所有场景差异均在 ±6% 噪声带内。
6. 正确性：优化前后 4 组文本的排版尺寸签名完全一致（断行结果未改变），Release 下 `ctest` **15/15 全部通过**。

## 2. 测试环境与方法

- 平台：Windows x64，Release 配置（MSVC），Skia Raster 后端离屏画布（`CreateOffscreenRender`，800 x 600）
- 构建：`python3 build/build.py build --config Release --arch x64 --offline`
- 基准程序：`tests/bench_richedit2.cpp`（新增，不注册到 ctest，属于手工评估工具）
- 控件设置：`RichEdit2` 尺寸 800 x 600、`Microsoft YaHei` 16px、文本内边距 4px、多行 + 自动换行
- 字体回退：`Microsoft YaHei, SimSun, Segoe UI Emoji`（emoji 由 `Segoe UI Emoji` 提供）
- 指标定义：`每帧(us)` = 含"绘制 + 必要的排版 / 增量排版"的单次 `Paint` 平均耗时
- 抗噪方法：
  1. 采用**同一份基准源码分别链接"优化前/优化后"的 duilib 静态库**，得到 `bench_richedit2_base.exe` 与 `bench_richedit2_opt.exe`；
  2. 两个程序**交替运行 12 轮**（每轮每场景 200 次稳态迭代、50 次重新排版迭代）；
  3. 丢弃前 2 轮（进程冷启动、字体首次加载、CPU 频率爬升的影响），对剩余 10 个样本取**中位数**；
  4. 同时给出**最小值**作为交叉验证（最小值受后台进程干扰更小，通常更接近真实成本）。

> 说明：本机（8 GB 内存，同时运行 IDE 等后台进程）实测抖动较大，单次运行的不同场景可能出现 ±20% 波动；
> 因此本报告只采信"交替多轮 + 中位数/最小值"的对比结果，绝对值仅供参考。

复现命令：

```bat
rem 1) 构建 Release
python3 build/build.py build --config Release --arch x64 --offline

rem 2) 运行基准（默认 200 次迭代；--iterations 可调整）
out\x64\Release\tests\bench_richedit2.exe --iterations 200

rem 3) 关闭字体回退的对照实验（emoji 显示为缺字形占位）
out\x64\Release\tests\bench_richedit2.exe --iterations 200 --no-fallback
```

## 3. 基准程序与场景设计

### 3.1 驱动方式

`tests/bench_richedit2.cpp` 不创建真实窗口，而是用两个轻量桩对象直接驱动控件绘制：

| 类型 | 作用 |
| --- | --- |
| `BenchWindow : ui::Window` | 重写 `GetRender()`，返回离屏 `IRender`，使控件无需真实窗口即可绘制 |
| `BenchRichEdit2 : ui::RichEdit2` | 用 `using ui::RichEdit2::Paint;` 暴露 protected 的绘制入口，直接调用完整绘制流程 |

计时采用 `std::chrono::steady_clock`，每个场景先执行 `setup`（设置文本并完成首次排版、建立绘制缓存），
再预热 20 次后开始计时。

### 3.2 文本与字体

| 文本 | 行数 | 字符数（UTF-16） | 需回退字符占比 |
| --- | --- | --- | --- |
| 中文+英文（small） | 15 | 1184 | 0.0% |
| 中文+英文（large） | 400 | 31599 | 0.0% |
| 中文+英文（xlarge） | 2000 | 157999 | 0.0% |
| 中文+英文+emoji（small） | 15 | 3839 | 69.0% |
| 中文+英文+emoji（large） | 400 | 102399 | 69.0% |
| 中文+英文+emoji（xlarge） | 2000 | 511999 | 69.0% |

- 每行基础文本：`Duilib 文本绘制性能评估：简体中文与 English mixed text 0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ`；
- emoji 文本在每行后追加约 88 个增补平面 emoji（`😀` 等，非 BMP 字符），必须经字体回退才能绘制。

### 3.3 场景列表

| 场景 | 覆盖的绘制路径 |
| --- | --- |
| `steady_empty` | 空文本，仅控件自身绘制，用于扣除固定开销 |
| `steady_zhen_*` | 缓存有效的稳态绘制（文本不变，每帧仅绘制） |
| `steady_emoji_*` | 同上，但文本含大量需回退的 emoji |
| `scroll_zhen_large` / `scroll_emoji_large` | 每帧改变滚动偏移（缓存有效，仅改变绘制位置） |
| `typing_zhen_large` / `typing_emoji_large` | 每帧在文末插入 1 个字符，走**增量排版** `CalcTextRectsPart` |
| `relayout_zhen_*` / `relayout_emoji_*` | 每帧在 A/B 两份文本间切换，触发 `SetText` 的**全量排版** `CalcTextRectsFull` |

`relayout` 场景在 `textB` 末尾追加一个字符（emoji 场景追加 `😀`），使每帧都真正重排整篇文档。

### 3.4 排版尺寸签名

程序在计时前输出每类文本的 `CalcRequiredSize` 结果（`[layout signature]`），
用于校验优化前后的**断行位置与文本总尺寸是否完全一致**（宽度计算不能有偏差）。

## 4. 性能基线（优化前）

优化前（`bench_richedit2_base.exe`，字体回退开启，10 个有效样本的中位数）：

| 场景 | 字符数 | 每帧(us) | 说明 |
| --- | --- | --- | --- |
| `steady_empty` | 0 | 0.11 | 控件自身绘制固定开销 |
| `steady_zhen_small` | 1184 | 166.20 | 稳态绘制，无 emoji |
| `steady_zhen_large` | 31599 | 181.82 | 同上，大文档 |
| `steady_zhen_xlarge` | 157999 | 249.74 | 同上，超大文档（片段遍历项更多） |
| `steady_emoji_small` | 3839 | 423.74 | 稳态绘制，含大量 emoji |
| `steady_emoji_large` | 102399 | 449.08 | 同上，大文档 |
| `steady_emoji_xlarge` | 511999 | 560.96 | 同上，超大文档 |
| `scroll_zhen_large` | 31599 | 182.54 | 滚动绘制，无 emoji |
| `scroll_emoji_large` | 102399 | 446.28 | 滚动绘制，含 emoji |
| `typing_zhen_large` | 31599 | 243.98 | 输入字符（增量排版） |
| `typing_emoji_large` | 102399 | 589.35 | 输入字符（增量排版，含 emoji） |
| `relayout_zhen_small` | 1184 | 262.02 | 全量重新排版 |
| `relayout_zhen_large` | 31599 | 2570.94 | 全量重新排版，大文档 |
| `relayout_emoji_small` | 3839 | 1030.62 | 全量重新排版，含 emoji |
| `relayout_emoji_large` | 102399 | 16785.87 | 全量重新排版，大文档 + 含 emoji |

### 4.1 基线数据解读

1. **稳态绘制与可见字符数近似线性**：
   - 无 emoji：166.2 µs / 1184 字符 ≈ 0.140 µs/字符；大文档 181.8 µs（可见行数相同，耗时基本持平）；
   - 含 emoji：423.7 µs / 3839 字符 ≈ 0.110 µs/字符；
   - 两者单位字符成本接近，说明稳态绘制成本主要由"可见字符数 × 单字符绘制成本"决定，
     含 emoji 文本"更慢"首先是因为同样行数下字符数多 3 倍以上。
2. **超大文档稳态绘制的额外开销来自"片段数量"**：`xlarge` 比 `large` 多约 28%~25%，
   原因是绘制缓存中的 `TPendingDrawRichText` 片段数随文档长度增长，即使片段在视区外仍要执行坐标换算与对齐计算。
3. **重新排版成本与文档总字符数成正比**：
   - 无 emoji：2570.9 µs / 31599 字符 ≈ 0.081 µs/字符；
   - 含 emoji：16785.9 µs / 102399 字符 ≈ 0.164 µs/字符（约为前者 2 倍）；
   - 回退字符（69% 的字符需要回退）使单位字符的排版成本翻倍，是排版阶段的主要额外开销。
4. **增量排版（输入字符）明显便宜**：`typing_*` 只重排受影响的行，243.98 / 589.35 µs，
   远低于整篇重排（2570.94 / 16785.87 µs），说明"输入"不是瓶颈，整篇重排（`SetText`）才是。

### 4.2 "是否含字体回退"的对照（`--no-fallback`）

在同一份含 emoji 文本上关闭回退字体（emoji 退化为缺字形占位框）的基线数据：

| 场景 | 开启回退（us/帧） | 关闭回退（us/帧） | 备注 |
| --- | --- | --- | --- |
| `steady_emoji_small` | 423.74 | 524.13 | 稳态绘制未下降 |
| `steady_emoji_large` | 449.08 | 593.94 | 同上 |
| `steady_emoji_xlarge` | 560.96 | 640.98 | 同上 |
| `scroll_emoji_large` | 446.28 | 571.63 | 同上 |
| `typing_emoji_large` | 589.35 | 718.98 | 同上 |
| `relayout_emoji_small` | 1030.62 | 1004.58 | 排版耗时相当 |
| `relayout_emoji_large` | 16785.87 | 20763.04 | 排版耗时相当或更差 |

结论：

- 稳态绘制阶段**不存在"回退查询"瓶颈**（关闭回退不会更快），彩色的 emoji 字形光栅化并不是唯一成本，
  单字符绘制成本本身即占主导；
- 关闭回退后**排版结果改变**（`emoji_small` 尺寸签名 799 x 1328 → 798 x 998，`emoji_large` 799 x 35208 → 798 x 26408），
  即断行不同，因此该对照**只用于判断回退逻辑是否影响性能，不能作为产品配置**；
- 该对照同时证明：优化措施不应围绕"减少回退查询次数"做过度设计，而应聚焦"回退字符的逐字符处理路径"，
  以及"视区外片段的无效计算"。

## 5. 绘制流程与瓶颈分析

### 5.1 `RichEdit2::Paint` 流程

```
RichEdit2::Paint                              (RichEdit2.cpp)
 ├─ 脏区裁剪：IsClipEmpty / 与 box-shadow 区域求交
 ├─ Control::Paint                            控件背景、边框
 ├─ m_pTextData->CheckCalcTextRects()         文本区域脏时 → CalcTextRectsFull（全量排版）
 │    └─ CalcTextRectsFull → RichText::MeasureRichText → DrawSkiaText::BreakText（逐行断行、测宽）
 ├─ PaintCurrentRowBkColor                    当前行背景
 ├─ PaintSelectionColor                       选区背景
 ├─ GetRichTextForDraw(richTextDataList)      每帧构造绘制数据（UiFontEx 对象只创建一次）
 ├─ pRender->IsValidDrawRichTextCache(...)    绘制缓存是否有效
 ├─ 缓存无效：CreateDrawRichTextCache(...)     建立绘制缓存（glyph runs + 每个片段的目标矩形）
 └─ DrawRichTextCacheData(...)                逐片段：坐标偏移、行内对齐 → Skia 绘制字形
```

关键点：

- 文本内容变化才触发重新排版；单纯滚动、悬停、光标闪烁只走"缓存绘制"路径；
- `GetRichTextForDraw` 每帧都构造一次 `std::vector<RichTextData>`（小对象、可承受，但不是零成本）；
- `DrawRichTextCacheData` 会遍历**缓存中所有片段**（含视区外片段），对每个片段做偏移、行内对齐、
  裁剪判断后才绘制。

### 5.2 瓶颈定位（临时插桩结果）

在 Release 下用 `PerformanceUtil` 临时插桩（定位完成后已恢复，未保留在提交中）：

| 函数 | 平均耗时 | 构成 |
| --- | --- | --- |
| `RichEdit2::Paint` | 约 0.263 ms | 几乎全部为 `DrawRichTextCacheData`（Skia 字形绘制） |
| `RichEditData::CalcTextRectsFull` | 约 4.62 ms（最大 64 ms） | 几乎全部为 `DrawRichText::InternalDrawRichText` |
| `DrawSkiaText::BreakText` | 占排版 40%~60% | 回退字体解析 + 逐字形测宽 + 断点计算 |

进一步的验证实验：临时跳过"逐字符回退字形测量"后，`relayout_emoji_large` 由 13.5 ms 降到 8.1 ms，
但同时断行结果发生变化（`BreakText` 调用次数 54k → 45k），说明**该收益中有一部分来自少排了几行**，
必须在"保持排版结果完全一致"的前提下优化。

由此确定 3 个瓶颈点：

| # | 瓶颈 | 表现 |
| --- | --- | --- |
| B1 | 回退字体查询每次排版都重新扫描回退字体列表 | 69% 的字符（每行约 88 个 emoji）逐个查询；同一字符反复查询，结果恒定 |
| B2 | 回退字形逐个调用 `Skia getWidthsBounds` 测量 | 每次调用有固定开销；含大量 emoji 时调用次数与字符数同阶 |
| B3 | `DrawRichTextCacheData` 对视区外片段仍做完整处理 | 超大文档（2000 行 / 2000+ 片段）时，稳态绘制仍遍历全部片段 |

## 6. 优化计划与实现

### 6.1 优化计划

| 目标 | 措施 | 预期收益 | 风险控制 |
| --- | --- | --- | --- |
| B1 | `FallbackFontCacheEntry` 增加 `std::unordered_map<uint32_t, IFont*> m_charFontMap`，按 UTF32 字符缓存查询结果（含"未找到"的 nullptr） | 消除重复扫描回退字体列表 | 回退字体配置变化时清空缓存（`SetFallbackFontFamilyNames` 会调用 `ClearFontCache`） |
| B2 | `BreakText` 重构为三步：①解析回退字形（不测宽）②按命中字体分组批量测宽 + 线程内字形宽度缓存 ③计算断点 | 把 O(字符数) 次 Skia 测量调用合并为"连续同字体段"次 | 宽度缓存上限 16384 条；每字形宽度只与 typeface + 字号 + glyph 有关，缓存安全 |
| B3 | `DrawRichTextCacheData` 在 `pRichTextRects == nullptr` 时先做可视区域快速剔除 | 超大文档稳态绘制跳过大量无效片段 | 仅在不需要返回片段位置的调用中生效，避免影响需要全部片段位置的调用方 |

### 6.2 实现细节

#### 6.2.1 按字符缓存回退字体（`duilib/Core/FontManager.h/.cpp`）

- `FallbackFontCacheEntry` 新增：

```cpp
/** 每个字符对应的回退字体缓存：Key是UTF32字符编码，Value是对应的回退字体
    （nullptr表示没有找到可用的回退字体）。 */
std::unordered_map<uint32_t, IFont*> m_charFontMap;
```

- `CreateFallbackFont(fontKey, unicodeChar)` 的缓存命中分支先查 `m_charFontMap[unicodeChar]`，命中直接返回；
- 扫描回退字体列表命中时写入 `m_charFontMap`；新建回退字体成功后也写入（未找到时缓存 nullptr，
  避免同一缺失字符反复扫描）；
- `ClearFontCache()` 清空整个 `m_fallbackFontMap`（含字符缓存），因此切换回退字体配置后不会读到旧结果。

#### 6.2.2 `BreakText` 的回退字形批量测量与宽度缓存（`duilib/RenderSkia/DrawSkiaText.cpp/.h`）

原实现：在"宽度累加到上限"的循环内逐字符处理——遇到 `glyphID == 0` 就调用回退字体创建器，
再对单个字形调用 `getWidthsBounds`。该写法便于在达到宽度上限时提前结束，但每次 Skia 调用都有固定开销。

新实现分为三步：

1. **一次遍历**填充"每个字形实际命中的字体"（`glyphFonts`，新增到 `MeasureTextTempData`），
   同时检测是否存在缺失字形（`bHasMissingGlyph`）。纯中文/英文文本在此步即确定"无需回退"，
   不会引入额外遍历；
2. 存在缺失字形时：先逐字符解析回退字体与回退字形 ID（**不做测量**），再把"连续命中同一回退字体"的字形分组，
   对每组批量调用一次 `getWidthsBounds`；
   - 优先查线程内的字形宽度缓存 `g_fallbackGlyphWidthCache`（Key = `typeface uniqueID + 字号 + glyphID`），
     全命中时**完全跳过 Skia 测量**；
   - 未命中时批量测量后写回缓存，缓存上限 `kMaxFallbackGlyphWidthCacheSize = 16384`，防止异常文本导致无限增长；
   - 缓存使用 `thread_local`，避免多线程渲染时的数据竞争；
3. 最后按与原实现完全一致的规则"按宽度累加 + 达到上限提前结束"计算断点。

说明：批量测量会在断点之前多测量少量字形（原实现达到上限即停止），
但每个字形宽度只测量一次并进入缓存，重复排版（滚动、输入、逐帧重绘）时测量成本降为零，
整体收益为正；断点计算逻辑未改动，排版结果与优化前完全一致（见第 8 节签名校验）。

#### 6.2.3 可视区域快速剔除（`duilib/RenderSkia/DrawRichText.cpp`）

`DrawRichTextCacheData` 的片段循环开头增加：

```cpp
if (pRichTextRects == nullptr) {
    const int32_t nChunkTop = rcNewTextRect.top + textData.m_destRect.top - szNewScrollOffset.cy;
    if (nChunkTop >= rcNewTextRect.bottom) {
        continue;
    }
    if ((nChunkTop + textData.m_destRect.Height()) <= rcNewTextRect.top) {
        continue;
    }
}
```

- 只做一次加法与两次比较，即可跳过视区外片段的坐标偏移、行内对齐、裁剪等后续处理；
- 仅在 `pRichTextRects == nullptr`（调用方不需要返回每个片段位置）时生效，保证其他调用方行为不变。

### 6.3 未采用的方案

| 方案 | 未采用原因 |
| --- | --- |
| 缓存整行/整屏位图 | 内存占用随文档与缩放增长明显；滚动、光标闪烁、选区变化时命中率低 |
| 跳过回退字形的宽度测量（直接用占位宽度） | 会改变断行结果（实测 `BreakText` 调用次数由 54k 降为 45k），排版不正确 |
| 修改 Skia 字形光栅化 / 彩色 emoji 绘制 | 属于 Skia 内部实现，改动风险大，且稳态绘制成本与字符数线性相关，收益不确定 |
| 为 `BreakText` 计算结果做整段缓存 | 文本与宽度约束组合多，命中率与内存难以平衡；本次的字形级宽度缓存已覆盖主要重复成本 |

## 7. 优化前后对比（Release，12 轮交替、丢弃 2 轮冷启动、取中位数）

| 场景 | 基线 us/帧 | 优化后 us/帧 | 变化 |
| --- | --- | --- | --- |
| `steady_empty` | 0.11 | 0.12 | 持平（噪声） |
| `steady_zhen_small` | 166.20 | 162.04 | -2.5%（噪声带内） |
| `steady_zhen_large` | 181.82 | 176.60 | -2.9%（噪声带内） |
| `steady_zhen_xlarge` | 249.74 | 238.96 | -4.3% |
| `steady_emoji_small` | 423.74 | 437.72 | +3.3%（噪声带内） |
| `steady_emoji_large` | 449.08 | 444.24 | -1.1%（噪声带内） |
| **`steady_emoji_xlarge`** | **560.96** | **517.74** | **-7.7%** |
| **`scroll_zhen_large`** | **182.54** | **171.10** | **-6.3%** |
| `scroll_emoji_large` | 446.28 | 431.94 | -3.2% |
| **`typing_zhen_large`** | **243.98** | **231.33** | **-5.2%** |
| `typing_emoji_large` | 589.35 | 577.43 | -2.0% |
| `relayout_zhen_small` | 262.02 | 248.38 | -5.2% |
| `relayout_zhen_large` | 2570.94 | 2568.27 | 持平（-0.1%） |
| **`relayout_emoji_small`** | **1030.62** | **867.38** | **-15.8%** |
| **`relayout_emoji_large`** | **16785.87** | **14375.84** | **-14.4%** |

以"最小值"（受后台干扰更小）交叉验证关键场景：

| 场景 | 基线 min | 优化后 min | 变化 |
| --- | --- | --- | --- |
| `relayout_emoji_large` | 14732.15 | 13274.64 | -9.9% |
| `relayout_emoji_small` | 895.51 | 812.15 | -9.3% |
| `steady_emoji_xlarge` | 510.66 | 491.63 | -3.7% |
| `scroll_zhen_large` | 171.17 | 163.39 | -4.5% |
| `typing_zhen_large` | 229.33 | 219.45 | -4.3% |
| `relayout_zhen_large` | 2240.57 | 2314.50 | +3.3%（噪声） |

### 7.1 效果分析

1. **含大量 emoji 的全量重排收益最大（-14.4% ~ -15.8%）**：
   该场景 69% 的字符需要回退，且每帧都整篇重排，最能体现"分组批量测宽 + 字形宽度缓存 + 按字符回退字体缓存"的效果。
   以 `relayout_emoji_large` 为例，单位字符成本由约 0.164 µs 降到约 0.140 µs。
2. **超大文档稳态/滚动绘制有 4%~8% 收益**：主要来自可视区域快速剔除，
   文档越长、缓存片段越多，跳过视区外片段的效果越明显（`xlarge` 的收益高于 `large`）。
3. **中英文本（无回退）变化在噪声带内**：`steady_zhen_*` 仅 -2.5%~-4.3%，`relayout_zhen_large` 基本持平；
   说明优化没有触碰非回退路径的关键计算（新实现中"一次性填充 glyphFonts + 检测缺失字形"与断点计算保持等价）。
4. **增量输入场景略降（-2%~-5%）**：增量排版本身不涉及整篇重排，收益主要来自绘制阶段的视区剔除。
5. **关闭字体回退的对照实验中，优化前后所有场景差异均在 ±6% 噪声带内**（中位数），
   说明这些优化只作用于"存在回退字符"和"存在视区外片段"的路径，不会给无回退场景带来退化。

## 8. 正确性验证

### 8.1 排版尺寸签名（优化前 / 优化后）

| 文本 | 优化前 | 优化后 | 结论 |
| --- | --- | --- | --- |
| `zhen_small` | 798 x 668 | 798 x 668 | 一致 |
| `zhen_large` | 798 x 17608 | 798 x 17608 | 一致 |
| `emoji_small` | 799 x 1328 | 799 x 1328 | 一致 |
| `emoji_large` | 799 x 35208 | 799 x 35208 | 一致 |

宽度与总高度完全一致，说明分组批量测量得到的字形宽度与原逐字符测量结果逐位相同，断行位置未发生变化。

### 8.2 单元测试

```
ctest --test-dir out/x64/Release -C Release
100% tests passed, 0 tests failed out of 15
```

（含 `text_draw`、`rich_text`、`rich_text_util`、`render_api`、`offscreen_render` 等与本次改动相关的用例。）

## 9. 变更文件清单

| 文件 | 变更 |
| --- | --- |
| `duilib/Core/FontManager.h` | `FallbackFontCacheEntry` 新增按字符的回退字体缓存 `m_charFontMap` |
| `duilib/Core/FontManager.cpp` | `CreateFallbackFont` 优先命中字符缓存，扫描/新建后写入缓存（含 nullptr） |
| `duilib/RenderSkia/DrawSkiaText.h` | `MeasureTextTempData` 新增 `glyphFonts`（每个字形实际命中的字体） |
| `duilib/RenderSkia/DrawSkiaText.cpp` | `BreakText` 重构为"解析回退字形 → 分组批量测宽 + 字形宽度缓存 → 计算断点" |
| `duilib/RenderSkia/DrawRichText.cpp` | `DrawRichTextCacheData` 增加可视区域快速剔除 |
| `tests/bench_richedit2.cpp` | 新增 RichEdit2 文本绘制性能基准（含 `--no-fallback` 对照） |
| `tests/CMakeLists.txt` | 新增 `bench_richedit2` 目标（不注册到 ctest） |

## 10. 注意事项

1. Debug 配置含断言与未优化代码，绝对值不具参考意义；性能评估必须使用 Release。
2. 本机抖动较大（单次运行可能出现 ±20% 波动），建议使用"同一份基准分别链接优化前/优化后静态库、
   交替运行多轮、丢弃冷启动轮次、取中位数"的方式对比，并同时观察最小值。
3. 字形宽度缓存是**进程内、线程内**缓存（上限 16384 条），字体文件热替换后需要重新启动进程或
   调用 `ClearFontCache` 相关流程才会失效。
4. `--no-fallback` 只用于性能对照：关闭回退会改变排版尺寸与显示效果（emoji 变为缺字形方框），
   不能作为产品配置。
5. 本轮优化不改变 `RichEdit2` 的绘制结果与断行结果（已用尺寸签名校验），
   但批量测量会在断点前多测量少量字形；对超长单行文本（无换行的极端段落）会多一次分段测量，
   实际文档场景中由字形宽度缓存抵消。
6. 输入法组合文本、密码模式、光标/选区绘制等路径未改动。

## 11. 后续优化与最终性能（2026-11-16）

本报告的 3 项优化（分组批量测宽 + 字形宽度缓存 + 按字符回退字体缓存、视区外片段剔除、绘制缓存复用）之后，
RichEdit2 又陆续落地了以内存为主线的 6 项优化，实施细节、A/B 数据与逐场景分析见
《[RichEdit2内存占用分析报告.md](RichEdit2内存占用分析报告.md)》第 12 ~ 17 节（O1、O1.1、O2、O3、O4、O6），
最终复测见该文档**第 19 节**。下表只归纳"相对本报告第 4 节基线"的最终效果（DLL 模式 Release，同机，
`bench_richedit2` 12 轮弃前 2 轮取中位数）：

| 场景 | 第 4 节基线 us/帧 | 最终 us/帧 | 变化 |
| --- | --- | --- | --- |
| `steady_zhen_xlarge` | 249.74 | **171.47** | **-31.3%** |
| `steady_emoji_xlarge` | 560.96 | **418.79** | **-25.3%** |
| `steady_zhen_small` | 166.20 | 167.71 | +0.9%（噪声） |
| `steady_emoji_large` | 449.08 | 422.25 | -6.0%（噪声带内） |
| `scroll_zhen_large` | 182.54 | 167.70 | -8.1%（噪声带内偏快） |
| `scroll_emoji_large` | 446.28 | 399.15 | -10.6% |
| `typing_zhen_large` | 243.98 | **211.74** | **-13.2%** |
| `typing_emoji_large` | 589.35 | **504.42** | **-14.4%** |
| `relayout_zhen_large` | 2,570.94 | **1,636.67** | **-36.3%** |
| `relayout_emoji_large` | 16,785.87 | **11,993.14** | **-28.6%** |
| `relayout_zhen_small` | 262.02 | 299.69 | **+14.4%**（已知折中） |
| `relayout_emoji_small` | 1,030.62 | 1,179.99 | **+14.5%**（已知折中） |

- 大文档的稳态绘制、滚动、全量排版与输入路径全部优于本报告第 7 节的优化后数据；
- `relayout_*_small` 相对本报告基线偏高，是 O1 窗口化绘制缓存的结构性代价（小文档的窗口覆盖全部行，
  排版后 `Paint()` 会再建一次字形运行缓存），O1.2/O1.3 已决策不实施（见内存报告 12.5 节），
  绝对值仍在 1.2ms 以内，远低于 16.7ms 帧预算；
- 与本报告相关的前提结论不变：`[layout signature]` 仍为 `798x668 / 798x17608 / 799x1328 / 799x35208`，
  断行结果与优化前逐位一致；`ctest` 16/16 通过。

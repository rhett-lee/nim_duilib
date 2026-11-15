# RichEdit2 纯文本内存占用分析报告

本文针对 `RichEdit2`（自绘编辑框控件）在"设置纯文本（`SetText`）→ 全文排版 → 完成绘制"整个流程中的内存占用进行实测，
回答"每增加 1MB 纯文本，内存会增加多少"。

| 关注点 | 主要实现文件 |
| --- | --- |
| 编辑框入口与文本设置 | `duilib/Control/RichEdit2.cpp`（`RichEdit2::SetText` / `RichEdit2::Paint` / `RichEdit2::UpdateScrollRange`） |
| 文本存储与全文排版 | `duilib/Control/RichEditData.cpp`（`RichEditData::SetText` / `CalcTextRectsFull`） |
| 绘制数据准备 | `duilib/Control/RichEdit2.cpp`（`RichEdit2::GetRichTextForDraw`） |
| 绘制缓存与字形运行 | `duilib/RenderSkia/Render_Skia.cpp`（`CreateDrawRichTextCache`）、`duilib/RenderSkia/DrawRichText.cpp`、`duilib/RenderSkia/TextGlyphRun.h` |

## 1. 结论摘要

测试环境：Windows x64、Release（MSVC）、Skia Raster 离屏画布 800 x 600、`Microsoft YaHei` 16px、
多行 + 自动换行、文本内边距 4px、每行 64 个字符（含换行符）。

1. **每增加 1MB（1 MiB）UTF-16 纯文本（即 524,288 个字符），控件内存占用增加约 12MB**：
   - 线性回归（2MB、4MB、8MB、16MB 四个规模）：**12.06 MB / MB**，即约 **12.1 倍**；
   - 折算到字符：**每字符约 24 字节**（`12.06 × 1048576 / 524288 = 24.1`）；
   - 若不排除调用方持有的原始文本（1MB），总占用增加约 **13.1 倍**。
2. 内存随文本规模**线性增长**，无超线性突变：

   | 文本规模（UTF-16） | 控件内存增量（不含调用方文本） | 放大倍数 |
   | --- | --- | --- |
   | 2MB | 24.70 MB | 12.35 |
   | 4MB | 50.06 MB | 12.51 |
   | 8MB | 99.13 MB | 12.39 |
   | 16MB | 193.92 MB | 12.12 |

3. 内存的绝大部分不是"文本本身"，而是**全文排版数据 + 绘制缓存**：
   - 每字符固定 12 字节结构数据：行文本副本 2B + 字符位置信息（`RichTextCharInfo`）4B + 字形 ID 与步进 6B；
   - 此外每行还有数百字节的固定开销（行对象、绘制片段对象、字形运行容器、容器容量冗余、堆块对齐），
     因此**行越短，单位字符成本越高**：32 字符/行约 34B/字符（17.2 倍），64 字符/行约 24B/字符，
     128/256 字符/行约 23B/字符（11.5~12.2 倍）；
   - 若文本中混入 emoji 等增补平面字符，字形数量与回退字体缓存还会额外增加（未计入本报告的纯文本基线）。
4. **分配时机**：`RichEdit2::SetText` 内部会触发 `Redraw()`、光标定位（`InternalSetSel`）与 `UpdateScrollRange()`，
   这些操作会同步完成**全量排版**，并在多数情况下同时建立**绘制缓存**，因此 `SetText` 返回时内存已基本到位；
   首次 `Paint` 仅补充少量字形运行数据（16MB 文档约 2.3MB），**稳态重复绘制不再增长（0MB）**。
5. 结论：RichEdit2 适合中小规模文本；对 **2MB 以上的纯文本**应评估内存预算
   （2MB 约 25MB、8MB 约 100MB、16MB 约 200MB），并考虑分页、分片或按需加载。

> 说明：文中的"1MB 纯文本"默认指 **1 MiB 的 UTF-16 内存文本**（524,288 个字符），
> 这是控件实际持有的文本形态。若以磁盘上的 UTF-8 文件大小衡量，请参考第 6 节的换算。

## 2. 测试环境与方法

- 平台：Windows x64，Release 配置（MSVC），DLL 模式（`duilib.dll`）
- 渲染：Skia Raster 后端，离屏画布 `CreateOffscreenRender`，800 x 600
- 构建：
  ```bat
  python3 build/build.py build --config Release --arch x64 --offline --dll --target bench_richedit2_memory
  ```
- 基准程序：`tests/bench_richedit2_memory.cpp`（本次新增，不注册到 ctest，属于手工评估工具）
- 控件配置：`RichEdit2` 尺寸 800 x 600、`Microsoft YaHei` 16px、文本内边距 4px、行间距系数 1.0、
  多行 + 自动换行，回退字体 `Microsoft YaHei, SimSun`
- 内存指标（Windows `GetProcessMemoryInfo`）：
  - **主指标**：`PrivateUsage`（进程私有提交内存），即任务管理器中"提交大小"的私有部分，
    包含堆、字体对象、Skia 内部缓存；
  - 辅助指标：`WorkingSetSize`（常驻物理内存）、`PeakWorkingSetSize`（峰值工作集）。
- 抗噪方法：
  1. **每个文本规模在独立进程中运行**，避免不同规模的堆复用互相污染；
  2. 先设置一小段同样字符集的文本完成排版与绘制，使字体对象、Skia 字形缓存等"与规模无关"的内存提前分配，
     再清空文本采集空白基线；
  3. 分阶段采样：空白基线 → 生成调用方文本 → `SetText` 后 → 首次绘制后 → 稳态绘制后；
  4. 每个规模重复 3 次，1MB 规模的波动约 ±0.8MB（约 6%），2MB 及以上波动 ≤2%。

### 2.1 测试文本

- 行模板：`Duilib RichEdit2 纯文本内存评估：简体中文与 English mixed text 0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ。`
- 每行固定 64 个字符（含行尾 `\n`），整篇文本由同一行模板重复拼接，字符集恒定（字形缓存为常量）；
- 本样本的 UTF-8 / UTF-16 字节比为 **0.703**（英文占比较高），用于第 6 节的换算。

## 3. 实测数据

表 3-1：内存随纯文本规模的变化（Release，每行 64 字符，单位 MB）

| 文本规模（UTF-16） | 字符数 | 行数 | SetText 后增量 | 首次绘制增量 | 控件合计（不含调用方文本） | 含调用方文本合计 | 稳态工作集 | 峰值工作集 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0.25MB | 131,072 | 2,048 | 3.20 | 0.29 | 3.48 | 3.74 | 19.2 | 19.4 |
| 1MB | 524,288 | 8,192 | 12.2 ~ 13.2 | 0 ~ 1.04 | 12.3 ~ 13.2（中位数 13.1） | 13.3 ~ 14.2 | 29.6 | 30.6 |
| 2MB | 1,048,576 | 16,384 | 24.41 | 0.29 | 24.70 | 26.71 | 41.9 | 45.1 |
| 4MB | 2,097,152 | 32,768 | 49.64 | 0.42 | 50.06 | 54.07 | 68.4 | 74.6 |
| 8MB | 4,194,304 | 65,536 | 97.56 | 1.57 | 99.13 | 107.15 | 120.8 | 133.4 |
| 16MB | 8,388,608 | 131,072 | 191.65 | 2.27 | 193.92 | 209.95 | 221.5 | 249.8 |

> 空白基线（空文本、控件已创建、字体与渲染缓存已就绪）约为 6.2~6.7MB 私有提交内存、15.5~16.3MB 工作集。
> 表中"稳态工作集/峰值工作集"为进程绝对值。

表 3-2：线性回归（2MB、4MB、8MB、16MB）

| 项目 | 结果 |
| --- | --- |
| 斜率（每 1MB UTF-16 文本的内存增量） | **12.06 MB / MB** |
| 每字符内存 | **24.1 字节 / 字符** |
| 固定偏移（截距） | 约 1.5MB（容器容量冗余、进程级差异等） |
| 与文本大小的放大倍数 | **约 12.1 倍** |

补充：1MB 规模的绝对增量存在 ±0.8MB 抖动（是否在 `SetText` 阶段就建立绘制缓存会影响阶段归属），
因此回归只使用 2MB 及以上的样本。

## 4. 内存构成分析

### 4.1 流程与分配点

```
RichEdit2::SetText(strText)                       (RichEdit2.cpp:1173)
 ├─ DStringW text = strText;                      临时整篇副本（调用方文本的 1 份）
 ├─ RichEditData::SetText(text)                   (RichEditData.cpp:785)
 │    ├─ SplitLines(...)                          行视图列表（16B/行，临时）
 │    └─ 每行复制为 m_lineTextInfo[i]->m_lineText 行文本副本（2B/字符 + 每行分配头）
 ├─ Redraw()                                      标记脏区
 ├─ InternalSetSel(文本末端)                       光标定位 → 触发 CheckCalcTextRects
 └─ UpdateScrollRange() → SetPos()                滚动范围 → 触发全文测量
        └─ CalcTextRectsFull()                     (RichEditData.cpp:351)
             ├─ 每行 RichTextLineInfo（40B）+ 行内 RichTextRowInfo（56B）
             ├─ 每行 std::vector<RichTextCharInfo>  4B/字符（字符矩形/字体序号）
             └─ DrawRichText 建立绘制缓存
                  ├─ 每行 RichTextData（88B）       绘制用的行数据
                  ├─ 每行 TPendingDrawRichText      目标矩形 + 字形运行列表
                  └─ 每行 TextGlyphRunList          字形 ID（2B）+ 步进（4B）/字符
RichEdit2::Paint(...)                             (RichEdit2.cpp:1931)
 └─ 缓存有效：仅按片段绘制；缓存缺失：补齐字形运行（大文档约 0~2.3MB）
```

因此，`SetText` 返回后内存占用已基本达到峰值；之后的绘制（滚动、光标闪烁）不再随文本规模增长。

### 4.2 结构性数据（理论下界）

| 数据 | 单位大小 | 说明 |
| --- | --- | --- |
| 行文本副本 | 2 B/字符 | `RichTextLineInfo::m_lineText`（`std::wstring`） |
| 字符位置信息 | 4 B/字符 | `RichTextCharInfo`（实测 `sizeof = 4`） |
| 字形 ID + 步进 | 6 B/字符 | `SkGlyphID`（2B）+ `SkScalar`（4B），BMP 字符 1 字符 = 1 字形 |
| 合计 | **12 B/字符** | 不含行对象、容器与堆开销 |

按 12B/字符计算，1MB UTF-16 文本（524,288 字符）的数据下界约 6.3MB；
实测为 12.1MB，多出的约 5.8MB 来自：

- 每行固定对象与容器：`RichTextLineInfo`（40B）、`RichTextRowInfo`（56B）、`RichTextData`（88B）、
  绘制片段 `TPendingDrawRichText`、`TextGlyphRunList` 及其运行数据、行内行指针向量等；
- 容器容量冗余：`FastVector` 按 1.5 倍扩容、`std::vector` 按 2 倍扩容，容量可能大于实际元素数；
- 堆块对齐与分配头：大量小对象（每行文本、字形缓冲、字符向量）由 Windows 堆按 16B 对齐管理；
- `SetText` 过程中的临时缓冲（整篇副本、行视图列表、测量用临时 vector）会推高进程提交内存，
  释放后通常也不会立即归还操作系统，仍计入"内存占用"。

> 结构体尺寸为本次实测（x64 Release）：`RichTextCharInfo` 4B、`RichTextLineInfo` 40B、
> `RichTextRowInfo` 56B、`RichTextData` 88B。

### 4.3 行长的影响

同样 1MB 文本，改变每行字符数（表 4-1）：

| 每行字符数（含 `\n`） | 行数 | 控件内存增量 | 每字符字节 | 放大倍数 |
| --- | --- | --- | --- | --- |
| 32 | 16,384 | 17.22 MB | 34.4 | 17.2 |
| 64 | 8,192 | 12.3 ~ 13.2 MB | 24.4~26.7 | 12.3~13.2 |
| 128 | 4,096 | 12.18 MB | 24.4 | 12.2 |
| 256 | 2,048 | 11.53 MB | 23.1 | 11.5 |

结论：**行越短，单位字符的内存成本越高**（每行固定开销被摊到更少的字符上）。
对以短行（如聊天记录、日志）为主的大文本，内存放大倍数会明显高于 12 倍。

### 4.4 与源码经验注释的差异

`RichEdit2::SetText` 中的历史注释记录"2MB 的 UTF16 格式文本，Release 版本占用约 12MB 内存"，
而本次实测 2MB 文本对应约 24.7MB。差异可能来自统计口径（是否包含绘制缓存、临时堆提交、
进程基线）与实现演进。本报告给出的是**可复现的端到端实测**：从 `SetText` 到首次绘制完成，
进程私有提交内存的净增量。

## 5. 对使用方的建议

1. **控制单文档规模**：按 12 倍估算内存预算，2MB 文本约 25MB、8MB 约 100MB、16MB 约 200MB；
   32 位进程或低配设备上建议单文档不超过 2~4MB。
2. **大文本优先分页/分片**：只读展示场景建议按页、按段落或按行窗口加载，避免一次性 `SetText` 整篇文本。
3. **避免反复设置大文本**：每次 `SetText` 都会触发全文重排并重建绘制缓存，内存与耗时都会出现一次尖峰。
4. **注意行数**：短行文本（如日志）比长行文本的单位字符成本高 40% 以上；必要时限制单行长度。
5. **含 emoji/生僻字时额外评估**：字形回退会增加字形数量与回退字体缓存，本报告数据未包含这部分。
6. **如需优化实现**（不在本次范围内）：绘制缓存按可视区懒加载、行文本与调用方数据共享、
   压缩 `RichTextCharInfo`（4B → 2~3B）、限制单行字符数等，均可显著降低放大倍数。

## 6. "1MB 纯文本"的换算

每字符约 24.1 字节内存，因此：

| 文本形态 | 1MB 文本对应字符数 | 预计内存增量 |
| --- | --- | --- |
| UTF-16 内存文本（本报告基准） | 524,288 | **约 12.1MB** |
| UTF-8 纯 ASCII | 1,048,576 | 约 25.3MB |
| UTF-8 纯中文（3B/汉字） | 约 349,525 | 约 8.4MB |
| UTF-8 中英混排（本测试样本，0.703 字节比） | 约 745,700 | 约 18.0MB |

> 若业务口中的"1MB 文本"指文本文件大小，请按上表换算；控件真正关心的是字符数。

## 7. 复现方法

```bat
rem 1) 构建（Release + DLL 模式）
python3 build/build.py build --config Release --arch x64 --offline --dll --target bench_richedit2_memory

rem 2) 运行单个规模（1 MB = 524,288 个字符，每行 64 个字符）
out\x64\Release\tests\bench_richedit2_memory.exe --mb 1

rem 3) 其他规模
out\x64\Release\tests\bench_richedit2_memory.exe --mb 2
out\x64\Release\tests\bench_richedit2_memory.exe --mb 4
out\x64\Release\tests\bench_richedit2_memory.exe --mb 8
out\x64\Release\tests\bench_richedit2_memory.exe --mb 16

rem 4) 调整行长（观察行固定开销的影响）
out\x64\Release\tests\bench_richedit2_memory.exe --mb 1 --chars-per-line 32
out\x64\Release\tests\bench_richedit2_memory.exe --mb 1 --chars-per-line 128
out\x64\Release\tests\bench_richedit2_memory.exe --mb 1 --chars-per-line 256
```

DLL 模式下运行前需要让进程找到 `duilib.dll`（例如把 `bin` 目录加入 `PATH`）。

## 附录 A：原始输出示例（2MB）

```
==== RichEdit2 plain text memory benchmark ====
build config      : Release
render size       : 800 x 600 (raster, offscreen)
primary font      : Microsoft YaHei, 16px
fallback fonts    : Microsoft YaHei,SimSun
text size         : 2.00 MB (UTF-16, 1 MB = 1048576 bytes = 524288 chars)
chars per line    : 64 (including '\n')
steady paints     : 1 (after the first paint)

[stage] empty,6660096,16306176,16498688,6660096
[stage] text_created,8765440,18419712,18423808,8765440
[stage] after_settext,34365440,43696128,47247360,34365440
[stage] after_first_paint,34664448,43970560,47247360,34664448
[stage] after_steady_paints,34664448,43970560,47247360,34664448

[root] mb,2.000,chars,1048576,lines,16384,utf16_bytes,2097152,utf8_bytes,1474560
[delta] control_text_copy,25600000,24.414
[delta] layout_and_first_paint,299008,0.285
[delta] steady_paints,0,0.000
[delta] control_total_excluding_caller_text,25899008,24.699
[delta] control_total_including_caller_text,28004352,26.707
[delta] caller_text_buffer,2105344,2.008
[peak] working_set_peak,47247360,45.059
[ratio] per_char_bytes,24.70
[ratio] control_vs_utf16_text,12.350
[ratio] control_vs_utf8_text,17.564
[size] RichTextCharInfo,4,RichTextData,88,RichTextRowInfo,56,RichTextLineInfo,40
```

`[stage]` 各列为：阶段名、私有提交内存（字节）、工作集（字节）、峰值工作集（字节）、页面文件占用（字节）。

## 8. 内存占用高的根因分析

### 8.1 成本分解

以 1MB 文本（524,288 字符、每行 64 字符、共 8,192 行）为例，实测控件增量约 12.1MB，成本构成估算如下：

| 成本项 | 估算大小 | 占比 | 说明 |
| --- | --- | --- | --- |
| 字形运行缓存（`TextGlyphRunList` + 字形/步进缓冲） | 约 4.7 ~ 5.0MB | 约 40% | 全文每个字符 2B 字形 ID + 4B 步进，`FastVector` 按 1.5 倍扩容，容量冗余推高到约 9B/字符 |
| 待绘制片段（`TPendingDrawRichText`） | 约 1.1MB | 约 9% | 每个逻辑行约 136B：文本视图、目标矩形、字体 `shared_ptr`、颜色/属性、字形运行引用 |
| 绘制行数据（`RichTextData`） | 约 0.7MB | 约 6% | 每个物理行 88B（实测 `sizeof = 88`），含文本视图、两种颜色、字体 `SharePtr`、对齐/缩进等 |
| 字符排版数据（`RichTextCharInfo`） | 约 2.1MB | 约 17% | 每个字符 4B（实测 `sizeof = 4`），包含字符矩形/字号等紧凑属性 |
| 行/逻辑行对象 | 约 0.8MB | 约 7% | `RichTextLineInfo` 40B/物理行 + `RichTextRowInfo` 56B/逻辑行 + 指针向量 |
| 行文本副本 | 约 1.35MB | 约 11% | 每行 `std::wstring` 副本 2B/字符 + 每行独立堆分配头/对齐（约 16~32B/行） |
| 其它 | 约 0.7MB | 约 6% | 行偏移向量、容器容量、堆对齐、`SetText` 临时缓冲（整篇副本、行视图、测量临时 vector） |

**约 55% 的内存（字形运行 + 待绘制片段 + 绘制行数据）只服务于"绘制"，且是为全文每一行准备的；
其中真正与排版强相关的只有约 24%，文本本身只占约 11%。**

### 8.2 为什么会为全文构建绘制缓存

`RichEdit2::SetText` 在文本变化后会执行 `Redraw()`、光标定位 `InternalSetSel()` 与 `UpdateScrollRange()`，
后者会触发 `CheckCalcTextRects()` → `CalcTextRectsFull()` 完成全文排版，
随后 `Render_Skia::CreateDrawRichTextCache()` → `DrawRichText::CreateDrawRichTextCache()`
**按传入的完整文本区域遍历全部物理行与逻辑行**，为每一行生成：

1. `TPendingDrawRichText`（目标矩形、字体、颜色、字形运行引用）；
2. `TextGlyphRunList`（字形 ID + 步进，即"已经解析好的绘制数据"）。

因此即使 800 x 600 的窗口一屏只能显示约 30 行，8,192 行的文档也会为全部 8,192 行保存字形运行数据。
这是内存放大到 12 倍的首要原因，也是**唯一可以大幅下降而不影响排版正确性**的部分。

## 9. 优化方案

### 9.1 方案清单（按收益排序）

| # | 措施 | 预期每 1MB 文本内存 | 预期降幅 | 风险/成本 | 优先级 |
| --- | --- | --- | --- | --- | --- |
| O1 | **绘制缓存按可视区域懒构建**：只为可见行（+预取 1 屏）生成 `TPendingDrawRichText` 与 `TextGlyphRunList`，离屏行只保留排版数据；滚动时按需增量构建并淘汰离屏行 | 约 6.0 ~ 6.5MB | 约 -45% ~ -50% | 中：需处理滚动重建、缓存失效与选中/光标路径；滚动时有一定 CPU 成本（可按行预取平滑） | 高 |
| O2 | **行结构瘦身**：`std::vector<SharePtr<RichTextRowInfo>>` 改为 `std::vector<RichTextRowInfo>`；`RichTextData` 用字体索引/颜色索引替代 `SharePtr<UiFontEx>`(16B) 与两个 `UiColor`，目标 88B → 约 40B | 约 5.5 ~ 6.0MB | 约 -5% | 低：改动集中在 `RichEditData` 与 `GetRichTextForDraw`，需保证富文本字体/颜色语义不变 | 中 |
| O3 | **行文本共享**：`RichTextLineInfo::m_lineText` 改为"整篇文本 + (offset, len) 视图"，同时去掉 `RichEdit2::SetText` 中的整篇临时副本（`DStringW text = strText;`） | 约 5.3 ~ 5.8MB | 约 -3%（峰值额外 -1 倍文本） | 中：编辑/替换文本时视图失效需重建；需谨慎处理单行模式规整 | 中 |
| O4 | **压缩字符信息**：`RichTextCharInfo` 由 4B/字符 压缩为行内 x 偏移（uint16）+ 属性位，约 2.5B/字符 | 约 4.5 ~ 5.0MB | 约 -8% | 中高：需要评估超大行宽、字体混排与对齐精度 | 低 |
| O5 | **大文本模式/分页 API**：提供分段加载（只保留窗口附近若干段），或允许设置"仅排版可视区域、滚动时增量排版" | 可降到 1 ~ 2 倍文本 | 约 -80% 以上 | 高：涉及控件模型与滚动条、选中、查找等全部功能 | 长期 |
| O6 | **内存归还**：文本清空/控件隐藏/缓存失效时显式释放绘制缓存并 `shrink_to_fit`；复用 `GetRichTextForDraw` 的 `vector<RichTextData>` 而非每帧新建 | 峰值下降，稳态持平 | 约 -5% ~ -10%（消除峰值常驻） | 低 | 中 |

> O1、O2、O3 组合后，预计可从 **12.1MB/MB 降到约 5.5 ~ 6.5MB/MB（约 -45% ~ -55%）**；
> 再叠加 O4 可到约 5MB/MB。O5 是数量级方案，但工作量和回归面最大。
>
> 状态：**O1、O2、O6 已实施**，实现方式与实测结果分别见第 12、13、14 节。

### 9.2 O1 的实现要点

> 状态：**已实施**（2026-11-15），实现方式与实测结果见第 12 节。

1. `DrawRichTextCache` 增加"已构建行区间"（如 `m_nCacheRowBegin` / `m_nCacheRowEnd`）；
   `CreateDrawRichTextCache()` 接收当前可视矩形（`Paint` 中的 `rcPaint`，而不是全文文本区域），
   只对可视行及其前后各 1 屏的行构建 `TPendingDrawRichText` 与 `TextGlyphRunList`；
2. `DrawRichTextCacheData()` 只遍历已构建区间；
3. 滚动位置变化超过预取边界时，按行增量补建/释放（可复用现有行循环逻辑，以行号区间参数化）；
4. 文本内容或宽度变化时整体失效重建；
5. 选中区、光标、查找定位仍使用 `RichTextCharInfo` 排版数据，不依赖绘制缓存，行为不变；
6. 与之前"视区外片段快速剔除"的优化区别：那项只省 CPU（仍遍历全部片段），O1 才真正省内存。

### 9.3 验证方法

- 内存：用本报告的 `tests/bench_richedit2_memory.exe`，对比优化前后 1/2/4/8/16MB 的
  `control_total_excluding_caller_text`；
- 性能：用 `tests/bench_richedit2.exe` 观察 `scroll_*`、`steady_*` 场景，确认滚动重建未造成明显回退；
- 正确性：对比优化前后的排版尺寸签名（`[layout signature]`）与选中/光标位置，确保断行结果一致。

### 9.4 建议的实施顺序

1. 先做 **O6**（低风险，立刻降低峰值常驻）；
2. 再做 **O1**（收益最大，独立性强）；
3. 随后做 **O2 + O3**（结构瘦身，为 O4/O5 打基础）；
4. O4、O5 视产品对超大文本的支持需求再评估。

## 10. 性能优化提交 5037729 的内存影响（A/B 实测）

提交 `5037729a perf(richedit): 优化 RichEdit2 文本绘制与回退字形排版性能` 常被怀疑与内存放大有关，因此做一次只针对该提交的 A/B 实测。

### 10.1 方法

- 保持基准程序与构建配置不变（Release、x64、DLL 模式、800 x 600 离屏、每行 64 字符）；
- 仅把该提交涉及的 5 个文件（`FontManager.h/.cpp`、`DrawSkiaText.h/.cpp`、`DrawRichText.cpp`）
  回退到父提交 `e6234658`，重新构建 `duilib.dll` 后测量；
- 测量完成后恢复 HEAD 源码并复测，确认数值可复现。

### 10.2 结果

| 文本规模 | 父提交（优化前） | 5037729（优化后） | 变化 |
| --- | --- | --- | --- |
| 2MB | 24.66 MB | 24.70~24.79 MB | +0.04 ~ +0.13 MB（+0.2% ~ +0.5%） |
| 4MB | 49.90 MB | 50.06 MB | +0.15 MB（+0.3%） |
| 8MB | 99.29 MB | 99.13~99.16 MB | -0.13 ~ -0.16 MB（-0.2%） |
| 16MB | 194.58 MB | 193.92 MB | -0.66 MB（-0.3%） |

**结论：纯文本（不触发字体回退）场景下，该提交的内存变化在 ±0.5% 噪声范围内，可视为没有增加。**
12 倍放大的主因（全文每行缓存 `TPendingDrawRichText` + `TextGlyphRunList`）在父提交中已经存在
（`m_spGlyphRuns` 字段于 `5037729^` 即已存在），并非该提交引入。

### 10.3 该提交新增的内存点及上界

| 新增项 | 位置 | 内存上界 | 说明 |
| --- | --- | --- | --- |
| `m_charFontMap`（按字符的回退字体缓存） | `FontManager.h` | 约 40 ~ 48B / 不同回退字符 | 仅对需要回退的字符插入；普通中英文（微软雅黑可覆盖）为 0；上万生僻字最多约 0.5MB |
| `g_fallbackGlyphWidthCache`（线程内字形宽度缓存） | `DrawSkiaText.cpp` | **约 0.65MB / 线程封顶**（16384 条） | 只在测量回退字形宽度时增长；典型 emoji 文本约 100 个不同字形，约 4KB |
| `MeasureTextTempData::glyphFonts` | `DrawSkiaText.h` | 8B / 字形，**函数级临时** | 随 `BreakText` 返回释放，不常驻 |
| 可视区快速剔除 | `DrawRichText.cpp` | 0 | 纯计算优化，无新增分配 |

因此该提交是"用极小的、有明确上界的缓存换排版性能"：不需要回退字体的纯文本内存开销为 0，
含 emoji/生僻字时最多增加约 0.7MB/线程。若要降低整体内存，应针对第 9 节的 **O1（绘制缓存按可视区懒构建）**。

## 11. 性能基线（2026-11-15）

本节按《[RichEdit2性能优化报告.md](RichEdit2性能优化报告.md)》与《[字体回退性能优化分析.md](字体回退性能优化分析.md)》
定义的方法重跑两个性能基准，把结果固化为**长期基线**。以后修改 RichEdit2 绘制/排版或字体回退相关代码时，
在**同一台机器、同一构建配置**下复跑本节的命令，与本节数据逐行对比，即可判断是否发生性能降级。

### 11.1 采集环境

| 项目 | 配置 |
| --- | --- |
| 采集日期 | 2026-11-15 |
| 平台 | Windows x64，Release（MSVC），**DLL 模式（`duilib.dll`）** |
| 渲染 | Skia Raster 后端，`CreateOffscreenRender`，800 x 600 |
| 构建 | `python3 build/build.py --offline --dll`（Release） |
| `bench_richedit2` 字体 | 主字体 `Microsoft YaHei` 16px；回退链 `Microsoft YaHei, SimSun, Segoe UI Emoji` |
| `bench_text_render` 字体 | 主字体 `Arial` 16px；回退链 `Microsoft YaHei, SimSun, Segoe UI Emoji`；回退字符占比 **25.0%**（要求 ≥20%） |
| 迭代设置 | 每场景预热 20 次、统计 200 次 |

> 两个基准程序均不注册到 ctest，属手工评估工具，输出为可直接引用的 Markdown 表行。

### 11.2 抗噪方法

- **`bench_richedit2`**：连续运行 **12 轮**，**丢弃前 2 轮**（进程冷启动、字体首次加载、CPU 频率爬升），
  对剩余 10 轮取**中位数**作为主值，并同时给出**最小值**（受后台进程干扰更小）作交叉验证；
- **`bench_text_render`**：连续运行 **3 轮**，每场景取**最小值**（该基准单次耗时短、对瞬时抖动更敏感）。
- 判定规则：同一环境复跑时，某场景中位数变化超出 **±10%**（`relayout_*`、`steady_*_small` 等短场景到 **±20%**）
  才视为真实性能变化；否则计入噪声。

### 11.3 RichEdit2 绘制性能基线（`bench_richedit2`）

单位：`每帧(us)` 为单次 `RichEdit2::Paint` 的平均耗时。

| 场景 | 说明 | 字符数 | 每帧中位数(us) | 每帧最小值(us) | 每千字符(us) |
| --- | --- | --- | --- | --- | --- |
| `steady_empty` | 稳态绘制：空文本（控件基础开销） | 0 | 0.11 | 0.11 | — |
| `steady_zhen_small` | 稳态绘制：中文+英文（小文档） | 1,184 | 158.84 | 150.08 | 134.16 |
| `steady_zhen_large` | 稳态绘制：中文+英文（大文档） | 31,599 | 178.35 | 159.73 | 5.64 |
| `steady_zhen_xlarge` | 稳态绘制：中文+英文（超大文档） | 157,999 | 234.62 | 227.63 | 1.48 |
| `steady_emoji_small` | 稳态绘制：中文+英文+大量 emoji（小文档） | 3,839 | 384.06 | 374.50 | 100.04 |
| `steady_emoji_large` | 稳态绘制：中文+英文+大量 emoji（大文档） | 102,399 | 408.40 | 396.63 | 3.99 |
| `steady_emoji_xlarge` | 稳态绘制：中文+英文+大量 emoji（超大文档） | 511,999 | 483.83 | 474.44 | 0.94 |
| `scroll_zhen_large` | 滚动绘制：中文+英文（大文档） | 31,599 | 174.94 | 163.14 | 5.54 |
| `scroll_emoji_large` | 滚动绘制：中文+英文+大量 emoji（大文档） | 102,399 | 412.90 | 401.80 | 4.03 |
| `typing_zhen_large` | 输入字符：中文+英文（增量排版） | 31,599 | 231.64 | 217.21 | 7.33 |
| `typing_emoji_large` | 输入字符：中文+英文+大量 emoji（增量排版） | 102,399 | 552.30 | 510.54 | 5.39 |
| `relayout_zhen_small` | 全量重新排版：中文+英文（小文档） | 1,184 | 236.79 | 221.05 | 199.99 |
| `relayout_zhen_large` | 全量重新排版：中文+英文（大文档） | 31,599 | 2,285.42 | 2,186.39 | 72.33 |
| `relayout_emoji_small` | 全量重新排版：中文+英文+大量 emoji（小文档） | 3,839 | 805.91 | 754.89 | 209.93 |
| `relayout_emoji_large` | 全量重新排版：中文+英文+大量 emoji（大文档） | 102,399 | 13,182.06 | 12,945.01 | 128.73 |

排版尺寸签名（用于校验断行结果未改变，与优化报告一致）：

```
zhen_small  : 798 x 668
zhen_large  : 798 x 17608
emoji_small : 799 x 1328
emoji_large : 799 x 35208
```

与《RichEdit2性能优化报告》第 4 节（优化前基线）及第 7 节（优化后）的对照：

| 场景 | 优化前基线(us) | 本次中位数(us) | 相对基线 |
| --- | --- | --- | --- |
| `steady_empty` | 0.11 | 0.11 | 0% |
| `steady_zhen_small` | 166.20 | 158.84 | -4.4% |
| `steady_zhen_large` | 181.82 | 178.35 | -1.9% |
| `steady_zhen_xlarge` | 249.74 | 234.62 | -6.1% |
| `steady_emoji_small` | 423.74 | 384.06 | -9.4% |
| `steady_emoji_large` | 449.08 | 408.40 | -9.1% |
| `steady_emoji_xlarge` | 560.96 | 483.83 | -13.8% |
| `scroll_zhen_large` | 182.54 | 174.94 | -4.2% |
| `scroll_emoji_large` | 446.28 | 412.90 | -7.5% |
| `typing_zhen_large` | 243.98 | 231.64 | -5.1% |
| `typing_emoji_large` | 589.35 | 552.30 | -6.3% |
| `relayout_zhen_small` | 262.02 | 236.79 | -9.6% |
| `relayout_zhen_large` | 2,570.94 | 2,285.42 | -11.1% |
| `relayout_emoji_small` | 1,030.62 | 805.91 | -21.8% |
| `relayout_emoji_large` | 16,785.87 | 13,182.06 | -21.5% |

第 7 节中给出优化后数值的 4 个场景，本次复测与之吻合（差异均在噪声带内）：

| 场景 | 报告"优化后"(us) | 本次中位数(us) | 差异 |
| --- | --- | --- | --- |
| `relayout_emoji_large` | 14,375.8 | 13,182.06 | -8.3% |
| `relayout_emoji_small` | 867.4 | 805.91 | -7.1% |
| `steady_emoji_xlarge` | 517.7 | 483.83 | -6.5% |
| `scroll_zhen_large` | 171.1 | 174.94 | +2.2% |

结论：当前 HEAD 已包含优化报告的 3 项优化，性能与报告记录一致；其中 `relayout_emoji_*` 的 -21% 与报告的
"-14% ~ -16%" 同向且略优（属于环境与样本差异）。

### 11.4 字体回退性能基线（`bench_text_render`）

3 轮取每场景最小值，单位 `us/帧`；`每千字符(us)` = `us/帧 × 1000 / 字符数`。

| 场景 | 覆盖路径 | 字符数 | 最小值(us) | 每千字符(us) |
| --- | --- | --- | --- | --- |
| `single_line_measure` | P1 默认横向-单行测量 | 64 | 4.62 | 72.16 |
| `single_line_draw` | P1 默认横向-单行绘制 | 64 | 23.50 | 367.14 |
| `multi_line_measure` | P7 多行文本测量 | 389 | 36.23 | 93.13 |
| `multi_line_draw` | P1 默认横向-多行绘制 | 389 | 183.36 | 471.36 |
| `hcenter_vcenter_draw` | P1 默认横向-居中绘制 | 389 | 169.65 | 436.11 |
| `ellipsis_draw` | P1 默认横向-省略号 | 64 | 19.62 | 306.55 |
| `justify_draw` | P2 两端对齐绘制 | 389 | 155.18 | 398.93 |
| `word_spacing_draw` | P2 字间距绘制 | 389 | 152.72 | 392.60 |
| `vertical_draw` | P3 纵向绘制 | 389 | 194.94 | 501.14 |
| `rich_text_measure` | P4 富文本布局 | 91 | 16.98 | 186.54 |
| `rich_text_draw` | P4 富文本直接绘制 | 91 | 43.86 | 481.98 |
| `rich_text_cache_create` | P5 富文本缓存创建 | 91 | 13.09 | 143.80 |
| `rich_text_cached_draw` | P5 富文本缓存绘制 | 91 | 16.74 | 183.99 |
| `rich_text_letter_spacing` | P6 富文本字间距绘制 | 91 | 49.14 | 540.04 |

> 说明：该基准的**绝对值**与《字体回退性能优化分析》6.3/6.5 节的历史数值存在差异（历史数据在静态库模式下采集，
> 且机器负载不同），例如 `multi_line_draw` 本次 183.36us 对应历史基线 260.31us（该项历史优化幅度 -31.6% 同向），
> `rich_text_cached_draw` 本次 16.74us 对应历史 15.55us（+7.7%，在噪声带内）。
> 因此**跨文档/跨环境的绝对值不可直接比较**，只与本节基线（同机、同构建模式）比较。

### 11.5 复跑命令

```bat
rem 1) 构建两个性能基准（Release + DLL 模式）
python3 build/build.py --offline --dll --target bench_richedit2
python3 build/build.py --offline --dll --target bench_text_render

rem 2) DLL 模式运行前让进程能找到 duilib.dll
set PATH=%CD%\bin;%PATH%

rem 3) RichEdit2 绘制基准：连续运行 12 轮，丢弃前 2 轮，取中位数/最小值
for /L %i in (1,1,12) do out\x64\Release\tests\bench_richedit2.exe --iterations 200 > out\bench_richedit2_r%i.txt

rem 4) 字体回退基准：连续运行 3 轮，取最小值
for /L %i in (1,1,3) do out\x64\Release\tests\bench_text_render.exe --iterations 200 > out\bench_text_render_r%i.txt
```

- 只有 `bench_richedit2` 需要核对 `[layout signature]` 是否与 11.3 一致（断行结果正确性）；
- 可选用 `--no-fallback` 关闭字体回退作对照（仅用于定位问题，**不能作为产品配置**，因为断行结果会改变）。

### 11.6 后续性能对比规则

1. 在**同一台机器**上用 11.5 的命令复跑（Release、x64、DLL 模式，不要在 Debug 或静态库模式下比较）；
2. 逐场景对比 11.3 / 11.4 的**中位数**（`bench_text_render` 用最小值），按下表判定：

   | 变化幅度 | 判定 |
   | --- | --- |
   | ≤ ±10% | 噪声，视为持平 |
   | ±10% ~ ±20% | 需结合最小值交叉验证，并复跑一次确认 |
   | > ±20% | 视为性能变化，需定位原因 |

3. 若 `relayout_*` 变慢但 `steady_*` 不变，优先检查 `DrawSkiaText::BreakText` 与回退字体缓存；
   若 `steady_*` 变慢，优先检查 `DrawRichText::DrawRichTextCacheData` 与绘制缓存构建；
4. 内存侧同时对照第 3 节的 `12.06 MB/MB`（`bench_richedit2_memory`）。

## 12. O1 实施效果：绘制缓存按可视区域懒构建（2026-11-15）

本节记录第 9 节 **O1** 实施后的正确性验证、内存与性能实测，与第 3 节（内存基线）、第 11 节（性能基线）逐项对比。

### 12.1 实现方式

优化前：`SetText` → `CalcTextRectsFull()` 在**一次** `MeasureRichText` 调用中同时完成全文排版和**全文**绘制缓存
（`TPendingDrawRichText` 约 136B/行 + `TextGlyphRunList`），离屏行也一直持有绘制数据。

优化后：排版与绘制数据分离，绘制缓存只覆盖"可视区 ± 1 屏"的窗口。

| 层 | 文件 | 改动 |
| --- | --- | --- |
| 渲染接口 | `duilib/Render/IRender.h` | 新增 `RichTextDrawWindow`（窗口首物理行号、首逻辑行号、首行顶部 Y、窗口底部 Y、全文逻辑行数）与 `CreateDrawRichTextCacheInRange()`；该接口带默认实现（回退到全量 `CreateDrawRichTextCache`），不破坏第三方渲染后端 |
| Skia 后端 | `RenderSkia/DrawRichText.h/.cpp`、`RenderSkia/Render_Skia.h/.cpp` | `InternalDrawRichText()` 增加 `pDrawWindow` 参数：从窗口首行顶部开始排版、到窗口底部停止，仅窗口内行生成 `TPendingDrawRichText`/`TextGlyphRunList`；缓存记录覆盖范围（`m_nFirstRowIndex`/`m_nFirstLineNumber`/`m_nLineCount`/`m_nTotalRowCount`），行宽表按**局部行号**索引 |
| 排版数据 | `Control/RichEditData.h/.cpp` | 新增行索引 `m_lineRowBeginIndex`（每个物理行之前的逻辑行数）与 `m_lineTop`（每个物理行顶部 Y）；新增 `GetVisibleLineRange()`（在 `m_lineTop` 上二分）与缓存范围校验 `IsDrawRichTextCacheRangeMatched()`；`CalcTextRectsFull()`/`CalcTextRectsPart()` 调用 `MeasureRichText(..., nullptr, nullptr)`，**不再建绘制缓存**，增量排版后直接丢弃缓存 |
| 控件 | `Control/RichEdit2.h/.cpp` | `Paint()` 计算窗口（`[scrollY - 1 屏, scrollY + 2 屏]`）→ 只取窗口内行的 `RichTextData` → 缓存缺失/范围不匹配才 `CreateDrawRichTextCacheInRange()` 重建，否则直接复用；新增 `GetRichTextForDraw(richTextDataList, nFirstLine, nLineCount)` |

关键正确性处理：

1. **窗口从任意物理行边界开始排版**：`RichEdit2` 中每个 `RichTextData` 对应一个物理行（段落起点），
   因此可以直接以 `m_nFirstRowIndex`/`m_nFirstLineNumber` 为起点排版，断行结果与全量排版一致；
2. **窗口首行为空行**（如段落末尾的空行）时，全量排版下的行高等于字体高度，而窗口模式下按"无文本"会得到 0；
   已在 `InternalDrawRichText()` 中按 `m_nFirstRowIndex > 0` 分支补齐 `nRowHeight = nFontHeight`，避免窗口内后续行整体上移；
3. 缓存失效条件：文本/宽度/对齐方式变化（`RichEditData` 原有脏标记）、窗口行范围变化、`IsValidDrawRichTextCache()` 校验失败
   （`rcTextRect`、滚动偏移、窗口内 `RichTextData` 内容）；
4. 窗口数据未就绪（首次排版前）时，`Paint()` 回退到全量 `CreateDrawRichTextCache()` 路径。

### 12.2 正确性验证

| 项目 | 结果 |
| --- | --- |
| 构建（DLL 模式） | `duilib.dll` 编译通过，但 `test_text_draw` 等测试目标链接失败（引用未导出的 `ui::DrawSkiaText` 等符号） |
| 构建（非 DLL 模式，按约定切换） | `python3 build/build.py --offline` → 43/43 目标成功，`[OK] 编译完成` |
| 单元测试 | `ctest --test-dir out\x64\Release -C Release` → **16/16 通过**（含本节新增的 `richedit2_edit`，见 12.6） |
| 排版签名（`bench_richedit2` 的 `[layout signature]`） | 与第 11.3 节完全一致：`zhen_small 798x668`、`zhen_large 798x17608`、`emoji_small 799x1328`、`emoji_large 799x35208` |

### 12.3 内存对比（`bench_richedit2_memory`，每行 64 字符）

| 文本规模 | 行数 | 优化前控件合计（不含调用方文本） | 优化后 | 变化 | 优化后每字符 | 优化后含调用方文本 | 优化后峰值工作集 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 2MB | 16,384 | 24.70 MB | **10.09 MB** | **-59.2%** | 9.62 B | 11.63 MB（原 26.71） | 30.9 MB（原 45.1） |
| 4MB | 32,768 | 50.06 MB | **19.43 MB** | **-61.2%** | 9.71 B | 23.44 MB（原 54.07） | 47.6 MB（原 74.6） |
| 8MB | 65,536 | 99.13 MB | **37.64 MB** | **-62.0%** | 9.41 B | 45.66 MB（原 107.15） | 80.8 MB（原 133.4） |
| 16MB | 131,072 | 193.92 MB | **73.13 MB** | **-62.3%** | 9.14 B | 89.17 MB（原 209.95） | 147.2 MB（原 249.8） |

线性回归（2/4/8/16MB，与第 3 节同口径）：

| 项目 | 优化前 | 优化后 | 变化 |
| --- | --- | --- | --- |
| 每 1MB UTF-16 文本的内存增量 | 12.06 MB | **约 4.50 MB** | **-62.7%** |
| 每字符内存 | 24.1 B | **约 9.0 B** | -62.7% |
| 与文本大小的放大倍数 | 约 12.1 倍 | **约 4.5 倍** | — |
| 首次绘制的额外增量（`layout_and_first_paint`） | 0.29 ~ 2.27 MB | **≈ 0**（只构建 1~3 屏窗口缓存） | — |

结论：内存与文本规模的关系从"约 12 倍文本"降到"约 4.5 倍文本"，优于第 9.1 节 O1 预估的 -45% ~ -50%；
剩余内存主要是排版数据结构本身（`RichTextCharInfo` 4B/字符、`RichTextRowInfo`、行文本副本），与第 4.2 节的理论下界分析一致，
后续收益需要靠 O2 ~ O5。

> 口径说明：第 3 节基线为 DLL 模式采集，本节为按 12.2 切换后的**非 DLL（静态库）模式**。
> 度量项是控件内部堆增量，与库的链接方式无关；如需严格同口径，可在 DLL 模式恢复后复测确认。

### 12.4 性能对比（`bench_richedit2`，12 轮弃前 2 轮取中位数）

单位 `us/帧`，基线取第 11.3 节；判定按 11.6 节规则（≤±10% 噪声；>±20% 视为真实变化）。

| 场景 | 基线中位数 | 本次中位数 | 变化 | 本次最小值 | 判定 |
| --- | --- | --- | --- | --- | --- |
| `steady_empty` | 0.11 | 0.10 | -0.01us | 0.10 | 持平 |
| `steady_zhen_small` | 158.84 | 146.68 | **-7.7%** | 138.36 | 持平（偏快） |
| `steady_zhen_large` | 178.35 | 150.86 | **-15.4%** | 142.85 | 改善 |
| `steady_zhen_xlarge` | 234.62 | 149.13 | **-36.4%** | 139.75 | 改善 |
| `steady_emoji_small` | 384.06 | 383.13 | -0.2% | 368.26 | 持平 |
| `steady_emoji_large` | 408.40 | 389.74 | -4.6% | 373.07 | 持平 |
| `steady_emoji_xlarge` | 483.83 | 384.19 | **-20.6%** | 364.96 | 改善 |
| `scroll_zhen_large` | 174.94 | 148.12 | **-15.3%** | 137.24 | 改善 |
| `scroll_emoji_large` | 412.90 | 384.82 | -6.8% | 373.53 | 持平（偏快） |
| `typing_zhen_large` | 231.64 | 286.61 | **+23.7%** | 269.90 | **回归** |
| `typing_emoji_large` | 552.30 | 830.59 | **+50.4%** | 805.54 | **回归** |
| `relayout_zhen_small` | 236.79 | 267.17 | **+12.8%** | 228.27 | 需复核（最小值 +3% 在噪声内） |
| `relayout_zhen_large` | 2,285.42 | 1,537.81 | **-32.7%** | 1,469.02 | 改善 |
| `relayout_emoji_small` | 805.91 | 1,100.99 | **+36.6%** | 1,044.35 | **回归** |
| `relayout_emoji_large` | 13,182.06 | 11,051.30 | **-16.2%** | 10,695.05 | 改善 |

变化的两条主线：

1. **绘制侧（`steady_*`、`scroll_*`、`relayout_*_large`）整体变快**：`Paint()` 遍历的 `TPendingDrawRichText`
   从"全文"降到"1~3 屏"，大文档场景收益随文本规模放大（`steady_zhen_xlarge -36.4%`）；
   全量排版不再生成字形运行结果（`bBuildGlyphRuns = false`），`relayout_*_large` 因此减少 -16% ~ -33%。
2. **输入与小文档（`typing_*`、`relayout_*_small`）变慢**，原因见 12.5。

### 12.5 回归原因与遗留项

| 场景 | 原因 | 量化 |
| --- | --- | --- |
| `typing_*` | 每次按键走 `CalcTextRectsPart()` 增量排版，按其设计需要丢弃绘制缓存；随后 `Paint()` 要重建"可视区 ± 1 屏"（约 3 屏、emoji_large 约 20 行 / 5000 字符）的字形运行结果。优化前是增量**合并**缓存，未变化的可见行直接复用 | +55us（`typing_zhen_large`）、+278us（`typing_emoji_large`），均在 16.7ms 帧预算内 |
| `relayout_*_small` | 小文档的窗口覆盖全部行，全量排版 + `Paint()` 再建一次缓存 ≈ 布局做两遍（断行一次、字形运行一次）；大文档不受影响，因为第二遍只覆盖窗口 | `relayout_emoji_small` +295us；`relayout_zhen_small` 中位数 +12.8% 但最小值仅 +3%，属高方差场景 |

结论：本次改动以"每帧 0.05 ~ 0.3ms 的编辑路径开销"换"约 -62% 内存 + 大文档绘制 -15% ~ -36%"，
对用户可感知的输入延迟影响很小（远低于一帧），因此判定为**可接受**，并登记以下后续项：

| 后续项 | 方案 | 预期 |
| --- | --- | --- |
| O1.1（消除 `typing_*` 回归） | 把窗口缓存的 `nRowIndex` 改为**全局行号索引**（`m_nFirstRowIndex` 恒为 0、去掉构建后的行号偏移），从而恢复原 `UpdateDrawRichTextCache()` 的增量合并：仅当修改/删除行落在缓存覆盖范围内才降级为丢弃 | `typing_*` 回到基线附近 |
| O1.2（消除 `relayout_*_small` 回归） | 布局单遍内"边排版边按窗口过滤"建立缓存（需把 `RichTextDrawWindow` 透传到 `IRender::MeasureRichText`），使 `Paint()` 直接命中布局期缓存 | 小文档回到基线附近，省一次断行 |
| O1.3（可选） | 预取窗口从"上下各 1 屏"改为"上下各 0.5 屏" | 降低重建成本，滚动重建频率上升，需实测权衡 |

另注：`typing_*` 的绝对开销由窗口大小决定（约 3 屏），与"每次按键重建"的耦合是 O1 的结构性代价，
O1.1 / O1.2 任一项落地后该项即可收敛。

### 12.6 编辑功能回归测试（新增 `richedit2_edit`）与修复

窗口化绘制缓存改变了 `Paint()` 的取数范围，为确认编辑行为（光标、选区、滚动）不受影响，
新增单元测试 `tests/test_richedit2_edit.cpp`（注册到 ctest，用例名 `richedit2_edit`，
Windows 平台构建）：使用离屏渲染 + 轻量窗口桩对象驱动真实 `RichEdit2`，逐像素读取绘制结果。

| 用例 | 校验点 |
| --- | --- |
| 替换文本 | `SetText` / `AppendText` 后的内容、行数、`GetTextLength`、光标定位 |
| 插入字符 | `InsertText`、`OnChar` 键盘输入后的文本、返回值与光标位置 |
| 插入行 | 回车键（`kVK_RETURN`）插入 `\r\n`，光标落在新行行首 |
| 删除字符 | 退格 / 删除键；行首退格与上一行合并、行尾删除与下一行合并、删除全部 |
| 删除行 | 删除首行 / 中间行 / 末行（含换行符） |
| 替换文本行 | 选中行内文本替换；选中整行替换为多行 |
| 方向键 | 左/右 ±1 与边界不动；上/下保持 x 坐标跨行；下→上往返回到原字符；编辑后仍正确 |
| 滚动条滚动 | 滚动 5 行后画面 = 滚动前画面上移 5 行；滚回顶部画面完全恢复 |
| 方向键切换视图 | 连续下移 40 行时视图跟随滚动、光标始终在可视区内；上移回顶部后滚动归零 |
| 窗口缓存正确性 | 5 个滚动位置（0/22/150/1000/3000）下，按可视区构建的窗口缓存与全量缓存**逐像素一致**（diff = 0） |
| 粘贴后滚动 | 小尺寸编辑框（360 x 180、多行、**不自动换行**）中粘贴 160 行源码样式文本（含空行、超宽行），在 6 个滚动位置上每个可视行都必须有文字像素；且粘贴路径（`ReplaceSel`）与全量排版路径（`SetText`）画面逐像素一致 |
| 编辑后重绘 | 内容变化后画面随之变化；清空文本后无残留 |
| 选项：只读 | 只读时 `OnChar`/`InsertText`/`ReplaceSel`/退格/删除/`Clear` 均不改变文本（`ReplaceSel` 返回 false），`SetText` 仍可写入，关闭只读后恢复可编辑 |
| 选项：长度限制 | `SetLimitText` 边界（正好达到/跨越限制）、整次修改丢弃、`kEventTextLimit` 触发次数；缩小限制与 `SetText` 超限静默截断；`SetLimitText(0)` 表示不限制 |
| 选项：允许字符集 | `SetLimitChars` 仅过滤键盘输入路径（`OnChar`），`InsertText`/`SetText` 不过滤 |
| 选项：密码模式 | `GetText`/`GetLine` 返回真实文本、`PosFromChar`/`CharFromPos` 可用、密码字符与 `SetShowPassword` 切换、绘制不崩溃 |
| 选项：只允许数字 | 非法字符拒绝、上下限夹取、`GetTextNumber`/`SetTextNumber`/`AdjustTextNumber` |
| 选项：对齐 | 左/中/右与上/中/下切换后 `PosFromChar` 坐标单调变化，往返定位仍一致 |
| 选项：单行/回车/Tab | 单行模式下回车不插入换行并触发 `kEventReturn`；`want_return`/`want_tab`/`want_ctrl_return` 的行为 |
| 选项：行间距/缩放 | 行间距乘数与附加量改变行高；`SetZoomPercent(200)` 时坐标按比例变化且行数不变，恢复后复原 |
| 撤销/重做 | `CanUndo`/`Undo`/`Redo`/`CanRedo`、新编辑使重做失效、`EmptyUndoBuffer`、`SetUndoLimit(1)` |
| 选区与行查询 API | `SetSel`/`GetSel`/`SetSelAll`/`GetSelText`/`HasSelText`/`GetTextRange`/`Clear`，以及 `LineIndex`/`LineLength`/`LineFromChar`/`GetLine`/`GetTextRange` 的越界与负值 |
| 光标命令键 | Home/End（行首行尾）、Ctrl+Home/Ctrl+End、PageUp/PageDown（视图滚动）、Shift+左右扩展/收缩选区 |
| 鼠标选区 | 单击定位与 `CharFromPos` 一致、拖动选择范围与文本一致、双击选词、绘制不崩溃 |
| 事件 | `kEventTextChanged`/`kEventSelChanged`/`kEventReturn`/`kEventTab`/`kEventZoom`/`kEventTextLimit` 的触发次数；`SetText` 与 `SetTextNoEvent` 的差异 |
| 系统剪贴板 | `Copy`/`Cut`/`Paste`/`CanPaste` 的文本与选区变化、事件与撤销；只读/密码模式禁止复制剪切粘贴；单行模式把换行规整为空格；数字/字符集/长度限制对粘贴的拦截；多行与 CRLF 文本的复制粘贴往返；Ctrl+C/X/V 与 Shift+Insert 快捷键 |
| 文本查找 | `FindRichText` 的正向/反向查找、大小写匹配、全字匹配（含查找范围外的词边界判断）、跨行匹配、中文文本、范围越界/空范围/负值/空查找文本等异常参数，以及查找不改变文本与选区 |

结果：**538 项检查全部通过**；`ctest` **16/16 通过**。

测试过程中发现并修复的编辑缺陷（均为既有问题，与 O1 无关）：

| 缺陷 | 现象 | 修复 |
| --- | --- | --- |
| 行首退格（仅 `\n` 换行的文本） | `SetText("ab\ncd")` 后在 `c` 前退格，会连 `b` 一起删除（结果 `acd`），期望 `abcd` | `RichEditData::GetPrevValidCharIndex()` / `GetPrevValidWordIndex()` 按前一行实际结尾判断：`\r\n` 跳过 2 个字符，仅 `\n` 跳过 1 个 |
| `ReplaceSel()` 替换后仍保持选区 | 替换后 `GetSel()` 返回 `[start, end]`，继续输入会覆盖刚插入的文本 | 替换后光标折叠到插入文本末尾（与 Win32 `EM_REPLACESEL` 一致） |
| `InsertText()` / `AppendText()` 返回值 | 返回操作前的光标位置，与头文件注释"插入/追加后的位置"不符 | `InsertText` 返回 `nInsertAfterChar + 文本长度`；`AppendText` 返回新的 `GetTextLength()` |
| `End` 键在最后一行少移动一个字符 | `SetText("abc")` 后按 `End`，光标停在字符 `b` 之后（索引 2）而不是文本末尾（索引 3）；`Shift+End` 同样少选最后一个字符 | `RichEditData::GetRowEndCharIndex()` 对"最后一行且无换行符"返回 `nTextLength`（行尾位置在末尾字符之后）；有 `\r\n`/`\n`/`\r` 的行仍返回换行符之前的位置 |
| 无选区复制会清空剪贴板 | 光标处没有选中文本时调用 `Copy()`（或按 Ctrl+C）会把剪贴板内容清空，用户此前复制的内容丢失 | `RichEdit2::Copy()` 在选中文本为空时直接返回，不写剪贴板（与系统编辑框一致） |
| 空剪贴板时 `CanPaste()` 返回 true | 剪贴板中没有文本（或被清空）时 `CanPaste()` 仍返回 true，右键菜单"粘贴"保持可用，但执行 `Paste()` 没有任何效果 | `RichEdit2::CanPaste()` 同时要求剪贴板文本非空 |

说明：

- `PosFromChar()` 对"最后一个字符 / 文本末尾 / 换行符"返回行尾边界坐标，
  `CharFromPos()` 对边界点按字符区间归位，两者边界语义不对等（属既有行为，本次未改动），
  因此往返一致性校验对这三类位置不做严格断言；
- **未复现 O1 相关的编辑异常**：窗口缓存与全量缓存逐像素一致，方向键/光标定位/选区绘制均正确。
  若实际使用中仍出现方向键异常，需要补充复现条件（文本内容、缩放比例、DPI、是否含富文本/emoji）；
- 本次修复只涉及键盘导航与编辑 API，不经过绘制/排版热路径，**12.4 的性能结论不受影响**。
  复跑 12 轮时本机后台负载较高（中位数被整体抬高约 15%~20%，但**最小值与 12.4 的中位数接近**），
  属于环境噪声，待环境空闲时可再复测确认。

### 12.7 粘贴长行文本后滚动出现空白带的修复（2026-11-15）

现象：多行且**不自动换行**的 `RichEdit2`（演示程序 `rich_edit2` 的配置：`multi_line=true`、
`want_return=true`、`hscrollbar=false`、无 `word_wrap`）粘贴源码等长行文本后，向下滚动时
某一超宽行之后的文字全部不显示（截图中表现为若干空白行）。

根因（`duilib/RenderSkia/DrawRichText.cpp`，`InternalDrawRichText()`）：不换行文本在生成绘制缓存时，
若某一行内容宽度超出可视宽度，代码会设置 `bBreakAll = true` 并直接跳出整个绘制构建循环。
该逻辑本意是"单行文本截断"，但同样作用于多行文本，导致该行之后的所有行都不再生成绘制数据
（与 O1 的窗口化缓存无关，全量缓存路径同样存在该缺陷）。

修复：

- 单行模式（`TEXT_SINGLELINE`）保持原行为：截断并终止绘制；
- 多行且不换行：只截断当前行的剩余内容（`xPos`/行宽按已绘制的前缀推进），
  由换行符推进到下一行继续构建，不再终止整个过程；
- 省略号（`text_overflow` 默认 "ellipsis"）：新增每行独立的省略号绘制数据
  （记录行号、行顶、字体、颜色），被裁剪的行在行末各自追加"…"，替代原先只支持一个全局省略号位置的做法。

验证：新增用例 `TestPasteThenScroll`（见 12.6），修复前在 6 个滚动位置分别有 4~7 行不显示，修复后 0 行；
`richedit2_edit` **全部检查通过**（当时 194 项，当前累计 538 项），`ctest` **16/16 通过**。

性能：该改动只影响"多行 + 不换行 + 行超宽"的裁剪分支；`bench_richedit2` 的各基准场景均为自动换行文本，
复跑结果与 12.4 的中位数差异在噪声范围内（`steady_zhen_large` 173.8→154.9us、
`steady_emoji_large` 446.6→373.6us、`relayout_zhen_large` 1573→1590us、`relayout_emoji_large` 12115→11738us），
**无性能回归**。

### 12.8 编辑功能测试覆盖补齐（2026-11-15）

按"选项 / 命令键 / 鼠标 / 事件"四类补齐了 13 组用例（共 246 项新检查，累计 538 项），
覆盖范围见 12.6 的用例矩阵。补齐过程中发现并修复 1 个既有缺陷（`End` 键在最后一行少移动一个字符，见 12.6 缺陷表）。
另记录一个框架层观察（未修改代码）：`ScrollBox::EnableScrollBar(false, ...)` 销毁滚动条后不会再重置滚动位置，
`SetScrollPos()` 因无滚动条直接返回、不会触发 `OnScrollOffsetChanged`，控件内缓存的滚动偏移会保留；
测试中用"启用滚动条→制造一次滚动→复位"规避。实际业务中若在已滚动的控件上关闭滚动条，需要显式复位滚动位置。

性能：本次改动只涉及 `End` 键的行尾字符索引计算，不经过绘制/排版热路径；
`bench_richedit2` 复跑（Release）与 12.4 中位数一致（`steady_zhen_large` 153.3us、
`steady_emoji_large` 413.9us、`relayout_zhen_large` 1615.6us、`relayout_emoji_large` 11734.6us），**无性能回归**。

### 12.9 系统剪贴板用例补齐（2026-11-15）

新增用例 `TestClipboard`（57 项检查），覆盖 `Copy`/`Cut`/`Paste`/`CanPaste` 与各编辑选项的交互：

- 内容与选区：复制/剪切写入剪贴板的内容，剪切后选区折叠到起点，粘贴插入光标处/替换选区并移动光标；
- 事件与撤销：`Copy` 不触发 `kEventTextChanged`，`Cut`/`Paste` 各触发一次；剪切可撤销，密码模式粘贴不记录撤销；
- 只读与密码模式：只读可复制但禁止剪切/粘贴；密码模式禁止复制与剪切，粘贴时移除换行/TAB；
- 模式与限制：单行模式把 `\r\n`/`\r`/`\n` 各规整为一个空格；数字模式、`limit_chars`、`limit_text` 超限时整次粘贴被丢弃并触发 `kEventTextLimit`；
- 多行文本：复制后粘贴可完整还原（含换行符），Windows 常见的 CRLF 文本字符保持不变；
- 快捷键：Ctrl+C/Ctrl+X/Ctrl+V/Shift+Insert（通过线程键盘状态模拟修饰键）。

同时修复 2 个既有缺陷（见 12.6 缺陷表）：无选区复制清空剪贴板、空剪贴板时 `CanPaste()` 返回 true。
用例会读写系统剪贴板：开始前用 `ClipboardBackup`（RAII）备份剪贴板的**全部格式**（文本、`CF_HDROP` 文件、`CF_BITMAP` 位图、增强图元文件、调色板及各类 HGLOBAL 格式），
结束后（含提前返回）按原格式恢复，避免破坏用户剪贴板中的非文本数据；若环境无法访问剪贴板则打印 `WARN` 后跳过。
已手工验证：运行前分别放置文本、文件拖放、8x8 位图，运行后均能完整取回。

验证：`test_richedit2_edit.exe` → **497 项检查全部通过**；`ctest` **16/16 通过**。
本次改动只涉及 `Copy()`/`CanPaste()` 的空文本判断，不经过绘制/排版热路径；
`bench_richedit2` 复跑（Release）与 12.4 中位数差异在噪声范围内（`steady_zhen_large` 155.8us、
`steady_emoji_large` 395.5us、`relayout_zhen_large` 1642.6us、`relayout_emoji_large` 11619.0us），**无性能回归**。

### 12.10 文本查找用例补齐（2026-11-15）

新增用例 `TestFindRichText`（41 项检查，累计 538 项），覆盖 `FindRichText` 的参数语义与边界：

- 正向查找：`bFindDown=true` 时在 `[cpMin, cpMax)` 内返回第一个匹配，匹配文本恰好在 `cpMax` 结束时可命中；
- 反向查找：`bFindDown=false` 时 `cpMin` 为起始位置、`cpMax` 为下界（`cpMin > cpMax`），在 `[cpMax, cpMin)` 内从后向前返回最后一个匹配；
- 大小写与全字匹配：区分/忽略大小写，全字匹配跳过 `scatter` 中的 `cat`、连续相同字符 `aaaa` 中的 `aa`；
- 词边界：全字匹配使用查找范围之外的字符判断首尾边界（`xabc`/`abcx`），也覆盖文本末尾与无字母数字模式（`++`）；
- 其他：跨行匹配（匹配文本包含 `\n`）、中文文本、范围越界自动收敛、空范围/负值/空查找文本等异常参数返回 false 且输出范围为 `-1`，以及查找不改变文本与选区。

本组用例未发现 `FindRichText` 的功能缺陷（仅修正了用例自身的一处错误预期：`abcabc` 中两处 `abc` 都不是全字）。

验证：`test_richedit2_edit.exe` → **538 项检查全部通过**；`ctest` **16/16 通过**；本次仅新增测试，**无性能影响**。

### 12.11 复跑命令

```bat
rem 1) 构建（DLL 模式；若测试目标链接失败，则去掉 --dll 用静态库模式）
python3 build/build.py --offline --dll

rem 2) 单元测试（含 RichEdit2 编辑功能用例 richedit2_edit，见 12.6）
ctest --test-dir out\x64\Release -C Release --output-on-failure
rem 也可单独运行：out\x64\Release\tests\test_richedit2_edit.exe

rem 3) 内存（2/4/8/16MB，对比 12.3）
out\x64\Release\tests\bench_richedit2_memory.exe --mb 2
out\x64\Release\tests\bench_richedit2_memory.exe --mb 4
out\x64\Release\tests\bench_richedit2_memory.exe --mb 8
out\x64\Release\tests\bench_richedit2_memory.exe --mb 16

rem 4) 性能（12 轮弃前 2 轮取中位数，对比 12.4；核对 [layout signature]）
for /L %i in (1,1,12) do out\x64\Release\tests\bench_richedit2.exe --iterations 200 > out\bench_richedit2_r%i.txt
```

## 13. O2 实施效果：行结构瘦身（2026-11-15）

### 13.1 实现方式

方案：**索引表随数据传入**。新增 `RichTextDataList`（`RichTextData` 数组 + 字体/颜色/图片索引表 `RichTextAttrTable`），
绘制/测量接口以它作为参数，不再在 `RichTextData` 内保存 `SharePtr<UiFontEx>` 与颜色值，也不引入任何全局状态。

| 层 | 文件 | 改动 |
| --- | --- | --- |
| 渲染接口 | `duilib/Render/IRender.h` | 新增 `RichTextImageInfo`（图片 + 尺寸）、`RichTextAttrTable`（1-based 索引表，0 表示"未设置"；`AddFont` 按 `UiFontEx::operator==` 去重、`AddColor` 按值去重、`AddImage` 按"指针 + 尺寸"去重）、`RichTextDataList`（`m_data` + `m_attrs`）；`RichTextData` 删除 `SharePtr<UiFontEx>` 与两个 `UiColor`，改为 `uint16` 的 `m_nFontIndex`/`m_nTextColorIndex`/`m_nBgColorIndex`/`m_nImageIndex`，图片宽高改由 `RichTextImageInfo` 提供；`MeasureRichText`/`DrawRichText`/`CreateDrawRichTextCache`/`CreateDrawRichTextCacheInRange`/`IsValidDrawRichTextCache`/`UpdateDrawRichTextCache` 的参数改为 `RichTextDataList` |
| 行容器 | `IRender.h`、`Control/RichEditData.cpp` | `RichTextLineInfo::m_rowInfo`：`std::vector<SharePtr<RichTextRowInfo>>` → `std::vector<RichTextRowInfo>`；`RichTextRowInfo` 去掉 `NVRefCount` 基类（其删除拷贝构造，无法按值存入 vector）；删除 `RichTextRowInfoPtr`，行访问器返回 `const RichTextRowInfo*` |
| 富文本解析 | `Control/RichTextImpl.h/.cpp` | `RichTextDataEx` 承担解析期字段（字体指针、两种颜色、图片与尺寸）；`ParseText()` 末尾一次性生成 `m_textDataAttrs` 与每个片段的索引；`GetRichTextLayoutData()` 复制索引表后按片段 `PushBack`；`CalcDestRect()` 通过索引取色 |
| Skia 绘制 | `RenderSkia/DrawRichText.cpp`、`Render_Skia.cpp/.h` | 循环内按索引解析出 `const UiFontEx*`/`UiColor`/`RichTextImageInfo` 局部变量；字体缓存改用**指针身份**比较（同一索引表内相同字体必为同一对象）；缓存有效性比较改用索引表逐项 `IsFontEqual`/`IsColorEqual`/`IsImageEqual` |
| 控件 | `Control/RichEdit2.h/.cpp` | `GetRichTextForDraw(RichTextDataList&)` 直接写索引并 `PushBack`，去掉逐行 `new` 与 `SharePtr` 计数 |
| 测试 | `tests/bench_text_render.cpp`、`test_text_draw.cpp`、`test_richedit2_edit.cpp` | 按 `RichTextDataList` 构造数据（先 `AddFont`/`AddColor` 再 `PushBack`） |

去重效果：纯文本场景索引表只有 1 个字体、2 个颜色（文字/背景），索引表开销可忽略；富文本混排时索引数等于不同字体/颜色的数量。

### 13.2 结构大小变化（`bench_richedit2_memory` 的 `[size]` 输出）

| 结构 | 优化前 | 优化后 | 变化 |
| --- | --- | --- | --- |
| `RichTextData` | 88 B | **56 B** | **-32 B（-36%）** |
| `RichTextRowInfo` | 56 B | **48 B** | **-8 B（-14%）** |
| `RichTextLineInfo` | 40 B | 40 B | 0 |
| `RichTextCharInfo` | 4 B | 4 B | 0 |

关于"目标约 40B"：按字段实际计算为 `16(wstring_view) + 4×2(索引) + 3×4(浮点) + 3×4(整型) + 2(样式) + 3(bool) = 53`，
对齐后 56B。剩余 12B 是 `m_nBaselineOffset`/`m_nFirstLineIndent`/`m_nMaxLines` 三个排版参数，
压缩它们需要牺牲布局精度或引入解包分支，收益（每行 12B）小于维护成本，因此本轮不做。

### 13.3 内存对比（同机同构建 A/B，`bench_richedit2_memory`，每行 64 字符）

方法：先备份改动 → `git stash` 回到优化前代码构建并测量 → 恢复改动重新构建测量；两次均为 **DLL 模式 Release / 800x600 离屏 / steady paints = 3**。

| 文本规模 | 行数 | 优化前控件合计 | 优化后控件合计 | 变化 | 优化前每字符 | 优化后每字符 |
| --- | --- | --- | --- | --- | --- | --- |
| 2MB | 16,384 | 26.38 MB | 26.84 MB | +1.7% | 25.16 B | 25.59 B |
| 4MB | 32,768 | 53.06 MB | **51.43 MB** | **-3.1%** | 25.30 B | **24.52 B** |
| 8MB | 65,536 | 104.16 MB | **101.49 MB** | **-2.6%** | 24.83 B | **24.20 B** |
| 16MB | 131,072 | 205.24 MB | **198.70 MB** | **-3.2%** | 24.47 B | **23.69 B** |

含调用方文本：2MB 28.49→28.90、4MB 57.27→55.63、8MB 112.57→109.90、16MB 222.05→215.52（MB）。

说明：

- 4MB 以上收益随规模单调增长（每物理行 -32B × 布局数据与绘制缓存两处 + 行信息 -8B），与结构大小变化一致；
- 2MB 的 +0.46MB 在测量噪声量级（同构建重复测量波动约 ±0.2MB，另有约 1.5MB 固定偏移），
  且解析期 `RichTextDataEx` 新增了图片相关字段（+32B/片段），小规模下会抵消部分收益，故判定为**基本持平**；
- 本轮不改动绘制缓存范围，因此绝对内存仍是"全量缓存"口径（24~25 B/字符），显著高于 12.3 的 O1 数据（约 9 B/字符）——
  原因见 13.6。

### 13.4 性能对比（同机 A/B，`bench_richedit2`，12 轮弃前 2 轮取中位数）

单位 `us/帧`；判定规则同 11.6 节（≤±10% 为噪声）。

| 场景 | 优化前中位数 | 优化后中位数 | 变化 | 优化后最小值 | 判定 |
| --- | --- | --- | --- | --- | --- |
| `steady_empty` | 0.13 | 0.14 | +0.01us | 0.11 | 持平 |
| `steady_zhen_small` | 167.03 | 173.36 | +3.8% | 153.45 | 持平 |
| `steady_zhen_large` | 179.26 | 186.12 | +3.8% | 169.90 | 持平 |
| `steady_zhen_xlarge` | 243.52 | 247.61 | +1.7% | 223.45 | 持平 |
| `steady_emoji_small` | 423.44 | 413.82 | -2.3% | 389.95 | 持平 |
| `steady_emoji_large` | 432.52 | 440.02 | +1.7% | 419.90 | 持平 |
| `steady_emoji_xlarge` | 527.19 | 518.28 | -1.7% | 490.89 | 持平 |
| `scroll_zhen_large` | 188.29 | 176.14 | -6.5% | 170.43 | 持平 |
| `scroll_emoji_large` | 425.00 | 465.03 | +9.4% | 421.75 | 持平（接近上限） |
| `typing_zhen_large` | 1681.12 | 1709.34 | +1.7% | 1609.63 | 持平 |
| `typing_emoji_large` | 11716.28 | 11863.71 | +1.3% | 11649.48 | 持平 |
| `relayout_zhen_small` | 272.34 | 295.03 | +8.3% | 267.39 | 持平（接近上限） |
| `relayout_zhen_large` | 3425.22 | 3522.84 | +2.9% | 2987.34 | 持平 |
| `relayout_emoji_small` | 1190.99 | 1187.39 | -0.3% | 1125.50 | 持平 |
| `relayout_emoji_large` | 21857.91 | 22168.53 | +1.4% | 21303.71 | 持平 |

结论：**全部场景在 ±10% 噪声内，无性能回归**；`[layout signature]` 与 11.3 节一致（`zhen_small 798x668`、
`zhen_large 798x17608`、`emoji_small 799x1328`、`emoji_large 799x35208`），排版结果未变。

### 13.5 字体回退路径对比（同机 A/B，`bench_text_render`，3 轮取最小值）

| 场景 | 优化前最小值 | 优化后最小值 | 变化 |
| --- | --- | --- | --- |
| `single_line_measure` | 4.17 | 4.35 | +4.3% |
| `single_line_draw` | 21.56 | 22.15 | +2.7% |
| `multi_line_measure` | 41.33 | 35.84 | -13.3% |
| `multi_line_draw` | 180.99 | 183.28 | +1.3% |
| `hcenter_vcenter_draw` | 179.04 | 180.39 | +0.8% |
| `ellipsis_draw` | 21.03 | 19.12 | -9.1% |
| `justify_draw` | 171.45 | 159.30 | -7.1% |
| `word_spacing_draw` | 173.81 | 160.19 | -7.8% |
| `vertical_draw` | 203.38 | 197.76 | -2.8% |
| `rich_text_measure` | 17.05 | 17.49 | +2.6% |
| `rich_text_draw` | 41.98 | 41.76 | -0.5% |
| `rich_text_cache_create` | 21.03 | 21.37 | +1.6% |
| `rich_text_cached_draw` | 19.19 | 20.86 | +8.7% |
| `rich_text_letter_spacing` | 51.22 | 51.33 | +0.2% |

富文本相关路径（P4/P5/P6）与普通文本路径均在噪声内；`multi_line_measure` 的 -13.3% 是 3 轮最小值的高方差表现，不作结论。

### 13.6 与第 11 节基线的对比（全量绘制缓存状态下的数据）

13.3~13.5 的 A/B 是在"全量绘制缓存"状态下测得的：`b6db237e` 曾把 `RichEdit2::Paint()` 从
"按可视区窗口构建"改回"全量构建"（删除 `GetVisibleLineRange` 分支与 `CreateDrawRichTextCacheInRange` 调用），
因此当时"优化前"的内存为 24.5~25.3 B/字符（与第 3 节原始口径一致），而 12.3/12.4 的 O1 数据不适用于该状态。
**该回退已被判定为错误修改，并已在 13.8 中恢复**，最终数据以 13.8 为准；本节表格仅用于说明
"全量缓存 + O2"这一中间状态的性能水平。

以第 11.3 节基线（同为全量缓存时代）对照该中间状态：

| 场景 | 11.3 基线中位数 | O2 后中位数 | 变化 | 归因 |
| --- | --- | --- | --- | --- |
| `steady_zhen_small` | 158.84 | 173.36 | +9.1% | 噪声/小幅 |
| `steady_zhen_large` | 178.35 | 186.12 | +4.4% | 噪声 |
| `steady_zhen_xlarge` | 234.62 | 247.61 | +5.5% | 噪声 |
| `steady_emoji_small` | 384.06 | 413.82 | +7.7% | 噪声 |
| `steady_emoji_large` | 408.40 | 440.02 | +7.7% | 噪声 |
| `steady_emoji_xlarge` | 483.83 | 518.28 | +7.1% | 噪声 |
| `scroll_zhen_large` | 174.94 | 176.14 | +0.7% | 持平 |
| `scroll_emoji_large` | 412.90 | 465.03 | +12.6% | 小幅变慢 |
| `typing_zhen_large` | 231.64 | 1709.34 | **+638%** | **非 O2**：增量合并路径缺失 |
| `typing_emoji_large` | 552.30 | 11863.71 | **+2048%** | **非 O2**：同上 |
| `relayout_zhen_small` | 236.79 | 295.03 | +24.6% | 非 O2：同上 |
| `relayout_zhen_large` | 2,285.42 | 3,522.84 | **+54.1%** | 非 O2：全量重建 |
| `relayout_emoji_small` | 805.91 | 1,187.39 | **+47.3%** | 非 O2：全量重建 |
| `relayout_emoji_large` | 13,182.06 | 22,168.53 | **+68.2%** | 非 O2：全量重建 |

归因依据：13.4 的同机 A/B 中，优化前后 `typing_*`/`relayout_*` 完全一致（±3%），说明该差异来自
`b6db237e`（窗口化缓存被移除，每次绘制为全文构建绘制数据），与 O2 无关。影响面：

- 每帧绘制（`steady_*`/`scroll_*`）与 11 节基线基本一致，用户滚动/查看无感知差异；
- **每次按键与每次重新排版需要为全文重建字形运行结果**，31,599 字符文档约 1.7ms/帧、
  102,399 字符 emoji 文档约 11.9ms/帧，已接近或超过 16.7ms 帧预算的一半，属需要优先处理的性能问题。

处理结果：窗口化绘制缓存已按 13.8 恢复；`UpdateDrawRichTextCache()` 的增量合并路径仍未使用（见 13.8.4）。

### 13.7 验证与遗留项

| 项目 | 结果 |
| --- | --- |
| 构建（DLL 模式） | `duilib.dll` 与全部基准/测试目标编译通过；`test_text_draw` 仍受既有 DLL 导出限制 |
| 构建（静态库模式） | `python build/build.py build --offline` → `[OK] 编译完成`（873 个构建目标） |
| 单元测试（静态 Release） | `ctest` → **16/16 通过**；`text_draw`（覆盖 `RichTextDataList` 的测量/绘制/缓存路径）、`richedit2_edit`（548 项检查）、`rich_text`、`label`、`rich_text_util`、`image_draw`、`render_*`、`controls_pages_xml` 等全部通过 |
| 编辑行为回归 | `richedit2_edit` 548 项检查全部通过；此前 5 项失败的行为（`AppendText`/`InsertText` 返回位置、光标无选区、无选区 `Copy`、空剪贴板 `CanPaste`）被 `b6db237e` 一并回退，已在 13.9 恢复 |
| 排版一致性 | `bench_richedit2` 的 `[layout signature]` 与 11.3 完全一致 |

遗留可选项：`RichTextData` 从 56B 进一步压缩到 48B（打包三个 bool/样式位、把 `m_nMaxLines` 改为 `uint16`），
预计再省约 1MB/8MB 文本，收益小于本轮，暂不实施。

### 13.8 恢复窗口化绘制缓存（修正 b6db237e 的错误改动）

#### 13.8.1 改动内容

`b6db237e` 在修正"文本溢出不应绘制省略号"的同时，把 `RichEdit2::Paint()` 的窗口化绘制缓存一并删除了。
本轮恢复窗口化路径，并保留该提交真正需要修复的行为：

| 项 | 处理 |
| --- | --- |
| 可视行窗口 | 恢复 `m_pTextData->GetVisibleLineRange(rcDrawText, szScrollOffset, visibleRange)`（上下各预取一屏） |
| 窗口数据生成 | 恢复 `GetRichTextForDraw(RichTextDataList&, size_t nFirstLine, size_t nLineCount)`，内部调用 `RichEditData::GetTextView(textView, nFirstLine, nLineCount)`，按 O2 的 `RichTextDataList` 生成 |
| 窗口缓存构建 | 恢复 `CreateDrawRichTextCacheInRange(rcDrawText, szScrollOffset, richTextDataList, drawWindow, cache)`，`drawWindow` 填入全局行号、首个逻辑行索引、首行顶部坐标、窗口底部与总逻辑行数 |
| 缓存有效性 | 行范围变化用 `IsDrawRichTextCacheRangeMatched(nFirstLine, nLineCount)` 判断，内容变化用 `IsValidDrawRichTextCache()` 判断；任一失效则清除并重建 |
| 省略号行为（保留 b6db237e 的修复） | `m_bTextOverflowEllipsis = false`，文本超出绘制区域时直接裁剪，由滚动条显示 |
| 横向滚动（保留 8671011e 的修复） | `m_bCacheFullTextWidth = true`，窗口内每行缓存整行宽度，横向滚动后新区域有绘制数据 |
| 回退路径 | 行索引数据尚未就绪（首次排版前）时，回退到全量构建 |

#### 13.8.2 内存（DLL Release，每行 64 字符，控件合计）

| 规模 | 12.3（O1，优化前口径） | 13.3（全量缓存 + O2） | 最终（窗口缓存 + O2） | 对比 12.3 |
| --- | --- | --- | --- | --- |
| 2MB | 10.43 MB（9.95 B/字符） | 26.84 MB（25.59） | **9.52 MB（9.08）** | **-8.7%** |
| 4MB | 20.25 MB（9.66） | 51.43 MB（24.52） | **19.87 MB（9.47）** | **-2.0%** |
| 8MB | 40.12 MB（9.57） | 101.49 MB（24.20） | **38.54 MB（9.19）** | **-4.0%** |
| 16MB | 75.53 MB（9.01） | 198.70 MB（23.69） | **74.42 MB（8.87）** | **-1.6%** |

相比"全量缓存 + O2"，恢复窗口化后内存下降约 **61%~64%**；相比 O1 原始数据，O2 的行结构瘦身再带来 1.6%~8.7% 的收益。
用户最初的目标（O1 + O2 叠加）至此达成。

#### 13.8.3 性能（`bench_richedit2`，同机，12 轮弃前 2 轮取中位数，单位 `us/帧`）

| 场景 | 全量缓存 + O2 | 窗口缓存 + O2 | 变化 | 11.3 基线 | 对比基线 |
| --- | --- | --- | --- | --- | --- |
| `steady_zhen_small` | 173.36 | 173.93 | +0.3% | 158.84 | +9.5% |
| `steady_zhen_large` | 186.12 | 176.68 | -5.1% | 178.35 | -0.9% |
| `steady_zhen_xlarge` | 247.61 | **174.25** | **-29.6%** | 234.62 | **-25.7%** |
| `steady_emoji_small` | 413.82 | 427.13 | +3.2% | 384.06 | +11.2% |
| `steady_emoji_large` | 440.02 | 431.07 | -2.0% | 408.40 | +5.6% |
| `steady_emoji_xlarge` | 518.28 | **434.38** | **-16.2%** | 483.83 | **-10.2%** |
| `scroll_zhen_large` | 176.14 | 174.24 | -1.1% | 174.94 | -0.4% |
| `scroll_emoji_large` | 465.03 | 440.96 | -5.2% | 412.90 | +6.8% |
| `typing_zhen_large` | 1709.34 | **333.49** | **-80.5%** | 231.64 | +44.0% |
| `typing_emoji_large` | 11863.71 | **972.05** | **-91.8%** | 552.30 | +76.0% |
| `relayout_zhen_small` | 295.03 | 320.86 | +8.8% | 236.79 | +35.5% |
| `relayout_zhen_large` | 3522.84 | **1786.53** | **-49.3%** | 2285.42 | **-21.8%** |
| `relayout_emoji_small` | 1187.39 | 1219.47 | +2.7% | 805.91 | +51.3% |
| `relayout_emoji_large` | 22168.53 | **12265.38** | **-44.7%** | 13182.06 | **-7.0%** |

结论：

- 大文档的每帧绘制（`steady_*_xlarge`）与重新排版（`relayout_*_large`）显著改善，其中
  `relayout_zhen_large` 已优于 11.3 基线 21.8%；
- 文本规模较小（`*_small`）时窗口构建的固定开销占比更高，`relayout_*_small` 比基线慢 35%~51%
  （绝对值 236→321us / 806→1219us），其中窗口化本身带来的变化为 +2.7%~+8.8%，其余差异来自基线之后的排版改动；
  整体符合"窗口化以少量固定开销换取大文档线性收益"的权衡；
- `typing_*` 相对基线仍慢 44%~76%，原因是 `UpdateDrawRichTextCache()` 增量合并路径早已不被
  `Paint()` 调用（早于 `b6db237e`），每次编辑后需要重建一屏窗口缓存。该项与 `b6db237e` 无关，
  属后续独立优化（可基于 O2 的按值行结构做整行替换，替代旧的 `SharePtr` 合并）。

#### 13.8.4 验证

- `richedit2_edit` 的 `TestHScrollPaint`（横向滚动后新区域必须绘制文字）与 `TestScrollViewSwitch`
  （窗口缓存与全量缓存逐像素一致、滚动后画面按偏移平移一致）均通过；
- 静态 Release 构建 873 个目标全部成功，`ctest` **16/16 通过**（`text_draw`、`richedit2_edit` 均通过）；
- `[layout signature]` 与 11.3 一致，窗口化未改变排版结果。

### 13.9 恢复 b6db237e 一并回退的编辑行为

`b6db237e` 除删除窗口化绘制缓存外，还回退了 12.6/12.9 中已修复的 5 项编辑行为，本轮一并恢复：

| 行为 | 恢复内容 |
| --- | --- |
| `InsertText` | 返回插入文本后的字符位置：`nInsertAfterChar + 插入文本的 UTF-16 长度` |
| `AppendText` | 返回追加后的文本长度：`GetTextLength()` |
| `ReplaceSel` | 替换完成后光标位于插入文本末尾（与 Win32 `EM_REPLACESEL` 一致），不再保留选区 |
| `Copy` | 无选区时不写剪贴板，保持用户剪贴板原内容不变 |
| `CanPaste` | 剪贴板中没有文本时返回 `false` |

验证：静态 Release 构建通过，`ctest` **16/16 全部通过**，`richedit2_edit` 548 项检查 0 失败。

## 14. O6 实施效果：内存归还与绘制数据复用（2026-11-15）

### 14.1 实现方式

| 改动点 | 位置 | 说明 |
| --- | --- | --- |
| 属性表/数据表支持归还容量 | [IRender.h](file:///e:/develop/nim_duilib/duilib/Render/IRender.h) | `RichTextAttrTable::Release()`、`RichTextDataList::Release()` 用 `swap` 与空容器交换，清空且归还容量 |
| 绘制数据复用容器 | [RichEdit2.h](file:///e:/develop/nim_duilib/duilib/Control/RichEdit2.h)、[RichEdit2.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichEdit2.cpp) | `Paint()` 中的局部 `RichTextDataList` 改为成员 `m_richTextDrawData`；`GetRichTextForDraw` 内部先 `Clear()` 再填充，容量跨帧复用，稳态不再每帧申请/释放文本片段数组 |
| 文本清空时归还 | [RichEdit2.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichEdit2.cpp) | `SetText()` 在文本为空时调用 `ReleaseDrawData()`：清除 `DrawRichTextCache` 并 `m_richTextDrawData.Release()` |
| 控件隐藏时归还 | [RichEdit2.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichEdit2.cpp) | 新增 `OnSetVisible()` override，`!IsVisible()` 时调用 `ReleaseDrawData()`，再次显示时由 `Paint()` 按需重建 |
| 行索引容量归还 | [RichEditData.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichEditData.cpp) | `RichEditData::Clear()` 中 `m_lineRowBeginIndex`、`m_lineTop` 由 `clear()` 改为 `swap` 归还容量（每逻辑行 8 字节，O6 前会随大文本长期常驻） |
| 缓存失效 | 不额外动作 | `IsValidDrawRichTextCache()` 失效时已由 `ClearDrawRichTextCache()` 释放缓存对象；复用容器**不做**同步 `shrink_to_fit`，避免逐字符输入场景每帧重新分配导致抖动（容量上限为一屏窗口的数据量） |

> 复用容器是安全的：`DrawRichText::CreateDrawRichTextCacheInRange` 内部执行
> `spDrawRichTextCache->m_richTextData = richTextData;`（值拷贝），缓存不持有成员容器的引用。

### 14.2 内存 A/B（同机、静态 Release、每行 64 字符）

`bench_richedit2_memory` 新增两个阶段与 `[release]` 输出：`after_hide`（隐藏控件后）、
`after_clear_text`（重新显示并绘制后再清空文本），用于量化"内存归还"。

| 规模 | 稳态每字符字节（O6 前 → O6） | 清空后常驻（O6 前） | 清空后常驻（O6） | 变化 | 峰值工作集（前 → 后） |
| --- | --- | --- | --- | --- | --- |
| 2MB | 9.09 → 9.11 | 9.11 MB | **8.39 MB** | **-7.8%** | 30.07 → 30.20 MB |
| 4MB | 9.51 → 9.46 | 10.53 MB | **10.25 MB** | **-2.6%** | 46.22 → 46.18 MB |
| 8MB | 9.20 → 9.16 | 14.67 MB | **13.94 MB** | **-5.0%** | 78.17 → 78.07 MB |
| 16MB | 8.88 → 8.90 | 23.61 MB | **22.46 MB** | **-4.9%** | 141.33 → 141.25 MB |

结论：

- **稳态持平**：`per_char_bytes` 四档差异均 ≤ 0.6%，在噪声范围内；复用容器只保留一屏窗口的数据量，
  不会把峰值缓存常驻下来；
- **清空文本后常驻下降 2.6%~7.8%**：这是 O6 的主要收益，来源是行索引/行顶部坐标的容量归还
  （2/4/8/16MB 文本对应约 0.13/0.26/0.52/1.05MB），加上复用容器的释放；
- **隐藏控件释放量在 `PrivateUsage` 上读数为 0**：窗口化缓存只有几十 KB，低于堆向系统归还内存的粒度，
  但缓存对象与复用容器在逻辑上已释放，再次显示时按需重建；
- **峰值工作集基本不变**：窗口化绘制缓存（O1 + 13.8）已经把峰值限制在一屏范围内，O6 的收益集中在
  "清空/隐藏后的常驻内存"，而不是运行期峰值。

### 14.3 性能 A/B（`bench_richedit2`，同机，12 轮弃前 2 轮取中位数，单位 `us/帧`）

| 场景 | 14 节前（13.8.3） | O6 | 变化 |
| --- | --- | --- | --- |
| `steady_zhen_small` | 173.93 | 157.13 | -9.7% |
| `steady_zhen_large` | 176.68 | 155.30 | -12.1% |
| `steady_zhen_xlarge` | 174.25 | 152.26 | -12.6% |
| `steady_emoji_small` | 427.13 | 404.65 | -5.3% |
| `steady_emoji_large` | 431.07 | 439.55 | +2.0% |
| `steady_emoji_xlarge` | 434.38 | 404.77 | -6.8% |
| `scroll_zhen_large` | 174.24 | 162.14 | -6.9% |
| `scroll_emoji_large` | 440.96 | 408.05 | -7.5% |
| `typing_zhen_large` | 333.49 | 301.14 | -9.7% |
| `typing_emoji_large` | 972.05 | 911.85 | -6.2% |
| `relayout_zhen_small` | 320.86 | 292.00 | -9.0% |
| `relayout_zhen_large` | 1786.53 | 1664.85 | -6.8% |
| `relayout_emoji_small` | 1219.47 | 1178.73 | -3.3% |
| `relayout_emoji_large` | 12265.38 | 12451.27 | +1.5% |

- 全部场景在 ±10% 噪声区间内或略有改善，无回退；
- `typing_*` 的改善（-6.2%~-9.7%）来自复用容器后每帧不再新建文本片段数组；
- `[layout signature]` 仍为 798x668 / 798x17608 / 799x1328 / 799x35208，排版结果未变。

### 14.4 验证与复跑

- 静态 Release 构建（411 个目标）全部成功，`ctest` **16/16 通过**；
- 复跑命令：

```bat
rem 内存（含隐藏/清空阶段与 [release] 输出）
set PATH=%CD%\bin;%PATH%
out\x64\Release\tests\bench_richedit2_memory.exe --mb 2
out\x64\Release\tests\bench_richedit2_memory.exe --mb 4
out\x64\Release\tests\bench_richedit2_memory.exe --mb 8
out\x64\Release\tests\bench_richedit2_memory.exe --mb 16

rem 性能（12 轮弃前 2 轮取中位数）
for /L %i in (1,1,12) do out\x64\Release\tests\bench_richedit2.exe --iterations 200 > out\o6\bench_r%i.txt
```


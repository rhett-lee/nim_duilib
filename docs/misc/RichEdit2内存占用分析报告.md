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

### 9.2 O1 的实现要点

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

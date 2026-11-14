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

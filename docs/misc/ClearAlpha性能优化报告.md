# ClearAlpha / RestoreAlpha / Render Clear / ClearRect 性能优化报告

本文针对以下 4 个高频调用的渲染函数，完成单元测试补充、Release 性能基线采集、实现优化与优化效果评估：

| 函数 | 实现文件 |
| --- | --- |
| `BitmapAlpha::ClearAlpha` | `duilib/Render/BitmapAlpha.cpp` |
| `BitmapAlpha::RestoreAlpha`（两个重载） | `duilib/Render/BitmapAlpha.cpp` |
| `Render_Skia::Clear` | `duilib/RenderSkia/Render_Skia.cpp` |
| `Render_Skia::ClearRect` | `duilib/RenderSkia/Render_Skia.cpp` |

## 1. 结论摘要

1. `BitmapAlpha::RestoreAlpha` 是唯一的计算瓶颈：优化前逐像素分支判断，优化后改为 SSE2 一次处理 4 个像素（分支消除），
   在 1920x1080 上耗时下降约 **50%**（约 **2.0 倍**加速），512x512 上下降约 **59%**（约 **2.4 倍**加速），已达到 memset 级别的内存带宽上限。
2. `Render_Skia::ClearRect` 在"区域横向覆盖整行"时改为一次连续填充（原来按 1080 行逐行填充），整画布清除耗时下降约 **13.7%**。
3. `BitmapAlpha::ClearAlpha`、`Render_Skia::Clear` 原本已使用 `memset` / 编译器自动优化的填充循环，优化前后处于同一水平（噪声带内），
   已接近内存带宽上限，没有进一步优化空间。
4. `RestoreAlpha`（无条件置 255 的重载）原实现已被编译器自动向量化，无需手写 SIMD；实测手写 SSE2 反而更慢，已保持原标量写法。
5. 全部单元测试在 Debug / Release 下均通过：`bitmap_alpha: 473 checks, 0 failed`、`render_api: 630 checks, 0 failed`。

## 2. 测试环境与方法

- 平台：Windows x64，Release 配置（MSVC），CPU 后端（Raster）离屏画布
- 构建：`python3 build/build.py build --config Release --arch x64 --offline`
- 基准程序：`tests/bench_clear_alpha.cpp`（新增，不注册到 ctest，属于手工评估工具）
- 指标定义：
  - `us/call`：单次调用平均耗时（微秒）
  - `MB/s`：估算的有效内存流量（只写 = 1N，读+写 = 2N，N 为区域字节数），用于判断是否达到带宽上限
- 抗噪方法：因机器存在 ±20% 的抖动，采用**同一份基准程序分别链接"优化前/优化后"的 duilib 静态库**，
  交替运行 4 轮（各场景取 4 轮中位数），以消除系统负载漂移的影响。迭代次数 300。

复现命令：

```bat
rem 1) 构建 Release
python3 build/build.py build --config Release --arch x64 --offline

rem 2) 运行基准（默认 200 次迭代，可用 --iterations 调整）
out\x64\Release\tests\bench_clear_alpha.exe --iterations 300
```

## 3. 新增单元测试

### 3.1 `tests/test_bitmap_alpha.cpp`（新增，注册到 ctest，名称为 `bitmap_alpha`）

| 用例 | 覆盖内容 |
| --- | --- |
| BA-01 ~ BA-03 | `ClearAlpha` 整图清除，alpha = 0 / 255 / 77，校验 RGB 与 Alpha 全部被覆盖 |
| BA-04 ~ BA-05 | 局部矩形、单像素清除，矩形外数据保持不变 |
| BA-06 ~ BA-07 | 矩形完全越界（裁剪为整图）与部分越界（只清除相交部分） |
| BA-08 | 空矩形、反向矩形、完全在位图外均不修改数据 |
| BA-09 ~ BA-12 | `RestoreAlpha(alpha)`：alpha==0 与 alpha!=0 的两条恢复规则，RGB 必须保持不变 |
| BA-13 ~ BA-15 | 阴影边距（`UiPadding`）裁剪，边距大于矩形时不做修改 |
| BA-16 ~ BA-17 | 矩形越界裁剪、空矩形/反向矩形 |
| BA-18 ~ BA-20 | alpha == 255 边界值、横向 1 像素、纵向 1 像素的窄条区域 |
| BA-21 ~ BA-27 | `RestoreAlpha()` 无条件置 255：整图/局部/边距/越界/空矩形/幂等性 |
| BA-28 ~ BA-30 | `ClearAlpha` + `RestoreAlpha` 配合：透明→不透明、半透明清除值→0、上层改写过的 Alpha 必须保留 |

### 3.2 `tests/test_render_api.cpp`（补充 RC-01 ~ RC-14）

| 用例 | 覆盖内容 |
| --- | --- |
| RC-01 ~ RC-03 | `Clear` 不透明黑、全透明、半透明（校验 Alpha 预乘结果 `0x80FF0000 -> 0x80800000`） |
| RC-04 | `Clear` 不受视区原点（`SetWindowOrg`）影响，仍清除整个画布 |
| RC-05 | `Resize` 后 `Clear` 覆盖新的画布区域 |
| RC-06 ~ RC-07 | `ClearRect` 基本用法与 1x1 像素，逐边界像素校验 |
| RC-08 ~ RC-10 | 左上越界、右下越界、完全在画布外或空矩形均不修改像素 |
| RC-11 | `ClearRect` 半透明颜色的 Alpha 预乘结果（`0x4000FF00 -> 0x40004000`） |
| RC-12 | `ClearRect` 的客户区坐标按视区原点换算为设备坐标 |
| RC-13 | `ClearRect` 覆盖整画布时结果与 `Clear` 一致 |
| RC-14 | `Clear` 传入 0 值颜色时整画布清零 |

## 4. 性能基线（优化前）

首次独立运行（迭代 200 次，3 轮）的基线数据，用于确认量级与瓶颈定位：

| 场景 | us/call（3 轮） | 结论 |
| --- | --- | --- |
| `ClearAlpha` full 1920x1080 alpha=0 | 285.8 / 310.2 / 264.8 | memset，约 27 GB/s |
| `ClearAlpha` rect 960x540 alpha=0 | 72.7 / 65.0 / 74.5 | 按行 memset |
| `ClearAlpha` full alpha=128 | 266.8 / 260.3 / 330.4 | memset |
| `Clear+Restore` pair full alpha=0 | 1838 / 2051 / 2197 | **瓶颈**：逐像素分支 |
| `Clear+Restore` pair full alpha=128 | 1927 / 2157 / 2400 | **瓶颈**：逐像素分支 |
| `RestoreAlpha` full (=>255) | 388.9 / 651.4 / 885.1 | 已被编译器向量化 |
| `RestoreAlpha` rect 960x540 (=>255) | 77.4 / 81.2 / 94.0 | 同上 |
| `Clear+Restore` pair 512x512 alpha=0 | 173.7 / 174.0 / 181.9 | **瓶颈**：逐像素分支 |
| `Render::Clear` 1920x1080 不透明白 | 266.2 / 318.7 / 255.3 | 带宽上限 |
| `Render::Clear` 1920x1080 全透明 | 279.5 / 336.2 / 268.8 | memset |
| `Render::ClearRect` rect 960x540 | 61.6 / 78.0 / 80.7 | 与 memset 相当 |
| `Render::ClearRect` 整画布 | 310.3 / 403.6 / 302.2 | 逐行填充，有合并空间 |

## 5. 优化后的实现

### 5.1 `BitmapAlpha`（`duilib/Render/BitmapAlpha.cpp`）

- 新增内部辅助函数（匿名命名空间，按行内连续像素整段处理）：
  - `RestoreAlphaZeroSpan`：SSE2 处理 4 像素/次，`isZero = (v & 0xFF000000) == 0` 分支消除后 `v |= (isZero & 0xFF000000)`；
  - `RestoreAlphaValueSpan`：SSE2 处理 4 像素/次，一次性完成"等于 alpha → 0、等于 0 → 255、其余保留"三条规则；
  - `RestoreAlphaOpaqueSpan`：保持标量 OR 循环（编译器已能自动向量化）。
- 三个 `RestoreAlpha` 主循环由"逐像素判断"改为"逐行调用 span 函数"，行指针递增方式不变。
- `ClearAlpha`：矩形横向覆盖整行时，把多行连续内存合并为一次 `memset`。
- SSE2 仅在 `_M_X64 / _M_AMD64 / __SSE2__ / _M_IX86_FP>=2` 时启用，其他平台（如 ARM64）自动退化为等价的标量实现。

### 5.2 `Render_Skia`（`duilib/RenderSkia/Render_Skia.cpp`）

- `ClearRect`：区域横向覆盖整行（`left==0 && right==width`）时，改为对整个连续内存块做一次填充，替代逐行填充。
- `Clear` / `ClearRect` 的填充统一走内部 `FillPixelSpan`（标量循环，编译器会生成 `rep stosd` 或向量存储，
  实测与 `memset` 性能相当；手写 SSE2 存储反而略慢，因此未采用）。
- 未修改语义：`PremultiplyARGB`、视区原点换算、GPU 后端分支、边界裁剪逻辑均保持原样。

## 6. 优化前后对比（Release，4 轮交替取中位数）

| 场景 | 基线 us/call | 优化后 us/call | 变化 |
| --- | --- | --- | --- |
| `ClearAlpha` full 1920x1080 alpha=0 | 274.3 | 277.1 | 持平（memset 带宽上限） |
| `ClearAlpha` rect 960x540 alpha=0 | 71.2 | 65.4 | -8.1%（噪声带内） |
| `ClearAlpha` full 1920x1080 alpha=128 | 298.8 | 258.5 | -13.5%（噪声带内） |
| **`Clear+Restore` pair full alpha=0** | **1805.5** | **899.6** | **-50.2%（约 2.0 倍）** |
| **`Clear+Restore` pair full alpha=128** | **1856.6** | **1181.9** | **-36.3%（约 1.6 倍）** |
| `RestoreAlpha` full (=>255) | 439.2 | 433.5 | 持平 |
| `RestoreAlpha` rect 960x540 (=>255) | 80.2 | 77.0 | 持平 |
| **`Clear+Restore` pair 512x512 alpha=0** | **169.9** | **70.4** | **-58.6%（约 2.4 倍）** |
| `RestoreAlpha` 512x512 (=>255) | 21.8 | 22.9 | 持平 |
| `Render::Clear` 1920x1080 不透明白 | 260.8 | 285.1 | 持平（噪声带内） |
| `Render::Clear` 1920x1080 全透明 | 269.6 | 277.6 | 持平（均为 memset） |
| `Render::ClearRect` rect 960x540 | 72.1 | 69.4 | 持平 |
| **`Render::ClearRect` 整画布** | **314.5** | **271.4** | **-13.7%** |

### 6.1 效果分析

1. `RestoreAlpha`（alpha==0）在 1920x1080 上的单独耗时由约 1530 us 降到约 630 us，读+写有效带宽由约 11 GB/s 提升到约 26 GB/s，
   接近同机 `memset` 的写带宽（约 28~30 GB/s），说明已经消除计算瓶颈、转为内存带宽受限。
2. 512x512（1 MB，可驻留 L2/L3）场景提升更明显（2.4 倍），验证收益主要来自"分支消除 + 4 像素并行"。
3. `ClearRect` 整画布场景由 1080 次逐行填充合并为 1 次连续填充，省去每行循环建立开销，收益约 13.7%。
4. 其余函数原本已处于内存带宽上限，改造前后的差异在 I/O 噪声（±10%）以内，不做过度优化。

## 7. 变更文件清单

| 文件 | 变更 |
| --- | --- |
| `duilib/Render/BitmapAlpha.cpp` | 新增 SSE2 span 实现；`RestoreAlpha` 改为按行整段处理；`ClearAlpha` 增加整行合并 memset |
| `duilib/RenderSkia/Render_Skia.cpp` | 新增 `FillPixelSpan`；`Clear`/`ClearRect` 改用统一填充；`ClearRect` 增加整行合并填充 |
| `tests/test_bitmap_alpha.cpp` | 新增 BitmapAlpha 单元测试（BA-01 ~ BA-30） |
| `tests/test_render_api.cpp` | 补充 `Clear`/`ClearRect` 单元测试（RC-01 ~ RC-14） |
| `tests/bench_clear_alpha.cpp` | 新增性能基准程序 |
| `tests/CMakeLists.txt` | 注册 `test_bitmap_alpha`（ctest）与 `bench_clear_alpha`（手工运行） |

## 8. 注意事项

1. Debug 配置含断言与未优化代码，绝对值不具参考意义；性能评估必须使用 Release。
2. 该基准对机器负载敏感，单次运行可能有 ±20% 波动，建议使用本报告的"交替运行多轮取中位数"方式对比。
3. `RestoreAlpha` 的语义未改变：`ClearAlpha` 之后被上层直接改写过 Alpha 的像素，仍会保留其新值（不会被统一置为 255），
   对应单元测试 BA-30。

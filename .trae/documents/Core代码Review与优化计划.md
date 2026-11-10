# duilib/Core 代码 Review 与优化计划（排除 Control / PlaceHolder / Box）

## Context

用户要求审查 `E:\develop\nim_duilib\duilib\Core` 目录下的代码（Control、PlaceHolder、Box 三个类已在上一轮完成，本轮排除），目标是：
1. 发现并修复错误；2. 优化可改进点；3. 分析并降低内存占用；4. 分析并提升执行性能；5. **不做大改，避免引入新 bug**。

本轮已完成对下列文件的实际阅读与初步审查（其余大文件在执行阶段按同一原则继续审查）：

- 基础数据类型：`UiRect.h`、`UiRectF.h`、`UiPoint.h`、`UiPointF.h`、`UiSize.h`、`UiSize16.h`、`UiSize64.h`、`UiMargin.h`、`UiPadding.h`、`UiColor.h`、`UiFixedInt.h`、`UiEstInt.h`、`UiFont.h`、`UiString.h`、`UiTypes.h`
- 颜色体系：`ColorMap.*`、`StateColorMap.*`、`StateColorMap2.*`、`ColorManager.cpp`、`ColorConverter.cpp`
- 运行时设施：`TimerManager.*`、`ThreadManager.cpp`、`WindowManager.cpp`、`WindowRoot.*`、`DpiManager.cpp`、`ImageList.cpp`、`IconManager.cpp`、`ZipManager.cpp`、`LangManager.cpp`、`ControlFinder.cpp`、`BoxShadow.cpp`、`DragWindow.cpp`

## 已确认的问题与修改方案（均为小范围、可回退）

### A. 正确性修复

| # | 文件 | 问题 | 修改 |
|---|------|------|------|
| A1 | [DpiManager.cpp](file:///e:/develop/nim_duilib/duilib/Core/DpiManager.cpp#L271-L278) | `GetScaleInt(int32_t, uint32_t)` 把 `MulDiv` 的 `int32_t` 结果强制转成 `uint32_t` 再赋回 `int32_t`，负值时为实现定义行为 | 去掉错误的 `(uint32_t)` 强转，直接接收 `int32_t` 结果 |
| A2 | [DpiManager.cpp](file:///e:/develop/nim_duilib/duilib/Core/DpiManager.cpp#L248-L261) | `GetScaleFloat(int32_t/uint32_t)` 先做 32 位整数乘法，坐标值偏大时会整数溢出（UB / 精度丢失） | 用 `int64_t`/`uint64_t` 作为中间量后再除以 100.0f，结果不变但无溢出 |
| A3 | [ImageList.cpp](file:///e:/develop/nim_duilib/duilib/Core/ImageList.cpp#L17-L28) | `ASSERT` 要求宽高均 `> 0`，但实际判断是 `< 0`，`0` 尺寸会通过并污染缓存 | 判断改为 `<= 0` 时直接返回，与断言语义一致 |
| A4 | [WindowRoot.cpp](file:///e:/develop/nim_duilib/duilib/Core/WindowRoot.cpp#L568-L590) | `ASSERT(m_pControlFinder->GetRoot() == ...)` 在 Debug 下会解引用可能为空的指针 | 断言与后续条件里先判断 `m_pControlFinder != nullptr`（Release 行为不变） |
| A5 | [ZipManager.cpp](file:///e:/develop/nim_duilib/duilib/Core/ZipManager.cpp#L158-L171) | 缓存压缩包文件列表时构造了 `DString fileName`，仅用于转换却从未使用（每文件一次无谓的编码转换+分配） | 删除该临时变量，仅保留 `DStringW` 结果 |
| A6 | [ControlFinder.cpp](file:///e:/develop/nim_duilib/duilib/Core/ControlFinder.cpp#L173-L191) | `RemoveControl` 删空 vector 后不删除 map 条目，动态创建/销毁唯一名控件时 `m_controlNameMap` 的节点无限增长 | 列表删空时 `erase` 掉该键（查找与内存同时受益） |

### B. 性能优化

| # | 文件 | 问题 | 修改 |
|---|------|------|------|
| B1 | [ZipManager.cpp](file:///e:/develop/nim_duilib/duilib/Core/ZipManager.cpp#L151-L188)、[GetZipFileList](file:///e:/develop/nim_duilib/duilib/Core/ZipManager.cpp#L265-L318) | 循环内每个文件都 `resize(1024)` 一个 `std::vector<char>`（一次堆分配/文件） | 把文件名缓冲提到循环外，循环内仅 `resize` 到实际需要（复用容量，零重复分配） |
| B2 | [ColorMap.cpp](file:///e:/develop/nim_duilib/duilib/Core/ColorMap.cpp#L72-L80) | `for (auto iter : uiColors)` 每项拷贝一对 `DString`（启动期标准色表初始化） | 改为 `for (const auto& iter : uiColors)` |
| B3 | [IconManager.cpp](file:///e:/develop/nim_duilib/duilib/Core/IconManager.cpp#L63-L75) | `GetIconBitmapData` 先把整块位图数据拷贝出来再校验，无效数据也付出一次大块内存拷贝 | 先校验尺寸/长度，通过后再拷贝 |
| B4 | [WindowManager.cpp](file:///e:/develop/nim_duilib/duilib/Core/WindowManager.cpp#L112-L124) | `GetWindowById` 命中后不 `break`，始终全表扫描 | 命中后 `break` |
| B5 | [UiString.h](file:///e:/develop/nim_duilib/duilib/Core/UiString.h#L61-L134) | `UiStringT` 没有移动构造/移动赋值，`std::vector<UiString>` 扩容与容器交换均为深拷贝 | 增加 `noexcept` 移动构造与移动赋值（纯增量，不改变现有语义） |

### C. 内存占用分析结论（写进最终汇报）

- `UiString`（8 字节指针 + 按需堆分配）对比 `DString`（MSVC x64 下 `std::wstring` 为 32 字节）在每个控件上可省约 24 字节；`StateColorMap`（`kControlStateCount` 个 `UiString`）与 `StateColorMap2`（`kControlStateCount` 个 `TColorProperty = 8+8+4`）已经是紧凑布局，本轮不再压缩，避免破坏接口。
- `ImageList::GetImageStringId` 为 O(n) 线性比对（新增图片时 O(n²)）。可通过"图片字符串 → id"的哈希索引优化，但会额外占用内存；按用户"降低内存"的要求，本轮**不加索引**，只在报告中说明取舍。
- `ControlFinder::RemoveControl` 空条目清理（A6）是纯收益的内存修复。

### D. 需要在执行阶段确认后才能决定的点（先读代码，再决定是否改）

- `ThreadManager::GetNextTaskId()` 未加锁，需用 Grep 确认调用点是否都在 `m_threadMutex` 保护内；若存在跨线程调用，改为 `std::atomic<size_t>`。
- `ControlFinder::FindDroppableBox` 把 `nDropInId == 0` 转成 `nullptr` 传入 `Box::FindControl`，需确认 `FindControl` 对空 `pData` 的处理是否安全。

### E. 执行阶段继续覆盖的文件（只改明确缺陷）

按同一"最小改动"原则继续审查：`FontManager.cpp`、`ImageManager.cpp`、`ThemeManager.cpp`、`Shadow.cpp`、`WindowBuilder.cpp`、`GlobalManager.cpp`（仅命中明确 bug / 明显浪费时才修改，并逐条说明理由）。

## 明确不做的事

- 不重写 `Window.cpp` / `WindowBase.cpp` / `WindowBuilder.cpp` 的架构，不改公开接口签名，不做批量格式化。
- 不新增文件、不加额外索引容器（与"降低内存"冲突）。
- 不提交代码（只做本地修改）。

## 验证

1. 用显式 MSVC 环境变量编译 Debug（只编 `duilib` target，避免与其它构建争用 PDB）：
```powershell
$msvc="C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231"
$sdk="C:\Program Files (x86)\Windows Kits\10"; $sdkver="10.0.26100.0"
$env:INCLUDE="$msvc\include;$sdk\Include\$sdkver\ucrt;$sdk\Include\$sdkver\um;$sdk\Include\$sdkver\shared;$sdk\Include\$sdkver\winrt;$sdk\Include\$sdkver\cppwinrt"
$env:LIB="$msvc\lib\x64;$sdk\Lib\$sdkver\ucrt\x64;$sdk\Lib\$sdkver\um\x64"
cmake --build "E:\develop\nim_duilib\out\x64\Debug" --target duilib
```
2. 编译通过后逐条自检改动点（负数缩放、0 尺寸图片、压缩包文件列表、动态控件增删等路径），并在最终汇报中给出"问题 → 修改 → 影响面 → 内存/性能收益"的清单。

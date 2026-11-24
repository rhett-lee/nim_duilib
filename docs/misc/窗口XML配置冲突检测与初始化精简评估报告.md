# 窗口 XML 配置冲突检测与初始化精简评估报告

> 类型：代码审查 + 评估结论（2026-11-24）
> 实施状态：已按本报告第四节顺序实施完成（2026-11-24），变更明细见 [窗口XML配置冲突检测与初始化精简重构变更摘要.md](./窗口XML配置冲突检测与初始化精简重构变更摘要.md)
> 范围：`WindowBuilder` 的 XML 解析、`Resolve*Attributes` 的冲突规则、窗口创建到显示之间的属性应用流程
> 评估目标：
> 1. 针对错误的配置给予纠错，并输出提示信息；
> 2. 窗口大小设置、属性设置、重绘动作等做到最精简，降低界面启动成本、减少抖动。

## 一、当前流程

窗口创建与应用的完整链路：

1. `ParseWindowCreateAttributes` 单趟解析 XML（`duilib/Core/WindowBuilder.cpp` L546-L622）→ Resolve 出分层/阴影/初始尺寸 → `SyncCreateWindowAttributes` 把 caption/层窗口/尺寸写进 `WindowCreateParam`（`duilib/Core/NativeWindow_Windows.cpp` L498-L548）。
2. `CreateWindowEx` 创建**隐藏窗口**（style 不含 `WS_VISIBLE`，`duilib/Core/NativeWindow_Windows.cpp` L177-L182）。
3. WM_CREATE 内依次执行：
   - `PreInitWindow`：CreateShadow → CreateControls → **ApplyWindowAttributes 阶段 1~6** → AttachShadow → AttachBox → OnUseSystemCaptionBarChanged（`duilib/Core/Window.cpp` L517-L566）；
   - `PostInitWindow`：创建 Render、`Resize(m_szInitSize)`、AutoResize、InitializeShadow（`duilib/Core/Window.cpp` L568-L602）。
4. 应用层随后 `ShowWindow`。

整体设计（单趟解析、Resolve 集中规则、固定阶段应用、隐藏创建）是合理的，问题集中在“纠错不提示”和“初始化期重复动作”两类。

## 二、目标（1）：冲突检测与提示

### 2.1 已有的机制

- 未知属性：断言 + 日志（`duilib/Core/WindowBuilder.cpp` L787-L793）；
- min/max 夹取、百分比与屏幕范围校验集中在 Resolve / AttributeUtil；
- GL + opacity 降级有日志（`duilib/Core/WindowBuilder.cpp` L654-L656）。

### 2.2 缺口（按影响排序，实施前现状）

| 级别 | 问题 | 现状 |
|---|---|---|
| 高 | 已知属性、非法值 | `shadow_type` 非法字符串静默忽略；`render_backend_type` 非法值仅 `ASSERT(0)`（`duilib/Core/WindowAttributes.cpp` L184-L190）；`alpha/opacity` 越界直接丢弃；`round_corner` 单轴非 0 静默 return。release 下全部无提示 |
| 高 | 互斥规则静默纠正 | `use_system_caption=true` + `layered_window=true` 被强制改为 false（`duilib/Core/WindowAttributes.cpp` L364-L367），无任何日志；SDL 下“未配置 layered 默认 true”会覆盖互斥规则（L369-L377），这是既定设计，但同样无提示 |
| 中 | `layered_window=false` + `opacity<255` | 仅 debug 有 `ASSERT`（`duilib/Core/WindowBuilder.cpp` L596-L601）；release 下阶段 4 的 opacity 会反向把窗口变成层窗口，与 XML 显式 false 矛盾且无提示 |
| 中 | GL + 层窗口 | Resolve 只处理 opacity（`duilib/Core/WindowAttributes.cpp` L416-L426），未处理显式 `layered_window=true`：最终 GL 关层窗口、XML 值被静默丢弃 |
| 低 | 尺寸类冲突 | size 超屏被压到 95%、低于 min/超过 max 被夹取、非法百分比被改成 75%，均无日志（`duilib/Utils/AttributeUtil.cpp` L359-L367、`duilib/Core/WindowAttributes.cpp` L452-L463） |

### 2.3 建议

在 `Resolve*Attributes` 里返回一个 `std::vector<ConfigConflict>`（属性名、原值、纠正后值、原因），由 `WindowBuilder` 统一输出一条日志并保留 debug 断言。这样纠错逻辑和提示只有一个出口，不会散落到各个 setter。

> 实施说明：实际实现为 `WindowAttributeValues::m_warnings`（字符串列表）+ Resolve 接口的可选 `pWarnings` 出参，由 `WindowBuilder` 统一输出，语义与本建议一致，未引入新的 `ConfigConflict` 类型。

## 三、目标（2）：动作精简

### 3.1 已经合理的部分

- 隐藏创建（无可见抖动）；
- `SetUseSystemCaption` 的 style 写入有比较（`duilib/Core/NativeWindow_Windows.cpp` L883-L896），且重绘仅在状态变化时发生；
- min/max 只是成员赋值；
- `SetSizeBox / SetCaptionRect / SetSysMenuRect` 无重绘；
- `SetInitSize` 在 root 未挂载时只存值，仅 PostInitWindow 应用一次（`duilib/Core/Window.cpp` L1339-L1349）。

### 3.2 仍可精简的点（实施前现状）

1. **初始尺寸解析两次、应用两次**
   创建前按 `GlobalManager::Dpi()` + 主屏工作区解析一次（`duilib/Core/WindowBuilder.cpp` L609-L620），PostInitWindow 又按窗口 DPI + 所在屏解析并 `Resize`。两者相等时 `Resize` 仍无条件 `SetWindowPos`（`duilib/Core/WindowBase.cpp` L552-L578 无 no-op 判断），会多一次 `WM_SIZE` → 重新布局 + Render resize。跨 DPI 屏幕时两次结果还会不同。
   建议：PostInitWindow 先比较当前窗口尺寸，不同才 Resize；更彻底的做法是创建阶段不再设置尺寸，统一在 PostInitWindow 设置一次（会影响创建时的居中计算，需一并调整）。

2. **阴影状态刷新重复**
   `SetShadowType`、`SetShadowAttached`、`InitializeShadow` 各自调用 `ProcessWindowShadowTypeChanged`（`duilib/Core/Window.cpp` L1257-L1262），每次做 `UpdateWindowRGN` + `UpdateLayeredWindowStyleEx`。初始化期最多执行 3 次。
   建议：加一个“初始化批量模式”，把 RGN/层窗口刷新延迟到 PreInitWindow 末尾或 PostInitWindow 统一执行一次。

3. **阴影先建后关**
   `CreateShadow(IsLayeredWindow() || !IsUseSystemCaption())`（`duilib/Core/Window.cpp` L530-L531）使用近似值，而 XML 的 `shadow_attached` 已在 `createAttributes` 中，未传递。XML 写 `shadow_attached="false"` 或系统标题栏时，会先创建阴影再在阶段 3/OnUseSystemCaptionBarChanged 关闭。
   建议：创建前就用解析结果决定初始阴影状态。

4. **阶段 2 的 `SetUseSystemCaption` 重复下发**
   值通常与创建时一致，style 写入已被比较挡住，但仍会执行 `UpdateMinMaxBoxStyle` 与 `OnNativeUseSystemCaptionBarChanged` 回调。成本低，可在值与当前一致时跳过（需确认回调中自绘标题栏可见性的初始化时机，不要跳过首次绑定）。

5. **阶段 4 的冲突日志**
   只在真正冲突时输出，已精简，无需改动。

## 四、建议实施顺序

1. **低风险、收益明确**：补已知属性非法值日志；把互斥/冲突的纠正集中到 Resolve 并统一输出（目标 1）。
2. **尺寸去重**：PostInitWindow 的 `Resize` 增加尺寸比较，顺带减少一次 `WM_SIZE`（目标 2 第 1 点）。
3. **阴影刷新合并**：初始化批量模式，减少 RGN/层窗口重复刷新（目标 2 第 2 点）。
4. **阴影初始状态前置**：用 `createAttributes` 的解析结果初始化阴影，去掉“先建后关”（目标 2 第 3 点）。
5. **GL + layered 规则并入 Resolve**，与 opacity 降级同处处理并提示（目标 1 第 2 点）。

## 五、结论

流程骨架合理，不需要重构；主要欠缺是“纠错静默”和“初始化期的重复下发/重复刷新”。上述缺口已按第四节的顺序完成实施。

## 六、实施结果（2026-11-24）

| 报告项 | 实施方式 | 状态 |
|---|---|---|
| 目标 1：已知属性非法值提示 | `shadow_type`、`render_backend_type`、`alpha/opacity`、`msaa`、`round_corner` 增加校验与告警，并按规则纠正 | 已完成 |
| 目标 1：互斥/冲突提示 | 系统标题栏与层窗口冲突、非分层窗口的透明度配置、`size` 与 min/max 冲突均记录告警 | 已完成 |
| 目标 1：统一出口 | 告警记录在 `WindowAttributeValues::m_warnings`，由 `WindowBuilder` 统一输出 | 已完成（未新增强制类型） |
| GL + 层窗口并入 Resolve | 显式 `layered_window="true"` 与 `opacity<255` 一样触发 CPU 降级并提示；**SDL 后端不参与该判断**（SDL 透明窗口与 GL 兼容） | 已完成 |
| 目标 2 第 1 点：初始尺寸去重 | `PostInitWindow` 在 Resize 前比较窗口当前尺寸，相同则跳过 | 已完成 |
| 目标 2 第 2 点：阴影刷新合并 | 初始化应用 XML 期间延迟阴影刷新，在 `PostInitWindow` 创建渲染接口前统一执行一次 | 已完成 |
| 目标 2 第 3 点：阴影先建后关 | `CreateShadow` 的初始附加状态改用创建前解析出的 `shadow_attached`；`Shadow` 对象仍需创建（阶段 3 属性设置依赖它） | 已完成（实现方式调整） |
| 目标 2 第 4 点：阶段 2 重复下发 | 未处理。值一致时 style 写入本已被比较挡住，剩余成本仅为 `UpdateMinMaxBoxStyle` 与回调调用 | 不实施 |
| 目标 2 第 5 点：阶段 4 冲突日志 | 保持仅在真正冲突时输出，并配合告警机制补充原因 | 已完成 |

遗留：
1. `size` 超出屏幕压缩到 95%、非法百分比改为 75% 的纠正在 `AttributeUtil` 层，仍无日志（该函数被控件解析共用，本次未改动）。
2. `docs/misc` 中的评估结论与实施结果以本文件与变更摘要为准。

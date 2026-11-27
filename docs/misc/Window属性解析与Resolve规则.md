# Window 属性解析与 Resolve 规则

> 适用范围：`duilib/Core/WindowAttributes.h`、`duilib/Core/WindowBuilder.cpp`、`duilib/Core/Window.cpp` 及 Windows/SDL 原生窗口实现。
>
> 文档状态：设计规则与现状差异分析。Windows 原生层的分层窗口规则与 SDL 原生层的透明窗口规则已实现，其余差异待修复。

## 1. 文档目的

本文档用于统一 Window 属性的解析、冲突处理与 Resolve 规则，重点覆盖：

- 窗口 XML、Class、全局配置、运行时 API 四类配置来源的优先级；
- 标题栏、渲染后端、阴影、分层窗口、透明度之间的冲突处理顺序；
- Windows 非 SDL 与 SDL（含 Windows SDL）两条流程的差异；
- 动态切换时"请求值"与"有效值"的保存与恢复；
- 当前实现与目标规则之间的差异，以及后续改造建议。

## 2. 核心概念

### 2.1 请求值（Requested）与有效值（Effective）

| 概念 | 含义 |
|---|---|
| 请求值 | 用户通过 XML、Class、全局配置或运行时 API 明确设置的属性值，冲突时也必须保留 |
| 有效值 | 经过 Resolve 后，当前真正生效的属性值 |
| 功能开关 | 是否启用某项功能，如 `use_system_caption`、`shadow_attached` |
| 功能参数 | 功能的具体形态，如 `shadow_type`、`render_backend_type` |

冲突处理只关闭"有效值"，**不得丢弃请求值**。后续动态切换（如关闭系统标题栏、GL 切回 CPU）时，应能重新 Resolve 并恢复请求值。

### 2.2 平台能力

Resolve 不应直接写死平台分支，而应基于平台能力描述：

| 能力 | 说明 |
|---|---|
| `glSupported` | 是否支持 NativeGL 后端（Windows 非 SDL 才可能为 true） |
| `osLayeredWindowSupported` | 是否支持操作系统分层窗口（仅 Windows 非 SDL 使用） |
| `sdlTransparentSupported` | SDL 窗口是否支持 `SDL_WINDOW_TRANSPARENT`（取决于平台与渲染器） |
| `systemShadowSupported` | 是否支持系统阴影 |
| `layeredRequiredForSelfDrawnShadow` | 自绘阴影是否必须使用分层窗口（Windows 非 SDL 为 true；SDL 为 false） |

## 3. 属性来源与优先级

### 3.1 总体优先级

从高到低：

1. **运行时显式 API**：`Window::SetRenderBackendType`、`Window::SetAttribute`、`Window::SetLayeredWindow`、`Window::SetShadowType` 等；
2. **窗口 XML / Class 属性**；
3. **全局配置**（Global 节点或 `SetGlobalRenderBackendType`）；
4. **内置默认值**。

说明：

- XML 中显式设置了 `render_backend_type` 时，**该值优先级最高**；全局 `render_backend_type` 仅当窗口 XML 未设置时生效。
- 运行时显式 API 属于命令式覆盖，生效后会更新该窗口的请求值；此后该窗口不再回退到 XML 值，除非再次显式设置。
- 全局配置本质上是"默认值"，不应改写已经由 XML 明确配置的字段。

### 3.2 按字段独立解析

`render_backend_type`、`msaa`、`vsync` 应分别独立解析，而不是"XML 定义了后端就整组使用 XML"：

| 字段 | 解析顺序 |
|---|---|
| `render_backend_type` | XML > 全局 > 内置（Raster） |
| `msaa` | XML > 全局 > 内置（0） |
| `vsync` | XML > 全局 > 内置（`kDefaultDisableVsync`） |

示例：全局配置 `GL + msaa=4 + vsync=off`，窗口 XML 只写 `render_backend_type="GL"`，则最终应仍使用全局的 `msaa=4、vsync=off`，而不是回落到 `msaa=0、vsync=on`。

### 3.3 全局应用到已有窗口

`SetGlobalRenderBackendType(..., bApplyToAllWindows=true)` 是显式运行时命令：

- 对当前已创建的窗口立即生效；
- 同时更新这些窗口的请求值，保证后续重新 Resolve 时不会意外恢复旧值；
- 对之后新建的窗口，仍按 3.1 的 XML 优先规则解析。

## 4. 解析流程

### 4.1 创建前

`GetCreateWindowAttributes` → `ParseWindowCreateAttributes`：

1. 解析 XML/Class，得到请求值；
2. 合并全局默认值；
3. 按第 5 节顺序 Resolve；
4. 生成 `WindowCreateAttributes`，创建原生窗口；
5. SDL 的特殊点：`SDL_WINDOW_TRANSPARENT` 只能在创建时决定，必须在本阶段确定。

### 4.2 创建后

`PreInitWindow` → `ParseWindowAttributes` → `ApplyWindowAttributes`：

1. 重新解析得到请求值；
2. 按第 5 节顺序 Resolve；
3. 按依赖顺序应用：渲染后端 → 结构性属性 → 阴影 → 分层窗口/透明度 → 其他 → 初始尺寸；
4. SDL 的透明窗口标记不在本阶段修改，只做一致性校验与告警。

### 4.3 动态切换

支持动态切换的属性：

- `use_system_caption`
- `render_backend_type`（仅 Windows 非 SDL，GL ↔ CPU）
- `shadow_attached` / `shadow_type`
- `opacity` / `alpha`（受分层窗口与后端约束）

每次动态切换都应：

1. 更新请求值；
2. 重新执行完整 Resolve（第 5 节顺序）；
3. 应用有效值差异；
4. 对被关闭的冲突属性输出告警，但保留请求值。

## 5. Resolve 的总体顺序

优先级从高到低：

1. **确定标题栏类型（`use_system_caption`）**：与标题栏冲突的属性，有效值直接关闭，并告警；
2. **确定渲染后端（`render_backend_type`）**：与后端冲突的属性，有效值直接关闭，并告警；
3. **确定阴影类型（`shadow_attached` / `shadow_type`）**：与阴影冲突的属性，有效值直接关闭，并告警；
4. **最后确定分层窗口（`layered_window`）**：与分层窗口冲突的属性，有效值直接关闭，并告警。

约束：

- 高优先级步骤可以关闭低优先级属性的"有效值"，但不能删除其"请求值"。
- 低优先级步骤不能反向推翻高优先级结论。
- 任何一次 Resolve 都应是幂等的：相同请求值 + 相同平台能力 = 相同有效值。

## 6. 分项规则

### 6.1 标题栏（use_system_caption）

| 项目 | 规则 |
|---|---|
| 默认值 | `false` |
| 生效值 | XML/API 请求值 |
| 动态切换 | 支持 |

`use_system_caption=true` 时：

- **Windows 非 SDL**：
  - 关闭有效分层窗口（`layered_window`）；
  - 关闭有效 `opacity`、`alpha`；
  - 关闭有效 `shadow_attached`（自绘阴影不再绘制）；
  - 以上请求值全部保留；
  - 每关闭一项输出告警。
- **SDL（含 Windows SDL）**：
  - 关闭有效 `shadow_attached`（自绘阴影不再绘制）；
  - **不强制关闭 SDL 透明窗口**；透明窗口按 6.4 的 SDL 规则独立决定；
  - 不使用 `WS_EX_LAYERED`，因此不存在"系统标题栏与分层窗口"冲突；
  - `opacity` 仍可通过 SDL 窗口透明度生效；`alpha` 是否生效取决于透明窗口是否开启。

关闭标题栏后，应按请求值重新 Resolve，恢复此前被关闭的阴影、分层窗口、透明度等属性。

### 6.2 渲染后端（render_backend_type）

#### 6.2.1 支持范围

| 构建/平台 | 有效后端 | 说明 |
|---|---|---|
| Windows 非 SDL | GL 或 CPU | 支持运行时动态切换 |
| Windows SDL | CPU | 不支持 GL；请求值保存，有效值归一为 CPU |
| Linux/macOS/FreeBSD（SDL） | CPU | 不支持 GL |

补充约定：

- SDL 构建下，若 XML/全局请求 GL，Resolve 阶段就应归一为 CPU，并输出告警；
- 不要把"GL → CPU"的归一动作留到 `Window::SetRenderBackendType` 内部静默完成。

#### 6.2.2 Windows 非 SDL 下 GL 的冲突

GL 有效时：

- 不允许分层窗口：有效 `layered_window=false`；
- 不允许 `opacity`、`alpha`：有效值关闭；
- 自绘阴影不允许使用：有效阴影类型切换为"系统阴影"（支持时）或"无阴影"；
- 上述请求值全部保留，并逐项告警。

若原生层无法关闭分层窗口（例如窗口属性由外部强制设置）：

- 有效后端回退为 CPU；
- 按请求值重新 Resolve 分层窗口、透明度、阴影；
- 输出回退告警。

#### 6.2.3 动态切换

- **CPU → GL**：
  1. 更新请求后端为 GL；
  2. 关闭有效分层窗口/透明度/自绘阴影；
  3. 创建 GL 渲染对象；创建失败则回退 CPU，并恢复被关闭的属性。
- **GL → CPU**：
  1. 更新请求后端为 CPU；
  2. 按请求值恢复分层窗口、透明度、阴影类型；
  3. 重新 Resolve 一次，确保与当前标题栏状态不冲突。

#### 6.2.4 msaa 与 vsync

- 按 3.2 逐字段解析；
- 仅 GL 有效时有实际意义；
- CPU 有效时保留请求值，但不应影响绘制；
- 从 GL 回退 CPU 后，查询接口应能区分"请求值"与"当前实际参数"，避免 CPU 窗口报告无意义的 MSAA 值。

### 6.3 阴影（shadow_attached / shadow_type）

#### 6.3.1 开关

| 项目 | 规则 |
|---|---|
| `shadow_attached` 默认值 | `true` |
| `shadow_type` 保存 | 无论 `shadow_attached` 为 true 还是 false，都必须保存 `shadow_type` |
| 动态切换 | 支持 |

#### 6.3.2 阴影类型

1. 以 `shadow_type` 请求值为准；未设置按 `"default"` 处理；
2. `"default"`：系统支持系统阴影时，使用系统阴影；否则使用自绘默认阴影；
3. 系统阴影（System）：
   - Windows 非 SDL：不要求分层窗口；
   - SDL 及其他系统：无特殊要求；
4. 无阴影（None）：
   - Windows 非 SDL：不要求分层窗口；
   - SDL 及其他系统：无特殊要求；
5. 其他自绘阴影：
   - Windows 非 SDL：必须使用分层窗口，否则会出现绘制异常；
   - SDL 及其他系统：不要求操作系统分层窗口。

#### 6.3.3 冲突处理

- 标题栏有效（步骤 1）：有效 `shadow_attached=false`，请求值保留；
- GL 有效（步骤 2）：自绘阴影改为系统阴影（支持时）或 None，请求值保留；
- 分层窗口被明确关闭（步骤 4）：
  - Windows 非 SDL：自绘阴影无法工作，有效 `shadow_attached=false`，请求值保留，并告警；
  - SDL：不强制关闭阴影，但若透明窗口关闭，阴影边缘可能显示异常，建议告警。

### 6.4 分层窗口（layered_window）

#### 6.4.1 Windows 非 SDL

`layered_window` 对应 `WS_EX_LAYERED`。

| 条件 | 有效值 |
|---|---|
| 标题栏有效 | `false` |
| GL 有效 | `false` |
| 显式 `layered_window="true"` 且无高优先级冲突 | `true` |
| 显式 `layered_window="false"` | `false` |
| 未显式设置，但 `opacity < 255` | `true` |
| 未显式设置，但阴影类型要求分层 | `true` |
| 其他 | `false` |

显式 `layered_window="false"` 时：

- `opacity`、`alpha` 有效值关闭；
- 自绘阴影有效值关闭；
- 请求值全部保留；
- 逐项告警。

#### 6.4.2 SDL（含 Windows SDL）

SDL 不使用操作系统分层窗口，`layered_window` 语义映射为 **`SDL_WINDOW_TRANSPARENT`**。

**Windows SDL 明确规则：**

- 不设置 `WS_EX_LAYERED`，任何情况下都不设置；
- SDL 内部透明机制与分层窗口不同，混用会导致绘制提交方式不一致，出现绘制异常；
- 内部复用 `m_bIsLayeredWindow` 表示"SDL 透明窗口"状态，但该值不代表操作系统分层窗口；
- `IsLayeredWindow()` 在 SDL 下表示透明窗口是否启用，不能据此设置或查询 `WS_EX_LAYERED`。

**透明窗口默认值：**

1. XML 明确设置 `layered_window` → 以 XML 为准；
2. 创建窗口前调用过 `Window::SetLayeredWindow` → 以该 API 值为准；
3. 以上都没有 → 默认开启 `SDL_WINDOW_TRANSPARENT`；
4. 平台/渲染器不支持透明（`sdlTransparentSupported=false`）→ 有效值 false，并告警。

**创建时机：**

- `SDL_WINDOW_TRANSPARENT` 只能在 `SDL_CreateWindow` 时设置；
- 窗口创建后，`SetLayeredWindow` 无法再改变透明标记，应返回失败并告警；
- 创建后 Resolve 发现期望值与实际值不一致时，必须告警，不能静默忽略。

#### 6.4.3 分层窗口与透明度的关系

| 平台 | 关系 |
|---|---|
| Windows 非 SDL | `alpha`、`opacity` 都要求有效分层窗口 |
| SDL | `opacity` 直接映射 `SDL_SetWindowOpacity`，不要求透明窗口；`alpha` 参与绘制内容透明度，透明窗口关闭时效果不可见 |

#### 6.4.4 Windows 原生层的落地规则（WS_EX_LAYERED）

`WindowAttributes` 的 Resolve 结果是策略层；`NativeWindow_Windows` 只负责把结果落到窗口创建参数与 `WS_EX_LAYERED`，不重新定义策略。

创建窗口时，`m_bIsLayeredWindow` 的优先级从高到低：

1. **创建窗口前显式调用 `Window::SetLayeredWindow` 的请求值**；
2. `WindowCreateAttributes.m_bIsLayeredWindow`（应来自 Resolve）；
3. `WindowCreateAttributes` 未定义分层窗口、但 `opacity < 255` 时，推导为 true；
4. 默认 false。

**`WindowCreateParam::m_dwExStyle` 不参与分层窗口决策：**

- `kWS_EX_LAYERED` 常量已删除；迁移旧代码时直接删除该标记位即可，不需要修改调用逻辑；
- 即使调用方直接使用原生 `WS_EX_LAYERED`，创建窗口时该位也会被清除并忽略；
- 分层窗口由窗口内部根据 Resolve 结果、阴影、透明度等配置自动推导，不需要调用 `SetLayeredWindow` 或 `SetLayeredWindowForConfig`。

补充决策：

- `m_bIsLayeredWindowDefined=true` 时以它的值为准；显式 `false` 不会被 `opacity` 重新打开；
- 只有在 `m_bIsLayeredWindowDefined=false` 时，才允许根据 `opacity` 推导；
- 使用系统标题栏时，XML/Resolve 配置必须关闭分层窗口；创建前 API 显式设置优先（运行时显式命令高于配置）；
- 创建前 API 的请求值由原生窗口保存，创建窗口成功后清除"创建前设置"标志；
- `WindowCreateAttributes` 应只由 `WindowAttributes` 的 Resolve 产生。若调用方手工构造，必须自行保证 `m_bIsLayeredWindow` 已包含标题栏、opacity、阴影的最终结论；原生层不再二次裁决；
- 子窗口：父窗口为分层窗口时，Windows 8+ 子窗口继承 `WS_EX_LAYERED`；Windows 7 及更早不支持，创建后设置请求返回失败，创建前设置会被忽略；
- 窗口创建后，`InitNativeWindow` 以实际 `GetWindowLong(GWL_EXSTYLE)` 回写缓存，保证不变量：
  `IsLayeredWindow() == ((GetWindowLong(GWL_EXSTYLE) & WS_EX_LAYERED) != 0)`；
- 窗口创建后调用 `SetLayeredWindow`，只有系统样式设置成功才更新缓存；失败时保持原值并返回 false；
- 外部通过 `Window::SetLayeredWindow` 显式设置成功后，窗口配置应用（`WindowBuilder` 阶段 4）和内部阴影自动调整都不再覆盖该设置；仅系统标题栏切换作为强制互斥规则，会直接关闭分层窗口；
- 窗口销毁时重置 `m_bIsLayeredWindow` 与"创建前设置"标志；
- `opacity` 缓存只在设置成功时更新；`255` 作为"不透明"语义值可直接更新。

**Windows 原生层冲突决策表：**

| 冲突场景 | 决策 |
|---|---|
| 创建前 API=true，XML/Resolved=false | API 优先，`true` |
| 创建前 API=false，XML/Resolved=true | API 优先，`false` |
| 创建前 API=true，系统标题栏=true | API 优先，保持分层；高于配置层 |
| 外部 API 设置后，WindowBuilder 应用 layered 配置 | 外部 API 优先，不覆盖；系统标题栏切换除外 |
| XML/Resolved 分层=true，系统标题栏=true | 标题栏优先，`false` |
| XML 未定义分层，`opacity < 255` | 分层，`true` |
| XML 显式分层=false，`opacity < 255` | 显式 false 优先，`false`，opacity 有效值关闭 |
| `m_createParam.m_dwExStyle` 含 `WS_EX_LAYERED`（无其他分层来源） | 忽略并清除，`false` |
| `m_createParam.m_dwExStyle` 含 `WS_EX_LAYERED`，XML/Resolved 分层=true | 以 XML/Resolved 为准，`true`；该位不增加决策输入 |
| Windows 7 及更早的子窗口请求分层=true | 不支持，创建后请求失败；创建前设置被忽略 |
| 创建后设置系统样式失败 | 不更新缓存，返回 false |

### 6.5 alpha 与 opacity

| 项目 | Windows 非 SDL | SDL |
|---|---|---|
| 默认值 | 255 | 255 |
| `opacity` 依赖 | 分层窗口 | 不依赖透明窗口（`SDL_SetWindowOpacity`） |
| `alpha` 依赖 | 分层窗口 | 透明窗口（内容 Alpha 才有意义） |
| GL 有效时 | 有效值关闭，请求值保留 | 不适用（SDL 无 GL） |

发生冲突时：

- 只关闭有效值；
- 保留请求值；
- 输出明确告警，说明"因 xxx 冲突，当前未生效，配置已保留"。

### 6.6 尺寸相关属性（补充）

| 属性 | 规则 |
|---|---|
| `size` | 支持像素与百分比；百分比基于目标显示器工作区 |
| `min_size` / `max_size` | 像素值，按窗口 DPI 缩放；先夹取，再按 `size_contain_shadow` 扩展阴影区域 |
| `size_contain_shadow` | 仅对固定像素尺寸生效；百分比尺寸不扩展 |
| DPI | 固定像素尺寸与 min/max 必须在同一坐标系下比较；百分比尺寸直接使用工作区像素 |

补充约定：

- 创建前解析时，若目标显示器尚未确定，百分比尺寸可按主显示器估算；窗口创建后必须按实际显示器重新 Resolve 并校正；
- 创建后校正时应保持窗口中心位置或按原定位策略重新定位，避免出现可见跳动；
- 创建前与创建后必须使用同一套单位规则，不能一边逻辑值、一边物理像素混合比较。

## 7. 冲突处理矩阵

| 高优先级条件 | 被关闭的有效属性 | 请求值 | 告警 |
|---|---|---|---|
| `use_system_caption=true`（Windows 非 SDL） | `layered_window`、`opacity`、`alpha`、`shadow_attached` | 保留 | 是 |
| `use_system_caption=true`（SDL） | `shadow_attached` | 保留 | 是 |
| `render_backend_type=GL`（Windows 非 SDL） | `layered_window`、`opacity`、`alpha`、自绘阴影 | 保留 | 是 |
| GL 创建失败回退 CPU | 按请求值恢复分层/透明度/阴影 | 保留 | 是 |
| `layered_window=false`（Windows 非 SDL） | `opacity`、`alpha`、自绘阴影 | 保留 | 是 |
| SDL 透明窗口关闭 | `alpha` | 保留 | 是 |
| SDL Windows 请求分层窗口 | 不设置 `WS_EX_LAYERED` | 保留透明请求 | 是 |
| 运行时修改 SDL 透明窗口 | 创建后不可修改 | 保留 | 是 |

## 8. 平台对照

| 特性 | Windows 非 SDL | Windows SDL | Linux/macOS SDL |
|---|---|---|---|
| GL 有效支持 | 是（动态切换） | 否，归一为 CPU | 否，归一为 CPU |
| 分层窗口实现 | `WS_EX_LAYERED` | 禁止使用 | 无该概念 |
| 透明窗口实现 | 分层窗口 | `SDL_WINDOW_TRANSPARENT` | `SDL_WINDOW_TRANSPARENT` |
| 透明窗口默认 | 不适用 | 默认开启 | 默认开启 |
| 系统标题栏与透明 | 互斥 | 可共存（按平台支持） | 可共存（按平台支持） |
| 自绘阴影要求 | 必须分层窗口 | 不要求分层窗口 | 不要求分层窗口 |
| `opacity` | 依赖分层窗口 | `SDL_SetWindowOpacity` | `SDL_SetWindowOpacity` |
| `alpha` | 依赖分层窗口 | 依赖透明窗口 | 依赖透明窗口 |
| 分层/透明动态切换 | 支持 | 创建时决定，之后不可变 | 创建时决定，之后不可变 |

## 9. 动态切换与状态保存

### 9.1 必须保存的请求值

每个窗口至少保存：

- `render_backend_type`、`msaa`、`vsync`；
- `use_system_caption`；
- `shadow_attached`、`shadow_type`；
- `layered_window`、`opacity`、`alpha`；
- 尺寸相关属性（如果支持运行时修改）。

### 9.2 典型场景

**场景一：CPU → GL**

1. 请求后端改为 GL；
2. Resolve 关闭分层窗口、透明度、自绘阴影；
3. 创建 GL 渲染对象；
4. 若 GL 创建失败，回退 CPU 并按请求值恢复。

**场景二：GL → CPU**

1. 请求后端改为 CPU；
2. 按请求值恢复分层窗口、透明度、阴影类型；
3. 重新 Resolve，确保与标题栏状态一致。

**场景三：开启/关闭系统标题栏**

1. 更新请求值；
2. 按 Resolve 顺序重新计算；
3. 关闭或恢复阴影、分层窗口、透明度；
4. 所有被关闭的属性均保留请求值。

**场景四：切换阴影类型**

1. 更新 `shadow_type` 请求值；
2. 重新计算是否需要分层窗口；
3. 若当前是 GL，自绘阴影仍保持关闭，切换为系统阴影或 None；
4. 若当前是 CPU，按请求值启用分层窗口并绘制自绘阴影。

## 10. 当前实现与目标规则的差异

以下为现状分析结果，代码暂未修改。

### 10.1 全局与 XML 的文档描述错误

- 当前代码对 `render_backend_type` 已经是"XML 优先，全局兜底"；
- 但 `GlobalManager.h` 注释与 `docs/Global.md` 仍写成"全局开关启用后忽略窗口 XML"；
- 需要把文档改为本文档 3.1 的语义。

### 10.2 msaa / vsync 未按字段合并

`ResolveRenderBackendAttributes` 在 XML 定义了后端、但未定义 msaa/vsync 时，使用硬编码 `0` 与 `kDefaultDisableVsync`，不会继承全局值。需要按 3.2 改为逐字段解析。

### 10.3 未保存请求值，动态切换无法恢复

当前 `Window` 只保存有效值，XML 属性只在启动时解析并应用一次；没有保存 `shadow_type`、`layered_window`、`opacity`、`alpha` 等请求值，因此 GL ↔ CPU、标题栏开关等动态切换无法正确恢复冲突属性。

### 10.4 SDL 构建下 Resolve 未归一为 CPU

`ResolveRenderBackendAttributes` 在 SDL 分支只跳过分层窗口导致的降级，仍可能返回 GL；实际由 `Window::SetRenderBackendType` 静默改为 CPU，且不设置回退标志。需要 Resolve 阶段直接归一为 CPU 并告警。

### 10.5 SDL + Windows 的分层窗口处理（已修复原生层）

- SDL 原生层不再设置 `WS_EX_LAYERED`，改用 `SDL_WINDOW_TRANSPARENT` 表示透明窗口；
- 创建前 `SetLayeredWindow` 的请求值优先于 XML，创建后只校验实际透明标记；
- 仍需确认 `WindowBuilder` 的 opacity/alpha 应用路径与 6.4.2 的透明窗口规则一致。

### 10.6 后端降级使用了原始 opacity

`ResolveRenderBackendAttributes` 只要看到 `opacity < 255` 就降级 GL，但没有判断该 opacity 是否真的会导致分层窗口。例如系统标题栏或 `layered_window="false"` 会关闭 opacity，此时不应因 opacity 降级后端。应复用分层窗口的有效 Resolve 结果。

### 10.7 opacity 自动分层与文档冲突

代码会在 `opacity < 255` 时自动启用分层窗口，但 `docs/Window.md` 仍写"不会自动启用分层窗口"。需要统一文档与行为。

### 10.8 降级后 msaa/vsync 状态残留

GL 因冲突降级为 CPU 后，XML 中的 msaa/vsync 仍被保存并可能通过查询接口返回，但 CPU 下无效。需要明确"请求值"与"实际参数"的查询语义。

### 10.9 阴影与分层窗口的循环推导

创建前用固定 `layered=false` 推导默认阴影类型，创建后改用实际分层状态；同时分层 Resolve 使用原始阴影类型，阴影 Resolve 又可能把不支持的系统阴影转换为自绘阴影。两套推导可能不一致，最终依赖后续纠正。应统一为一次 Resolve。

### 10.10 创建前与创建后解析输入不同

创建前解析传入 `pWindow=nullptr`，百分比尺寸基于主显示器；创建后解析基于实际窗口显示器与 DPI，可能导致初始尺寸、居中位置与最终值不同。需要统一解析时机或增加创建后校正。

### 10.11 属性名与运行时接口不一致

XML 解析大小写不敏感且支持别名，`Window::SetAttribute` 严格区分大小写且只支持部分属性；Window 根节点的 `class` 属性使用精确大小写查找。应统一属性名匹配规则与支持范围，至少保证文档中列出的属性在两条路径上行为一致。

### 10.12 vsync 默认值文档不一致

代码默认 `kDefaultDisableVsync=true`（默认禁用），文档表写默认 `true`（启用）；SDL 下还会被强制设为 `false`。需要统一默认值定义和查询语义。

### 10.13 测试覆盖不足

现有测试主要覆盖 Windows 非 SDL、100% DPI；缺少：

- GL ↔ CPU 动态切换与冲突属性恢复；
- 标题栏切换与阴影/分层/透明度恢复；
- SDL 分支（尤其 Windows SDL）；
- 150% DPI 与多显示器；
- 全局与 XML 的优先级组合；
- 创建前/创建后 `SetLayeredWindow` API 仍然可用，但删除 `kWS_EX_LAYERED` 后无需迁移调用；
- `WindowCreateParam::m_dwExStyle` 携带 `WS_EX_LAYERED` 时应被忽略并清除；
- 系统标题栏 + opacity、显式 `layered_window=false` + opacity 的冲突；
- 原生设置失败路径。

### 10.14 Windows 原生层分层窗口逻辑（已修复）

`NativeWindow_Windows` 已按 6.4.4 统一设置规则：

- 创建前 `SetLayeredWindow` 的请求值由原生窗口保存，并在创建时优先于 XML 生效；不再依赖 `m_createParam.m_dwExStyle`；
- `WindowCreateParam::m_dwExStyle` 中的 `WS_EX_LAYERED` 会被忽略并清除；删除 `kWS_EX_LAYERED` 后不需要新增任何分层窗口调用；
- 创建窗口时只做优先级选择，不再用 opacity/标题栏二次推导；
- 创建后以实际 `WS_EX_LAYERED` 回写缓存，保证 `IsLayeredWindow()` 与实际样式一致；
- 窗口销毁时重置缓存与创建前标志；
- `opacity` 仅在设置成功时更新缓存。

剩余风险：手工构造 `WindowCreateAttributes` 且未经过 Resolve 时，调用方需自行保证 `m_bIsLayeredWindow` 已包含全部冲突处理结果。

## 11. 建议的改造点

1. **引入请求值结构**：在 `Window` 中保存 `WindowAttributeValues` 或精简后的请求值集合，供动态切换重新 Resolve。
2. **统一 Resolve 入口**：实现 `ResolveWindowAttributes(requested, platformCapabilities) -> ResolvedWindowAttributes`，严格按第 5 节顺序执行。
3. **后端 Resolve 复用分层结果**：避免用原始 opacity 重复推导。
4. **SDL 平台层的透明状态**：
   - Windows 非 SDL：`layeredWindow` 对应 `WS_EX_LAYERED`；
   - SDL：`m_bIsLayeredWindow` 仅表示 `transparentWindow`，对应 `SDL_WINDOW_TRANSPARENT`；
   - Windows SDL 永不设置 `WS_EX_LAYERED`；
   - 透明窗口只在创建时设置，创建后修改返回失败并告警。
5. **运行时 API 与请求值联动**：动态切换时更新请求值，再统一 Resolve；GL 失败回退时按请求值恢复。
6. **告警统一**：所有因冲突被关闭的有效属性都必须告警，并明确"配置已保留"。
7. **文档与测试同步**：按本文档修正 `Global.md`、`Window.md`、`WindowShadow.md` 与 XSD 描述，并补充第 10.13 节的测试。

## 附录 A：默认值汇总

| 属性 | 内置默认值 | 备注 |
|---|---|---|
| `render_backend_type` | `CPU` | XML > 全局 > 内置 |
| `msaa` | `0` | 仅 GL 有效 |
| `vsync` | `kDefaultDisableVsync` | 当前默认禁用 |
| `use_system_caption` | `false` | |
| `shadow_attached` | `true` | |
| `shadow_type` | `"default"` | 支持系统阴影时使用系统阴影 |
| `layered_window` | Windows 非 SDL：按需；SDL：默认透明 | SDL 默认开启透明 |
| `opacity` | `255` | |
| `alpha` | `255` | |
| `min_size` | `0,0` | 不限制 |
| `max_size` | `0,0` | 不限制 |
| `size_contain_shadow` | `false` | |

## 附录 B：告警文案建议

- `use_system_caption=true 与 shadow_attached 冲突，阴影已临时关闭，配置值已保留`
- `use_system_caption=true 与 layered_window 冲突，分层窗口已临时关闭，配置值已保留`
- `render_backend_type=GL 与 layered_window 冲突，分层窗口已临时关闭，配置值已保留`
- `render_backend_type=GL 与自绘阴影冲突，已临时切换为系统阴影/无阴影，配置值已保留`
- `render_backend_type=GL 创建失败，已回退为 CPU 绘制，相关属性已按配置恢复`
- `SDL 构建不支持 render_backend_type=GL，已使用 CPU 绘制，配置值已保留`
- `SDL 窗口的透明标记只能在创建时设置，运行时修改已忽略`
- `Windows SDL 不使用分层窗口，已忽略 WS_EX_LAYERED 设置`
- `layered_window=false 与 opacity 冲突，opacity 已临时关闭，配置值已保留`
- `SDL 透明窗口已关闭，alpha 已临时关闭，配置值已保留`

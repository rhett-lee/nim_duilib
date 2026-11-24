# 窗口 XML 配置冲突检测与初始化精简重构变更摘要

> 类型：重构变更摘要（2026-11-24）
> 关联文档：[窗口XML配置冲突检测与初始化精简评估报告.md](./窗口XML配置冲突检测与初始化精简评估报告.md)
> 范围：`WindowBuilder`、`WindowAttributes`、`Window`、`WindowBase` 的窗口配置解析与应用流程
> 代码状态：已实施并通过构建与单元测试，尚未提交

## 一、改动概览

| 文件 | 改动内容 |
|---|---|
| `duilib/Core/WindowAttributes.h/.cpp` | 新增配置告警收集机制；补齐非法值校验；层窗口冲突与透明度纠正；GL 与层窗口冲突并入渲染后端推导；初始尺寸夹取提示 |
| `duilib/Core/WindowBuilder.cpp` | 新增告警统一输出；各阶段 Resolve 传入告警列表；阶段 4 仅在窗口为分层窗口时设置不透明度 |
| `duilib/Core/Window.h/.cpp` | 阴影刷新延迟与合并；PostInitWindow 尺寸相同不再 Resize；阴影初始附加状态改为使用 XML 解析结果 |
| `duilib/Core/WindowBase.h/.cpp` | 保存创建前解析出的 `shadow_attached`，供 `Window::PreInitWindow` 使用 |

## 二、行为变化（对使用者可见）

### 2.1 新增配置告警

解析或推导阶段的错误配置会输出 `WindowBuilder: ...` 日志，并在结果中完成纠正。覆盖场景：

| 场景 | 纠正方式 | 日志内容 |
|---|---|---|
| `shadow_type` 非法字符串 | 忽略该属性，使用默认阴影类型 | `attribute="shadow_type", value="...": invalid value, ignored` |
| `render_backend_type` 非法值 | 按 CPU 绘制（保留 debug 断言） | `attribute="render_backend_type", value="...": invalid value, use CPU rendering` |
| `alpha/opacity` 超出 [0,255] | 忽略该属性 | `value out of range [0,255], ignored` |
| `msaa` 为负数 | 复位为 0 | `negative value, reset to 0` |
| `round_corner` 仅一个方向非 0 | 忽略该属性 | `the two values must be both zero or both positive, ignored` |
| `use_system_caption="true"` + `layered_window="true"` | 关闭层窗口（系统标题栏优先） | `conflicts with use_system_caption="true", layered window is disabled` |
| 非分层窗口配置 `alpha/opacity` | 忽略这两个属性，避免设置失败 | `requires a layered window, ignored` |
| `size` 超出 `min_size/max_size` | 夹取到合法范围 | `clamped to the min_size/max_size range` |
| `render_backend_type="GL"` + `layered_window="true"` | 降级为 CPU 绘制 | `OpenGL rendering does not support layered windows, fall back to CPU rendering` |

说明：`layered_window="false"` + `opacity<255` 不再出现“内部自动把窗口变成分层窗口”的隐式行为（该逻辑此前已移除），现在会按上表忽略 `opacity` 并提示。

### 2.2 渲染后端规则调整

- Windows 后端：显式 `layered_window="true"` 与 `opacity<255` 一样，会使 `render_backend_type="GL"` 降级为 CPU，保证窗口内容可见。
- SDL 后端：不参与该降级判断。SDL 窗口固定使用透明标记，GL 渲染可正常工作，若同样降级会导致 macOS/Linux 上 GL 失效。

### 2.3 初始化动作精简

1. **初始尺寸去重**：`PostInitWindow` 在设置初始大小前比较窗口当前尺寸，相同则跳过，减少一次 `SetWindowPos` → `WM_SIZE` → 重新布局 + 渲染 resize。窗口仍为隐藏创建，无可见抖动。
2. **阴影刷新合并**：初始化应用 XML 期间（阶段 1~6）延迟 `ProcessWindowShadowTypeChanged`，在 `PostInitWindow` 创建渲染接口前统一执行一次。原最多 3 次的 `UpdateWindowRGN` + `UpdateLayeredWindowStyleEx` 降为 1 次。
3. **阴影初始状态前置**：`CreateShadow` 的初始附加状态改为窗口配置中解析出的 `shadow_attached`，并结合是否使用系统标题栏。系统标题栏窗口、`shadow_attached="false"` 的窗口不再“先附加阴影再关闭”。
   - 注意：`Shadow` 对象仍需创建（阶段 3 的阴影属性设置依赖该对象），本次只修正初始附加状态。

## 三、对既有接口的影响

- `ResolveLayeredWindowAttributes`、`ResolveRenderBackendAttributes`、`ResolveWindowInitSize` 新增**可选**的 `std::vector<DString>* pWarnings` 参数，默认 `nullptr`，已有调用无需修改。
- `WindowAttributeValues` 新增 `m_warnings` 字段，属于新增成员，不影响既有字段语义。
- 新增日志可能使原有日志输出增多，仅在配置存在非法值或冲突时出现。

## 四、验证结果

- 非 DLL 的 x64/Release 构建（`python build\build.py build --no-deps`）：退出码 0。
- SDL 后端库编译（`--sdl --no-examples --target duilib`）：退出码 0。
- `ctest`：40 个用例 39 通过；`webview2` 因本机系统语言为 `zh-CN`、用例期望 `en-US` 失败，属环境问题，与本次改动无关。

## 五、遗留与后续可选

1. `size` 超出屏幕时被压缩到 95%、非法百分比被改为 75% 的纠正发生在 `AttributeUtil` 层，目前仍无日志（该工具函数被控件解析共用，未改动）。
2. 阶段 2 的 `SetUseSystemCaption` 在值与当前一致时仍会执行 `UpdateMinMaxBoxStyle` 与回调（成本低，未处理）。
3. `docs/misc` 中的评估报告已同步更新实施状态。

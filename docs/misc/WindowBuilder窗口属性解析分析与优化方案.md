# WindowBuilder 窗口属性解析分析与优化方案

> 类型：代码分析 + 重构方案（2026-11-16）
> 范围：`duilib/Core/WindowBuilder.cpp` 中的 `WindowBuilder::ParseWindowAttributes`（私有）与 `WindowBuilder::ParseWindowCreateAttributes`（公有，接口不可修改）
> 目标：消除重复解析，消除"边解析边应用"带来的顺序依赖与重复应用问题

## 一、现状

窗口属性在两条路径上被解析：创建窗口前解析一次（决定窗口样式与尺寸），窗口创建过程中再解析一次（逐条应用到窗口）。

```
Window::CreateWnd
  └─ NativeWindow_Windows::CreateWnd
       ├─ GetCreateWindowAttributes → ParseWindowCreateAttributes   // 窗口句柄还不存在
       ├─ CreateWindowEx(...)                                       // WM_CREATE
       └─ Window::PreInitWindow → CreateControls → ParseWindowAttributes → 逐条 Set* 到窗口
```

- `ParseWindowCreateAttributes`（`WindowBuilder.cpp` L543-L772）：单趟遍历属性，但只解析"建窗口要用"的 13 个属性，输出 `WindowCreateAttributes`（含最终初始尺寸、阴影九宫格、是否分层）。
- `ParseWindowAttributes`（`WindowBuilder.cpp` L774-L1098）：**7 趟**遍历同一批 `root.attributes()`，边解析边调用 `pWindow->Set*` 立即生效，靠注释维系顺序。

## 二、存在的问题

### 1. 重复解析：13 个属性被两套代码各实现一遍

| 属性 | 创建路径（记录） | 应用路径（Set） | 两套实现的差异 |
|---|---|---|---|
| use_system_caption | 记录 Defined | 第 4 趟 `SetUseSystemCaption` | 创建后又被 `OnUseSystemCaptionBarChanged` 二次影响 |
| size_box | `ParseRectValue`（默认校验） | `ParseRectValue(..., false)` | 调试断言行为不一致 |
| caption / shadow_corner | 记录 | Set | 逻辑重复 |
| shadow_attached | 记录 + 参与分层推导 | 主循环只记录，延迟到 L1017-L1025 设置 | 两处各有一套"延迟设置" |
| shadow_type | 解析局部变量，仅用于推导 corner/layered | 主循环立即 `SetShadowType` | 应用顺序与 attached 相反，靠注释说明 |
| layered_window | 无条件记录，并做默认值推导 | `IsUseSystemCaption()` 为真时直接忽略 | 语义不同 |
| alpha / opacity | `<255` 才置 Defined | 无条件 Set | `opacity` 在应用路径还会写一个从未被读的死变量（L1033 / L1082） |
| size | 用 `nShadowType` 推导的 corner + 全局 DPI 计算 | 最后再算一遍（L1032-L1085），用 `pWindow->GetShadowCorner()` + 窗口 DPI | 同一 XML 有两条公式，边界条件不同（`IsShadowAttached` / `IsWindowMaximized`） |
| min_size / max_size | 只用于夹取 size，不保存 | 又解析一次并 `SetWindowMinimumSize` | 同一配置处理两次 |
| size_contain_shadow | 局部变量 | 局部变量 | 纯重复 |

新增一个窗口属性，通常要改 3~4 处：创建路径、应用路径、`knownNames`、以及可能受影响的 size/shadow 推导。漏改任一处不会编译报错，只会出现"创建时对、应用时不对"的隐性 bug。

### 2. 单函数多趟遍历 + 硬编码顺序

`ParseWindowAttributes` 的 7 趟分别是：

1. `opacity` 预扫描（L787）；
2. `vsync` / `msaa`（L798）；
3. `render_backend_type`（L816）；
4. `min_size` / `max_size` / `use_system_caption`（L863）；
5. 主属性集合（L892）；
6. `size` / `opacity`（L1034）；
7. 未知属性校验（L1090）。

带来的问题：

- 顺序依赖是"隐式契约"：render 必须先建（后续 Set 会用到 Render）；`use_system_caption` 必须先于 `layered_window`；`shadow_attached` 必须晚于 `shadow_type`；`size` 必须最后。任何一处挪动都要重新推理全链路。
- `knownNames` 分散在 7 趟里填充，校验结果取决于哪几趟被执行（带 `break` 的趟尤其明显），新增属性忘登记就会触发"未知属性"断言。
- 大小写策略不统一：`opacity` 预扫描用 `IsEqualNoCase`，其余用精确比较，`Opacity="128"` 会"要求分层窗口但不生效"。
- 同样的"GL + opacity 降级"判断在创建分支（L831-L835）、全局分支（L852-L858）和 `Window::SetRenderBackendType` 里共三份。

### 3. 边解析边应用：每次 Set 都有原生窗口副作用

- `SetShadowType` / `SetShadowAttached` → `ProcessWindowShadowTypeChanged()` → `UpdateWindowRGN` + `UpdateLayeredWindowStyleEx`；
- `SetLayeredWindow` → 立刻改 `WS_EX_LAYERED`；
- `SetUseSystemCaption` → 重算标题栏 / 非客户区；
- `SetInitSize` → 直接 `Resize`。

于是解析期会出现"设 shadow_type（可能间接触发分层/阴影）→ 设 shadow_attached=false 回滚 → 设 layered_window 再改一次 → 最后 size 触发一次 Resize"的多次状态翻转。窗口尚未显示，这些开销和不必要的中间态都是纯粹浪费，也是"显示过程中属性被重复应用"的直接来源。

函数因此不是幂等的，只能在 `CreateControls` 里加 `IsWindowAttributesApplied()` 布尔量（L500-L504）来防重复（Include 场景）。

### 4. 为绕开 Shadow API 的状态依赖而做的 hack

创建路径 L707-L720 为了让 `Shadow::GetShadowParam` 算出正确的九宫格，先 `SetLayeredWindow(bLayeredWindow)` 再调用、再恢复。原因是 `Shadow::GetDefaultShadowType`（`Shadow.cpp` L568-L598）读取 `pWindow->IsLayeredWindow()`，形成了 `分层 ← 阴影类型 ← 分层` 的循环依赖；而 L664 处的分层推导又是在"未分层"状态下取默认阴影，两段推导的前提并不一致。

### 5. 其他一致性隐患

- 属性语义还有第三个入口：`ParseWindowAttributes(std::map<DString, DString>&)`（L2278-L2297）供 `XmlBox` 预览读取（手动 `find("shadow_type")`），形成第四处判断逻辑。
- 创建阶段拿不到 `render_backend_type`（只在应用阶段解析），因此 `WS_EX_LAYERED` 等窗口样式是按"未考虑后端"的结论创建的，GL 场景要再回头关闭分层。

## 三、优化方案

核心思路：**把"解析"与"应用"彻底分开，解析只做一次、只保留一份；应用按固定阶段批量执行，顺序由代码结构保证而不是注释。**

```
XML 属性 ──(单趟 Parse)──► WindowAttributeValues（纯数据，含 defined 标志 + 未知属性）
                                   │
                          (统一 Resolve 推导)
                                   ├─ ResolveShadow()        → 有效 attached/type/corner/image
                                   ├─ ResolveLayeredWindow() → 有效 layered/alpha/opacity + 有效后端
                                   └─ CalcInitSize()         → 最终尺寸（唯一公式）
                                   │
             ┌─────────────────────┴─────────────────────┐
   ParseWindowCreateAttributes（公有签名不变）      ParseWindowAttributes（私有）
       填充 WindowCreateAttributes                    按固定阶段 Apply 到 Window
```

### 具体步骤

1. **新增内部数据结构**（`WindowBuilder` 私有嵌套或 cpp 内匿名命名空间）：`WindowAttributeValues`，字段与 XML 一一对应，全部带 `Defined` 标志；同时收集 `std::vector<DString> unknownAttributes`。原生字符串只解析一次，`ShadowType`、`UiRect` 等转换只做一次。

2. **单趟解析 + 别名表**：`bool ParseWindowAttributeValues(const pugi::xml_node& root, WindowAttributeValues& values) const`，一个循环内用 `name → 属性枚举` 的分发表（表里同时登记别名：`sync`/`shadowattached`、`mininfo` 等），`knownNames` 的维护从"散落 7 处"变成"表就是白名单"。

3. **抽出三个共享推导函数（纯函数，不读窗口状态）**：
   - `ResolveShadow(const WindowAttributeValues&, bool bSystemCaption, bool bSystemShadowSupported, bool bLayeredWindow, ShadowParams&)`；
   - `ResolveLayeredWindow(...)`：把"显式 layered / opacity<255 / alpha<255 / 自绘阴影需要分层 / 系统标题栏互斥 / 后端不支持分层"合并成一处判断，输出最终 `layered`、`alpha`、`opacity` 和**有效后端**；
   - `CalcInitSize(...)`：min/max 夹取 + 阴影九宫格扩展 + DPI 缩放 + 校验，创建与应用共用。

   同时给 `Shadow` 增加显式参数版本（如 `GetShadowParam(ShadowType, bool bLayeredWindow, bool bSystemShadowSupported, ...)`），去掉"临时改窗口状态再恢复"的 hack 和上述循环依赖。

4. **`ParseWindowCreateAttributes` 变成薄封装**：内部 Parse + Resolve，再映射到 `WindowCreateAttributes`（公有签名保持不变）。`min_size` / `max_size` / `size_box` / `caption` 等原本丢弃的信息，如果创建阶段需要，直接加内部字段；否则明确注释"仅应用阶段使用"，避免两边各算一遍。

5. **`ParseWindowAttributes` 改成"先解析、后应用"**：

```cpp
void WindowBuilder::ParseWindowAttributes(Window* pWindow, const pugi::xml_node& root) const
{
    WindowAttributeValues values;
    ParseWindowAttributeValues(root, values);             // 1 趟，无副作用
    WindowAttributeValues resolved = ResolveAll(values);  // 唯一推导
    ApplyWindowAttributes(pWindow, resolved);             // 固定阶段，只写不读
}
```

`ApplyWindowAttributes` 内部按显式阶段执行，每个属性只出现在一处：

1. 渲染后端 / vsync / msaa（必须最先，后续 Set 依赖 Render）；
2. 结构性属性：`use_system_caption`、`min_size` / `max_size`、`size_box`、`caption`、`sys_menu*`；
3. 阴影：image / border / round / snap，最后一次性 `SetShadowAttached` + `SetShadowType`（不再靠"延迟设置"绕顺序）；
4. 分层与透明：先 `SetLayeredWindow`，再 `SetLayeredWindowAlpha` / `SetLayeredWindowOpacity`（Resolve 已完成互斥与降级判断）；
5. 无依赖属性：`text` / `icon` / `drag_drop` / `round_corner` 等；
6. `SetInitSize` 最后执行。

6. **让应用过程幂等**：同值 Set 提前返回（或在 `ApplyWindowAttributes` 里比对当前值），删掉 `IsWindowAttributesApplied` 对正确性的依赖；未知属性在单趟解析处统一断言/日志一次（Release 也输出告警）。

7. **补可测性**：把 `ParseWindowAttributeValues + ResolveAll` 做成不依赖 HWND 的纯逻辑，加表驱动单测覆盖：
   - 相同 XML 在"创建路径"和"应用路径"得到相同的 layered / shadow / size；
   - 属性任意重排结果不变；
   - GL + opacity、系统标题栏 + layered 等互斥组合的降级结果；
   - 未知属性告警。

### 预期收益

- 遍历 7 趟 → 1 趟；重复属性 13 个 → 0 个；新增属性从 3~4 处 → 1 处（加别名表一行）。
- 消除"边解析边应用"的中间态与多次 `UpdateWindowRGN` / `UpdateLayeredWindowStyleEx` / `Resize`，窗口创建期更轻、更稳定。
- 创建尺寸与显示尺寸由同一函数产出，GL / 分层 / 阴影的互斥规则集中一处，避免两条路径结论不一致。
- 顺序依赖从"注释契约"变成"阶段函数"，后续维护不必再推理 7 趟之间的先后关系。

## 四、实施建议

分两步推进，降低风险：

1. **第一步（行为不变）**：新增 `WindowAttributeValues` 与单趟解析、三个 Resolve 纯函数，并补表驱动单测；两条路径暂时仍走原有 Set 代码，仅复用解析结果做结果比对（先在测试中验证一致）。
2. **第二步（迁移）**：`ParseWindowCreateAttributes` 改为薄封装，`ParseWindowAttributes` 改为"Parse → Resolve → Apply"，随后删除重复代码、死变量 `bLayeredWindowOpacityDefined`、以及 `IsWindowAttributesApplied` 的正确性依赖。

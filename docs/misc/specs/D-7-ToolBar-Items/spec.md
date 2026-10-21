# D-7 ToolBar 工具项抽象

## 1. 目标

为 `ToolBar`（继承 `HBox`）补齐工具项的创建、管理、勾选与事件回调能力，
让应用层不再手工 `new Button/CheckBox/Line` + `AddItem` + 逐个绑定事件，
同时保留现有 XML 静态写法与溢出收纳行为。完成后回写
`docs/misc/Development-Plan.md` 的 D-7 行与跟踪项。

## 2. 需求要点

- 提供 `AddToolButton` / `AddToolCheckBox` / `AddToolSeparator` / 通用 `AddTool`。
- 提供 `RemoveTool(name)` / `RemoveToolAt(index)` / `RemoveAllTools()`。
- 提供 `SetToolChecked` / `IsToolChecked`、`SetToolEnabled` / `IsToolEnabled`、
  `SetToolVisible` / `IsToolVisible`。
- 提供工具项点击与勾选变化两类回调（`AttachToolItemClick` / `AttachToolItemCheckedChanged`），
  并支持清空。
- 工具项默认样式可通过 XML 配置（`tool_class` / `tool_checkbox_class` / `tool_separator_class`）。
- 溢出收纳（"更多"浮层）中被收纳的工具项点击时，同样触发上述回调；勾选类工具项会被切换。
- 现有 XML 静态子控件、`AttachOverflowItemActivated` 行为保持兼容。

## 3. 设计决策

### 3.1 工具项仍是普通控件

不新增 `ToolItem` 控件类型。工具项就是 `Button` / `CheckBox` / `Line`，由 ToolBar 统一创建、
命名、加入子项并绑定事件。这样：

- 沿用现有主题 Class、DPI、绘制与布局能力；
- 应用层可通过返回的控件指针继续设置图标、尺寸、tooltip；
- 不改变 `Box` 的子项模型与 XML 解析路径。

### 3.2 名称规则

- `AddTool*` 的 `name` 为空时自动生成 `tool_1`、`tool_2`……（跳过已存在名称）。
- 显式 `name` 与已有工具项重名时拒绝添加（返回 `nullptr` / `false`），避免 `FindTool` 歧义。
- 名称即回调参数，也是 `FindTool` / `RemoveTool` 的键。

### 3.3 索引规则

`GetToolCount` / `GetToolAt` / `RemoveToolAt` 的索引只统计"工具项"，
排除内部创建的"更多"按钮；XML 中的静态子控件同样计入（它们也是工具项）。

### 3.4 勾选语义

勾选态复用 `CheckBox::IsSelected()` / `Selected()`（选中态切换），
不使用 `SupportCheckMode` 的勾选图标模式（该模式用于树/列表的多选图标）。
主题通过 `selected_*` 颜色/图片体现选中态。

### 3.5 事件

```cpp
typedef std::function<void(const DString& itemName)> ToolItemClickEvent;
typedef std::function<void(const DString& itemName, bool bChecked)> ToolItemCheckedChangedEvent;
```

- 点击 `Button`：触发 `kEventClick` → `ToolItemClick`。
- 点击 `CheckBox`：触发 `kEventSelect` / `kEventUnSelect` → 先 `ToolItemClick`，再 `ToolItemCheckedChanged`。
- 溢出浮层激活工具项：与直接点击同一路径；`CheckBox` 会先切换勾选态，
  再派发 `ToolItemClick` / `ToolItemCheckedChanged`，随后仍触发 `OverflowItemActivated`。
- 回调列表遍历时使用副本，允许回调中增删监听。

### 3.6 新增 XML 属性

| 属性 | 默认值 | 用途 |
|------|--------|------|
| tool_class | 空 | `AddToolButton` 的默认 Class |
| tool_checkbox_class | 空 | `AddToolCheckBox` 的默认 Class（为空时回退 `tool_class`）|
| tool_separator_class | 空 | `AddToolSeparator` 的默认 Class |

### 3.7 主题新增 Class

在 `global.xml` 增加：

- `toolbar_button`：`btn_global` + 64×28 + 圆角 3；
- `toolbar_check`：同上，另加 selected 颜色/文字色；
- `toolbar_separator`：9×18 垂直 Line。

仅为示例与推荐默认值，ToolBar 不强制依赖这些 Class。

## 4. 精确改动清单

1. `duilib/Control/ToolBar.h`：新增回调类型、添加/删除/查询/勾选/启用/可见 API、
   默认 Class 属性与私有辅助函数。
2. `duilib/Control/ToolBar.cpp`：实现上述 API；实现名称生成、重复校验、
   事件绑定（Button 的 click、CheckBox 的 select/unselect）、移除时清理收纳缓存并销毁控件；
   溢出激活改为复用 `ActivateTool`。
3. `bin/resources/themes/default/global.xml`：新增 `toolbar_button` / `toolbar_check` / `toolbar_separator`。
4. `bin/resources/themes/default/controls/toolbar.xml`：新增一个空 `ToolBar`（`dynamic_toolbar`）
   与事件回显 Label。
5. `examples/controls/ToolBarForm.h/.cpp`：用新 API 动态创建按钮、勾选按钮、分隔线，
   绑定两类回调并回显事件。
6. `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`：补齐动态项与提示文案。
7. `docs/Control.md` / `docs/Control.en.md`：ToolBar 小节新增属性、接口、事件与动态示例。
8. `docs/misc/Development-Plan.md`、`docs/misc/Control-Roadmap.md` / `.en.md`：状态回写。
9. 三份技能参考：ToolBar 速查行/小节同步。
10. `docs/misc/specs/D-7-ToolBar-Items/`：spec / tasks / checklist。

## 5. 验证步骤

```bash
MSBuild build\examples.sln /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1
python .trae/skills/nim-duilib/scripts/verify_docs.py --extra docs
```

运行时：
- 打开 `bin/Controls_d.exe`（或对应示例入口）中的 ToolBar 示例；
- 动态工具条应出现按钮/勾选按钮/分隔线；
- 点击按钮与勾选按钮，事件回显 Label 正确刷新；
- 缩小窗口，动态项可被收纳进"更多"，浮层点击仍能触发回调；
- 中英文切换、浅色/深色、100%/150% DPI 显示正常。

## 6. 验收标准

- [ ] 动态添加的 Button / CheckBox / Line 正常显示与布局。
- [ ] `AddToolButton` 返回的指针可继续设置尺寸/图标。
- [ ] 重名添加被拒绝，空名称自动生成且可查询。
- [ ] 点击回调参数为工具项名称；勾选回调携带变化后的状态。
- [ ] `SetToolChecked` 不触发事件（`bTriggerEvent=false`），传 true 时触发。
- [ ] `RemoveTool` / `RemoveToolAt` / `RemoveAllTools` 后无残留控件、无崩溃。
- [ ] 溢出收纳与浮层激活行为正常，原有 `AttachOverflowItemActivated` 仍可用。
- [ ] 静态 XML 写法不受影响。
- [ ] Debug|x64 编译零错误，文档校验零漂移。

## 7. 待用户确认

- 无阻塞项。默认样式 Class 名称与事件命名如需与其它框架对齐，可在评审时调整。

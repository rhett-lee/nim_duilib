# 新增控件端到端流程（New Control Playbook）

本文是"给 nim_duilib 新增一个**独立控件**"的完整作业流程，按阶段执行即可避免常见遗漏。
给**已有控件**加属性请改看 `framework-extend.md`；只是**使用**控件请改看 `add-control.md`。

> 本文由 B4 Stepper、B3 RangeSlider、B9 TreeGrid 等实际交付过程沉淀而来。
> 权威范例：`docs/misc/specs/B4-Wizard/`（spec / tasks / checklist 三件套）。

---

## 0. 先判断：真的需要新控件吗

动手前先做三件事，避免重复造轮子或选错扩展方式：

1. 搜已有控件：`duilib/duilib_defs.h` 的 `DUI_CTR_*`、`duilib/Control/`、`docs/Control.md`。
2. 搜规划文档：`docs/misc/Control-Roadmap.md`、`docs/misc/Project-Audit.md`，确认该需求是否已有结论（已完成 / 明确不做）。
3. 判断实现方式：

| 情况 | 做法 |
|------|------|
| 语义是全新控件（如步骤条、评分、轮播） | **新增独立控件**，走本文 |
| 只是已有控件缺一两个 XML 属性 / API | 扩展已有控件，走 `framework-extend.md`（数值/格式化类扩展见其第六节） |
| 组合几个已有控件即可（如搜索框） | 优先新增轻量组合控件，避免堆属性 |
| 已有控件能"勉强模拟"但语义不符 | 新增独立控件，但在 spec 里写清与既有控件的语义差异 |

**判定原则**：语义不同就独立；只是表现差异就扩展。

---

## 1. 阶段 0：需求澄清（不写代码）

先产出一段可评审的需求结论，必须回答下面这些点：

- **使用场景**：安装向导 / 审批流程 / 数据展示…… 列出 2~3 个真实场景。
- **与已有控件的差异**：为什么 `Timeline`、`NavigationView`、`ListBox` 不能替代。
- **数据模型**：条目结构、当前索引、禁用集合、分组、层级等。
- **输入方式**：XML 属性（静态配置）+ C++ API（动态增删改）+ 多语言 `*_id`。
- **状态模型**：有哪些状态、状态如何派生（例如"当前索引 + 禁用集合"派生 4 态）。
- **交互**：点击 / 拖拽 / 键盘 / hover / pressed / 禁用态。
- **事件**：是否需要事件；复用哪个 `EventType`；`pEventData` 携带什么。
- **外观**：自绘还是图片驱动；默认 Class；语义色；DPI 缩放。
- **明确不做**：本次范围外的能力（写进 spec，避免评审时反复讨论）。

产出：需求要点清单 + 命名（XML 节点名、C++ 类名）+ 验收标准草案。

---

## 2. 阶段 1：写 spec（评审通过后再动手）

### 2.1 目录与命名

```
docs/misc/specs/<编号>-<ShortName>/
├── spec.md        # 目标、设计决策、精确改动清单、验证、验收
├── tasks.md       # 可勾选的任务序列 T1..Tn
└── checklist.md   # 分类验收清单
```

`<编号>` 用规划文档里的编号（如 `B4-Wizard`、`B3-RangeSlider`、`B9-TreeGrid`），便于回写
`Project-Audit.md` / `Development-Plan.md`。

### 2.2 spec.md 必备七节

1. **目标**：一句话说明解决什么问题，并写明"完成后回写哪份文档的哪一行"。
2. **需求要点**：来自盘点表 / 用户指令的原始条目，逐条列出。
3. **设计决策**：继承谁、数据模型、状态派生、事件编码、布局公式、交互命中规则。
4. **精确改动清单**：按文件逐个列，精确到"新增哪个宏 / 哪个 map 项 / 哪个属性分支"。
5. **验证步骤**：编译命令、运行时核对步骤、文档校验命令。
6. **验收标准**：可逐条打勾的行为描述。
7. **待用户确认**：列出需要拍板的点（视觉方案、事件编码、文件清单）。

> 需求中途变更时**追加"增量需求"章节**（如 B4 的节点 hover 图标），不要重写历史结论。

### 2.3 精确改动清单建议顺序

```
控件源码 → 注册（5 处）→ 主题/资源 → 示例（C++/XML/入口/语言）→ 文档（中英）→ 计划文档回写
```

写清单时就把下面第 3~5 阶段的内容全部落到具体文件，实现阶段只做"照单执行"。

### 2.4 tasks.md / checklist.md

- `tasks.md` 按依赖顺序拆 6~8 个任务，每项带验证动作。
- `checklist.md` 按「源码 / 注册 / 示例 / 验证 / 文档」分组，全部可勾选。
- 完成定义（DoD）固定包含：Debug|x64 编译通过、功能验收、文档零漂移、计划文档已回写。

---

## 3. 阶段 2：控件实现

### 3.1 文件位置与类骨架

| 控件类型 | 目录 |
|----------|------|
| 自绘 / 叶子控件（Stepper、Rating、Chart、Timeline） | `duilib/Control/` |
| 容器 / 布局（Panel、NavigationView） | `duilib/Box/` 或 `duilib/Layout/` |
| 与窗口/系统能力相关（对话框、通知） | `duilib/Utils/` |

头文件骨架（可直接照抄改）：

```cpp
#ifndef UI_CONTROL_XXX_H_
#define UI_CONTROL_XXX_H_

#include "duilib/Core/Control.h"

namespace ui {

class DUILIB_API Xxx : public Control
{
    typedef Control BaseClass;

public:
    explicit Xxx(Window* pWindow);
    ~Xxx() override;

    /** 获取控件类型（必须返回 DUI_CTR_XXX） */
    DString GetType() const override;

    /** 解析 XML 属性；未识别的属性回退 BaseClass::SetAttribute */
    void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 自绘入口（具体名字按基类虚函数选择） */
    void PaintStateImages(IRender* pRender) override;

    /** 语言切换：重解析 *_id 数据 */
    void OnLanguageChanged(bool bRedraw) override;

    /** 释放图片缓存 */
    void ClearImageCache() override;

    // 数据 / 状态 / 外观的 Set/Get API
    // 事件：void AttachXxxChanged(const EventCallback&, EventCallbackID = 0);
};

} // namespace ui

#endif // UI_CONTROL_XXX_H_
```

实现要点：

- **构造初始化所有成员**；颜色默认值写语义色名（如 `text_muted`、`color_accent`），不写死色值。
- **`GetType()` 必须返回 `DUI_CTR_XXX`**，否则主题 Class 与调试识别会失效。
- **属性名同时支持主名与历史别名**（如 `direction` / `orientation`），未识别的分支最后回退基类。
- **DPI**：像素尺寸先存原始值，绘制/命中时用 `Dpi().GetScaleInt(...)` 缩放。
- **绘制**：先算控件相对坐标再绘制；文本统一走 `DrawStringParam` + 字体 ID。
- **状态优先级**写进代码注释（如 禁用 > 当前 > 已完成 > 待开始），避免后续改动破坏语义。
- **平台隔离**：公共实现里不要直接调 Win32；必须分平台时拆 `Xxx_Windows.cpp` / `Xxx_SDL.cpp`。

### 3.2 注册（5 处，缺一不可）

| # | 文件 | 改什么 |
|---|------|--------|
| 1 | `duilib/duilib_defs.h` | 新增 `#define DUI_CTR_XXX (_T("Xxx"))` |
| 2 | `duilib/Core/WindowBuilder.cpp` | `#include "duilib/Control/Xxx.h"`；`CreateControlMap()` 增加 `{DUI_CTR_XXX, [](Window* pWindow){ return new Xxx(pWindow); }}` |
| 3 | `duilib/duilib.vcxproj` | `<ClCompile Include="Control\Xxx.cpp" />` + `<ClInclude Include="Control\Xxx.h" />` |
| 4 | `duilib/duilib.vcxproj.filters` | 上述两项归入 `Control` 过滤器 |
| 5 | `duilib/duilib.h` | `#include "Control/Xxx.h"` |

CMake 构建使用 `aux_source_directory` 自动收集 `duilib/Control/*.cpp`，**新增文件后需重新 configure**；
VS 工程必须手工登记，否则链接/编译找不到文件。

### 3.3 主题与默认样式

- 图片放 `bin/resources/themes/default/public/<name>/`，SVG 占位色用 `#0078D4`，配
  `svg_replace_colors='#0078D4|color_accent'` 跟随主题强调色。
- `bin/resources/themes/default/global.xml`：
  - 新增 `<Var name="PATH_XXX" value="public/<name>"/>`；
  - 新增 `<Class name="xxx_default" .../>`，只放"默认皮肤"需要的属性，控件自身仍要能无 Class 工作。
- 所有颜色写**语义色名**；写死色值会导致深色主题失效（必须在浅色/深色两套主题下各看一次）。

### 3.4 多语言

- 文本类属性提供 `xxx_id` 变体（多值字段用 `|` 分隔，逐段解析）。
- 控件内部**保存原始 ID 串**，在 `OnLanguageChanged()` 里重新解析，不要只换一次文本。
- 示例新增的 `STRID_*` 写 `bin/resources/lang/zh_CN-examples.txt` 与 `en_US-examples.txt`；
  框架内置文案写 `zh_CN.txt` / `en_US.txt`。两文件均为 UTF-8 无 BOM，格式 `键=值`。

### 3.5 事件约定

- 优先复用已有 `EventType`（值变化类用 `kEventValueChanged`），用 `wParam` 做模式标记区分。
- 事件数据用栈上临时结构体，经 `pEventData` 传递；**注释必须写明"仅本次回调有效，勿保存指针"**。
- 只在值**实际变化**时派发一次；提供 `SetXxxNoEvent` / `GoToNoEvent` 等静默接口，防止与业务回调自递归。
- 便捷包装 `AttachXxxChanged(...)`，内部转 `AttachEvent(...)`。

### 3.6 健壮性自检

- 空数据、单条数据、越界索引（自动钳位）、重复设置同一值（幂等、不派发事件）。
- 禁用态不可交互；`click_to_jump` 关闭时点击不跳转。
- 运行时增删条目后，当前索引与绘制仍一致。

---

## 4. 阶段 3：示例与测试入口

统一放在 `examples/controls`（新增控件默认都进这个示例程序）：

1. **新建 `examples/controls/XxxForm.h/.cpp`**
   - `class XxxForm : public ui::WindowImplBase`，自带 `typedef ui::WindowImplBase BaseClass;`
   - `GetSkinFolder()` 返回 `_T("controls")`；`GetSkinFile()` 返回 `_T("xxx.xml")`。
   - `OnInitWindow()` 里 `FindControl` + `dynamic_cast` + 判空，绑定事件与演示按钮。
2. **新建 `bin/resources/themes/default/controls/xxx.xml`**
   - 标准标题栏 + 提示 Label + 内容卡片，至少覆盖：默认形态、另一方向/另一种配置、禁用态。
   - `class` 属性写在所有属性最前面。
3. **入口按钮**：`bin/resources/themes/default/controls/test.xml` 新增分组与
   `<Button class="btn_global" name="xxx_btn" text_id="STRID_CONTROLS_XXX_TEST" .../>`。
4. **入口逻辑**：
   - `examples/controls/TestForm.h` 新增 `void ShowXxxWindow();`
   - `examples/controls/TestForm.cpp` 引入头文件、绑定按钮、实现弹窗
     （`kWS_POPUP + kWS_EX_LAYERED + m_bCenterWindow`，`ShowModalFake()`）。
5. **工程登记**：`examples/controls/controls.vcxproj` + `.filters` 加入新 `ClInclude` / `ClCompile`。
6. **语言资源**：中英两份 `examples.txt` 补齐窗口标题、提示、按钮、示例条目文案。
7. **交互覆盖**：示例里要能手工触发所有公开 API（增删、跳转、禁用、reset、事件回显）。

---

## 5. 阶段 4：文档同步（中英双语成对）

新增控件必须**中英同步**，不允许只改中文。固定清单：

| 文档 | 必改内容 |
|------|----------|
| `docs/Control.md` / `docs/Control.en.md` | 新增控件小节：属性表（名称/默认值/类型/用途）、XML 示例、C++ 接口、事件说明；两文标题保持对应（中文 `## Xxx 控件（...）`，英文 `## Attributes of Xxx (...)`） |
| `docs/Summary.md` / `docs/Summary.en.md` | 控件总表新增一行：类名、基类、头文件链接、能力摘要、指向 Control 锚点 |
| `README.md` / `README.en.md` | 功能列表新增条目（保持编号连续）与首页描述/计数同步 |
| `docs/misc/Control-Roadmap.md` / `.en.md` | 若来自路线图，更新对应项状态与"现有能力盘点" |
| `docs/misc/Project-Audit.md` | 对应审计编号改为 ✅ 已完成，列源码/节点名/示例 |
| `docs/misc/Development-Plan.md` | 对应跟踪项 `- [ ]` → `- [x]`，填完成日期 |
| `.trae/skills/nim-duilib/references/api-reference.md` | 控件速查行 + 属性小节 + 事件 |
| `.workbuddy/skills/nim-duilib/references/api-reference.md` | 与上一份**同源同文本**覆盖 |
| `.claude/docs/nim-duilib-llm-reference.md` | 速查行/描述同步 |
| `docs/<Feature>.md` / `.en.md` | 仅当控件足够大、需要独立专题文档时新增（参考 Toast / SystemNotification） |

文档规则：

- 属性表默认值必须与代码构造函数一致；属性别名用「主名（别名 xxx）」写法。
- 事件说明写清 `EventType`、`wParam` 含义、`pEventData` 类型与生命周期。
- 英文文档的锚点按英文标题生成（如 `#attributes-of-stepper-wizard-step-indicator`），Summary 里的
  链接必须指向英文锚点。
- 完成后必须跑校验（见阶段 6）。

---

## 6. 阶段 5：编译

### 6.1 常规命令（Debug|x64）

```bash
MSBuild "c:\develop\nim_duilib\build\examples.sln" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1
```

- 只验证 **Debug|x64**（项目约定：不编译 Release，Release 由用户自行验证）。
- `/m:1` 规避本机内存限制；`/clp:ErrorsOnly` 无输出即无错误。
- 构建会同时产出 `duilib` 与示例；示例可执行文件在 `bin/` 下。

### 6.2 改了头文件的额外要求

改了 `duilib/` 下被库内实例化的头文件（尤其模板类），**必须先重建 duilib 再重建示例**，
否则会出现旧布局 `new` + 新布局读写的 ODR 崩溃（0xC0000005）。详见 `framework-extend.md` 第三节。

### 6.3 快速语法检查

只验证单个新 `.cpp` 的 API/模板写法时，可用 MSVC `/Zs`（不生成 obj、不链接），
命令见 `SKILL.md`「编译前自检：只编译不链接」。

---

## 7. 阶段 6：手工测试清单

编译通过 ≠ 功能正确。按下表逐项操作并截图留证：

| 类别 | 检查项 |
|------|--------|
| 创建 | XML 中写节点名/Class 能创建；`FindControl` 能取到；无 Class 时也能正常显示 |
| 属性 | 属性表里每个属性都实际改一次并观察效果；别名与主名等效；未知属性不崩 |
| 默认值 | 不写任何属性时与文档默认值一致 |
| 边界 | 空数据、单条、越界索引、重复设置、极值、超长文本 |
| 交互 | 鼠标 hover / pressed / 点击 / 拖拽（若有）；禁用态不可交互；失去焦点恢复 |
| 键盘 | 方向键 / Home / End / Enter / Esc（若控件声明支持） |
| 状态与事件 | 状态派生正确；事件只在变化时触发一次；`pEventData` 内容正确；`NoEvent` 接口不触发 |
| 多语言 | 中文 → 英文 → 中文切换，所有 `*_id` 文案自动刷新，布局不串位 |
| 主题 | 浅色 / 深色主题各看一遍，无写死色值导致的对比度问题 |
| DPI | 100% / 125% / 150% / 200% 下节点、间距、命中区域一致 |
| 布局 | 拉伸窗口、滚动、最小尺寸下不重叠、不裁剪异常 |
| 生命周期 | 反复打开/关闭示例窗口无崩溃；图片缓存释放（`ClearImageCache`） |
| 回归 | 相邻控件示例（如 test.xml 其他入口）仍正常工作 |

截图建议用系统截图或项目内已有的 DPI-aware 辅助脚本，保留"默认态 / 交互后 / 深色 / 英文"四类。

---

## 8. 阶段 7：校验与收尾

1. **文档漂移校验**（仓库根执行）：

   ```bash
   python3 .trae/skills/nim-duilib/scripts/verify_docs.py --extra docs
   ```

   期望：`全部命中，未发现文档漂移`。若报某名字不存在：
   - 真实漂移 → 改文档；
   - 有意引用（旧名/规划名/控件内部 name）→ 登记到 `scripts/verify_allowlist.txt`，写清原因。

2. **回写计划文档**：Project-Audit、Development-Plan、Control-Roadmap 的状态与完成日期。
3. **输出交付摘要**：新增/修改文件清单、编译结果、手工测试证据、文档校验结果。
4. **不执行 git commit**（项目约定：提交由用户手动执行）。

---

## 9. 一页检查清单（复制到任务里逐项勾）

```text
[ ] 需求澄清：场景 / 差异 / 数据模型 / 状态 / 交互 / 事件 / 范围外
[ ] spec.md / tasks.md / checklist.md 已评审
[ ] Xxx.h/.cpp 实现（GetType/SetAttribute/Paint/OnLanguageChanged/ClearImageCache）
[ ] duilib_defs.h DUI_CTR_XXX
[ ] WindowBuilder.cpp include + 工厂 map
[ ] duilib.vcxproj / .filters 登记
[ ] duilib.h include
[ ] global.xml PATH_XXX + Class；SVG 资源（如需要）
[ ] 示例 XxxForm.h/.cpp + xxx.xml
[ ] test.xml 入口按钮 + TestForm ShowXxxWindow
[ ] controls.vcxproj / .filters 登记示例文件
[ ] 中英语言资源 STRID_* 齐全
[ ] docs/Control.md + .en.md（属性表/示例/API/事件）
[ ] docs/Summary.md + .en.md
[ ] README.md + .en.md
[ ] Control-Roadmap / Project-Audit / Development-Plan 状态回写
[ ] 三份技能参考（.trae / .workbuddy / .claude）同步
[ ] Debug|x64 编译零错误（改头文件时先重建 duilib）
[ ] 手工测试：默认/边界/交互/事件/多语言/主题/DPI/生命周期
[ ] verify_docs.py 零漂移
[ ] 交付摘要（不执行 git commit）
```

---

## 10. 高频遗漏与坑

| 现象 | 根因 | 处理 |
|------|------|------|
| XML 里创建不出控件，或启动崩溃 | `WindowBuilder.cpp` 未注册工厂 | 补 `DUI_CTR_XXX` + map 项 |
| VS 编译报找不到文件 | `duilib.vcxproj` / 示例 `.vcxproj` 未登记 | 同时改 `.filters` |
| CMake 构建漏掉新 cpp | `aux_source_directory` 在 configure 时求值 | 重新运行 cmake configure |
| 深色主题下颜色刺眼/看不清 | 属性默认值写死色值 | 改语义色名，并在两套主题验证 |
| 切换语言后控件文案不变 | 只解析一次 `*_id`，未覆写 `OnLanguageChanged` | 保存原始 ID 串并重解析 |
| 事件回调里再次 Set 导致死循环 | 缺少 `NoEvent` 静默接口 | 增加 `SetXxxNoEvent` / 守卫标志 |
| 回调里保存 `pEventData` 后崩溃 | 指针只在本次派发期间有效 | 文档与注释写明，业务侧拷贝数据 |
| 高分屏节点/命中区域错位 | 绘制用了原始像素，命中未同步缩放 | 统一用 `Dpi().GetScaleInt()` |
| 改了头文件后示例崩溃 0xC0000005 | duilib 库未重建，ODR 布局冲突 | 先重建 `duilib`，再重建示例 |
| 文档检查报"不存在" | 文档名与源码不一致，或引用了控件内部 name | 改文档；确属有意引用才进 allowlist |
| 英文文档链接 404 | Summary 用了中文锚点 | 英文锚点按英文标题生成 |
| 扩展控件时 `value` 被格式属性舍入 | XML 属性按声明顺序生效，`value` 先于 `decimals` | 格式类属性写在数值之前（详见 `framework-extend.md` 6.5） |
| 删除子控件时重复 `delete` | `Box::RemoveItem` 在 `AutoDestroyChild=true` 时会销毁控件 | 先判断 `IsAutoDestroyChild()`，仅在 false 时手动 delete（详见 `framework-extend.md` 第七节） |
| 编译通过但一操作就崩溃 | 只验证了编译，未执行实际交互路径 | 增删/事件类改动必须实际点击一次，不能只看编译结果 |

---

## 11. 参考实例

| 控件 | spec 目录 | 可学习点 |
|------|-----------|----------|
| Stepper | `docs/misc/specs/B4-Wizard/` | 完整三件套、事件编码、状态派生、增量需求写法 |
| RangeSlider | `docs/misc/specs/B3-RangeSlider/` | 双滑块交互、`kEventValueChanged` 的 `wParam` 模式标记 |
| Carousel | `docs/misc/specs/B7-Carousel/` | 图片驱动 + 主题色替换、定时器与生命周期 |
| TreeGrid | `docs/misc/specs/B9-TreeGrid/` | 新控件而非改 ListCtrl 的边界判断 |
| SpinBox 小数/格式化（D-3） | `docs/misc/specs/D-3-SpinBox-Decimal/` | 已有控件能力扩展：基类回调改虚、双模式输入过滤、XML 属性顺序、解析/格式化成对 |
| ToolBar 工具项管理（D-7） | `docs/misc/specs/D-7-ToolBar-Items/` | 已有容器加子项管理：删除所有权、索引排除内部控件、动态插入维护内部按钮位置 |
| DateTime 时间弹层（D-5） | `docs/misc/specs/D-5-TimePicker/` | 已有控件加弹层：新增枚举避免破坏旧语义、动态 XML 三种形态、SpinBox 复用、确定/取消提交语义、禁用日期与键盘焦点跳过 |
| SystemNotification | `docs/SystemNotification.md` | 大型能力独立成类 + 独立专题文档（中英） |

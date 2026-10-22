# D-10 规格：HFlowLayout 主轴 justify 分布（WrapPanel 语义）

> 状态：✅ 已批准并实现（2026-10-22）
> 日期：2026-10-22

## 一、背景与结论

`docs/misc/Development-Plan.md` 中 D-10 的「现状依据」与代码不符：

| 调研结论 | 证据 |
| :--- | :--- |
| 不等宽换行已存在 | `HFlowBox` = `HFlowLayout`，按子控件自身宽度依次排列，超出容器宽度时自动换行 |
| 已公开为 XML 控件 | `duilib/Core/WindowBuilder.cpp:111` 注册 `HFlowBox`；`docs/Box.md:36` 已登记 |
| TagInput 直接复用它 | `duilib/Control/TagInput.cpp:17` 的 `TagInputLayout : public HFlowLayout`，仅额外处理「编辑框占满行尾」 |
| 与 WPF WrapPanel 的真正差距 | 行内主轴剩余空间只能整体靠左/居中/靠右（`child_halign`），缺 `space-between` / `space-around` / `space-evenly` 分布 |

因此**不新建 WrapPanel 控件，也不修改 `Layout` 基类**：全部能力收敛在 `HFlowLayout` 内实现；`HFlowBox` 无需改代码（`Box::SetAttribute` 已把未知属性转发给布局对象）。

## 二、目标与非目标

### 目标

1. `HFlowLayout` 新增 XML 属性 `justify`（别名 `justify_content`）。
2. 取值：`none`（复位为未设置）、`start`、`center`、`end`、`space-between`、`space-around`、`space-evenly`（分隔符 `-` 与 `_` 均可）。
3. `justify` 显式设置后优先于 `child_halign` 的行内对齐行为；未设置时保持现有行为，**零回归**。
4. 补齐 `examples/layout` 的 HFlowBox 演示与中英文文档。
5. 更新 Development-Plan，D-10 标记完成。

### 非目标

- 不修改 `duilib/Layout/Layout.h/.cpp`（枚举、成员、解析、偏移计算都在 HFlowLayout 内）。
- 不改 `LayoutType` 枚举、不新建控件类、不改 `WindowBuilder`、不改 `HFlowBox`。
- 本次不改 `VFlowLayout`（D-10 的诉求是「按内容宽度换行」，纵向流式布局不在本条范围）。
- 不实现统一 `item_width` / `item_height`、交叉轴拉伸或 `align_items`。

## 三、API 设计（仅 `HFlowLayout`）

### 3.1 `duilib/Layout/HFlowLayout.h`

在类内新增嵌套枚举与公开接口，避免污染 `ui` 命名空间、也不影响其它布局：

```cpp
class DUILIB_API HFlowLayout : public Layout
{
    typedef Layout BaseClass;
public:
    /** 主轴（水平方向）的分布方式 */
    enum class JustifyContent
    {
        kNotSet,        //未设置：沿用 child_halign 的原有行为
        kStart,         //起始端对齐
        kCenter,        //居中
        kEnd,           //末端对齐
        kSpaceBetween,  //两端对齐，相邻子项之间均分剩余空间
        kSpaceAround,   //每个子项两侧均分剩余空间（首尾各半个间距）
        kSpaceEvenly    //所有间隙（含首尾）相等
    };

    HFlowLayout();

    /** 获取主轴分布方式 */
    JustifyContent GetJustifyContent() const { return m_justifyContent; }

    /** 设置主轴分布方式（值变化且存在 Owner 时触发重新布局） */
    void SetJustifyContent(JustifyContent justifyContent);

    /** 设置布局属性：解析 justify / justify_content，其余交给基类 */
    virtual bool SetAttribute(const DString& strName,
                              const DString& strValue,
                              const DpiManager& dpiManager) override;

    // ... 现有 ArrangeChildren / EstimateLayoutSize 声明保持不变 ...

private:
    /** 主轴分布方式（仅本布局使用） */
    JustifyContent m_justifyContent;
};
```

### 3.2 `duilib/Layout/HFlowLayout.cpp`

- 构造函数：`m_justifyContent(JustifyContent::kNotSet)`。
- `SetJustifyContent`：若值变化且 `GetOwner() != nullptr`，调用 `GetOwner()->Arrange()`（与 `Layout::SetChildHAlignType` 的行为一致）。
- `SetAttribute`：
  - `justify` / `justify_content`：解析成功则 `SetJustifyContent` 并返回 true；解析失败返回 false；
  - 其它属性：`return BaseClass::SetAttribute(...)`。
- 解析与偏移计算放在 .cpp 的匿名命名空间（不新增头文件、不新增源文件）：

```cpp
namespace {
/** 解析 justify 属性值 */
bool ParseJustify(const DString& strValue, HFlowLayout::JustifyContent& justify);

/** 计算主轴剩余空间的分布偏移（长度 = nCount，单位像素） */
std::vector<int32_t> CalcJustifyOffsets(size_t nCount, int32_t nExtra,
                                        HFlowLayout::JustifyContent justify);
}
```

取值映射：

| 输入 | 结果 |
| :--- | :--- |
| `none` / `not-set` / `not_set` | `kNotSet`（复位为未设置） |
| `start` | `kStart` |
| `center` | `kCenter` |
| `end` | `kEnd` |
| `space-between` / `space_between` / `spacebetween` | `kSpaceBetween` |
| `space-around` / `space_around` / `spacearound` | `kSpaceAround` |
| `space-evenly` / `space_evenly` / `spaceevenly` | `kSpaceEvenly` |
| 其它 | 解析失败（返回 false） |

偏移公式（`int64_t` 乘法防溢出，整数除法取整）：

| 方式 | 第 i 项偏移 |
| :--- | :--- |
| `kStart` / `kNotSet` | 0 |
| `kCenter` | `nExtra / 2` |
| `kEnd` | `nExtra` |
| `kSpaceBetween` | `nCount > 1` 时 `nExtra * i / (nCount - 1)`；`nCount == 1` 时为 0 |
| `kSpaceAround` | `nExtra * (2*i + 1) / (2*nCount)`（`nCount == 1` 自然居中） |
| `kSpaceEvenly` | `nExtra * (i + 1) / (nCount + 1)` |

### 3.3 布局应用位置

`HFlowLayout::ArrangeChildInternal` 中替换现有「水平方向按行对齐」代码块（`HFlowLayout.cpp:208-235`）：

- `m_justifyContent == kNotSet` 时**原样保留原逻辑代码块**（仅 `child_halign` 为 center/right 时按行整体偏移），不经过 `CalcJustifyOffsets`，确保行为完全一致。
- 其它取值进入新增的分布分支：

```
生效方式 = m_justifyContent
if (生效方式 != kStart):
    对每一行:
        nCount = 本行子项数
        nExtra = rcBox.Width() - rowRect.Width()
        若 nCount == 0 或 nExtra <= 0，跳过
        offsets = CalcJustifyOffsets(nCount, nExtra, 生效方式)
        逐项 rcChild.Offset(offsets[i], 0)
        用「子项矩形 + 其 margin」重新求并集得到新的 rowRect
        childrenRect.Union(rowRect)
```

- 保留垂直方向整块对齐逻辑（含 `ScrollBox` 纵向滚动条判断）不变。
- 评估模式（`bEstimateLayoutSize`）仍提前返回，不参与分布。

## 四、示例与文档改动

### 4.1 layout 示例（仅横向流式页面）

`bin/resources/themes/default/layout/page_hflow_box.xml`：

- 顶部 `VBox height="84"` 改为 `height="112"`（容纳新增一行）。
- 在现有 halign/valign 选项行之后新增一行：
  - 标签 `text_id="STRID_LAYOUT_PAGE_31"`；
  - 5 个 `Option`（`class="option_1"`，`group="hflowbox_group_justify"`，`receiver="hflowbox_test"`）：
    - `STRID_LAYOUT_PAGE_37` → `apply_attribute="justify='none'"`（`selected="true"`，用于验证 kNotSet 与原逻辑一致）
    - `STRID_LAYOUT_PAGE_35` → `apply_attribute="justify='start'"`
    - `STRID_LAYOUT_PAGE_32` → `apply_attribute="justify='space-between'"`
    - `STRID_LAYOUT_PAGE_33` → `apply_attribute="justify='space-around'"`
    - `STRID_LAYOUT_PAGE_34"` → `apply_attribute="justify='space-evenly'"`
  - 行尾标签 `text_id="STRID_LAYOUT_PAGE_36"`（提示「设置后覆盖 child_halign」）。

### 4.2 语言资源

`bin/resources/lang/zh_CN-examples.txt` 与 `en_US-examples.txt` 在 `STRID_LAYOUT_PAGE_30` 之后追加：

| ID | 中文 | 英文 |
| :--- | :--- | :--- |
| `STRID_LAYOUT_PAGE_31` | 主轴分布方式（justify）： | Main-axis distribution (justify): |
| `STRID_LAYOUT_PAGE_32` | 两端对齐(space-between) | Space between |
| `STRID_LAYOUT_PAGE_33` | 环绕分布(space-around) | Space around |
| `STRID_LAYOUT_PAGE_34` | 均匀分布(space-evenly) | Space evenly |
| `STRID_LAYOUT_PAGE_35` | 默认分布(start) | Default (start) |
| `STRID_LAYOUT_PAGE_36` | （设置后覆盖 child_halign） | (Overrides child_halign when set) |
| `STRID_LAYOUT_PAGE_37` | 未设置(兼容原逻辑) | Not set (original logic) |

### 4.3 文档

- `docs/Box.md` 第 4 节「水平流式布局（HFlowLayout）」：在「可用属性继承浮动布局」之外新增属性表：

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| justify | 未设置 | string | 主轴（行内）剩余空间的分布方式。可选值：`start`、`center`、`end`、`space-between`、`space-around`、`space-evenly`；未设置时沿用 `child_halign` |

  并补充说明与 XML 示例（含「仅当行宽小于容器宽度时生效」）。
- `docs/Box.en.md` 第 4 节同步英文内容。
- 不修改「1. 浮动布局（Layout）」的通用属性表（`justify` 不是基类属性）。

## 五、兼容性

- 默认 `kNotSet`，未配置 `justify` 的既有界面行为完全不变。
- `justify` 只由 `HFlowLayout::SetAttribute` 解析；`HBox`、`VBox`、`VFlowBox`、`HTileBox` 等不受影响。
- 不涉及序列化、`LayoutType` 枚举或控件工厂注册。

## 六、计划文档更新

`docs/misc/Development-Plan.md`：

1. 第 4 行基线日期改为 `2026-10-22`，并补记 D-10 完成。
2. 第 40 行「P2 控件 / 待做 / D-10」改为「已完成 / —」。
3. 第 716 行 D-10 单元格改为 `✅ 已完成（2026-10-22）`，说明：不等宽换行已由 `HFlowBox`/`HFlowLayout` 提供；本次为 `HFlowLayout` 增加 `justify`（6 种分布）并补示例与文档，未修改 `Layout` 基类。
4. 第 845 行实际进展说明中「剩余集中在 D 组 1 项（D-10…）」改为 D 组 0 项。
5. 第 872 行 `- [ ]` 改为 `- [x]`，完成日期填 `2026-10-22`。

## 七、验证步骤

1. MSBuild Debug|x64 编译 `build/examples.sln`（不做 Release），要求 0 error。
2. `python .trae/skills/nim-duilib/scripts/verify_docs.py --repo . --extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang` 退出码 0。
3. `git status` 仅包含本规格列出的文件，无临时脚本/截图。
4. 人工视觉验证（交由用户）：运行 `layout` 示例 → HFlowBox 页切换各 `justify` 选项：
   - 默认与 `start` 行为与改动前一致；
   - `space-between` 每行首尾贴边、间隙均分；
   - `space-around` / `space-evenly` 间隙符合定义；
   - 换行数量不变（子项宽度未被修改）。

## 八、审批事项

请确认：
1. 交付范围：仅增强 `HFlowLayout`（HFlowBox 直接受益），不修改 `Layout` 基类、不改 `VFlowLayout`。
2. 属性名 `justify`（别名 `justify_content`）与 6 种取值。
3. 示例改动落在现有 `examples/layout` 的 HFlowBox 页面。

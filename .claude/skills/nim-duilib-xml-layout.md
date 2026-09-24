---
name: nim-duilib-xml-layout
description: 为 nim_duilib 窗口设计和生成 XML 布局
trigger: 当用户需要设计界面布局、创建XML界面、修改界面结构时触发
---

# nim_duilib XML 布局设计

## 执行步骤

### 1. 理解需求
确认用户想要的界面布局:
- 界面包含哪些区域（标题栏、侧边栏、内容区、底部栏等）
- 每个区域包含哪些控件
- 布局方式（垂直排列、水平排列、网格等）

### 2. 选择合适的容器

**布局选择指南:**
| 需求 | 推荐容器 |
|------|---------|
| 从上到下排列 | VBox |
| 从左到右排列 | HBox |
| 自动换行的水平排列 | HFlowBox |
| 等宽网格 | VTileBox / HTileBox |
| 行列网格（支持合并） | GridBox |
| 需要滚动 | VScrollBox / HScrollBox |
| 可选择的列表 | VListBox / HListBox |
| 大数据量列表 | VirtualVListBox |
| 多页签切换 | TabBox |

**尺寸设置指南:**
| 需求 | width/height 值 |
|------|----------------|
| 固定尺寸 | 数值如 "200" |
| 填充父容器 | "stretch"（默认值） |
| 适应内容 | "auto" |
| 父容器百分比 | "50%" |

### 3. 常用布局模板

**表单布局（标签+输入框）:**
```xml
<VBox padding="20,20,20,20" child_margin_y="10">
    <HBox height="auto" child_margin_x="10">
        <Label text="用户名：" width="80" height="30" text_align="right,vcenter"/>
        <RichEdit name="edit_username" width="stretch" height="30"
                  single_line="true" prompt_text="请输入用户名"
                  border_size="1" border_color="gray" border_round="4,4"/>
    </HBox>
    <HBox height="auto" child_margin_x="10">
        <Label text="密码：" width="80" height="30" text_align="right,vcenter"/>
        <RichEdit name="edit_password" width="stretch" height="30"
                  single_line="true" password="true" prompt_text="请输入密码"
                  border_size="1" border_color="gray" border_round="4,4"/>
    </HBox>
    <HBox height="auto" halign="right" child_margin_x="10" margin="0,20,0,0">
        <Button name="btn_cancel" text="取消" width="80" height="32" class="btn_global_white_80x30"/>
        <Button name="btn_ok" text="确定" width="80" height="32" class="btn_global_blue_80x30"/>
    </HBox>
</VBox>
```

**左右分栏布局:**
```xml
<HBox>
    <!-- 左侧面板 -->
    <VBox width="200" bkcolor="bk_wnd_lightcolor">
        <VListBox name="nav_list" width="stretch" height="stretch"/>
    </VBox>
    <!-- 分隔线 -->
    <Line vertical="true" line_color="gray" line_width="1" width="1"/>
    <!-- 右侧内容 -->
    <VBox width="stretch" padding="12,12,12,12">
        <Label text="内容区域" text_align="hcenter,vcenter"/>
    </VBox>
</HBox>
```

**工具栏+内容:**
```xml
<VBox>
    <!-- 工具栏 -->
    <HBox height="40" bkcolor="bk_wnd_lightcolor" padding="4,4,4,4" child_margin_x="4">
        <Button name="btn_new" text="新建" width="auto" height="stretch"/>
        <Button name="btn_open" text="打开" width="auto" height="stretch"/>
        <Button name="btn_save" text="保存" width="auto" height="stretch"/>
        <Line vertical="true" line_color="gray" width="1" margin="4,0,4,0"/>
        <Control /><!-- 弹性空间 -->
        <RichEdit name="search_edit" width="200" height="28" single_line="true"
                  prompt_text="搜索..." valign="center" border_size="1" border_color="gray"/>
    </HBox>
    <!-- 内容 -->
    <Box width="stretch" height="stretch">
        <!-- 内容区域 -->
    </Box>
</VBox>
```

**卡片网格:**
```xml
<VTileScrollBox columns="3" item_size="200,150" child_margin="10"
                padding="10,10,10,10" vscrollbar="true">
    <Box bkcolor="white" border_size="1" border_color="gray" border_round="8,8">
        <VBox padding="12,12,12,12">
            <Label text="卡片1" font="system_bold_14"/>
            <Label text="描述文本" normal_text_color="gray"/>
        </VBox>
    </Box>
    <!-- 更多卡片... -->
</VTileScrollBox>
```

### 4. 关键规则
- XML 文件编码必须是 **UTF-8**
- `class` 属性必须写在**所有属性最前面**
- 属性值内嵌引号用**单引号 `'`** 或 **花括号 `{}`** 代替双引号
- `<Control />` 作为弹性占位符，在 HBox/VBox 中占据剩余空间
- 窗口标题栏容器的 name 必须是 `window_title_bar`（旧名 `window_caption_bar` 仅作兼容 fallback）
- 标题栏按钮的 name 必须是: `btn_window_min`, `btn_window_max`, `btn_window_restore`, `btn_window_close`, `btn_window_fullscreen`
  （旧名 `minbtn`/`maxbtn`/`restorebtn`/`closebtn`/`fullscreenbtn` 仅作 fallback，新代码不要再用；
  见 `duilib/duilib_defs.h:145-152` 与 `duilib/Utils/WinImplBase.cpp:9-14`）
- 颜色值格式: "#AARRGGBB"(ARGB) 或颜色名

### 5. `<Include>` 共享 XML 片段

nim_duilib 支持用 `<Include src="fragment.xml"/>` 复用 XML 片段，让多个窗口共享同一段布局（比如公共状态栏、工具栏）。

**路径解析规则**（`WindowBuilder::ParseIncludeXmlNode`，`WindowBuilder.cpp:1405-1425`）
- `src` 相对于**宿主 XML 所在目录**解析；找不到则回退到 window resource path
- 推荐把共享片段与宿主 XML 放在**同一个 skin 目录**，写 `src="fragment.xml"` 最省心
- `src` 为空时会尝试读同名的 `source` 属性；支持 `count` 属性重复包含多次

**被包含 XML 的根节点**
- 推荐用单根 `<Window>` 包裹（符合 pugixml 单根要求）；`<Window>` 上的窗口属性会被 `IsWindowAttributesApplied()` guard 忽略，**不要在被包含的 `<Window>` 上写 size/caption/layered_window 等属性**，否则会误导
- 也可以直接用 Box 作为根（HBox/VBox/Box），实测同样工作

**`FindControl` 作用域**
- 按 Window 独立寻址，两个不同 Window 用相同 `name` **不会冲突**——这是跨窗口复用共享片段的基础

**历史问题（已修复，仅存档说明）**

早期版本有一个坑：`<Include>` 作为窗口根 `<Window>` 下的**第一个非样式子节点**时，窗口会被静默创建成
完全空白（看不见、无任何控件，且不报错）。原因是旧版 `WindowBuilder::ParseXmlNodeChildren` 处理
`<Include>` 分支时用 `continue` 跳过了尾部的 `pReturn = pControl` 赋值，`CreateControls` 因此返回
`nullptr`，框架不会为该 Window 建立 root Box。

**该缺陷已在 2026-05-21 的提交 `62c56e491` 修复**：现在 `<Include>` 分支（`WindowBuilder.cpp:1233-1240`）
会自行判定并给 `pReturn` 赋值：

```cpp
else if (strClass == _T("Include")) {
    Control* pNewControl = ParseIncludeXmlNode(node, pParent, pWindow);
    if ((pNewControl != nullptr) && (pReturn == nullptr)) {
        pReturn = pNewControl;
    }
    continue;
}
```

所以下面这种写法在当前代码库上是**正常可用**的：

```xml
<!-- ✅ 现在可以：<Include> 直接作为 <Window> 的唯一子节点 -->
<Window size="900,32" layered_window="true" ...>
    <Include src="status_bar.xml"/>
</Window>
```

**仍然推荐的保守写法**：用 Box 包裹一层。理由不是"避免崩溃"，而是便于后续在同一窗口里叠加其他控件、
以及让共享片段的根节点和宿主结构保持一致。

```xml
<Window size="900,32" layered_window="true" ...>
    <HBox>
        <Include src="status_bar.xml"/>
    </HBox>
</Window>
```

**其余两条 `<Include>` 约束依然有效**
- 被包含 XML 建议用单根 `<Window>` 包裹（符合 pugixml 单根要求）；`<Window>` 上的窗口属性会被
  `IsWindowAttributesApplied()` guard 忽略（`WindowBuilder.cpp:333-334`），**不要在被包含的 `<Window>`
  上写 size/caption/layered_window 等属性**，写了也不会生效，只会误导
- `FindControl` 按 Window 独立寻址，两个不同 Window 用相同 `name` 不会冲突——这是跨窗口复用共享片段的基础

**完整示例**

```xml
<!-- status_bar.xml：共享片段 -->
<?xml version="1.0" encoding="UTF-8"?>
<Window>
    <HBox name="status_bar_root" height="32" bkcolor="bg_statusbar">
        <Label name="status_state" text="就绪" />
        <!-- ... -->
    </HBox>
</Window>

<!-- host_a.xml：主窗口（Include 嵌在 VBox 里） -->
<Window ...>
    <VBox>
        <!-- ...主内容... -->
        <Include src="status_bar.xml"/>
    </VBox>
</Window>

<!-- host_b.xml：HUD 小窗口（推荐包一层 Box） -->
<Window size="900,32" layered_window="true" ...>
    <HBox>
        <Include src="status_bar.xml"/>
    </HBox>
</Window>
```

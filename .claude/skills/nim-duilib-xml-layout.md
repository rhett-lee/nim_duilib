---
name: nim-xml-layout
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
- 窗口标题栏按钮的 name 必须是: `minbtn`, `maxbtn`, `restorebtn`, `closebtn`, `fullscreenbtn`
- 颜色值格式: "#AARRGGBB"(ARGB) 或颜色名

# nim_duilib 主题颜色配置 XML 规范 (v2.0)

## 1. 文件结构概览

```xml
<Global>
    <!-- 主题元数据：定义主题的基本属性 -->
    <Theme 
        name="Light"            <!-- 主题名称 -->
        type="Color"            <!-- 主题类型，固定为 Color -->
        style="Light"           <!-- 主题风格 -->
        accent_support="true"   <!-- 是否支持强调色替换 -->
        version="2.0"           <!-- 格式版本号 -->
    />
    
    <!-- 主题元数据扩展（可选）：存储额外的主题配置信息 -->
    <ThemeMeta>
        <Property name="author" value="nim_duilib"/>
        <Property name="contrast_ratio" value="7.2"/>   <!-- 对比度，用于 accessibility 检查 -->
        <Property name="accent_color" value="#FF0078D4"/> <!-- 主题默认强调色 -->
        <Property name="base_luminance" value="light"/>    <!-- 基础亮度：light/dark -->
    </ThemeMeta>
    
    <!-- 颜色定义区：所有的颜色配置都在这里 -->
    <ThemeColor 
        name="color_blue"        <!-- 颜色唯一标识符，全局唯一，程序中通过此名称引用颜色 -->
        value="#FF0078D4"        <!-- 颜色实际值，格式为 #AARRGGBB，Alpha 通道在前 -->
        type="common"            <!-- 控件类型分组，按所属功能模块划分，便于按组件分类管理 -->
        category="basic_color"   <!-- 颜色用途分类，按视觉作用划分，便于按功能批量筛选 -->
        role="primary"           <!-- 颜色角色：用于强调色派生 -->
        derived_from=""          <!-- 派生来源：从哪个颜色派生而来 -->
        adjust=""                <!-- 调整公式：派生时的颜色调整规则 -->
        fixed="false"            <!-- 是否固定：强调色替换时是否保持不变 -->
        comment_cn="核心强调色"         <!-- 中文注释说明 -->
        comment_en="Core accent color"  <!-- 英文注释说明 -->
    />
</Global>
```

---

## 2. 命名规范 (Naming Convention)

### 2.1 全局命名规则

**所有属性名统一采用蛇形命名法 (snake_case)**，即全小写字母 + 下划线分隔。

| 规则 | 示例 | 反例 |
|-----|------|------|
| 全小写字母 | `derived_from`, `accent_support` | `derivedFrom`, `accentSupport` |
| 下划线分隔单词 | `contrast_ratio`, `base_luminance` | `contrastRatio`, `baseLuminance` |
| 简洁明了 | `fixed`, `adjust`, `role` | `isFixed`, `adjustFormula`, `colorRole` |

### 2.2 颜色名称命名规则

**统一格式**：`{用途前缀}_{组件名}[_{子组件}][_{状态}]`

| 部分 | 规则 | 说明 | 示例 |
|-----|------|------|------|
| **用途前缀** | `bg_` | 背景色 (Background) | `bg_window`, `bg_btn_normal` |
| | `border_` | 边框色 (Border) | `border_btn_normal` |
| | `text_` | 文本/前景色 (Text/Foreground) | `text_default`, `text_btn_disabled` |
| | `color_` | 基础颜色/调色板色 | `color_blue`, `color_gray` |
| **状态后缀** | `_normal` | 默认/常态 | `border_btn_normal` |
| | `_hover` | 鼠标悬浮状态 | `bg_btn_hover` |
| | `_pressed` | 鼠标点击/按下状态 | `bg_btn_pressed` |
| | `_disabled` | 禁用状态 | `text_btn_disabled` |
| | `_focus` | 键盘聚焦状态 | `border_control_focus` |
| | `_selected` | 选中状态 | `bg_tab_ctrl_item_selected` |

### 2.3 命名一致性验证 (100% 一致)

| 检查项 | 状态 | 验证结果 | 说明 |
|--------|------|---------|------|
| 用途前缀 | ✅ 100% 一致 | 4种前缀：`bg_`(62) / `border_`(24) / `color_`(19) / `text_`(14) | 总计 119 个颜色定义 |
| 状态词位置 | ✅ 100% 一致 | 所有状态词都在名称末尾 | 如 `_hover`, `_pressed`, `_disabled` 等 |
| 命名模式 | ✅ 100% 一致 | `{用途}_{组件}[_{子组件}]_{状态}` | 可通过 `*_hover`, `*btn*` 等模式精确搜索 |
| 大小写 | ✅ 100% 一致 | 全小写 + 下划线分隔 | 蛇形命名法 (snake_case) |
| 属性名命名 | ✅ 100% 一致 | 所有属性均采用 snake_case | `fixed`, `derived_from`, `accent_support` |

### 2.4 组件分类统计

| 组件类型 | 颜色数量 | 包含颜色 |
|---------|---------|---------|
| **基础色** | 19 | `color_*` |
| **按钮 (btn)** | 16 | 普通按钮 + 主按钮 + 窗口按钮 |
| **文本 (text)** | 14 | 通用文本 + 链接 + 按钮文字 |
| **边框 (border)** | 24 | 各种控件边框 |
| **背景 (bg)** | 62 | 各种控件背景 |
| **菜单 (menu)** | 7 | 菜单栏 + 菜单项 |
| **列表 (list)** | 13 | ListCtrl 控件 |
| **标签页 (tab)** | 7 | TabCtrl 控件 |
| **富编辑 (richedit)** | 11 | RichEdit 控件 |
| **组合框 (combo)** | 5 | Combo 控件 |
| **属性框 (property)** | 10 | PropertyGrid 控件 |
| **其他** | 42 | 窗口、标题栏、分割条等 |

---

## 3. 属性完整定义 (Attributes Reference)

### 3.1 Theme 节点属性

| 属性名 | 命名风格 | 类型 | 必填 | 说明 | 示例值 |
|--------|---------|------|------|------|--------|
| `name` | snake_case | string | ✅ | 主题名称 | `Light`, `Dark` |
| `type` | snake_case | string | ✅ | 主题类型 | 固定为 `Color` |
| `style` | snake_case | string | ✅ | 主题风格 | `Light`, `Dark`, `HighContrastBlack` |
| `version` | snake_case | string | ✅ | 格式版本号 | `2.0` |
| `accent_support` | snake_case | boolean | - | 是否支持强调色替换 | `true` / `false` |

### 3.2 ThemeMeta 节点属性

| 属性名 (name) | 命名风格 | 类型 | 说明 | 示例值 |
|---------------|---------|------|------|--------|
| `author` | snake_case | string | 主题作者/来源 | `nim_duilib` |
| `contrast_ratio` | snake_case | float | **对比度**：主题默认的 WCAG 对比度值，用于无障碍访问检查 | `7.2`, `4.5` |
| `accent_color` | snake_case | ARGB | **默认强调色**：主题出厂的主色调锚点 | `#FF0078D4` |
| `base_luminance` | snake_case | enum | **基础亮度**：主题的整体明暗基调 | `light`, `dark` |

### 3.3 ThemeColor 节点属性（核心）

| 属性名 | 命名风格 | 类型 | 必填 | 说明 | 示例值 |
|--------|---------|------|------|------|--------|
| `name` | snake_case | string | ✅ | **颜色唯一标识符**，全局唯一，程序中通过此名称引用颜色 | `color_blue`, `bg_btn_normal` |
| `value` | - | ARGB | ✅ | **颜色实际值**，格式为 `#AARRGGBB`，Alpha 通道在前 | `#FF0078D4` |
| `type` | snake_case | string | ✅ | **控件类型分组**，按所属功能模块划分，便于按组件分类管理 | `common`, `button`, `menu`, `window` |
| `category` | snake_case | string | ✅ | **颜色用途分类**，按视觉作用划分，便于按功能批量筛选 | `basic_color`, `bg_color`, `text_color`, `border_color` |
| `role` | snake_case | enum | ⭐ | **颜色角色**，在强调色派生系统中的定位 | `primary`, `neutral` |
| `derived_from` | snake_case | string | ⭐ | **派生来源**，引用另一个颜色的 `name`，表示此颜色派生自该颜色 | `color_blue` |
| `adjust` | formula | string | ⭐ | **派生调整公式**，派生时的颜色变换规则 | `lightness:+15` |
| `fixed` | snake_case | boolean | ⭐ | **是否固定不变**：<br>`true` = 强调色替换时**保持不变**<br>`false` = 强调色替换时**需要重新计算** | `true` / `false` |
| `comment_cn` | - | string | - | **中文注释说明** | "核心强调色" |
| `comment_en` | - | string | - | **英文注释说明** | "Core accent color" |

> **重要说明**：`fixed` 是唯一判断标准 - 只有 `fixed="false"` 的颜色会参与强调色派生链的计算。

### 3.4 type / category 属性取值说明

#### `type` 属性（按控件类型分组）

| 取值 | 说明 |
|------|------|
| `common` | 通用颜色，不属于特定控件的基础颜色 |
| `text` | 文本颜色 |
| `window` | 窗口相关颜色 |
| `button` | 按钮颜色 |
| `menu` | 菜单颜色 |
| `menu_bar` | 菜单栏颜色 |
| `list_ctrl` | 列表控件颜色 |
| `combo` | 组合框颜色 |
| `tab_ctrl` | 标签控件颜色 |
| `richedit` | 富编辑框颜色 |
| `property_grid` | 属性网格颜色 |
| `address_bar` | 地址栏颜色 |
| `scroll_bar` | 滚动条颜色 |
| ... | 其他控件类型标识 |

#### `category` 属性（按用途分类）

| 取值 | 说明 |
|------|------|
| `basic_color` | 基础颜色：如 `color_blue`, `color_red`, `color_gray` 等调色板色 |
| `bg_color` | 背景颜色：所有控件/窗口的背景色 |
| `text_color` | 文本颜色：所有前景文字颜色 |
| `border_color` | 边框颜色：所有控件边框、分割线 |

### 3.5 `role` 属性可选值与语义

| role 值 | 语义说明 | 强调色替换行为 |
|---------|---------|-------------|
| `primary` | **主色调锚点** | ✅ 被新强调色完全替换（派生链的根节点） |
| `primary_light` | **主色调亮变体** | ✅ 根据派生关系从新主色重新计算 |
| `primary_dark` | **主色调暗变体** | ✅ 根据派生关系从新主色重新计算 |
| `semantic_success` | **成功语义色** | ❌ 保持不变（语义固定：绿色系） |
| `semantic_success_light` | **成功语义色（亮）** | ❌ 保持不变 |
| `semantic_error` | **错误语义色** | ❌ 保持不变（语义固定：红色系） |
| `semantic_error_light` | **错误语义色（亮）** | ❌ 保持不变 |
| `semantic_error_dark` | **错误语义色（暗）** | ❌ 保持不变 |
| `semantic_warning` | **警告语义色** | ❌ 保持不变（语义固定：黄色系） |
| `neutral` | **中性灰度/固定色** | ❌ 保持不变（灰色、白色、黑色、静态背景） |

### 3.6 `adjust` 颜色调整公式语法

```ebnf
调整公式:  {属性}:{符号}{数值} [, {属性}:{符号}{数值}]*

属性:      lightness | saturation | chroma | luminance | alpha
符号:      + | -
数值:      整数，表示调整幅度（0-100 范围）
```

支持的颜色调整属性：

| 属性 | 颜色空间 | 说明 | 示例 |
|-----|---------|------|------|
| `lightness` | HSL | 调整**明度**，数值越大越亮 | `lightness:+15` = 明度增加 15 |
| `saturation` | HSL | 调整**饱和度**，数值越大越鲜艳 | `saturation:-10` = 饱和度降低 10 |
| `chroma` | HCL | 调整**彩度**（感知均匀的饱和度） | `chroma:+20` = 彩度增加 20 |
| `luminance` | CIE | 调整**感知明度**（人眼感知均匀） | `luminance:-5` = 感知明度降低 5 |
| `alpha` | ARGB | 调整**透明度**（0-255） | `alpha:128` = 设置为半透明 |

**公式示例**：
```xml
<!-- 单个属性调整 -->
adjust="lightness:+15"

<!-- 多个属性组合调整 -->
adjust="lightness:-12,saturation:+5"

<!-- 深色主题按钮悬浮态典型调整 -->
adjust="lightness:+20,saturation:-15"
```

---

## 4. 强调色派生系统设计 (Accent Color Derivation)

### 4.1 派生链执行规则

```
当用户设置新强调色 new_accent = #RGB:

1. 遍历所有 ThemeColor，找到 role="primary" 的锚点颜色
   → 将其 value 设为 new_accent

2. 遍历所有 derived_from="锚点名称" 的颜色
   → 根据 adjust 公式计算新的颜色值

3. 递归处理所有派生自这些颜色的次级派生色

4. fixed="true" 的颜色完全不参与派生
```

### 4.2 完整派生链示例

```xml
<!-- ==============================================
     第一层级：主色锚点 (Primary Anchor)
     ============================================== -->
<ThemeColor 
    name="color_blue" 
    value="#FF0078D4" 
    role="primary"
    fixed="false"
    comment_cn="核心强调色锚点 - 被用户自定义颜色直接替换"
    comment_en="Primary color anchor - directly replaced by user color"
/>

<!-- ==============================================
     第二层级：主色变体 (Primary Variants)
     ============================================== -->
<ThemeColor 
    name="color_blue_light" 
    value="#FF3D9EFF" 
    role="primary_light"
    derived_from="color_blue"
    adjust="lightness:+15"
    fixed="false"
    comment_cn="强调色亮变体 - 用于悬浮态(hover)"
    comment_en="Light primary variant - for hover states"
/>
<ThemeColor 
    name="color_blue_deep" 
    value="#FF005A9E" 
    role="primary_dark"
    derived_from="color_blue"
    adjust="lightness:-15"
    fixed="false"
    comment_cn="强调色暗变体 - 用于点击态(pressed)"
    comment_en="Dark primary variant - for pressed states"
/>

<!-- ==============================================
     第三层级：UI组件引用 (UI Component References)
     ============================================== -->
<ThemeColor 
    name="bg_primary_btn_normal" 
    value="#FF0078D4" 
    derived_from="color_blue"
    fixed="false"
    comment_cn="主要按钮默认态背景"
    comment_en="Primary button normal state background"
/>
<ThemeColor 
    name="bg_primary_btn_hover" 
    value="#FF3D9EFF" 
    derived_from="color_blue_light"
    fixed="false"
    comment_cn="主要按钮悬浮态背景"
    comment_en="Primary button hover state background"
/>
<ThemeColor 
    name="text_link_normal" 
    value="#FF0078D4" 
    derived_from="color_blue"
    fixed="false"
    comment_cn="超链接默认颜色"
    comment_en="Hyperlink normal state color"
/>

<!-- ==============================================
     固定颜色：不参与派生 (Fixed Colors)
     ============================================== -->
<ThemeColor 
    name="color_red" 
    value="#FFDC2626" 
    role="semantic_error"
    fixed="true"
    comment_cn="错误提示色 - 强调色替换时保持不变"
    comment_en="Error semantic color - fixed during accent replacement"
/>
<ThemeColor 
    name="bg_window" 
    value="#FFFFFFFF" 
    role="neutral"
    fixed="true"
    comment_cn="窗口背景 - 固定不变"
    comment_en="Window background - fixed"
/>
```

### 4.3 派生算法伪代码

```cpp
// C++ 版本派生算法实现示意
void applyCustomAccentColor(Color32 newAccentColor) {
    // 1. 更新主锚点
    auto* primary = findColorByRole("primary");
    primary->value = newAccentColor;
    
    // 2. 拓扑排序：按派生依赖顺序处理
    auto sortedColors = topologicalSortByDerivedFrom();
    
    // 3. 按顺序重新计算颜色
    for (auto* color : sortedColors) {
        if (color->fixed) continue;
        
        if (!color->derived_from.empty()) {
            auto* source = findColorByName(color->derived_from);
            color->value = applyAdjustFormula(source->value, color->adjust);
        }
    }
    
    // 4. 所有 fixed="false" 但未设置 derived_from 的颜色，
    //    根据状态(_hover/_pressed)自动推导派生关系
    applyImplicitDerivations();
}
```

---

## 5. 主题类型定义

| `style` 属性值 | 主题类型 | `accent_support` | 说明 |
|---------------|---------|------------------|------|
| `Light` | 浅色主题 | `true` | 默认亮色主题（Win11 风格） |
| `Dark` | 深色主题 | `true` | 暗色主题 |
| `HighContrastBlack` | 高对比度黑 | `false` | 辅助功能主题，配色固定 |
| `HighContrastWhite` | 高对比度白 | `false` | 辅助功能主题，配色固定 |
| `Custom` | 自定义主题 | `true` | 用户完全自定义的主题 |

---

## 6. 主题的加载与使用

### 6.1 主题文件的存放位置

主题文件（`*.xml`）应放在资源根目录的 `themes/<theme_name>/` 子目录下，例如：

```
bin/resources/themes/
├── default/
│   ├── global.xml             # 全局资源（字体、颜色定义）
│   ├── light.xml              # 浅色主题（Theme 节点 + ThemeColor 节点集合）
│   ├── dark.xml               # 深色主题
│   ├── public/                # 共享图片资源
│   └── <your_app>/            # 应用专属 XML 与图片
```

### 6.2 加载流程

1. 应用程序调用 `ui::GlobalManager::Instance().Startup(...)` 启动全局资源管理器；
2. 框架按主题文件中的 `Theme` 节点属性加载对应主题；
3. 控件通过 `color="<ThemeColor.name>"` 或 `bkcolor="<ThemeColor.name>"` 引用颜色。

### 6.3 主题切换

可通过 `ui::ThemeManager` 在运行时切换主题。切换后，所有未设置 `fixed="true"` 的派生颜色会按强调色派生链重新计算。

### 6.4 主题扩展方式

如需新增自定义主题：
1. 复制现有主题文件（如 `light.xml`）为基础；
2. 修改 `Theme` 节点的 `name`、`style` 等属性；
3. 调整 `ThemeColor` 的 `value`，或调整 `derived_from`/`adjust` 派生关系；
4. 在 `global.xml` 中将新主题的 `Class` 样式指向新主题文件。

---

## 7. 现有颜色覆盖分析与补充建议

### 7.1 已覆盖的颜色分类 ✅

| 分类 | 已覆盖颜色示例 | 数量 | 完整度 |
|-----|--------------|------|-------|
| **基础色彩** | color_white, color_black, color_gray* | 19 | ✅ 完整 |
| **窗口系统** | bg_window_dark, bg_window_light, bg_titlebar, border_window | 4 | ✅ 完整 |
| **文本系统** | text_default, text_disabled, text_link_*, text_menu_bar | 14 | ✅ 完整 |
| **按钮控件** | bg_btn_*, border_btn_*, text_btn_*（普通+主要按钮） | 16 | ✅ 完整 |
| **菜单系统** | bg_menu, bg_menu_item_*, bg_menu_bar_* | 7 | ✅ 完整 |
| **标签页** | bg_tab_ctrl, bg_tab_ctrl_item_* | 7 | ✅ 完整 |
| **列表控件** | bg_list_ctrl_*, bg_list_item_* | 13 | ✅ 完整 |
| **组合框** | bg_combo, bg_combo_btn_*, border_combo_* | 5 | ✅ 完整 |
| **富编辑框** | bg_richedit, border_richedit_*, bg_richedit_btn_* | 11 | ✅ 完整 |
| **属性网格** | bg_property_grid_*, text_property_grid_* | 10 | ✅ 完整 |
| **地址栏** | bg_address_bar, bg_address_bar_btn_* | 3 | ✅ 完整 |
| **语义颜色** | color_red*, color_green*（错误/成功状态） | 5 | ✅ 完整 |
| **分割线** | color_splitline_* | 3 | ✅ 完整 |
| **进度条** | color_progress | 1 | ✅ 存在 |

### 7.2 建议补充的颜色定义 ⭐

以下颜色在现有 119 个定义中**缺失**，建议补充以增强主题的完整性和灵活性：

| 缺失颜色名称 | 建议值（浅色/深色） | 用途说明 | 重要性 |
|-------------|-------------------|---------|-------|
| `bg_container` | `#FFF8F8F8` / `#FF202020` | **通用容器背景** - 对话框、面板、分组框的默认背景 | 🔴 高 |
| `bg_content` | `#FFFFFFFF` / `#FF1A1A1A` | **内容区域背景** - 可滚动内容区域的基础背景 | 🔴 高 |
| `bg_header` | `#FFF0F0F0` / `#FF2D2D2D` | **页眉背景** - 列表/树/表格的表头背景（当前复用 bg_titlebar） | 🟡 中 |
| `bg_overlay` | `#80000000` / `#80000000` | **蒙层背景** - 弹窗/抽屉的黑色半透明蒙层（Alpha=128） | 🟡 中 |
| `bg_tooltip` | `#FFFFFFE6` / `#FF2B2B2B` | **工具提示背景** - Tooltip 控件背景 | 🟡 中 |
| `bg_selected_inactive` | `#FFE8E8E8` / `#FF3D3D3D` | **非活动选中态背景** - 失去焦点时的选中项背景 | 🟡 中 |
| `border_focus_ring` | `#4D0078D4` / `#4D3B82F6` | **焦点环/光晕** - 高对比度焦点指示器（半透明） | 🟡 中 |
| `shadow_control` | ~~不适用~~ | 控件阴影（非颜色值，需 DPI 感知） | ⚪ 低 |
| `text_selected` | `#FFFFFFFF` / `#FFFFFFFF` | **选中文本颜色** - 文本选中时的前景色 | 🟡 中 |
| `bg_text_highlight` | `#FFFFFF00` / `#FFE5A000` | **文本高亮背景** - 搜索/查找结果的高亮标记 | ⚪ 低 |
| `bg_scrollbar` | `#FFE0E0E0` / `#FF434343` | **滚动条背景** | 🟡 中 |
| `bg_scrollbar_thumb` | `#FFB0B0B0` / `#FF6A6A6A` | **滚动条滑块背景** | 🟡 中 |
| `bg_scrollbar_thumb_hover` | `#FF808080` / `#FF858585` | **滚动条滑块悬浮态** | 🟡 中 |

**补充后总颜色数**：119 + 13 = **132 个**

### 7.3 缺失颜色的定义优先级说明

| 优先级 | 颜色 | 理由 |
|-------|------|------|
| 🔴 高 | bg_container, bg_content | 多数控件的基础背景，当前依赖于 bg_window_dark/light 但语义不明确 |
| 🟡 中 | bg_header, bg_tooltip, scrollbar* | 提升主题完整性，当前部分控件复用相近颜色 |
| ⚪ 低 | shadow, highlight | 特定场景使用，部分由系统渲染 |

---

## 8. 版本兼容性保证

| 兼容性层级 | 承诺内容 | 保证版本 |
|-----------|---------|---------|
| **格式前向兼容** | v2.0 XML 格式可被 v1.x 版本库正确读取 | 永久 |
| | 新增属性会被旧版本 parser 安全忽略 | - |
| **名称后向兼容** | 所有 v1.x 存在的 `name` 在 v2.0 中继续存在 | 至少 3 个主版本 |
| | 新增语义化名称采用"别名模式"，不删除原有名称 | - |
| **值兼容** | 默认主题的所有颜色 `value` 与 v1.7 版本**完全一致** | v2.0.x |
| **运行时兼容** | 默认行为与 v1.7 版本完全一致 | v2.0.x |
| | 强调色派生功能需要**显式启用** | - |
| **API 兼容** | `GetColor(LPCTSTR name)` 等获取颜色的 API 签名不变 | 永久 |

---

## 9. 验证与工具

### 9.1 规范符合性检查清单

验证项 | XPath 表达式 | 预期结果 |
-------|-------------|---------|
| 所有颜色有唯一 name | `count(//ThemeColor[@name])` | = count(//ThemeColor) |
| | `count(//ThemeColor[preceding::ThemeColor/@name = @name])` | = 0 |
| 命名格式正确 | `//ThemeColor[not(matches(@name, '^[a-z]+_[a-z0-9_]+$'))]` | 空节点集 |
| 所有派生源存在 | `//ThemeColor[@derived_from and not(//ThemeColor/@name = @derived_from)]` | 空节点集 |
| adjust 格式正确 | `//ThemeColor[@adjust and not(matches(@adjust, '^[a-z]+:[+-]?[0-9]+(,[a-z]+:[+-]?[0-9]+)*$'))]` | 空节点集 |
| fixed 是布尔值 | `//ThemeColor[@fixed[not(. = 'true' or . = 'false')]]` | 空节点集 |
| role 在枚举范围内 | `//ThemeColor[@role[not(. = ('primary', 'primary_light', 'primary_dark', 'neutral', 'semantic_success', 'semantic_error', 'semantic_warning'))]]` | 空节点集 |
| 值是 ARGB 格式 | `//ThemeColor[not(matches(@value, '^#[0-9A-Fa-f]{8}$'))]` | 空节点集 |
| 无旧属性残留 | `//ThemeColor[@support_accent]` | 空节点集 |
| 属性命名统一 | `//ThemeColor[@*[starts-with(name(), 'd') and not(starts-with(name(), 'derived_from'))]]` | 空节点集 |

### 9.2 推荐验证工具

| 工具 | 用途 |
|-----|-----|
| `validate_theme.py` | Python 脚本：完整的 XML 验证 + 派生链计算 |
| 主题编辑器 HTML | 可视化工具：预览强调色替换效果 + 实时编辑 |
| `xmllint` / `XML Validator` | 通用 XML 语法验证工具 |
| Visual Studio | 配合 XSD Schema 实现编辑时智能提示 |



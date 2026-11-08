# Combo 下拉弹窗四角双线问题修复

## Context

Combo 下拉弹窗当前存在两条边框线在四角重叠：

1. 弹窗外层：窗口阴影类型为 `kShadowMenu`（`Shadow.cpp` 的 `GetShadowParam`），其 `szBorderRound = (0,0)`，因此 `ShadowBox::PaintBkImage` 在内容区矩形上用 `DrawRect` 画了 1px **直角**边框；同时 `shadow_menu.svg` 的主体矩形也是直角填充。
2. 弹窗内层：`combo_tree_view_class` 给铺满窗口的 TreeView 设置了 `border_size='1' border_round='6,6'`，即 1px **6px 圆角**边框。

两条线在四角处叠加，形成用户看到的"直角有边框 + 内部圆角边框"的丑陋效果。

目标：只保留**一层圆角边框**，四角干净。采用框架内菜单弹窗的既有范式：`shadow_type="menu_round"`（`shadow_menu_round.svg` 为 6px 圆角主体，`ShadowBox` 用 `DrawRoundRect` 画圆角边框，并对客户区做圆角剪切），TreeView 自身不再画边框/圆角。

## 修改内容

### 1. `bin/resources/themes/default/global.xml`（约 349-353 行）

`combo_base` 的 `combo_tree_view_class` 去掉自身边框与圆角，保留内边距和底色：

```xml
combo_tree_view_class="class='tree_view' padding='4,4,4,4' border_size='0,0,0,0' bkcolor='bg_menu' indent='20'"
```

说明：TreeView 仍用 `bg_menu` 铺满内容区，四角由窗口阴影的圆角剪切负责；`padding` 保留，避免列表项贴到圆角。

### 2. `duilib/Control/Combo.cpp`

- `Combo::Combo` 构造函数（约 329 行）：下拉弹窗默认阴影类型由 `kShadowMenu` 改为 `kShadowMenuRound`：
  ```cpp
  m_nShadowType(ShadowType::kShadowMenuRound)
  ```
- `CComboWnd::OnInitWindow`（约 251-265 行）：设置阴影类型后，把阴影边框统一为菜单的 1px 样式（`Shadow` 默认是 2px）：
  ```cpp
  SetShadowType(m_pOwner->GetComboWndShadowType());
  SetShadowBorderSize(1);
  SetShadowBorderColor(_T("border_window"));
  ```
  （`Window.h` 已提供这两个接口，颜色 token `border_window` 已有定义。）

主题 XML 中仍可用 `shadow_type` 属性覆盖默认值（`Combo::SetAttribute` 已支持）。

## 验证

1. 构建 Debug x64（SDL）：
   `.\build.bat`（或仓库既有构建脚本，仅 Debug x64）。
2. 运行 `bin` 下的示例程序，打开 Combo 下拉框（示例程序里字号下拉、`controls_d` 中的 combo 均可）。
3. 截图放大四角，确认：
   - 四角只有一层 6px 圆角的 1px 边框；
   - 悬停/选中条目不超出圆角；
   - 弹窗阴影正常，无黑边/白角。
4. 切换深色主题（color_dark）重复检查一次。
5. 回归：下拉列表可点击选中、可滚动、ESC/失焦可关闭。

## 回退方案（若 `menu_round` 在 SDL 下仍崩溃）

改用无阴影的圆角模式 `ShadowType::kShadowNoneRound`：`Shadow` 不绘制阴影图片，仅在内容区用 `DrawRoundRect` 画 1px 圆角边框并对客户区做 6px 圆角剪切，TreeView 保持无边框，同样可得到单层圆角边框（代价是没有外阴影）。

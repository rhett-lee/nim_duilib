# TreeView 图标系统化优化 Spec

## 1. 目标

对 `examples/TreeView` 程序自有（非公共主题）的 SVG 图标做系统化重绘，统一视觉语言并提升美观度；同时保持：

- 所有文件名不变，XML/代码中的引用路径不变；
- 主题颜色替换机制（`svg_replace_colors`）不变，浅色/深色主题都能正常显示；
- 控件尺寸、布局、DPI 缩放机制不变；
- 不改变任何交互行为。

## 2. 现状问题

| 图标 | 现状问题 |
| --- | --- |
| `tree_view/arrow-up.svg` / `arrow-left.svg` / `arrow-right.svg` | 48×48 画布、`stroke-width=4`，实际渲染 16×16；几何与视重和其他图标不一致 |
| `tree_view/refresh.svg` | Material 风格**填充**图标（24 网格），与其余描边图标风格割裂 |
| `tree_view/chevron-bottom.svg` | 32×32 画布、`stroke-width=2`，渲染 8×8 时线宽仅 0.5px，偏细且边缘模糊 |
| `tree_view/view-sort.svg` | 6.35 画布、路径坐标错位，线条与箭头比例失衡 |
| `tree_view/view-list-symbolic.svg` | 1px 细线 + 1×1 小方块，过细、对比不足 |
| `tree_view/menu/data_icons_display-symbolic.svg` | 形似“显示器”，与“大图标视图”语义不符 |
| `tree_view/menu/view-list-compact-symbolic.svg` | 4 条等宽横线，与列表图标区分度低 |
| `tree_view/menu/view-list-images-symbolic.svg` | 多层重叠边框 + 山形，缩小后噪点多 |
| `tree_view/menu/ui-item-symbolic.svg` | 空心圆环，不表达“选中/勾选”语义 |
| 全部 `tree_view/menu/*.svg` | 颜色写死 `#5d656b`，深色主题下几乎不可见 |

## 3. 统一视觉规范

- **画布**：界面/菜单图标统一 `16×16` viewBox（`image-photo.svg` 除外，见第 7 节）。
- **描边**：`1.5px ~ 1.6px`，`stroke-linecap="round"`、`stroke-linejoin="round"`，`fill="none"`；仅“圆点/实心勾”允许填充。
- **留白**：内容距边 2px 左右，保证 16px 下不贴边。
- **颜色令牌（保持不变，由 XML 替换）**：
  - 工具栏图标：`#1E1E1E` → `border_svg_image`
  - 工具栏菜单图标与私有菜单图标：`#5d656b` → `border_svg_image`
  - 菜单“当前选中”勾选：`#0078D4` → `color_accent`（主题强调色）
- **语义**：
  - 大图标视图 = 2×2 磁贴
  - 列表视图 = 圆点 + 横线
  - 详细信息视图 = 带表头的表格
  - 图片视图 = 相框 + 圆点 + 山形
  - 排序 = 三条递减横条
  - 当前选中 = 勾选

> 渲染器为 `nanosvg`（`duilib/third_party/svg`），已确认支持 `A` 圆弧命令、`rect` 的 `rx`、`circle` 与 `stroke-linecap/linejoin`，下述写法均可解析。

## 4. 改动清单（12 个 SVG）

### 4.1 界面图标（`bin/resources/themes/default/tree_view/`）

#### `arrow-up.svg`
```svg
<svg width="16" height="16" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg"
     fill="none" stroke="#1E1E1E" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round">
  <path d="M8 13V3.5"/>
  <path d="M3.5 8L8 3.5L12.5 8"/>
</svg>
```

#### `arrow-left.svg`
```svg
<svg width="16" height="16" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg"
     fill="none" stroke="#1E1E1E" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round">
  <path d="M13 8H3.5"/>
  <path d="M8 3.5L3.5 8L8 12.5"/>
</svg>
```

#### `arrow-right.svg`
```svg
<svg width="16" height="16" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg"
     fill="none" stroke="#1E1E1E" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round">
  <path d="M3 8H12.5"/>
  <path d="M8 3.5L12.5 8L8 12.5"/>
</svg>
```

#### `refresh.svg`
```svg
<svg width="16" height="16" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg"
     fill="none" stroke="#1E1E1E" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">
  <path d="M2.6 8a5.4 5.4 0 0 1 5.4-5.4 5.85 5.85 0 0 1 4.044 1.644L13.4 5.6"/>
  <path d="M13.4 2.6v3h-3"/>
  <path d="M13.4 8a5.4 5.4 0 0 1-5.4 5.4 5.85 5.85 0 0 1-4.044-1.644L2.6 10.4"/>
  <path d="M5.6 10.4h-3v3"/>
</svg>
```

#### `chevron-bottom.svg`
```svg
<svg width="16" height="16" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg"
     fill="none" stroke="#1E1E1E" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
  <path d="M3.5 5.5L8 10L12.5 5.5"/>
</svg>
```

#### `view-sort.svg`
```svg
<svg width="16" height="16" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg"
     fill="none" stroke="#5d656b" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">
  <path d="M2.5 4.5H13.5"/>
  <path d="M2.5 8H10.5"/>
  <path d="M2.5 11.5H7.5"/>
</svg>
```

#### `view-list-symbolic.svg`
```svg
<svg width="16" height="16" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg"
     fill="none" stroke="#5d656b" stroke-width="1.4" stroke-linecap="round" stroke-linejoin="round">
  <circle cx="3" cy="4.5" r="1.1" fill="#5d656b" stroke="none"/>
  <path d="M6 4.5H13.5"/>
  <circle cx="3" cy="8" r="1.1" fill="#5d656b" stroke="none"/>
  <path d="M6 8H13.5"/>
  <circle cx="3" cy="11.5" r="1.1" fill="#5d656b" stroke="none"/>
  <path d="M6 11.5H13.5"/>
</svg>
```

### 4.2 私有菜单图标（`bin/resources/themes/default/tree_view/menu/`）

#### `data_icons_display-symbolic.svg`
```svg
<svg width="16" height="16" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg"
     fill="none" stroke="#5d656b" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">
  <rect x="2.5" y="2.5" width="4.5" height="4.5" rx="1"/>
  <rect x="9" y="2.5" width="4.5" height="4.5" rx="1"/>
  <rect x="2.5" y="9" width="4.5" height="4.5" rx="1"/>
  <rect x="9" y="9" width="4.5" height="4.5" rx="1"/>
</svg>
```

#### `view-list-symbolic.svg`
与 4.1 中 `view-list-symbolic.svg` 内容完全一致（同一设计，两处使用）。

#### `view-list-compact-symbolic.svg`
```svg
<svg width="16" height="16" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg"
     fill="none" stroke="#5d656b" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">
  <rect x="2.5" y="3" width="11" height="10" rx="1.5"/>
  <path d="M2.5 6.3H13.5"/>
  <path d="M2.5 9.7H13.5"/>
</svg>
```

#### `view-list-images-symbolic.svg`
```svg
<svg width="16" height="16" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg"
     fill="none" stroke="#5d656b" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">
  <rect x="2.5" y="3" width="11" height="10" rx="1.5"/>
  <circle cx="5.6" cy="6" r="1.2" fill="#5d656b" stroke="none"/>
  <path d="M3.5 11.2L6.8 8.4L9 10.2L10.6 8.8L12.8 10.8"/>
</svg>
```

#### `ui-item-symbolic.svg`
```svg
<svg width="16" height="16" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg"
     fill="none" stroke="#0078D4" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round">
  <path d="M3.5 8.4L6.5 11.2L12.5 5.2"/>
</svg>
```

## 5. 引用侧改动

### 5.1 `bin/resources/themes/default/tree_view/menu/list_type_menu.xml`

8 个菜单项图标由“裸文件名”改为完整图片描述，增加主题色替换，使深色主题下可见（文件名与位置不变）：

- `bkimage="data_icons_display-symbolic.svg"` → `bkimage="file='data_icons_display-symbolic.svg' width='16' height='16' svg_replace_colors='#5d656b|border_svg_image'"`
- `bkimage="view-list-symbolic.svg"` → `bkimage="file='view-list-symbolic.svg' width='16' height='16' svg_replace_colors='#5d656b|border_svg_image'"`
- `bkimage="view-list-compact-symbolic.svg"` → 同上模式
- `bkimage="view-list-images-symbolic.svg"` → 同上模式

`tree_view.xml` 中 `view-sort.svg`、`view-list-symbolic.svg`、`chevron-bottom.svg` 已带 `svg_replace_colors`，无需改动。

### 5.2 `examples/TreeView/MainForm.cpp`

`ui-item-symbolic.svg` 由代码设置（2 个函数、3 处调用），改为带强调色替换的完整描述：

- 在文件顶部匿名 `namespace {}` 中新增：
```cpp
/** 菜单中“当前选中项”的勾选图标描述（颜色替换为主题强调色）
*/
DString GetMenuCheckImage()
{
    return _T("file='ui-item-symbolic.svg' width='16' height='16' svg_replace_colors='#0078D4|color_accent'");
}
```
- `MainForm::SwitchListType` 中 `pSelectBtn->SetBkImage(_T("ui-item-symbolic.svg"));` → `pSelectBtn->SetBkImage(GetMenuCheckImage());`
- `MainForm::SwitchSortMode` 中 `pSortBtn->SetBkImage(_T("ui-item-symbolic.svg"));`、`pSortColumnBtn->SetBkImage(...)` 同样替换。

## 6. 验证步骤

1. 构建：
```powershell
Import-Module "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath "C:\Program Files\Microsoft Visual Studio\18\Community" -DevCmdArguments "-arch=amd64 -host_arch=amd64"
cmake --build out\x64\Release --target duilib TreeView
```
2. 运行 `out\x64\Release\bin\TreeView.exe`（或对应输出目录），核对：
   - 工具栏：向上/后退/前进 16px 视觉重量一致；后退/前进禁用态淡出正常；
   - 刷新图标为描边风格；
   - 排序/视图按钮中的 16px 图标与 8px 下拉箭头对齐、清晰；
   - 打开“视图类型”菜单：大/中/小图标、列表、详细信息、图片视图图标语义可辨，当前项显示强调色勾选；
   - 打开“排序方式”菜单：所选列与排序方向显示强调色勾选；
   - 切换深色主题（标题栏主题按钮）：菜单图标与工具栏图标清晰可见；
   - 切换语言/缩放窗口，确认图标不模糊、不变形。
3. 回归：目录树展开/折叠、右键菜单、地址栏、搜索、状态栏功能不受影响。

## 7. 明确不在本次范围

以下为公共主题资源（被多个示例共用），本次不改动，避免影响其它示例：

- ~~`public/filesystem/*`~~（已在后续迭代中统一重绘，见第 8 节）
- `public/address_bar/*`、`public/button/*`（窗口控制等）、`public/checkbox/*`、`public/tree/*`
- `public/listctrl/arrow-sorted-*.svg`
- `tree_view/image-photo.svg`（文件列表缩略图占位图，彩色位图式插画，保留原样）
- `tree_view/logo_18x18.png`（非 SVG）

如后续需要，可另开一份 Spec 对上述公共图标做统一改造。

## 8. 后续迭代：`public/filesystem` 统一重绘（2026-11-05）

该目录仅被 TreeView 相关控件使用（`DirectoryTreeImpl_MacOS/Linux` 与 `global.xml` 地址栏类），已全部重绘为统一的扁平彩色风格：

- **96×96 网格**：`folder.svg`、`file.svg`、`folder-home/desktop/documents/download/music/pictures/videos.svg`
  - 文件夹：两级圆角造型，背板 `#E39B2A` + 前面板 `#FFC94D`，特殊文件夹在面板上叠加 `#8A5A12` 语义徽标（房子/显示器/文档/下载箭头/音符/山形/播放三角）
  - 文件：白色页面 + 折角 + 浅灰文本线，边框 `#CFD6DF` 保证浅色背景可见
- **16×16 网格**：`computer.svg`、`drive-harddisk.svg`、`drive-harddisk-usb.svg`、`drive-cdrom.svg`
  - 统一蓝色 `#5A8FD6` 设备主体 + 灰色 `#A8B4C0` 支架/接口 + 绿色 `#45C46B` 指示灯，深色主题下同样清晰
- 这些图标为彩色自绘，不使用 `svg_replace_colors` 主题替换。

> 平台差异：Windows 下 `DirectoryTreeImpl_Win.cpp` 与 `ExplorerView` 使用系统 Shell 图标，上述 SVG 在 Windows 上主要体现在地址栏面包屑（`此电脑`/磁盘）与资源加载；Mac/Linux 的目录树、文件列表直接使用这些 SVG。

## 9. 后续迭代：`public/address_bar` 统一重绘（2026-11-05）

目录内容盘点（6 个 SVG + 1 个菜单骨架 XML）：

| 文件 | 引用位置（`global.xml`） | 渲染尺寸 | 结论 |
| --- | --- | --- | --- |
| `folder-root.svg` | `address_bar_sub_path_root` | 16×16 | 重绘（原 22×22 贝塞尔斜线，缩放后偏小、构图不符） |
| `reset.svg` | `address_bar_reset_button` | 14×14 | 重绘（原三段实心弧 + 三角形，小尺寸易失真） |
| `history.svg` | `address_bar_history_button` | 14×14 | 重绘（统一 16 网格） |
| `more-horizontal.svg` | `address_bar_sub_path_collapse` | 14×14 | 重绘（圆点统一 16 网格） |
| `chevron-right.svg` | `address_bar_path_separator` | 12×12 | 重绘（原实心粗箭头 + 无用 `<style>` 类，改描边） |
| `filename-slash.svg` | 全仓库无引用 | — | 保留不动（疑似废弃资源） |
| `address_bar_menu.xml` | 非图标，仅是 `MenuListBox` 骨架 | — | 不改 |

重绘后统一为 16×16 网格、`#5d656b` 描边/填充，文件名与引用路径不变（XML 无需改动，渲染尺寸由图片串中的 `width/height` 决定）：

```svg
<!-- chevron-right.svg：描边 chevron，1.6px 圆头 -->
<path d="M6.0 3.6 10.4 8 6.0 12.4"/>

<!-- history.svg：5.6 半径圆 + 指针 -->
<circle cx="8" cy="8" r="5.6"/>
<path d="M8 4.8V8l2.6 1.6"/>

<!-- more-horizontal.svg：三点，r=1.4 -->
<circle cx="3.6" cy="8" r="1.4"/> …

<!-- reset.svg：300° 圆弧 + 实心三角箭头（箭头指向圆弧行进方向） -->
<path d="M8.44 3.02A5 5 0 1 0 12.53 5.89"/>
<path d="M13.66 5.36 11.43 3.53 11.40 6.42Z"/>

<!-- folder-root.svg：圆角线描边文件夹 -->
<path d="M2.4 5.5A1.6 1.6 0 0 1 4.0 3.9H6.0L7.6 5.5H12.0A1.6 1.6 0 0 1 13.6 7.1V11.1
          A1.6 1.6 0 0 1 12.0 12.7H4.0A1.6 1.6 0 0 1 2.4 11.1Z"/>
```

配色仍沿用各主题类中的 `svg_replace_colors='#5d656b|border_svg_image'`（禁用态用 `border_svg_image_disabled`），深色主题自动适配。

# 窗口阴影使用指南

本文从用户使用角度介绍 duilib 中窗口阴影（Window Shadow）的功能，包括 XML 配置、C++ 代码接口、可用的阴影类型以及跨平台兼容性说明等，供开发者参考。

---

## 1. 功能简介

duilib 的窗口阴影为窗口提供"立体悬浮"的视觉外观。实现方式分为两类：

- **自绘阴影**：duilib 内部通过 SVG 阴影素材（九宫格拉伸）+ 圆角/直角组合绘制出窗口外围的阴影效果，适用于所有平台，效果一致。
- **系统阴影**：调用操作系统自身提供的窗口阴影（Windows 的 DWM、macOS 的窗口阴影等），效果与系统其他原生窗口一致。

阴影通过在 `Window` 标签上配置 `shadow_*` 系列属性来启用，并可通过 C++ 代码在运行时动态切换。

---

## 2. 快速上手

最简配置（使用默认自绘阴影，圆角大阴影）：

```xml
<Window size="900,600" caption="0,0,0,36" shadow_type="default">
    <!-- 窗口内容 -->
</Window>
```

如果想用 Win11 风格的系统阴影：

```xml
<Window size="900,600" caption="0,0,0,36" shadow_type="system_default">
    <!-- 窗口内容 -->
</Window>
```

如果想关闭阴影：

```xml
<Window size="900,600" caption="0,0,0,36" shadow_attached="false">
    <!-- 窗口内容 -->
</Window>
```

---

## 3. XML 配置属性

以下属性均配置在 `<Window>` 标签上。

| 属性 | 默认值 | 类型 | 说明 |
| :--- | :--- | :--- | :--- |
| `shadow_attached` | `true` | bool | 是否启用阴影。`true` 启用，`false` 关闭。若同时设置 `shadow_type`，则 `shadow_type` 会隐式将 `shadow_attached` 置为 `true`，一般不必再单独设置。 |
| `shadow_type` | - | string | 阴影类型，可选值见下文。 |
| `shadow_image` | - | string | 自定义阴影图片，一般仅在 `shadow_type="custom"` 时使用。例如 `file='public/shadow/shadow_big.svg' corner='64,64,68,70'`。 |
| `shadow_corner` | `"0,0,0,0"` | rect | 阴影图片的九宫格描述（左、上、右、下），一般仅在 `custom` 模式时使用。 |
| `shadow_border_round` | `"0,0"` | size | 阴影的圆角大小（宽、高），仅在 `custom` 模式时使用。 |
| `shadow_border_color` | - | string | 阴影的边框颜色（`#RRGGBB` 或主题颜色名）。深色主题时默认为 `#FF444444`，浅色主题默认为 `#FFB5B5B5`。 |
| `shadow_border_size` | `2` | int | 阴影边框的像素大小。实际显示宽度为该值的一半（如 `2` 显示 1 像素）。 |
| `shadow_snap` | `true` | bool | 窗口贴近屏幕边缘时，对应侧的阴影是否自动隐藏。**仅自绘阴影有效**。 |

> 说明：所有 `shadow_*` 属性的代码接口都在 [Window.h](../duilib/Core/Window.h) 中，方法名与属性名一一对应（驼峰式），如 `SetShadowType`、`SetShadowImage` 等。

---

## 4. 阴影类型 (`shadow_type`)

`shadow_type` 是控制阴影外观的核心属性，取值如下：

### 4.1 自绘阴影（duilib 自带，跨平台一致）

| 取值 | 描述 | 适用场景 |
| :--- | :--- | :--- |
| `default` | 等同于平台的默认自绘阴影 | 通用 |
| `big` | 大阴影，直角，有边框 | 普通窗口 |
| `big_round` | 大阴影，圆角，有边框 | 普通窗口（推荐） |
| `small` | 小阴影，直角，有边框 | 紧凑型窗口 |
| `small_round` | 小阴影，圆角，有边框 | 紧凑型窗口 |
| `menu` | 小阴影，直角，有边框 | 弹出式窗口、菜单 |
| `menu_round` | 小阴影，圆角，有边框 | 弹出式窗口、菜单（推荐） |
| `none` | 无阴影，直角，仅有边框 | 无阴影窗口 |
| `none_round` | 无阴影，圆角，仅有边框 | 无阴影窗口 |
| `custom` | 自定义自绘阴影，需配合 `shadow_image` 等使用 | 个性化阴影 |

### 4.2 系统阴影（使用操作系统原生阴影）

| 取值 | 描述 |
| :--- | :--- |
| `system_default` | 跟随操作系统默认阴影（Win11、macOS 默认圆角） |
| `system_not_round` | 系统阴影，直角 |
| `system_round` | 系统阴影，圆角 |
| `system_small_round` | 系统阴影，小圆角 |

### 4.3 平台兼容性

| 平台 | 支持的 `system_*` |
| :--- | :--- |
| Windows 7 | 仅 `system_default`（其余会自动切换） |
| Windows 10 | 仅 `system_default`（其余会自动切换） |
| Windows 11 | 全部 4 个系统阴影类型均支持 |
| macOS | 全部 4 个系统阴影类型均支持 |
| Linux / FreeBSD | 不支持系统阴影，会自动切换为等效的自绘阴影 |

> 当设置了不支持的 `system_*` 类型时，duilib 会自动选择等效的替代方案。例如在 Win10 上设置 `system_round`，会被自动替换为 `system_default`；在 Linux 上设置 `system_round` 会被替换为 `big_round`。

### 4.4 平台默认阴影

未设置 `shadow_type` 时，duilib 会按以下规则选择默认值：

| 平台 / 窗口条件 | 默认阴影类型 |
| :--- | :--- |
| Windows 10 / 11 | `system_default` |
| Windows 7 / 8 | `big_round`（自绘） |
| macOS | `system_default` |
| Linux / FreeBSD | `big_round`（自绘） |
| 分层窗口（`layered_window="true"`） | `big_round`（自绘） |

---

## 5. 自定义阴影（`shadow_type="custom"`）

如果内置阴影无法满足需求，可通过 `custom` 模式使用自己的阴影素材：

```xml
<Window shadow_type="custom"
        shadow_image="file='public/shadow/shadow_big.svg' window_shadow_mode='true' corner='64,64,68,70'"
        shadow_corner="30,30,34,36"
        shadow_border_round="6,6"
        shadow_border_color="border_window"
        shadow_border_size="1">
    <!-- 窗口内容 -->
</Window>
```

属性说明：

- `shadow_image`：必填，遵循 [图片属性语法](./XmlNode.md) 的图片描述字符串。`window_shadow_mode='true'` 标记该图为阴影专用，`corner` 是图片自身的九宫格。
- `shadow_corner`：必填，阴影素材的九宫格描述（与 `shadow_image` 的 `corner` 含义相同，且未经 DPI 缩放）。
- `shadow_border_round`：可选，阴影圆角大小（未经 DPI 缩放）。
- `shadow_border_color` / `shadow_border_size`：可选，边框样式。

> `custom` 模式下，`shadow_type` 不会再覆盖上述属性；这些属性以你设置的值为准。

---

## 6. C++ 代码接口

所有 `shadow_*` 属性都有对应的 C++ 接口，集中在 `Window` 类和 `Shadow` 类中（见 [Window.h](../duilib/Core/Window.h) 与 [Shadow.h](../duilib/Core/Shadow.h)）。

### 6.1 启用 / 关闭

```cpp
pWindow->SetShadowAttached(true);   // 启用阴影
pWindow->SetShadowAttached(false);  // 关闭阴影
bool bAttached = pWindow->IsShadowAttached();
```

### 6.2 切换阴影类型

```cpp
pWindow->SetShadowType(ui::ShadowType::kShadowBigRound);
pWindow->SetShadowType(ui::ShadowType::kShadowSystemDefault);
```

### 6.3 自定义阴影（运行时）

```cpp
pWindow->SetShadowType(ui::ShadowType::kShadowCustom);
pWindow->SetShadowImage(_T("file='public/shadow/shadow_big.svg' window_shadow_mode='true' corner='64,64,68,70'"));
pWindow->SetShadowCorner(ui::UiPadding(30, 30, 34, 36));
pWindow->SetShadowBorderRound(ui::UiSize(6, 6));
```

### 6.4 边框、贴边

```cpp
pWindow->SetShadowBorderSize(2);
pWindow->SetShadowBorderColor(_T("border_window"));
pWindow->SetEnableShadowSnap(true);
```

### 6.5 动态切换示例

```xml
<!-- 在窗口内放置一个下拉框/单选框，运行时切换阴影类型 -->
<Option group="shadow_type" selected="true" text="大阴影（圆角）">
    <Event type="select" receiver="#window#" apply_attribute="shadow_type={big_round}" />
</Option>
<Option group="shadow_type" text="系统阴影（圆角）">
    <Event type="select" receiver="#window#" apply_attribute="shadow_type={system_round}" />
</Option>
<Option group="shadow_type" text="无阴影">
    <Event type="select" receiver="#window#" apply_attribute="shadow_attached={false}" />
</Option>
```

完整示例参见 [render/page_window_shadow.xml](../../bin/resources/themes/default/render/page_window_shadow.xml)。

---

## 7. 常用窗口 XML 模板

### 7.1 普通主窗口（推荐）

```xml
<Window size="900,600" min_size="320,240"
        caption="0,0,0,36" use_system_caption="false"
        sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" shadow_snap="true"
        size_box="4,4,4,4">
    <!-- 窗口内容 -->
</Window>
```

### 7.2 弹出菜单

```xml
<Window shadow_type="menu_round" shadow_border_size="1" shadow_border_color="border_window">
    <MenuListBox class="menu" padding="0,4,0,4">
        <!-- 菜单项 -->
    </MenuListBox>
</Window>
```

### 7.3 Tooltip（自定义阴影）

```xml
<Window shadow_type="custom"
        shadow_image="file='shadow_tooltip_round.svg' window_shadow_mode='true' corner='16,16,18,18'"
        shadow_corner="12,12,14,14"
        shadow_border_round="4,4"
        shadow_border_size="0"
        shadow_snap="false">
    <VBox bkcolor="bg_tooltip">
        <!-- 内容 -->
    </VBox>
</Window>
```

---

## 8. 与其他窗口属性的关系

- **`use_system_caption`**：若启用系统标题栏，未显式设置 `shadow_attached="false"` 时，duilib 会自动关闭自绘阴影以避免双重阴影；如需自绘阴影，请同时设置 `shadow_attached="true"` 并选择自绘类型。
- **`layered_window`**：使用自绘阴影时无需手动设置 `layered_window`，duilib 会按需开启分层窗口属性。系统阴影与分层窗口互斥。
- **`size`**：默认情况下，`size` 只表示客户区大小，不包含阴影。设置 `size_contain_shadow="true"` 可让 `size` 包含阴影的尺寸。
- **`round_corner`**：使用自绘阴影时窗口自然有圆角（由 `shadow_*` 决定），一般不必再设置 `round_corner`。
- **`render_backend_type="GL"`**：OpenGL 渲染的窗口不能是分层窗口，因此与自绘阴影不兼容；通常配合系统阴影使用。

---

## 9. 常见问题

**Q1：设置了阴影但看不到效果？**

- 检查窗口是否处于最大化状态：最大化时自绘阴影会自动隐藏。
- 检查 `use_system_caption` 是否为 `true`：开启系统标题栏时自绘阴影不会显示，请显式设置 `shadow_attached="true"` 并选择自绘类型。
- 检查 `shadow_attached` 是否被设置为 `false`。

**Q2：在 Win7 / Win10 上设置了 `system_round`，为什么不生效？**

Win7、Win10 仅 `system_default` 可用，其他 3 个 `system_*` 类型会自动切换。Win11 才有 4 种系统阴影。

**Q3：自绘阴影和系统阴影能同时启用吗？**

不能。设置任一种 `system_*` 后，自绘阴影会被关闭；反之亦然。

**Q4：如何在运行时切换阴影？**

使用 `Event` + `apply_attribute` 机制（如第 6.5 节示例），或直接调用 `Window::SetShadowType`。

**Q5：自定义阴影图片有什么要求？**

- 推荐使用 SVG（duilib 渲染器对 SVG 支持完善）。
- 需要正确设置 `corner` 九宫格，以避免拉伸时变形。
- 图片内容应"四角不透明、中间不透明"以正确支持窗口贴边和圆角裁剪。
- `window_shadow_mode='true'` 标记该图为阴影专用，会启用阴影相关的渲染优化。

**Q6：阴影的颜色怎么和主题保持一致？**

`shadow_border_color` 支持直接填写主题颜色名（如 `border_window`），duilib 会自动从主题色表中查找，并随主题切换（深/浅色）自动更新。

---

## 10. 相关文档与源码

- 源码：[Shadow.h](../duilib/Core/Shadow.h) / [Shadow.cpp](../duilib/Core/Shadow.cpp)
- 窗口接口：[Window.h](../duilib/Core/Window.h) / [Window.cpp](../duilib/Core/Window.cpp)
- 系统阴影定义：[NativeWindowShadow.h](../duilib/Core/NativeWindowShadow.h)
- XML 解析：[WindowBuilder.cpp](../duilib/Core/WindowBuilder.cpp)
- 窗口属性表：[Window.md](./Window.md)
- 阴影演示页：[page_window_shadow.xml](../../bin/resources/themes/default/render/page_window_shadow.xml)
- 自绘阴影素材：`public/shadow/`（`shadow_big.svg`、`shadow_big_round.svg`、`shadow_small.svg`、`shadow_small_round.svg`、`shadow_menu.svg`、`shadow_menu_round.svg`）

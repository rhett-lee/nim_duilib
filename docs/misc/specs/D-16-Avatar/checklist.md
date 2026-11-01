# Checklist: D-16 Avatar 头像

> 全部勾选才视为完成（DoD）。第 7 节为运行时人工验收项，由用户在示例程序中确认。

## 1. 控件源码

- [x] `duilib/Control/Avatar.h` / `.cpp` 已新增，命名空间 `ui`，基类为 `Control`（`typedef Control BaseClass`）。
- [x] `GetType()` 返回 `DUI_CTR_AVATAR`。
- [x] 枚举 `AvatarShape{kCircle = 0, kSquare = 1}` 为 `enum class : uint8_t`（默认 `kCircle`）。
- [x] 构造函数设置默认固定宽高 32（`SetFixedWidth/SetFixedHeight(UiFixedInt(32), false, true)`），
      不沿用 `PlaceHolder` 默认的 `Stretch`，避免被父容器拉伸成非正方形。
- [x] 构造函数设置默认 `corner = 8`（走 `SetBorderRound(UiSize(8,8))`）、`font = system_regular_14`、
      `text_color = text_avatar`、`text_scale = 50`、`icon_scale = 50`、`palette = true`。
- [x] `SetAttribute()` 支持 `size` / `shape` / `corner` / `image` / `icon` / `icon_scale` / `text` /
      `text_id` / `font` / `text_color` / `text_scale` / `color` / `palette`，并支持无下划线别名；
      未知属性回退 `BaseClass::SetAttribute()`。
- [x] 属性值先经 `GetExpandVarStrings()` 展开，再进入本控件分支（与 `CircleProgress` 等控件一致）。
- [x] `size` 支持 `small`(24) / `default`(32) / `large`(40) 预设与任意正整数；`0` 或负数被忽略（不破坏布局）。
- [x] `shape` / `palette` 解析忽略大小写与 `_` / `-`（与 `Popconfirm` 的 `ParseIcon()` 风格一致）。
- [x] `SetSize()` 同时设置固定宽高（`bNeedDpiScale = true`），`ChangeDpiScale` 后按同一视觉比例缩放。
- [x] `SetCorner()` 走 `SetBorderRound()`（存原始值，读取时按 DPI 缩放），`corner` 不超过控件短边一半。
- [x] `SetImage()` / `SetIcon()` 复用 `Image` + `SetImageString(..., Dpi())`，值变化时自动 `Invalidate()`。
- [x] `ClearImageCache()` 覆写：先 `BaseClass::ClearImageCache()`，再清头像图片与图标缓存。
- [x] `GetEstimateImage()` 覆写：返回 `image`（优先）或 `icon`，使 `width/height="auto"` 时可按图片估算。
- [x] `OnLanguageChanged(bool bRedraw)` 覆写：设置了 `text_id` 时调用基类并重绘。
- [x] 未新增任何事件，`duilib/Core/EventArgs.cpp` 未改动。

## 2. 绘制（形状 / 内容 / 边框）

- [x] `PaintBkColor()` 覆写：按形状填充底色（`color` > `bkcolor` > 调色板 > `bg_avatar`），
      使用 `FillRoundRect()`（保留 `bkcolor2` 渐变能力）。
- [x] `PaintStateColors()` 覆写：用户配置了 `normal_color` / `hovered_color` 等状态色时，按同一形状填充，
      不出现"圆形头像背后的方形色块"；回退顺序与 `StateColorMap` 一致
      （`pressed→hovered→normal`、`hovered→normal`、`disabled→normal`）；未配置时不绘制。
- [x] `PaintText()` 覆写：按优先级绘制图片 → 图标 → 文字；绘制前用 `AutoClip`（`SetRoundClip`）按形状裁剪。
- [x] 图片按 **cover** 计算目标矩形（等比放大到覆盖整个控件矩形并居中裁切），调用
      `PaintImage(..., &rcDest, ...)`；绘制失败（解码错误/路径不存在）时退化为只显示背景，不崩溃。
- [x] 图标按 **contain** 计算目标矩形（等比缩小到落在 `icon_scale` 比例的居中方框内）。
- [x] 文字：目标字号 = 控件短边 × `text_scale`%；用 `FontManager::GetIFont(fontId, 100)` 取原始字号换算
      zoom 百分比复用同一 fontId（不新增全局字体 ID，不受主题 22px 字号上限限制）；
      测量超宽/超高时按比例降 zoom，最后仍超出用 `TEXT_END_ELLIPSIS` 收尾。
- [x] 文字使用 `TEXT_SINGLELINE | TEXT_HCENTER | TEXT_VCENTER` 居中，颜色取 `text_color`（默认 `text_avatar`）。
- [x] `text` 优先于 `text_id`（与 `Label` 一致）；两者皆空时退化为"只显示背景"。
- [x] `PaintBorder()` 覆写：`shape="circle"` 时按圆形/椭圆描边（`DrawRoundRect`，半径随 `GetRect()`）；
      `shape="square"` 时交回基类（基类负责 `border_round` 圆角描边与 `border_dash_style`）。
      边框色优先级与基类一致（焦点边框 > 状态边框）；边框宽度不是四边等宽时退化为基类行为。
- [x] 圆形半径为实时计算（`min(w,h)/2`），拉伸控件或切换 DPI 后不出现圆角/裁剪失真。

## 3. 注册

- [x] `duilib/duilib_defs.h` 新增 `DUI_CTR_AVATAR`（紧随 `DUI_CTR_BADGE`）。
- [x] `duilib/Core/WindowBuilder.cpp` 新增 include 与工厂项 `{DUI_CTR_AVATAR, ... new Avatar(pWindow)}`。
- [x] `duilib/duilib.vcxproj` / `.vcxproj.filters` 登记 `Control\Avatar.h/.cpp`（Filter = `Control`）。
- [x] `duilib/duilib.h` 新增 `#include "Control/Avatar.h"`。

## 4. 主题、资源与示例

- [x] `bin/resources/themes/default/global.xml` 新增变量 `PATH_AVATAR`（`public/avatar`）。
- [x] `global.xml` 新增 10 个 Class：`avatar`（32）、`avatar_small`（24）、`avatar_large`（40）、
      `avatar_xlarge`（64）、`avatar_square` / `avatar_square_large`（方形）、`avatar_user` /
      `avatar_user_large`（内置用户图标）、`avatar_photo`（内置示例图片）、`avatar_text`（粗体文字头像）。
      这些 Class **不设置** `bkcolor`，否则会压过自动调色板。
- [x] `color_light/global.xml` 与 `color_dark/global.xml` 各新增 `bg_avatar`、`bg_avatar_1..6`、
      `text_avatar`（注释分组 `type: avatar`），无写死色值进入控件代码。
- [x] 新增 `public/avatar/avatar_icon_user.svg`（16×16 人像图标，占位色由 `svg_replace_colors` 映射）
      与 `public/avatar/avatar_picture_demo.svg`（示例"照片"，用于演示 `image` 形态）。
- [x] `examples/controls/AvatarForm.h/.cpp` 存在：`GetSkinFolder()` = `controls`、
      `GetSkinFile()` = `avatar.xml`；`OnInitWindow()` 绑定 `size`/`shape`/`text`/`image`/`palette` 等演示按钮。
- [x] `bin/resources/themes/default/controls/avatar.xml` 存在，分组覆盖：尺寸、形状、图片、图标、
      文字 + 自动调色板、显式 `color`、空内容、动态修改。
- [x] `controls/test.xml` 有 Avatar 入口（`avatar_btn` + `STRID_CONTROLS_AVATAR_TEST`）。
- [x] `examples/controls/TestForm.h/.cpp` 有 `ShowAvatarWindow()` 与入口绑定。
- [x] `controls.vcxproj` / `.filters` 登记 `AvatarForm.h/.cpp`。
- [x] 中英 examples 语言文件键集合一致（`STRID_AVATAR_*` 段与 `STRID_CONTROLS_AVATAR_TEST`），
      UTF-8 无 BOM、中文无乱码。

## 5. 文档与计划

- [x] `docs/Control.md` 新增 `## Avatar 控件（头像）`（核心特性 / XML 属性表 / C++ 接口表 / 枚举 /
      10 个 Class 表 / 图标资源 / 使用示例 / 注意事项），并加 `<a name="avatar"></a>`。
- [x] `docs/Control.en.md` 对称更新。
- [x] `docs/Summary.md` / `Summary.en.md` 控件清单追加 `Avatar` 行与有效锚点 `#avatar`。
- [x] `docs/Differences.md` / `Differences.en.md` 的"3. 新增控件/容器"表格追加 `Avatar` 行（**序号 55**，中英同步）。
- [x] `README.md` / `README.en.md` 控件清单（若该处列出控件）同步。
- [x] `docs/misc/Development-Plan.md`：基线日期行、P2 进展汇总、D 组表格 D-16 行、
      P2 任务清单 `- [x] **D-16**` 均已回写。
- [x] `.trae/skills/nim-duilib/references/api-reference.md` 与
      `.workbuddy/skills/nim-duilib/references/api-reference.md` 同步：控件清单 `Avatar` 行 +
      `### Avatar 属性` 速查段（两份 `Get-FileHash` 一致）。
- [x] `.claude/docs/nim-duilib-llm-reference.md` 控件速查表追加 `Avatar` 行。
- [x] `docs/misc/specs/D-16-Avatar/{spec,tasks,checklist}.md` 已创建并回写。

## 6. 验证命令

> 命令在 `e:\develop\nim_duilib` 下、PowerShell 中执行。

- [x] `duilib`（Debug|x64）编译退出码 0。
- [x] `examples\controls\controls.vcxproj`（Debug|x64，`/p:SolutionDir=<repo>\build\`）编译退出码 0。
- [x] `python .trae/skills/nim-duilib/scripts/verify_docs.py --repo . --extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang` 退出码 0。
- [x] `git diff --check` 无空白错误。
- [x] 两份 `api-reference.md` 的 `Get-FileHash` 相同。
- [x] 未执行 `git commit`（由用户手动提交）。

> 验证记录（2026-11-01）：`duilib`（Debug|x64）与 `examples\controls\controls.vcxproj`（Debug|x64）编译退出码均为 0；
> `verify_docs.py --extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang`
> 输出"扫描文档 246 个……全部命中"，EXITCODE=0；`git diff --check` 无空白错误（仅 LF→CRLF 提示）；
> 两份 `api-reference.md` 的 SHA256 均为 `B83EAC191DFD9EACD99EA9D5EF10030829D93615F639B024CD33A9872B98AE96`。

## 7. 功能验收（手工）

> 以下为运行时人工验收项，由用户在 controls 示例（"Avatar"分组）中确认。

### 示例演示（对齐 Ant Design 演示页）
- [ ] 尺寸：24 / 32 / 40 / 64 四种尺寸的圆头像视觉比例正确，圆形不变形。
- [ ] 形状：`shape="square"` 时四角为 `corner` 圆角；`corner` 调大后圆角明显增大且不超过半边长。
- [ ] 图片：照片按 cover 填满整个头像并被圆形裁剪，不溢出到方形四角。
- [ ] 图标：内置用户图标在头像中居中，大小约为头像的一半（`icon_scale` 可调）。
- [ ] 文字：单字/双字文字头像居中显示，字号随头像尺寸变化，长文本自动缩小或显示省略号。
- [ ] 调色板：多个不同文字的头像自动得到不同底色，且同一文字多次重建颜色保持不变；
      `palette="false"` 时回退统一底色 `bg_avatar`。
- [ ] 显式 `color`：设置 `color="color_accent"` 等语义色后以该色为准。
- [ ] 空内容：既不设 `image` / `icon` / `text` 时只显示底色圆，不崩溃、不显示残留文字。

### 动态修改与边界
- [ ] 运行时切换 `size` / `shape` / `text` / `image` / 清空内容，头像立即刷新且布局不错位。
- [ ] 头像图片文件不存在或解码失败时不崩溃，仅显示底色。
- [ ] 切换语言后 `text_id` 头像文案立即更新；切换主题后底色/文字色/图标色自动适配。

### DPI 与主题
- [ ] 100% / 150% / 200% DPI 下尺寸、圆角、裁剪、字号比例一致，圆形边缘无锯齿或溢出。
- [ ] 浅色 / 深色主题下 `bg_avatar`、`bg_avatar_1..6`、`text_avatar` 对比度正常，文字可读。

# Spec: D-16 Avatar 头像

## 1. 目标

新增独立控件 `Avatar`（XML 节点名 `Avatar`，头文件 `duilib/Control/Avatar.h`），
对齐 Ant Design 的 [Avatar](https://ant-design.antgroup.com/components/avatar-cn) 语义：
**用圆形或圆角方形展示"图片 / 图标 / 文字"三类头像内容**，尺寸可配置（small / default / large / 数值），
文字头像按尺寸自动缩放字号并按文本做配色。

现有能力缺口：

| 现有控件 | 缺什么 |
| :--- | :--- |
| `Control` + `bkimage` | 只能矩形拉伸/按属性对齐，没有"圆形裁剪 + 等比 cover 填充"，文字也不会按控件尺寸自适应 |
| `Button` | 可以放图标文字，但按钮语义（状态色/边框/点击）与头像不符，且文字与图标是并列排布而不是居中覆盖 |
| `IconControl` | 只支持外部传入 32 位位图数据，不支持图片路径/文字，也没有圆形裁剪 |
| `Label` | 有文字，但没有圆形背景、没有图片 cover、字号不会随控件尺寸自动缩放 |

完成后回写 `docs/misc/Development-Plan.md` 的 D 组表格（新增 D-16 行）与 P2 任务清单
（新增 `- [x] **D-16**` 任务行）。

## 2. 需求要点

- 形态：`Avatar : public Control`（叶子自绘控件，无子控件）。
- 形状：`shape="circle|square"`（默认 `circle`）；方形圆角半径由 `corner`（默认 8）控制。
- 尺寸：`size` 支持 `small`(24) / `default`(32) / `large`(40) 预设或数值（未做 DPI 缩放的原始像素）；
  不写 `size` 时默认 32。
- 内容优先级：`image`（图片，等比 cover 填充 + 形状裁剪）→ `icon`（图标，等比 contain 居中）→
  `text`/`text_id`（文字，按尺寸自动缩放字号）→ 空（只画背景）。
- 背景：`color`（头像专用）> `bkcolor` > 文字头像自动调色板（`bg_avatar_1..6`，按文本哈希）> `bg_avatar`；
  显式 `palette="false"` 可关闭自动调色板。
- 文字：`font`（字体 ID，默认 `system_regular_14`）、`text_color`（默认语义色 `text_avatar`）、
  `text_scale`（字号占控件短边百分比，默认 50）、`icon_scale`（图标占控件短边百分比，默认 50）。
- 事件：不新增事件（纯展示控件，无交互语义）。
- 交付完整的主题 Class / 图标资源 / 示例 / 中英双语文档 / 技能参考。

## 3. 设计决策

### 3.1 形态：`Avatar : public Control`，不改现有控件

| 方案 | 评价 |
| :--- | :--- |
| A. 在 `Button` 上加 `avatar_mode` 属性 | 按钮的状态色/边框/点击/焦点语义都会带进来，需要大量开关屏蔽；属性表污染严格。**不采用** |
| B. 在 `Label` 上加圆形背景与图片 cover | `Label` 的文本测量/布局/自动换行/富文本/多语言一整套机制只用到很小一部分，且 `LabelImpl` 的文字区域计算与"按控件尺寸算字号"冲突。**不采用** |
| C. 新建 `Avatar : public Control`，自行绘制背景与内容 | 控件职责单一；绘制流水线可直接复用 `PaintBkColor` / `PaintStateColors` / `PaintText` / `PaintBorder` 四个钩子，且与 `Control` 的属性体系（margin/padding/border/state color）自然兼容。**采用** |

### 3.2 绘制流水线与钩子分配

`Control::Paint()` 的顺序是：`PaintShadow` → `PaintBkColor` → `PaintStateColors` → `PaintBkImage` →
`PaintStateImages` → `PaintText` → `PaintBorder`（`IsBordersOnTop()` 时延后）→ `PaintFocusRect`。

`Avatar` 的分配：

| 钩子 | 用途 |
| :--- | :--- |
| `PaintBkColor` | 按形状填充头像底色（`color` / `bkcolor` / 自动调色板 / `bg_avatar`） |
| `PaintStateColors` | 若用户设置了状态色（`normal_color` 等），按同一形状填充状态色，避免出现"圆形头像背后的方形色块" |
| `PaintText` | 绘制内容（图片 / 图标 / 文字），先设置形状裁剪再绘制 |
| `PaintBorder` | 圆形时按圆形描边（`DrawCircle`），方形时交给基类（基类负责 `border_round` 圆角描边） |

### 3.3 形状与 DPI

- `Control::ChangeDpiScale()` 会缩放固定宽高、margin/padding/border size，但**不缩放 `border_round`**；
  `GetBorderRound()` 在读取时按当前 DPI 缩放。因此方形圆角直接复用 `SetBorderRound()`（存原始值），
  不额外维护 DPI 状态。
- 圆形半径在绘制时按 `GetRect()` 实时计算（`min(w,h)/2`），不写入 `border_round`，
  DPI 切换或控件被拉伸后都不会失真。
- 图片内容用 `AutoClip`（`SetRoundClip`）做形状裁剪；文字在控件矩形内居中，不需要裁剪。

### 3.4 内容绘制细节

- 图片：读取 `ImageInfo` 宽高后计算 **cover** 目标矩形（等比放大到覆盖整个控件矩形，居中裁切溢出部分），
  再调用 `Control::PaintImage(pRender, m_pImage.get(), "", 255, nullptr, &rcDest, nullptr)`；
  绘制前用形状裁剪避免方角溢出。
- 图标：计算 **contain** 目标矩形（等比缩小到完整落在 `icon_scale` 比例的居中方框内），同样走 `PaintImage`。
- 文字：目标字号 = `min(w,h) * text_scale%`；用 `FontManager::GetIFont(fontId, 100)` 取原始字号，
  再按目标像素换算 zoom 百分比复用同一 fontId（避免新增全局字体 ID，也不受主题最大 22px 字号限制）；
  测量后若仍超宽/超高，按比例继续降低 zoom（下限 30%），最后仍超出则用 `TEXT_END_ELLIPSIS` 收尾。
- 多语言：`text_id` 按语言 ID 解析（`Lang().GetStringByID()`），`text` 优先于 `text_id`（与 `Label` 一致）。

### 3.5 背景色优先级与调色板

```
color（显式） > bkcolor（基类通用属性） > 自动调色板（仅文字头像且 palette=true） > bg_avatar
```

自动调色板取 `bg_avatar_1..6`，索引 = `hash(文本) % 6`，同一文本始终得到同一颜色（确定性）。
图片/图标头像不参与调色板（背景固定 `bg_avatar`），避免照片背后的颜色干扰画面。

### 3.6 属性与 API

| XML 属性 | API | 默认 | 说明 |
| :--- | :--- | :--- | :--- |
| `size` | `SetSize(int32_t)` | 32 | `small`(24)/`default`(32)/`large`(40) 或数值（原始像素）；同时设置固定宽高 |
| `shape` | `SetShape(AvatarShape)` | `circle` | `circle` / `square` |
| `corner` | `SetCorner(int32_t)` | 8 | 方形圆角半径（原始像素，经 `border_round` 自动 DPI 缩放） |
| `image` | `SetImage(DString)` | 空 | 头像图片（支持完整图片属性串，含 `file=` / `svg_replace_colors=`） |
| `icon` | `SetIcon(DString)` | 空 | 头像图标（同上） |
| `icon_scale` | `SetIconScale(int32_t)` | 50 | 图标边长占控件短边的百分比 |
| `text` / `text_id` | `SetText` / `SetTextId` | 空 | 文字头像内容（`text` 优先） |
| `font` | `SetFontId(DString)` | `system_regular_14` | 文字字体 ID（决定字体族与粗斜体） |
| `text_color` | `SetTextColor(DString)` | `text_avatar` | 文字颜色（语义色名） |
| `text_scale` | `SetTextScale(int32_t)` | 50 | 字号占控件短边的百分比 |
| `color` | `SetColor(DString)` | 空 | 头像底色（语义色名），优先级最高 |
| `palette` | `SetAutoPalette(bool)` | `true` | 文字头像是否按文本自动配色 |
| 通用 | 继承 | — | `bkcolor` / `border_size` / `border_color` / `margin` / `padding` / `state` 系列 / `visible` 等 |

额外 API：

```cpp
void SetImage(const DString& strImage);   DString GetImage() const;
void SetIcon(const DString& strIcon);     DString GetIcon() const;
DString GetText() const;                 // 返回解析后的文本（text 优先，其次 text_id）
void ClearImageCache() override;         // 主题切换后清除头像图片/图标缓存
```

### 3.7 明确不做

- `Avatar.Group`（头像组、`max_count` 折叠 +N、`max_style`、悬停浮层、堆叠间距）——后续单独排期。
- `srcSet` 响应式多尺寸图片、远程 URL 图片。
- 加载失败时的 `fallback` 兜底图/兜底字符（图片解码失败时按"无内容"绘制，即只显示背景）。
- 图片/图标的旋转、滤镜、边框描边叠加。
- 点击事件：头像本身不派发专用事件（需要点击行为时在外部挂 `kEventClick` 或包一层 `Button`）。

## 4. 精确改动清单

### 4.1 控件

- 新增 `duilib/Control/Avatar.h` / `.cpp`：
  - `enum class AvatarShape : uint8_t { kCircle = 0, kSquare = 1 }`；
  - 属性解析（忽略大小写；`size` 预设与数值两种形态）、形状/圆角/颜色/调色板/字号缩放；
  - 四个绘制钩子覆写（见 3.2）、`GetEstimateImage()` 覆写（图片/图标参与 auto 尺寸估算）、
    `ClearImageCache()` 覆写。

### 4.2 注册（5 处）

1. `duilib/duilib_defs.h`：`DUI_CTR_AVATAR (_T("Avatar"))`（紧随 `DUI_CTR_BADGE`）。
2. `duilib/Core/WindowBuilder.cpp`：`{DUI_CTR_AVATAR, [](Window* pWindow) { return new Avatar(pWindow); }}`。
3. `duilib/duilib.vcxproj`：`ClCompile` + `ClInclude`。
4. `duilib/duilib.vcxproj.filters`：`ClCompile` + `ClInclude`（`Control` 过滤器）。
5. `duilib/duilib.h`：`#include "Control/Avatar.h"`。

> 无新增事件，因此不改 `EventArgs.cpp` 的 `InitEventStringMap`。

### 4.3 主题与资源

- `bin/resources/themes/default/global.xml`：
  - `<Var name="PATH_AVATAR" value="public/avatar"/>`；
  - Class：`avatar`（size 32）、`avatar_small`（24）、`avatar_large`（40）、`avatar_xlarge`（64）、
    `avatar_square` / `avatar_square_large`（方形）、`avatar_user` / `avatar_user_large`（内置用户图标）、
    `avatar_photo`（内置示例图片）、`avatar_text`（粗体文字头像）。
- `bin/resources/themes/color_light/global.xml` 与 `color_dark/global.xml` 各新增一组语义色（`type: avatar`）：
  `bg_avatar`、`bg_avatar_1..6`、`text_avatar`。
- 新增 `bin/resources/themes/default/public/avatar/`：
  - `avatar_icon_user.svg`（人像图标，16×16，占位色由 `svg_replace_colors` 映射到 `text_avatar`）；
  - `avatar_picture_demo.svg`（示例"照片"图，256×256，用于演示 image 形态）。

### 4.4 示例

- `examples/controls/AvatarForm.h/.cpp`：尺寸、形状、图片/图标/文字、自动调色板、自定义颜色、
  动态修改（切换 size/shape/文字/图片/清空内容）演示。
- `bin/resources/themes/default/controls/avatar.xml`：演示窗口布局。
- `bin/resources/themes/default/controls/test.xml`：新增入口按钮。
- `examples/controls/TestForm.h/.cpp`：`ShowAvatarWindow()` 与入口绑定。
- `examples/controls/controls.vcxproj` / `.filters`：新增源码。
- `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`：示例文案（含语言 ID）。

### 4.5 文档

- `docs/Control.md` / `docs/Control.en.md`：新增 `Avatar` 章节（属性表 / API / Class 表 / 资源 / 示例 / 注意事项）。
- `docs/Summary.md` / `Summary.en.md`：控件清单追加。
- `docs/Differences.md` / `Differences.en.md`：第 3 节表格追加一行（**序号 55**）。
- `docs/misc/Development-Plan.md`：D 组表格 + P2 任务清单。
- `.trae/skills/nim-duilib/references/api-reference.md` +
  `.workbuddy/skills/nim-duilib/references/api-reference.md`（两份哈希一致）与
  `.claude/docs/nim-duilib-llm-reference.md`：控件速查追加 `Avatar`。
- `docs/misc/specs/D-16-Avatar/{spec,tasks,checklist}.md`。

## 5. 验证

1. `duilib`（Debug|x64）+ `examples/controls`（Debug|x64）编译通过。
2. `python .trae/skills/nim-duilib/scripts/verify_docs.py --repo . --extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang` 退出码 0（无漂移）。
3. `git diff --check` 无空白错误。
4. 手工运行示例程序，确认：圆形/方形形状、24/32/40/64 尺寸、图片 cover、图标居中、
   文字自动字号与自动配色、显式 `color`、空内容、深色主题下的配色、100%~200% DPI 下的清晰度。

## 6. 验收标准

- [ ] XML 写 `<Avatar class="avatar" icon="..."/>` / `<Avatar class="avatar" text="A"/>` /
  `<Avatar class="avatar_large" image="..."/>` 即可得到正确头像，无需业务侧自绘。
- [ ] `shape="square"` 时四角圆角为 `corner`，`shape="circle"` 时为正圆/正椭圆（随控件矩形）。
- [ ] 图片按 cover 填充且被形状裁剪；图标按 contain 居中；文字垂直水平居中，长文本自动缩小或省略。
- [ ] 文字头像不设置 `color` 时按文本稳定地取调色板颜色；`palette="false"` 时回退 `bg_avatar`。
- [ ] `size` 预设与数值在 100%/150%/200% DPI 下都按同一视觉比例缩放，切换 DPI 后不出现圆角/裁剪失真。
- [ ] 深色/浅色主题切换后头像底色、文字色、图标色自动适配；无写死色值。

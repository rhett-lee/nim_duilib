# Carousel / 分页器 控件 Spec（B7）

## Why
`docs/misc/Project-Audit.md` 的 **B7** 登记「Carousel / 分页器 / 无限滚动——图片轮播、卡片流」为待办（`Development-Plan.md` 的 **D-11** Carousel 与 **E-4** Pagination 分别登记；全仓 `Carousel` / `Pagination` 均无命中）。图片轮播（首页横幅、广告位、图片集）与分页器（报表 / 表格类软件的页码导航）是高频 UI 元素，当前只能靠业务层用按钮组 + 手写状态机拼装。本项新增 **两个控件**：`Carousel`（图片轮播 / 无限循环）与 `Pagination`（分页器）。

## What Changes
### 1. 新增 `Carousel` 控件（继承 `Control`，自绘）
- **显示**：按 `item_image` 列表（`|` 分隔的图片属性串）逐张展示，同一时刻显示一张（卡片流可配置每屏可见张数）。
- **主题自适应**：所有图片经 `svg_replace_colors`（如 `#0078D4|color_accent`）映射到主题语义色，切换主题由皮肤层自动接管，与 `Stepper` / `Rating` / `Slider` 既有范式一致（**图片驱动，不自绘图形**）。
- **分页 / 导航**：左右切换按钮（SVG 图标）+ 底部页码指示点（可点击跳转）；指示点当前页高亮。
- **无限滚动**：`loop` 开启时首尾相接、无限循环（最后一张的下一张回到第一张）。
- **自动播放**：`auto_play` + `interval`（毫秒），`hover_pause` 悬停暂停。
- **切换动画**：`transition_ms` 水平滑动过渡（0 = 立即切换），基于框架定时器逐帧推进。
- **交互**：点击导航按钮 / 指示点、按住鼠标左右拖拽（滑动切换）；非只读。
- **属性**：见下方「设计与约定」；**事件**：页切换派发 `kEventValueChanged`（`AttachCarouselChanged`），参数 `CarouselValue{index, oldIndex}`。

### 2. 新增 `Pagination` 控件（继承 `HBox`，动态组合 `Button`）
- **显示**：上一页 / 下一页 + 页码按钮（超出 `max_visible_count` 时用 `…` 省略）；当前页按钮为选中态（`SetSelected(true)`）。
- **数据**：`page_count` 直接设定，或 `total_count` + `page_size` 推导；`current_page`（1 起）钳位到 `[1, page_count]`。
- **皮肤驱动**：分页按钮通过 `button_class`（默认 `pagination_button`）引用 `global.xml` 样式，颜色走语义色 + 选中态，**不在 C++ 写死色值**。
- **事件**：页码变化派发 `kEventValueChanged`（`AttachPageChanged`），参数 `PaginationValue{page, oldPage}`。

### 3. 端到端接线与文档
- 类型常量（`DUI_CTR_CAROUSEL` / `DUI_CTR_PAGINATION`）、工厂注册、构建工程、演示窗口、示例入口、多语言文案。
- 文档同步：`docs/Control.md`、`docs/Control.en.md`、两份 `api-reference.md`、`nim-duilib-llm-reference.md`。
- **非 BREAKING**：全部为新增，不改动既有控件行为。

## Impact
- Affected specs：`Project-Audit.md` B7；`Development-Plan.md` D-11（Carousel）、E-4（Pagination）。
- Affected code：见下方「改动文件清单」。

## ADDED Requirements

### Requirement: Carousel 图片轮播控件
系统 SHALL 提供可主题化、支持分页导航、自动播放与无限循环的 `<Carousel>` 控件，其视觉外观 SHALL 由 SVG 图片（而非代码自绘图形）呈现，并 SHALL 通过 `svg_replace_colors` 跟随主题语义色。

#### Scenario: 手动切换
- **WHEN** 控件含 3 张幻灯片，用户点击"下一张"按钮
- **THEN** 索引由 0 变为 1，显示第 2 张，并派发 `kEventValueChanged`（携带新 / 旧索引）。

#### Scenario: 无限循环
- **WHEN** `loop="true"`，当前为最后一张，用户点击"下一张"
- **THEN** 索引回到 0（首尾相接），不越界。

#### Scenario: 自动播放
- **WHEN** `auto_play="true"` 且 `interval="3000"`
- **THEN** 每 3000ms 自动前进一张（循环模式下到达末尾后回到第一张）。

#### Scenario: 悬停暂停
- **WHEN** `hover_pause="true"`，鼠标移入控件
- **THEN** 自动播放暂停；鼠标移出后恢复。

#### Scenario: 指示点跳转
- **WHEN** 用户点击第 N 个指示点
- **THEN** 直接切换到第 N 张并派发事件。

#### Scenario: 主题切换
- **WHEN** 运行时切换主题
- **THEN** 导航按钮 / 指示点颜色随 `svg_replace_colors` 映射的语义色自动变化，无需改动 C++ 代码。

#### Scenario: 索引钳位
- **WHEN** 设置 `current_index` 越界，或 `item_image` 数量变化后当前索引失效
- **THEN** 索引自动钳位到有效范围（无幻灯片时为 -1 / 不绘制）。

### Requirement: Pagination 分页器控件
系统 SHALL 提供可主题化、数据驱动的 `<Pagination>` 分页器控件，按 `page_count` / `current_page` 生成页码按钮与前后翻页按钮，当前页高亮，页码超出显示上限时以省略号收纳，并在页码变化时派发事件。

#### Scenario: 基本分页
- **WHEN** `page_count="10"`、`current_page="3"`
- **THEN** 序号区显示含当前页 3 的页码按钮组，第 3 个按钮呈选中态。

#### Scenario: 点击翻页
- **WHEN** 用户点击"下一页"
- **THEN** `current_page` 变为 4 并派发 `kEventValueChanged`（携带新旧页码）。

#### Scenario: 省略收纳
- **WHEN** `page_count` 远大于 `max_visible_count`
- **THEN** 序号区以 `…` 收纳中间页码，始终可见首页 / 末页与当前页附近。

#### Scenario: 由总数推导
- **WHEN** 设置 `total_count` 与 `page_size`
- **THEN** `page_count = ceil(total_count / page_size)`，并据此重建页码按钮。

#### Scenario: 页码钳位
- **WHEN** `current_page` 超出 `[1, page_count]`
- **THEN** 自动钳位到边界值。

## 设计与约定

### Carousel：XML 属性
| 属性名称 | 默认值 | 类型 | 说明 |
| :--- | :--- | :--- | :--- |
| item_image |  | 图片属性串列表 | 幻灯片图片，`\|` 分隔；每项如 `file='${PATH_CAROUSEL}/slide1.png'` |
| current_index | 0 | int | 当前幻灯片索引（0 起），钳位到 `[0, count-1]` |
| loop | true | bool | 是否循环（首尾相接，无限滚动） |
| auto_play | false | bool | 是否自动播放 |
| interval | 3000 | int | 自动播放间隔（毫秒） |
| hover_pause | true | bool | 鼠标悬停时暂停自动播放 |
| transition_ms | 300 | int | 切换动画时长（毫秒，0 = 立即切换） |
| show_indicator | true | bool | 是否显示页码指示点 |
| indicator_normal_image |  | 图片属性串 | 指示点常态图（SVG 小圆点） |
| indicator_active_image |  | 图片属性串 | 指示点当前页图 |
| indicator_gap | 8 | int | 指示点间距（像素，DPI 缩放） |
| indicator_bottom | 10 | int | 指示点距底边距离（像素，DPI 缩放） |
| show_nav_button | true | bool | 是否显示左右切换按钮 |
| btn_prev_image |  | 图片属性串 | 上一张按钮图（SVG） |
| btn_next_image |  | 图片属性串 | 下一张按钮图（SVG） |
| btn_prev_hovered_image / btn_next_hovered_image |  | 图片属性串 | 按钮悬停态（可选，回退常态） |
| btn_prev_pressed_image / btn_next_pressed_image |  | 图片属性串 | 按钮按下态（可选，回退悬停 / 常态） |

`Carousel` 继承 `Control` 的全部属性；默认皮肤 `carousel` 类已内置导航按钮与指示点图片及语义色映射，可直接 `class="carousel"` 引用。

### Carousel：C++ 接口
```cpp
struct CarouselValue { int32_t index = 0; int32_t oldIndex = 0; };

int32_t GetCurrentIndex() const;                 // 当前索引，无幻灯片为 -1
void    SetCurrentIndex(int32_t nIndex);         // 钳位 + 变化时派发事件
void    SetCurrentIndexNoEvent(int32_t nIndex);
int32_t GetItemCount() const;
void    AddItem(const DString& strImage);        // 追加一张（图片属性串）
void    InsertItem(int32_t nIndex, const DString& strImage);
bool    RemoveItem(int32_t nIndex);
void    ClearItems();
void    SetItems(const std::vector<DString>& items);
DString GetItem(int32_t nIndex) const;

bool    Next();                                  // 前进一张（loop 越界回到 0；否则失败返回 false，派发事件）
bool    Prev();                                  // 后退一张
bool    GoTo(int32_t nIndex);                    // 跳转（越界返回 false）
bool    NextNoEvent(); bool PrevNoEvent(); bool GoToNoEvent(int32_t nIndex);

bool    IsLoop() const;       void SetLoop(bool bLoop);
bool    IsAutoPlay() const;   void SetAutoPlay(bool bAutoPlay);
int32_t GetInterval() const;  void SetInterval(int32_t nIntervalMs);
bool    IsHoverPause() const; void SetHoverPause(bool bHoverPause);
int32_t GetTransitionMs() const; void SetTransitionMs(int32_t nMs);

void    AttachCarouselChanged(const EventCallback& callback, EventCallbackID id = 0); // kEventValueChanged
void    ClearImageCache() override;              // 释放幻灯片 / 按钮 / 指示点图片缓存
```

### Pagination：XML 属性
| 属性名称 | 默认值 | 类型 | 说明 |
| :--- | :--- | :--- | :--- |
| page_count（别名 total_page） | 0 | int | 总页数 |
| current_page（别名 page） | 1 | int | 当前页码（1 起），钳位到 `[1, page_count]` |
| total_count | 0 | int | 总记录数（与 page_size 配合推导 page_count，可选） |
| page_size | 0 | int | 每页记录数（可选；`total_count`+`page_size` > 0 时优先推导） |
| max_visible_count | 7 | int | 序号区最多显示的页码按钮数（含首尾），超出用 `…` 收纳 |
| show_prev_next | true | bool | 是否显示上一页 / 下一页 |
| show_first_last | false | bool | 是否显示首页 / 末页按钮 |
| prev_text / next_text | ‹ / › | string | 上一页 / 下一页文本 |
| first_text / last_text | « / » | string | 首页 / 末页文本 |
| ellipsis_text | … | string | 省略号文本 |
| button_class | pagination_button | string | 页码按钮引用的皮肤 Class（含常态 / 悬停 / 选中态） |
| button_width / button_height | 32 / 32 | int | 页码按钮尺寸（像素，DPI 缩放；0 = 自适应） |

`Pagination` 继承 `HBox` 的全部布局属性（如 `item_gap` / `padding` / `halign`）；页面按钮由控件内部按数据动态创建。

### Pagination：C++ 接口
```cpp
struct PaginationValue { int32_t page = 1; int32_t oldPage = 1; };

int32_t GetPageCount() const;      void SetPageCount(int32_t nPageCount);
int32_t GetCurrentPage() const;    void SetCurrentPage(int32_t nPage);      // 钳位 + 变化时派发事件
void    SetCurrentPageNoEvent(int32_t nPage);
void    SetTotalCount(int32_t nTotalCount, int32_t nPageSize);              // 由总数推导 page_count
bool    Next(); bool Prev(); bool GoTo(int32_t nPage);
int32_t GetMaxVisibleCount() const; void SetMaxVisibleCount(int32_t n);
void    SetButtonClass(const DString& strClass); DString GetButtonClass() const;

void    AttachPageChanged(const EventCallback& callback, EventCallbackID id = 0); // kEventValueChanged
```

### 绘制约定（图片驱动，颜色走 SVG 主题映射）
- 幻灯片 / 导航按钮 / 指示点图片均以 `StateImage` 承载，绘制用 `Control::PaintImage`，`strModify` 携带 `destscale='false' dest='x,y,x,y'`（控件相对坐标）把图片缩放到目标位置。
- 幻灯片按控件内容区（去内边距）铺满；切换动画期间同时绘制"上一张 / 下一张"两张，按进度 `t∈[0,1]` 施加水平偏移。
- 导航按钮从左右两端内缩绘制；指示点在底部水平居中排列，当前页用 `indicator_active_image`。
- 颜色全部由图片 `svg_replace_colors` 决定，C++ 侧不持有任何颜色值。

### 交互与定时器约定
- **Carousel**：`ButtonDown` 记录拖拽起点；`MouseMove` 拖动超过阈值后按位移方向切换（松手吸附到目标页）；点击未被拖动的导航按钮区 → `Prev/Next`；点击指示点区 → `GoTo`。`MouseEnter/Leave` 维护 `hover_pause`。自动播放 / 切换动画通过 `GlobalManager::Instance().Timer().AddTimer(...)`（参照 `Marquee`）驱动，`Invalidate()` 触发重绘；`SetWindow` 变更 / 析构时清理定时器。
- **Pagination**：内部按钮 `AttachClick`（`kEventClick`）→ `GoTo`；`page_count` / `current_page` / 样式变化时重建或刷新按钮（当前页按钮 `SetSelected(true)`）。
- `IsEnabled()==false` 时整体降低透明度且不响应交互。

## 改动文件清单
1. `duilib/duilib_defs.h`：新增 `DUI_CTR_CAROUSEL (_T("Carousel"))`、`DUI_CTR_PAGINATION (_T("Pagination"))`。
2. `duilib/Control/Carousel.h` / `.cpp`（新增）：类声明、`CarouselValue`、XML 属性与 C++ 接口、图片驱动绘制 / 动画 / 交互 / 定时器。
3. `duilib/Control/Pagination.h` / `.cpp`（新增）：HBox 派生的分页器，动态生成按钮、数据模型与事件。
4. `duilib/Core/WindowBuilder.cpp`：include 两个头文件并在 `createControlMap` 注册。
5. `duilib/duilib.h`：汇总头引入两个头文件。
6. `duilib/duilib.vcxproj` / `.filters`：新增四个源文件条目。
7. `bin/resources/themes/default/public/carousel/`（新增目录）：`nav_prev.svg`、`nav_next.svg`、`nav_prev_hovered.svg`、`nav_next_hovered.svg`、`indicator_normal.svg`、`indicator_active.svg`（含 `#0078D4` 占位色）。
8. `bin/resources/themes/default/global.xml`：新增 `<Var name="PATH_CAROUSEL">`、`<Class name="carousel">`（导航 / 指示点图片 + 语义色映射）、`<Class name="pagination_button">`（页码按钮常态 / 悬停 / 选中态）。
9. `bin/resources/themes/default/controls/carousel.xml`（新增）：演示窗口（自动轮播、手动轮播、指示点变体、分页器）。
10. `bin/resources/themes/default/controls/test.xml`：新增 `carousel_btn`。
11. `examples/controls/CarouselForm.h` / `.cpp`（新增）：演示逻辑（自动播放开关、上一张 / 下一张、分页器事件回显）。
12. `examples/controls/TestForm.h` / `.cpp`：新增 `ShowCarouselWindow()` 与 `carousel_btn` 处理。
13. `examples/controls/controls.vcxproj` / `.filters`：新增 `CarouselForm` 条目。
14. `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`：新增 `STRID_CAROUSEL_*` / `STRID_PAGINATION_*` 文案。
15. `docs/Control.md`、`docs/Control.en.md`：新增「Carousel 控件（图片轮播）」「Pagination 控件（分页器）」小节。
16. `.trae/skills/nim-duilib/references/api-reference.md`、`.workbuddy/skills/nim-duilib/references/api-reference.md`：同步两个控件条目。
17. `.claude/docs/nim-duilib-llm-reference.md`：速查行补充两个控件。
18. `docs/misc/Project-Audit.md` B7、`docs/misc/Development-Plan.md` D-11（Carousel）与 E-4（Pagination）：标记完成。

## 验证方式
- **编译**：`build/examples_no_cef.sln` → Configuration `Debug` / Platform `x64` 编译通过（无错误）。
- **资源**：`public/carousel/` 下 SVG 存在，`class="carousel"` / `pagination_button` 正确引用，`svg_replace_colors` 映射到语义色。
- **运行时**（DPI 感知脚本 + 截图）：打开演示窗口，逐项核对——
  1. 点击"下一张"切换到第 2 张并事件回显索引；
  2. `loop` 下最后一张点击"下一张"回到第一张；
  3. `auto_play` 按 `interval` 自动前进，悬停暂停、移出恢复；
  4. 点击指示点跳转对应幻灯片；
  5. 分页器页码按钮组正确渲染、当前页高亮、点击翻页事件回显、超出显示上限时出现 `…`；
  6. 有主题切换入口时，切换主题后按钮 / 指示点颜色跟随（无则核对 `svg_replace_colors` 配置正确）。
- **文档**：`verify_docs.py` 全量扫描无漂移。
- **回归**：既有控件与示例行为不变。

## 非目标（Out of Scope）
- **列表级"无限滚动"加载**（卡片流滚动到底部自动加载下一页数据）：需与 `VirtualListBox` 数据源回调集成，属于列表容器能力而非轮播能力，本期不做，留待后续（若需要可在 `VirtualListBox` 上补"滚动到底触发"回调）。
- 键盘（方向键 / Home / End）切换幻灯片：本期不做。
- 幻灯片为"任意子控件"（富内容幻灯片，如标题 + 描述叠加）：本期仅支持图片幻灯，富内容幻灯片留待后续。
</content>
</write>
</invoke>

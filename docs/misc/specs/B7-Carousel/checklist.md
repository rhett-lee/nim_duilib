# Carousel / 分页器 控件 Checklist（B7）

## 骨架与注册
- [x] `DUI_CTR_CAROUSEL` 常量存在，且 `<Carousel>` XML 标签可通过 `WindowBuilder` 工厂创建。
- [x] `DUI_CTR_PAGINATION` 常量存在，且 `<Pagination>` XML 标签可通过 `WindowBuilder` 工厂创建。
- [x] `duilib.vcxproj` / `.filters` 正确包含 `Carousel.cpp`/`.h`、`Pagination.cpp`/`.h`，Debug|x64 可编译。
- [x] `examples/controls` 工程正确包含 `CarouselForm.h` / `.cpp`。

## Carousel 资源与主题映射
- [x] `public/carousel/nav_prev.svg` / `nav_next.svg`（含悬停态）存在。
- [x] `public/carousel/indicator_normal.svg` / `indicator_active.svg` 存在。
- [x] 图片使用 `svg_replace_colors` 映射语义色（导航 / 指示点 → `color_accent` 等）。
- [x] `global.xml` 的 `<Class name="carousel">` 可被 `class="carousel"` 引用且生效。
- [x] 控件外观完全来自 SVG 图片，**不含 `IPath` 自绘图形代码**。

## Carousel 数据模型与事件
- [x] 幻灯片增删（`AddItem` / `InsertItem` / `RemoveItem` / `ClearItems` / `SetItems`）正确维护索引。
- [x] `current_index` 始终钳位在 `[0, count-1]`（无幻灯片时为 -1）。
- [x] `loop="true"` 时首尾相接，`Next` 在末尾回到 0、`Prev` 在开头回到末尾。
- [x] 非 loop 时越界 `Next` / `Prev` 返回 false 且不越界。
- [x] 索引变化派发 `kEventValueChanged`，`pEventData` 为 `CarouselValue{index, oldIndex}`；`NoEvent` 变体不派发。

## Carousel 外观 / 动画 / 自动播放
- [x] 当前幻灯片按内容区铺满绘制，缩放无变形。
- [x] `transition_ms > 0` 时切换有水平滑动过渡，`= 0` 时立即切换。
- [x] `auto_play="true"` 按 `interval` 自动前进；`hover_pause="true"` 悬停暂停、移出恢复。
- [x] 定时器在 `SetWindow` 变更 / 析构时被正确清理（无残留回调）。
- [x] `show_indicator` / `show_nav_button` / `indicator_gap` / `indicator_bottom` 生效且随 DPI 缩放。
- [x] 禁用（`IsEnabled()==false`）时置灰且不响应交互。

## Carousel 交互
- [x] 点击"上一张"/"下一张"按钮正确切换。
- [x] 点击指示点跳转到对应幻灯片。
- [x] 按住左右拖拽可切换（松手吸附到目标页），与点击不冲突。
- [x] 悬停时导航按钮显示悬停态；按下显示按下态（未配置时回退）。
- [x] 点击在空白 / 越界区域不产生异常。

## Pagination 数据模型与外观
- [x] `page_count` / `current_page` 生效且 `current_page` 钳位在 `[1, page_count]`。
- [x] `total_count` + `page_size` 能正确推导 `page_count = ceil(total/page_size)`。
- [x] 页码按钮组正确渲染：首尾 + 当前页附近 + 超出 `max_visible_count` 时以 `…` 收纳。
- [x] 当前页按钮呈选中高亮态；`prev/next`（及可选 `first/last`）按钮可见且可用。
- [x] 页码按钮引用 `button_class`，颜色走 `global.xml` 语义色，**C++ 无写死色值**。
- [x] `button_width` / `button_height` / `prev_text` / `next_text` / `ellipsis_text` 生效且随 DPI 缩放。

## Pagination 交互与事件
- [x] 点击页码 / `next` / `prev` / `first` / `last` 正确翻页。
- [x] 页码变化派发 `kEventValueChanged`，`pEventData` 为 `PaginationValue{page, oldPage}`；`SetCurrentPageNoEvent` 不派发。
- [x] `page_count` / `current_page` 变化时按钮组正确重建 / 刷新，无残留旧按钮。
- [x] 越界 `GoTo` / `Next` / `Prev` 行为正确（钳位或按设计返回 false）。

## 演示与示例
- [x] `controls/carousel.xml` 演示窗口可正常打开，含自动轮播 / 手动轮播 / 指示点变体 / 分页器。
- [x] `test.xml` 的 `carousel_btn` 能打开 Carousel 演示窗口。
- [x] `zh_CN-examples.txt` / `en_US-examples.txt` 的 `STRID_CAROUSEL_*` / `STRID_PAGINATION_*` 文案齐全，切换语言正常。

## 编译与运行时
- [x] Debug|x64 编译无错误、无新增警告。
- [ ] 运行时截图核对：手动切换 / 无限循环 / 指示点跳转 / 自动播放与悬停暂停 / 分页器翻页与省略收纳 / 主题色跟随（交互视觉核对由人工手动测试）。

## 文档与收尾
- [x] `docs/Control.md` / `docs/Control.en.md` 均有 Carousel 与 Pagination 小节（属性表 + 说明 + XML 示例 + C++ 接口）。
- [x] 两份 `api-reference.md` 与 `nim-duilib-llm-reference.md` 已同步两个控件。
- [x] `verify_docs.py` 全量扫描无漂移。
- [x] `Project-Audit.md` B7 与 `Development-Plan.md` D-11（Carousel）/ E-4（Pagination）状态已更新。
- [ ] 既有控件与示例无回归（启动冒烟通过）。

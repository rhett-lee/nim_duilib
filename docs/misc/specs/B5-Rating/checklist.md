# Rating 星级评分控件 Checklist

## 骨架与注册
- [x] `DUI_CTR_RATING` 常量存在，且 `<Rating>` XML 标签可通过 `WindowBuilder` 工厂创建。
- [x] `duilib.vcxproj` / `.filters` 正确包含 `Rating.cpp` / `Rating.h`，Debug|x64 可编译。
- [x] `examples/controls` 工程正确包含 `RatingForm.h` / `.cpp`。

## 星形资源与主题映射
- [x] `public/rating/star_filled.svg` / `star_half.svg` / `star_empty.svg` 存在（另含 `star_hovered.svg`）。
- [x] 星图使用 `svg_replace_colors` 映射语义色（实心/半星→`color_accent`，空心→`border_control_normal`）。
- [x] `global.xml` 的 `<Class name="rating">` 可被 `class="rating"` 引用且生效。
- [x] 星形外观完全来自 SVG 图片，**不含 `IPath` 自绘星形代码**。

## 数值模型
- [x] `value` 始终钳位在 `[0, max]`，并按 `allow_half` 对齐到 0.5/1 步长。
- [x] 调小 `max` 时当前 `value` 被同步钳位。
- [x] `SetValue` 值变化时派发 `kEventValueChanged`，`pEventData` 为含新旧值的 `RatingValue`，`SetValueNoEvent` 不派发。
- [x] 别名属性 `star_count` / `readonly` 生效。

## 外观与绘制
- [x] 按 `value`/`max` 正确渲染实心 / 半星 / 空心图片。
- [x] 半星：配置了 `star_half_image` 时用该图；未配置时对实心图做左半裁剪，结果正确。
- [x] `star_size` / `star_gap` 生效且随 DPI 缩放，星图缩放无变形。
- [x] 悬停预览：配置 `star_hovered_image` 时用该图，否则回退实心图。
- [x] 控件禁用（`IsEnabled()==false`）时置灰且不响应交互。
- [x] `ClearImageCache()` 释放四类星图缓存。

## 交互
- [x] 非只读时悬停显示预览，移出恢复为 `value` 对应显示。
- [x] 非只读时点击命中星（含左右半边）正确更新 `value`。
- [x] 按住拖动可连续调分。
- [x] `read_only="true"` 时点击 / 悬停均不改变 `value`。
- [x] 点击在空白/越界区域不产生异常值。

## 演示与示例
- [x] `controls/rating.xml` 演示窗口可正常打开，含只读 / 交互 / 半星 / 事件回显。
- [x] `test.xml` 的 `rating_btn` 能打开 Rating 演示窗口。
- [x] `zh_CN-examples.txt` / `en_US-examples.txt` 的 `STRID_RATING_*` 文案齐全，切换语言正常。

## 编译与运行时
- [x] Debug|x64 编译无错误、无新增警告。
- [ ] 运行时截图核对：只读 4.5 显示 4 实心 + 1 半星。（交互视觉核对由人工手动测试）
- [ ] 运行时截图核对：悬停第 4 星出现预览样式、移出恢复。（交互视觉核对由人工手动测试）
- [ ] 运行时截图核对：点击第 3 星（整数）事件回显 3、显示 3 实心。（交互视觉核对由人工手动测试）
- [ ] 运行时截图核对：半星行点击产生 `x.5`。（交互视觉核对由人工手动测试）
- [ ] 运行时截图核对：星色与 `svg_replace_colors` 语义色一致（若可切主题则验证切换后跟随）。（交互视觉核对由人工手动测试）

## 文档与收尾
- [x] `docs/Control.md` / `docs/Control.en.md` 均有 Rating 小节（属性表 + 说明 + XML 示例 + C++ 接口）。
- [x] 两份 `api-reference.md` 与 `nim-duilib-llm-reference.md` 已同步 Rating。
- [x] `verify_docs.py` 全量扫描无漂移。
- [x] `Project-Audit.md` B5 与 `Development-Plan.md` D-11（Rating）状态已更新。
- [x] 既有控件与示例无回归（启动冒烟通过：`controls_d.exe` 正常加载 `global.xml`/多语言/`test.xml`）。

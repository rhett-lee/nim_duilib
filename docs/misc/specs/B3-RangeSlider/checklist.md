# Checklist: B3 RangeSlider

## 源码
- [x] `RangeSlider.h` 类声明与 API 完整（双值 + 事件 + 图片 + 颜色 + 半径）。
- [x] `RangeSlider.cpp` 覆写方法齐全且 `GetType()` 返回 `DUI_CTR_RANGE_SLIDER`。
- [x] 值钳位：`[min, max]` 且 `low <= high - min_range`。
- [x] 事件仅在实际变化时派发，`pEventData` 携带 `RangeValue`。
- [x] `SetLowValueNoEvent / SetHighValueNoEvent / SetRangeNoEvent` 不派发事件。
- [x] 水平与垂直两种方向的绘制/命中断点均正确。

## 注册
- [x] `duilib_defs.h` 宏。
- [x] `WindowBuilder.cpp` include + 工厂。
- [x] `duilib.vcxproj` / `.filters`。
- [x] `duilib.h` include。

## 示例
- [x] `RangeSliderForm.h/.cpp` 存在且 `GetSkinFolder/GetSkinFile` 正确。
- [x] `rangeslider.xml` 可加载，含水平/垂直示例。
- [x] `controls.xml` 新增按钮 + 首排高度已调整。
- [x] 中英语言字符串均已补齐。
- [x] `controls.vcxproj` / `.filters` 已登记。
- [x] `ControlForm` 按钮绑定与 `ShowRangeSliderWindow()` 实现。

## 验证
- [x] Debug|x64 `/clp:ErrorsOnly` 无错误。
- [x] 运行截图：双 thumb + 高亮区间显示正确。
- [x] 拖动与按钮交互后回调标签正确更新。
- [x] `verify_docs.py` 无漂移。
- [x] Project-Audit B3 已标记 ✅。

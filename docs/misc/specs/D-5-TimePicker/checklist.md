# D-5 TimePicker 自检清单

## 功能

- [x] `date_time_calendar`：日历 + 时/分（可选秒）+ 取消/确定。
- [x] `time_calendar`：仅时间选择，日期部分保持原值。
- [x] 日期+时间模式下点日期不关闭、不提交。
- [x] 取消 / Esc / 外部点击均不提交。
- [x] 旧 `date_calendar` 点日期即提交并关闭。

## 约束

- [x] `min_date` / `max_date` 越界日期置灰且不可点击。
- [x] `disabled_dates` 单日与区间均置灰且不可点击。
- [x] 键盘方向键/翻页后焦点不落在禁用日期。
- [x] 键盘 Enter/Space 无法选中禁用日期。
- [x] `first_day_of_week` 影响星期标题与网格起点。
- [x] `show_seconds=false` 时弹层无秒列，文本格式无秒。

## 边界

- [x] 时间模式下 `Calendar` 为空时 `InitControls` / `OnKeyDownMsg` 不崩溃。
- [x] `mktime` 返回 -1 时不提交非法值。
- [x] 浮层回调中 `Dismiss()` 后不再访问已销毁控件。
- [x] 重复打开/快速关闭无悬空指针。
- [x] 回调列表遍历使用副本，允许回调内增删监听。

## 文档/资源

- [x] 中英文公共资源、示例资源键值对齐。
- [x] `Control.md` / `Control.en.md` 属性与 API 一致。
- [x] 规划文档状态与技能速查已更新。
- [x] `verify_docs.py --extra docs` 零漂移。

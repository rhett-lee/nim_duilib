# D-3 SpinBox 支持小数与格式化

## 一、目标

`ui::SpinBox` 当前只支持 `int64_t`，无法表达小数、千分位分隔符和前后缀，与 Qt `QDoubleSpinBox` 有明显差距。
本项在**不破坏现有整数用法**的前提下，为 SpinBox 增加：

1. 小数位数（`decimals`）与 `double` 数值/步长/范围；
2. 千分位分隔符（`group_separator`）；
3. 前后缀（`prefix` / `suffix`）；
4. 失焦时按格式规范化文本，非法输入回退到最近一次有效值。

完成后回写：
- `docs/misc/Development-Plan.md` D-3 → 已完成
- `docs/misc/Control-Roadmap.md` / `.en.md` 第 3 项补充小数与格式化能力

## 二、需求要点

- 数值、步长、范围支持小数；`decimals=0` 时行为与现有整数 SpinBox 完全一致。
- 显示格式：`前缀 + 千分位整数 + 小数部分 + 后缀`，例如 `$ 1,234.50 kg`。
- 步进按钮、上下方向键按小数步长调整。
- 用户编辑后失焦时重新格式化；无法解析的文本恢复为上次有效值。
- 多语言/主题/DPI 相关：前缀后缀走普通字符串（可由调用方传入多语言文本），不新增 `*_id` 属性。
- 事件沿用 `kEventTextChanged`，不新增事件类型。

## 三、设计决策

### 3.1 不修改 RichEdit 的数值模型

RichEdit 的数字能力是 `int64_t` + `number_only` 输入过滤，直接改成 `double` 会影响 DateTime、IPAddress 等现有用法。
因此小数与格式化逻辑全部放在 `SpinBox` 内，RichEdit 仅做一处最小改动：

- 将 `RichEdit::AdjustTextNumber(int32_t)`（`RichEdit2.h` / `RichEdit_Windows.h` 两套实现）
  由普通成员函数改为 `virtual`。
- 步进按钮与自动重复定时器原本就调用 `AdjustTextNumber`，改虚后自动派发到 `SpinBox` 的重写实现。
- 两套实现均不做其它改动，整数 RichEdit 行为不变。

### 3.2 输入过滤

- `decimals == 0` 且无前后缀/千分位：保持 `number_only=true`，与旧行为一致。
- 否则：`number_only=false`，设置 `limit_chars` 为
  `0-9 + - . + 千分位分隔符 + 前缀字符 + 后缀字符`，允许输入/粘贴格式文本。
- 若调用方通过 XML/C++ 自行设置了 `limit_chars`，则以调用方的为准，不再自动覆盖。
- 位置合法性（多个小数点、多个负号等）在失焦提交时统一校验，不逐字符拦截。

### 3.3 值模型

- `m_value` 保存最近一次有效 `double` 值。
- `GetValueDouble()` 优先解析当前文本；解析失败返回 `m_value`。
- `SetValueDouble(v)`：钳位到 `[min, max]` → 按 `decimals` 四舍五入 → 格式化为文本 → 更新 `m_value`。
- `GetValue()` 保留 `int64_t` 返回类型，返回 `llround(GetValueDouble())`；小数精确值用 `GetValueDouble()`。
- 范围默认 `[-DBL_MAX, DBL_MAX]`；`min_number` / `max_number` 在 SpinBox 内按 `double` 解析，并支持
  `min_value` / `max_value` 别名。

### 3.4 格式化规则

- `decimals` 固定小数位，不足补 0，超出四舍五入；`0` 时不输出小数点。
- 千分位仅作用于整数部分，从右向左每 3 位插入一次；负号不参与分组。
- `prefix` / `suffix` 原样拼接，不自动补空格（需要空格由调用方写入）。
- `-0.0` 归一化为 `0.0`。

### 3.5 解析规则

1. 去除首尾空白；
2. 若文本以 `prefix` 开头则去掉，以 `suffix` 结尾则去掉（允许用户删掉前后缀）；
3. 删除全部千分位分隔符；
4. 校验 `[+-]?数字[.数字]` 形式（至少一位数字、至多一个小数点）；
5. 转 `double`，非有限值视为非法。

非法文本在失焦时回退到 `m_value` 并重新格式化。

## 四、精确改动清单

### 4.1 控件源码

- `duilib/Control/RichEdit2.h`：`AdjustTextNumber` 增加 `virtual`。
- `duilib/Control/RichEdit_Windows.h`：同上。
- `duilib/Control/SpinBox.h`：
  - 新增 `SetStep(double)` / `GetStepDouble()`、`SetValue(double)` / `GetValueDouble()`、
    `SetRange(double, double)`、`GetMinValue()` / `GetMaxValue()`；
  - 新增 `SetDecimals` / `GetDecimals`、`SetPrefix` / `GetPrefix`、`SetSuffix` / `GetSuffix`、
    `SetGroupSeparator` / `GetGroupSeparator`；
  - 重写 `SetAttribute`、`AdjustTextNumber`、`OnKeyDown`、`OnKillFocus`。
- `duilib/Control/SpinBox.cpp`：实现上述接口与格式化/解析/输入过滤。

### 4.2 XML 属性

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| decimals | 0 | int(0~10) | 小数位数 |
| step | 1 | double | 步长，必须 > 0 |
| value | 0 | double | 初始值 |
| min_number / max_number | 无限制 | double | 数值范围（同 min_value / max_value） |
| prefix | "" | string | 前缀 |
| suffix | "" | string | 后缀 |
| group_separator | "" | string | 千分位分隔符（别名 thousand_separator） |

### 4.3 示例与语言

- `bin/resources/themes/default/rich_edit/rich_edit.xml`：新增“小数与格式化”SpinBox 行。
- `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`：新增两条 `STRID_RICHEDIT_SPINBOX_DECIMAL*`。

### 4.4 文档

- `docs/Control.md` / `docs/Control.en.md`：属性表、C++ 接口、示例。
- `docs/misc/Control-Roadmap.md` / `.en.md`：第 3 项补充能力说明。
- `docs/misc/Development-Plan.md`：D-3 状态与跟踪项。
- `.trae` / `.workbuddy` 技能 `api-reference.md` 与 `.claude/docs/nim-duilib-llm-reference.md`：SpinBox 速查同步。

## 五、验证步骤

1. `MSBuild build\examples.sln /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1` 零错误。
2. 运行 `bin\rich_edit.exe`：
   - 步进按钮与上下方向键按 0.25 调整；
   - 显示为 `$ 1,234.50 kg`；
   - 输入 `abc` 后失焦恢复原值；
   - 切换英文/中文、浅色/深色、100%/150% DPI 显示正常。
3. `python .trae/skills/nim-duilib/scripts/verify_docs.py --extra docs` 零漂移。

## 六、验收标准

- [ ] `decimals=0` 的旧 XML 行为、事件、范围限制与修改前一致。
- [ ] 小数步长、双击/长按步进按钮、上下方向键均按 `step` 调整。
- [ ] 千分位、前后缀显示与解析正确，负数和 0 正确。
- [ ] 非法输入失焦后回退，不产生崩溃或空值。
- [ ] 中英文文档与示例同步，文档校验通过。

## 七、待确认

- 事件：本项沿用 `kEventTextChanged`，不新增 `kEventValueChanged`（避免破坏现有文档语义）。
- 范围回绕：不实现“到达边界后跳到另一端”，统一钳位，保持跨平台一致。

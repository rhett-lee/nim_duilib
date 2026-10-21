# 给框架控件扩展新属性（以 Panel 的 title_id 为例）

本文沉淀"给一个已有框架控件新增 XML 属性"的完整流程，以及中途会踩到的硬坑。
以本次给 `PanelTemplate` 新增 `title_id`（标题多语言 ID）为实例，但流程对其它模板类控件通用。

## 一、何时需要看这篇

你要做的事情符合下面任一条件时，按本文走，不要直接动手：

- 给某个已有控件的 `.h`/`.cpp` 增加一个新的 XML 属性（如 `title_id`、`xxx_image`）
- 给模板类（`PanelTemplate`、`BoxTemplate` 等）增加成员变量或虚函数重写
- 改动 `duilib/` 下任意头文件，并期望 `examples/` 里的示例立即用上

## 二、完整流程（五阶段）

### 阶段 1：框架代码（`duilib/` 下）

本次目标：让 `Panel/PanelVBox/PanelHBox` 支持 `title_id` 属性，切换语言时标题自动刷新。

**1. 找参照实现**——多语言属性的范式看 `Label`（`duilib/Control/LabelImpl.cpp`）：
- 成员 `UiString m_sTextId`
- `SetTextId()` 存 ID，`GetText()` 在 `m_sText` 为空时回退到 `GlobalManager::GetStringByID(m_sTextId)`
- XML 属性名 `text_id` / `textid`
- 重写 `OnLanguageChanged()` 刷新文本

**2. 在目标头文件加成员**（`duilib/Box/Panel.h`）：

```cpp
// private 区，紧跟 m_title 之后
UiString m_titleId;
```

**3. 加 public 接口**（`SetTitleId` / `GetTitleId`）：

```cpp
void SetTitleId(const DString& strTitleId)
{
    if (m_titleId == strTitleId) {
        return;
    }
    m_titleId = strTitleId;
    //立即按当前语言解析并显示；找不到该ID时保留原标题
    DString strTitle = GlobalManager::GetTextById(strTitleId);
    if (!strTitle.empty()) {
        SetTitle(strTitle);
    }
}

DString GetTitleId() const
{
    return m_titleId.c_str();
}
```

**4. 在 `SetAttribute` 里加分支**（紧跟 `title` 分支之后）：

```cpp
else if ((strName == _T("title_id")) || (strName == _T("titleid"))) {
    SetTitleId(strValue);
}
```

**5. 重写 `OnLanguageChanged`**：

```cpp
virtual void OnLanguageChanged(bool bRedraw) override;   // 类声明里
```

```cpp
template<typename InheritType>
void PanelTemplate<InheritType>::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);   // 先刷新基类（含子控件 text_id）
    if (!m_titleId.empty()) {
        DString strTitle = GlobalManager::GetTextById(m_titleId.c_str());
        if (!strTitle.empty()) {
            SetTitle(strTitle);
        }
    }
}
```

> `BaseClass` typedef 为 `InheritType`（Box/HBox/VBox），本文件已有 `BaseClass::SetAttribute` 先例，可直接用。

**6. 更新类头注释的 XML 属性列表**——在 `title` 之后加一行 `title_id` 说明。

### 阶段 2：示例（`examples/<name>/`）

**XML**：把所有静态 `title="中文"` 换成 `title_id="STRID_PANEL_TITLE_*"`，同时删掉仅为 C++ 切换语言而加的冗余 `name`（真正被 `FindControl` 引用的 name 必须保留）。

**C++**：
- 删除"控件名 → 语言 ID"的静态映射表与 `ApplyLocalizedPanelTitles()` 函数——标题已由框架自动刷新
- 窗口的 `OnLanguageChanged()` 只保留**动态文案**处理：如果业务用 `SetTitle()` 写了带参数的格式化串（如 `"...(Modified %d Times)"`），需在 `BaseClass::OnLanguageChanged()` 之后重新 `SetTitle()` 一次
- 删 `OnInitWindow` 末尾的手动刷新调用

### 阶段 3：语言资源（`bin/resources/lang/`）

在 `zh_CN-examples.txt` 与 `en_US-examples.txt` 里补齐所有 `STRID_*` 键。
两文件均为 UTF-8 无 BOM。格式：`键=值`。

### 阶段 4：文档同步

| 文档 | 改什么 |
|------|--------|
| `docs/Box.md` + `docs/Box.en.md` | 属性表新增 `title_id` 行；补充说明加一条多语言要点；C++ 接口列表补 `SetTitleId`/`GetTitleId` |
| `.trae/skills/nim-duilib/references/api-reference.md` | 同上（属性表 + 要点 + C++ 接口） |
| `.workbuddy/skills/nim-duilib/references/api-reference.md` | 同上（workbuddy 副本） |
| `.claude/docs/nim-duilib-llm-reference.md` | 同上（claude 副本） |

三处技能参考内容同源，改一处后用相同文本覆盖另外两处，避免漂移。

### 阶段 5：构建与验证（**最关键，见下一节硬坑**）

## 三、硬坑：改了头文件模板类，必须重建 duilib 库

### 症状

改完 `Panel.h` 后只执行：

```
cmake --build build/build_temp/msvc/panel --config Release --target panel
```

编译零错误。但运行 `panel.exe` 时：
- 窗口从未显示，约 5 秒后进程崩溃
- 异常码 `0xC0000005`（访问冲突）或 `0xC000041D`（未处理异常）
- 崩溃发生在 XML 解析 / 窗口创建阶段

### 根因

`PanelTemplate<Box/HBox/VBox>` 是头文件里的模板类，**在 `duilib/Core/WindowBuilder.cpp:180-182` 中被实例化并注册到控件工厂**：

```cpp
{DUI_CTR_PANEL,      [](Window* pWindow) { return new Panel(pWindow); }},
{DUI_CTR_PANEL_HBOX, [](Window* pWindow) { return new PanelHBox(pWindow); }},
{DUI_CTR_PANEL_VBOX, [](Window* pWindow) { return new PanelVBox(pWindow); }},
```

这段代码编译进了 `duilib.lib`。给 `PanelTemplate` 新增成员 `m_titleId` 改变了对象内存布局。

只重编示例时：示例代码用**新布局**读写对象，但链接的 `duilib.lib` 里 `WindowBuilder` 仍按**旧布局** `new` 对象。两者读写同一内存的偏移不一致 → 越界访问 → 崩溃。这是典型的 ODR（One Definition Rule）冲突。

### 正确做法

**先重建 duilib 库，再重建示例**：

```bash
# 1. 重建库（WindowBuilder.cpp 会重新编译，生成新布局的 duilib.lib）
cmake --build build/build_temp/msvc/duilib --config Release --target duilib

# 2. 再重建示例（链接新库）
cmake --build build/build_temp/msvc/panel --config Release --target panel
```

### 判断方法

只要你改了 `duilib/` 下任意被 `.cpp` 包含的头文件（尤其是模板类），就必须重建 `duilib` target。判断某个头文件是否在库内被实例化：

```bash
grep -rn "PanelHBox\|PanelVBox" duilib/*.cpp duilib/**/*.cpp
```

有命中就说明库内有实例化，改头文件后必须 Rebuild 库。

## 四、验证清单

| 验证项 | 通过标准 |
|--------|----------|
| 启动不崩溃 | 窗口正常显示，无 0xC0000005 |
| 静态标题多语言 | 中→英→中 切换，所有面板标题自动跟随，无 C++ 映射表 |
| 动态标题多语言 | `SetTitle()` 改过的动态文案在语言切换后由窗口 `OnLanguageChanged` 重新套用，计数/参数保留 |
| 默认态回退 | 未设置 `title_id` 或语言文件缺 ID 时，显示 `title` 属性的原始值 |
| 文档一致 | `docs/Box.md` 与 `api-reference.md` 三处副本的属性表、C++ 接口列表一致 |
| 无冗余 name | XML 中仅保留被 `FindControl` 引用的控件 name |

## 五、对照源码的权威源

| 要确认的东西 | 权威源 |
|------|------|
| 控件是否在库内被实例化 | `duilib/Core/WindowBuilder.cpp` 的 `kControlCreateInfoMap` |
| 多语言取文案静态 API | `GlobalManager::GetTextById()`（`duilib/Core/GlobalManager.h`） |
| 控件语言刷新机制 | `Control::OnLanguageChanged()`（`duilib/Core/Control.cpp`） |
| Label 多语言范式 | `duilib/Control/LabelImpl.cpp` 的 `SetTextId` / `OnLanguageChanged` |
| 语言文件格式 | `bin/resources/lang/zh_CN-examples.txt` |

## 六、扩展已有控件的数值 / 格式化能力（D-3 SpinBox 实例）

以 `D-3 SpinBox 支持小数与格式化`（spec：`docs/misc/specs/D-3-SpinBox-Decimal/`）为实例。
适用场景：已有控件只支持整数/单一格式，需要补小数、步长、范围、前后缀、千分位，且**不能破坏旧行为**。

### 6.1 先定边界：在子类扩展，不动基类的数据模型

SpinBox 继承 RichEdit，原有数值能力是 `int64_t` + `number_only`。正确做法：

- 基类（RichEdit）**不引入 double**，只做一处最小改动（见 6.2）；
- 所有小数、格式化、输入过滤、失焦规范化逻辑放在 SpinBox；
- 旧的 `SetValue(int64_t)` / `SetRange(int64_t,int64_t)` / `SetStep(int32_t)` 保留为重载，内部转 double，源码兼容。

判断原则：新能力只服务子类语义时，不要污染基类的成员与 API。

### 6.2 关键技巧：把基类回调改 `virtual`，而不是重绑内部按钮

问题：SpinBox 的步进按钮与自动重复定时器由基类 RichEdit 创建，回调写死调用
`RichEdit::AdjustTextNumber(nDelta)`。子类无法拦截。

做法：

```cpp
// RichEdit2.h / RichEdit_Windows.h（两套实现都要改）
virtual void AdjustTextNumber(int32_t nDelta);

// SpinBox.h
virtual void AdjustTextNumber(int32_t nDelta) override;
```

调用点是"通过成员函数指针调用"，**虚函数经指针到成员调用仍然虚派发**，因此基类创建的按钮、
定时器无需任何改动就会进入 SpinBox 的重写。重写内只取方向、乘以自己的 `m_step`：

```cpp
if ((nDelta == 0) || !IsEnabled() || IsReadOnly()) {
    return;
}
AdjustValue((nDelta > 0) ? m_step : -m_step);
```

要避免的做法：

- 在子类里 `FindControl` 找基类内部按钮再重绑事件——依赖内部实现，基类一改布局就失效；
- 复制基类的按钮创建/自动重复代码——两份维护，行为容易分叉。

### 6.3 双模式输入过滤，保证旧行为不变

整数模式与小数/格式化模式的输入限制不同，切换规则：

```text
整数模式 = decimals == 0 且 group_separator / prefix / suffix 均为空
  → SetNumberOnly(true)，SetLimitChars("")，完全沿用旧行为
小数/格式化模式
  → SetNumberOnly(false)，SetLimitChars("0123456789+-" + '.' + 分隔符 + 前后缀字符)
```

- 每次 `decimals` / `prefix` / `suffix` / `group_separator` 变化都要重新计算过滤（`UpdateInputFilter()`）；
- 调用方在 XML 显式写了 `limit_chars` 时置 `m_bCustomLimitChars = true`，**不再自动覆盖**；
- 注意 `SetNumberOnly(false)` 后基类不再拦非法字符，必须用 `limit_chars` 兜底。

### 6.4 解析与格式化必须成对实现

- 单一有效值 `m_value`；`GetValueDouble()` 先尝试解析当前文本，失败才回退 `m_value`。
- 格式化：`StringUtil::Printf(_T("%.*f"), decimals, value)`，再对整数部分每 3 位插分隔符
  （**正负号不参与分组**），最后拼 `prefix` + 数字 + `suffix`。
- 解析：Trim → 去前后缀 → 删分隔符 → 按 `[+-]?数字[.数字]` 逐字符校验 → `StringToDouble` → `isfinite`。
  不要直接把用户文本丢给 `strtod`，否则 `1e3`、`0x10`、`abc` 都可能被意外接受。
- `-0.0` 归一化为 `0.0`（避免显示 `-0`）；非有限值 `ASSERT` 并拒绝。
- 失焦（`OnKillFocus`）：可解析 → `SetValue(parsed)` 重新格式化；不可解析 → 恢复最近有效值；
  **空文本保持为空**，不要强制填 0。
- 范围钳位与小数舍入在 `SetValue(double)` 内统一完成，所有入口（API、XML、按钮、键盘）共享同一路径。

### 6.5 XML 属性按声明顺序生效（本次踩到的坑）

`SetAttribute` 是逐条按 XML 中书写顺序调用的。若写成：

```xml
<SpinBox value="1234.5" decimals="2" .../>   <!-- 错误：value 先按 0 位小数舍入成 1235 -->
```

正确顺序是**先格式、后数值**：

```xml
<SpinBox decimals="2" value="1234.5" .../>
```

约定：

- `decimals`、`prefix`、`suffix`、`group_separator` 等格式类属性写在 `value` 之前；
- 同类问题适用于一切"数据依赖默认配置"的属性组合；
- 示例 XML、`Control.md` / `Control.en.md` 的示例都要按同一顺序书写，并在文档里点明"按声明顺序生效"。

### 6.6 double 步长与基类 int 步长的同步

基类 `SetSpinStep` 仍是 `int32_t`（按钮增量只用于判方向）。子类：

- 保存 `double m_step`；
- `SetStep(double)` 时同步整数步长：`round(step)` 落在 `[1, INT32_MAX]` 就用它，否则用 1，
  避免 double 直接转 int32 的溢出 UB；
- 重写 `AdjustTextNumber` 时**忽略增量大小**，只按方向使用 `m_step`。

### 6.7 示例与双语资源

- 示例放在该控件既有示例文件中（本次 `bin/resources/themes/default/rich_edit/rich_edit.xml`），
  紧跟同类示例行，便于对比；
- 新增 `STRID_*` 在 `zh_CN-examples.txt` / `en_US-examples.txt` 成对补齐；
- 示例属性顺序遵守 6.5。

### 6.8 文档同步清单（本次实际改动）

| 文档 | 改什么 |
|------|--------|
| `docs/Control.md` / `docs/Control.en.md` | 控件小节重写：支持模式、属性表（含类型/默认值/别名）、C++ 接口、两个 XML 示例、`limit_chars` 注意事项 |
| `docs/misc/Control-Roadmap.md` / `.en.md` | 对应能力项补充本次增强说明 |
| `docs/misc/Development-Plan.md` | 条目标 ✅ + 完成日期、跟踪项勾选、速览计数与剩余项同步 |
| 三份技能参考（`.trae` / `.workbuddy` / `.claude`） | 速查表与 C++ 接口同步 |
| spec 三件套 | `spec.md` / `tasks.md` / `checklist.md`，完成回勾 |

### 6.9 验证清单

| 验证项 | 通过标准 |
|--------|----------|
| 编译 | `build\examples.sln` Debug\|x64 零错误 |
| 启动 | 示例程序正常打开，新 XML 示例行可见、无解析崩溃 |
| 旧行为 | `decimals=0` 且无前后缀/千分位时，输入过滤、范围、步进与改动前一致 |
| 步进 | 步进按钮、自动重复、上下方向键都按 `step` 调整（小数步长累计不丢精度） |
| 格式化 | 失焦后显示 `prefix + 千分位数字 + suffix`；`-0.0` 显示 `0` |
| 解析 | 用户删掉前后缀/分隔符后可重新格式化；非法文本失焦回退最近有效值；空文本保持空 |
| 兼容 | 自定义 `limit_chars` 不被覆盖 |
| 外观 | 浅色/深色、中文/英文、100%/150% DPI 下显示正常 |
| 文档 | `verify_docs.py --extra docs` 零漂移 |

### 6.10 本次坑表

| 现象 | 根因 | 处理 |
|------|------|------|
| 小数示例显示成整数 | `value` 写在 `decimals` 之前，先按 0 位小数舍入 | 格式属性写在 `value` 之前 |
| 子类重写步进无效 | 基类回调非虚，按钮/定时器直接调用基类实现 | 基类回调改 `virtual`，子类 `override` |
| 整数模式行为被改变 | 无条件关掉 `number_only` | 用"整数模式/格式化模式"双分支 |
| 输入框能输入 `e`/`x` 等字符 | 仅靠 `limit_chars` 未收紧或未更新 | 每次格式属性变化重建 `limit_chars` |
| 用户删除千分位后无法提交 | 解析未先删除分隔符/前后缀 | 解析顺序：去前后缀 → 删分隔符 → 校验 |
| `GetSpinStep()` 值突变 | `SetStep(double)` 未同步整数步长 | 按 `round(step)` 同步并做溢出保护 |

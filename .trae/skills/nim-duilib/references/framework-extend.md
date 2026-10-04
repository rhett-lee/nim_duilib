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

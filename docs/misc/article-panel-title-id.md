# nim_duilib界面库，手把手教学：给 C++ 桌面 UI 控件加上多语言支持

> 30 分钟，让你的 UI 框架控件像 Label 一样支持 `text_id` 式的语言 ID 属性：XML 里写一行，切换语言全自动刷新。本文以 nim_duilib 框架的 Panel 控件新增 `title_id` 为例，步骤可直接套用到任何自定义控件。

**适合读者**：用 C++ 写桌面客户端、需要做多语言（i18n）的开发者；正在用 nim_duilib 框架的同学直接照抄即可。

**你将得到**：

- 一套可复用的"控件多语言属性"实现模板
- 一份从框架代码到构建验证的完整 Checklist
- 一个必须避开的构建坑（最后一步会讲）

---

## 一、先看效果

改动前，XML 里标题只能写死一种语言：

```xml
<PanelVBox title="基础面板" ...>
```

改动后，标题引用语言 ID，切换语言时自动刷新：

```xml
<PanelVBox title_id="STRID_PANEL_TITLE_BASIC" ...>
```

运行时在菜单里点一下"English"，界面上所有 Panel 标题瞬间变成英文，**不需要重启、不需要任何业务代码参与**。

## 二、实现原理：30 秒看懂

框架里 Label 控件早就有 `text_id` 属性，它的套路只有四步：

1. 控件里存一个 ID 字符串成员（如 `m_sTextId`）
2. 设置 ID 时，立刻从语言文件查出当前语言的文案并显示
3. XML 解析时把 `text_id` 属性映射到第 2 步的接口
4. 重写 `OnLanguageChanged()`，切换语言时重新查一遍文案

我们要做的就是把这套范式搬到 Panel 上。**找到现成的参照实现照抄，是框架二次开发最稳的姿势。**

## 三、动手：五步实现 title_id

### Step 1：加成员变量和接口

打开 `duilib/Box/Panel.h`，在 `PanelTemplate` 类中：

```cpp
// private 成员区，紧跟 m_title
UiString m_titleId;
```

public 区加两个接口：

```cpp
void SetTitleId(const DString& strTitleId)
{
    if (m_titleId == strTitleId) {
        return;
    }
    m_titleId = strTitleId;
    // 立即按当前语言解析并显示；找不到该 ID 时保留原标题
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

> **细节 1**：`GetTextById` 查不到 ID 时返回空串，此时**不要清空原标题**——给用户一个兜底显示（通常是开发期写的中文），比显示空白友好得多。

### Step 2：注册 XML 属性

在 `SetAttribute` 的 `title` 分支后面追加：

```cpp
else if ((strName == _T("title_id")) || (strName == _T("titleid"))) {
    SetTitleId(strValue);
}
```

> **细节 2**：保留一个无下划线的别名（`titleid`），和框架里 `text_id`/`textid` 的既有习惯保持一致。

### Step 3：重写语言切换回调

```cpp
template<typename InheritType>
void PanelTemplate<InheritType>::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);   // 先让基类刷新（含子控件的 text_id）
    if (!m_titleId.empty()) {
        DString strTitle = GlobalManager::GetTextById(m_titleId.c_str());
        if (!strTitle.empty()) {
            SetTitle(strTitle);
        }
    }
}
```

> **细节 3**：`BaseClass::OnLanguageChanged` 必须先调。框架在切换语言时会递归调用整棵控件树的 `OnLanguageChanged`，你的重写只是补自己负责的那部分文案。

框架层到此完成，三个函数加起来不到 40 行。

### Step 4：写语言资源文件

在语言文件里补键值对。nim_duilib 的语言文件在 `bin/resources/lang/` 下，UTF-8 无 BOM，格式就是 `键=值`：

```ini
; zh_CN-examples.txt
STRID_PANEL_TITLE_BASIC=基础面板（静态）

; en_US-examples.txt
STRID_PANEL_TITLE_BASIC=Basic Panel (Static)
```

### Step 5：XML 里用起来

```xml
<PanelVBox title_id="STRID_PANEL_TITLE_BASIC"
           title_height="32" title_bk_color="bg_header"
           collapsible="true">
    <!-- 内容 -->
</PanelVBox>
```

顺手把原来为了"语言切换时在 C++ 里重设标题"而维护的控件映射表、手动刷新函数全部删掉——框架现在自己干了。

## 四、特殊情况：带参数的动态文案

有一种文案没法用单一语言 ID 表达——**带运行时参数的格式化串**。比如一个"修改标题"按钮，每点一次标题变成 `已修改 3 次` / `Modified 3 Times`，计数是变量。

这种场景的正确姿势：

1. 点击时照常 `SetTitle(StringUtil::Printf(格式串, count))`
2. 在**窗口**的 `OnLanguageChanged()` 里补一次重设：

```cpp
bool PanelForm::OnLanguageChanged()
{
    bool bRet = BaseClass::OnLanguageChanged();  // 框架先自动刷新所有 title_id
    // 动态文案无法走 title_id，在自动刷新之后重新套用
    if ((m_pCppPanel != nullptr) && (m_nCppTitleModified > 0)) {
        m_pCppPanel->SetTitle(ui::StringUtil::Printf(
            ui::GlobalManager::GetTextById(_T("STRID_TITLE_MODIFIED_FMT")).c_str(),
            m_nCppTitleModified));
    }
    return bRet;
}
```

> **顺序是重点**：必须放在 `BaseClass::OnLanguageChanged()` **之后**。框架的自动刷新会用 `title_id` 的默认文案把你 SetTitle 的动态文本覆盖掉，后写者胜出。

静态文案交给 `title_id`，动态文案留在窗口回调里——这就是分工。

## 五、构建时的坑：一定要重建框架库

改完代码，如果你只重编了示例工程：

```bash
cmake --build build/build_temp/msvc/panel --config Release --target panel
```

编译**零错误零警告**，但运行起来窗口闪一下就消失，事件查看器里是 `0xC0000005` 访问冲突。

**原因**：`PanelTemplate` 是头文件里的模板类，但它已经在框架库内部被实例化了——控件工厂表（`WindowBuilder.cpp`）里有 `new PanelVBox(...)`，这些代码编译进了 `duilib.lib`。你给类加了成员变量，对象内存布局变了：

- 示例代码按**新布局**读写对象
- 旧 `duilib.lib` 里的工厂按**旧布局**创建对象

两边对同一块内存的理解不一样，越界访问，崩。这就是 ODR（单一定义规则）冲突，编译器完全不会提示。

**修复只需两步**：

```bash
# 先重建框架库
cmake --build build/build_temp/msvc/duilib --config Release --target duilib
# 再重建示例
cmake --build build/build_temp/msvc/panel --config Release --target panel
```

**自查方法**：改了 `duilib/` 下任何头文件后，grep 一下这个类是否在库源码里被实例化：

```bash
grep -rn "PanelVBox\|PanelHBox" duilib/ --include="*.cpp"
```

有命中 → 必须先 Rebuild 库。

## 六、验收 Checklist

发布前对照过一遍：

- [ ] 切换语言后所有静态标题自动跟随，无任何 C++ 干预
- [ ] 动态文案切换语言后参数（计数等）保留，且双向切换可逆
- [ ] 语言文件里故意删掉某个 ID，界面显示 `title` 属性的兜底值而不是空白
- [ ] 中英文档、接口列表同步更新（属性表加一行，C++ 接口补两个）
- [ ] 改了头文件 → 框架库和示例都已重编

## 七、总结

给 UI 控件加多语言支持，核心就四步：**存 ID → 设 ID 时查文案 → XML 属性映射 → OnLanguageChanged 重查**。套路固定，照抄 Label 即可，40 行代码搞定。

真正值钱的经验在最后：**框架头文件改动必须连带重建框架库**，否则 ODR 冲突会让你面对一个编译完美、启动即崩的程序怀疑人生。

---

**相关链接**

- nim_duilib 仓库：https://github.com/rhett-lee/nim_duilib
- 本文示例代码见仓库 `examples/panel/` 目录

如果这篇教程帮到你，欢迎点赞收藏转发，也欢迎在评论区聊聊你在桌面端多语言上踩过的坑。

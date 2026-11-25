# PropertyGrid 控件代码审查报告

- 审查对象：`duilib/Control/PropertyGrid.h`（2257 行）、`duilib/Control/PropertyGrid.cpp`（3595 行）
- 关联资源：`bin/resources/themes/default/public/property_grid/property_grid.xml`、`color_combox.xml`、`bin/resources/themes/default/global.xml`
- 示例入口：`examples/render/RenderForm.cpp`（`TestPropertyGrid`）、`bin/resources/themes/default/render/page_property_grid.xml`
- 审查方式：静态代码走查 + 界面截图对照（未修改任何代码）

> **状态更新（2026-11-25）**：本报告第一至第三批修复项已实施，详见文末「九、修复实施记录」。
> 各章节条目已标注 ✅（已修复）/ ⏳（暂未实施），原始问题描述保留以便回溯。

总体结论：控件主体结构（TreeView + 分组节点 + 双列 LabelBox + 就地编辑控件）设计合理，多语言（文本 / 文本 ID 双通路）考虑得比较完整。审查时存在 **5 个确定性 Bug（含 1 个可能崩溃）**、一批"编辑控件创建时机导致的设置丢失"问题、绘制每帧 O(N) 且网格线压在子控件之上，以及一些明显的视觉一致性问题（选中态缺失、焦点框错位、表头居中、分组层级弱、颜色属性无色块）。**上述问题现已全部修复**，仅剩单元测试与构建环境的 DLL/运行库事项。

---

## 一、确定性 Bug

### P0-1 分组描述按 ID 取值时用错成员 ✅ 已修复

`PropertyGridGroup::GetDescription()` 回退分支传的是 `m_description` 而不是 `m_descriptionId`：

```cpp
DString PropertyGridGroup::GetDescription() const
{
    if (!m_description.empty()) {
        return m_description.c_str();
    }
    else if (!m_descriptionId.empty()) {
        return GlobalManager::GetTextById(m_description.c_str()); // ← 应为 m_descriptionId
    }
    return DString();
}
```

- 位置：`duilib/Control/PropertyGrid.cpp` L1297-L1306
- 影响：用 `AddGroupById()` 创建的分组，描述区永远空白；切换语言也无法得到描述。
- 修复：改为 `GetTextById(m_descriptionId.c_str())`。
- **已修复**：`PropertyGridGroup::GetDescription()` 回退分支已改为 `GetTextById(m_descriptionId.c_str())`；`SetDescription()` / `SetDescriptionId()` 之后追加 `UpdateDescriptionArea()`，分组描述即时刷新。

### P0-2 `SetPropertyValueId()` 把"文本 ID"当成"文本"显示 ✅ 已修复

```cpp
void PropertyGridProperty::SetPropertyValueId(const DString& propertyValueId)
{
    if (m_propertyValueId != propertyValueId) {
        m_propertyValueId = propertyValueId;
        if (m_pLabelBoxRight != nullptr) {
            m_pLabelBoxRight->SetText(propertyValueId); // ← 应为 SetTextId
        }
        OnPropertyValueChanged();
    }
}
```

- 位置：L1567-L1576
- 影响：属性值会直接显示 `STRID_xxx` 这类资源 ID。
- 修复：`m_pLabelBoxRight->SetTextId(propertyValueId);`，并参考 `SetPropertyText()` 的做法清掉另一个方向的残留（设置 ID 时应清空字面文本，避免 `LabelImpl::GetText()` 中"非空字面文本优先"导致 ID 不生效）。
- **已修复**：`SetPropertyValueId()` 改为 `SetTextId(propertyValueId)` + `SetText("")`，同时清除 `m_propertyValue` 并恢复正常字体（不再显示为"已修改"）；`SetPropertyValue()` 对称地清除 `m_propertyValueId` 并把文本 ID 置空。

### P0-3 颜色选择器"取消/关闭"分支恢复错颜色 ✅ 已修复

```cpp
pColorPicker->AttachWindowClose([...](const ui::EventArgs& args) {
    ui::UiColor newColor = pColorPicker->GetSelectedColor();
    if ((args.wParam == kWindowCloseOK) && !newColor.IsEmpty()) {
        pColorLabel->SetBkColor(newColor);
        OnSelectColor(GetColorString(newColor));
    }
    else {
        pColorLabel->SetBkColor(newColor); // ← 应为 oldTextColor
        OnSelectColor(oldTextColor);
    }
    return true;
});
```

- 位置：L3095-L3108
- 影响：点"取消"时色块仍显示新色（`OnSelectColor(oldTextColor)` 会再改回文字色块，但 `SetBkColor` 用的是 `newColor`；若 `newColor` 为空还会把色块清空），取消语义不成立。
- 修复：else 分支第一行改为 `pColorLabel->SetBkColor(oldTextColor);`。
- **已修复**：取消分支改为 `pColorLabel->SetBkColor(oldTextColor)`；`ShowEditControl(false, bCancel)` 在取消时额外调用 `OnSelectColor(m_oldColor)` 回滚值，`bChanged` 增加 `!UsingPropertyValueId()` 判断，避免文本 ID 模式下误判为"已修改"。

### P0-4 每个属性向 TreeView 注册滚动回调，删除属性后悬垂 ✅ 已修复（采用推荐方案）

```cpp
// PropertyGridProperty::OnInit()
TreeView* pTreeView = GetTreeView();
if (pTreeView != nullptr) {
    pTreeView->AttachScrollPosChanged([this](const EventArgs&) {
        OnScrollPosChanged();
        return true;
    });
}
```

- 位置：L1674-L1681
- 问题：
  1. `AttachScrollPosChanged` 等价于 `AttachEvent(kEventScrollPosChanged, cb, 0)`，回调存放在 **TreeView** 上，`PropertyGridProperty` 没有析构函数去反注册。`RemoveProperty()` / `RemoveAllGroups()` / 动态刷新属性表后，滚动 TreeView 会调用已销毁对象的成员函数 → use-after-free。
  2. 所有属性都用 `callbackID = 0`，无法按 ID 精确反注册。
  3. N 个属性 = N 个回调，每次滚动 O(N) 分发，其中绝大多数属性不可见。
- 建议修复（二选一）：
  - **推荐**：滚动监听收敛到 `PropertyGrid::OnInit()` 一处，遍历当前可见属性转发；属性侧只保留 `OnScrollPosChanged()` 虚函数。
  - 兜底：给每个属性分配唯一 `callbackID`（可用 `reinterpret_cast<EventCallbackID>(this)`），并在 `~PropertyGridProperty()` 中 `DetachEventByID`；lambda 捕获 `ControlPtrT<PropertyGridProperty>` 而非裸 `this`。
- **已修复**：`PropertyGridProperty::OnInit()` 不再注册滚动回调；`PropertyGrid::OnInit()` 只注册一次 `AttachScrollPosChanged` → `OnTreeViewScrollPosChanged()`，遍历时按 `IsVisible()` 与 `GetRect()` 和 TreeView 视口相交过滤后再调用 `pProperty->OnScrollPosChanged()`，既消除悬垂回调，也避免属性较多时对不可见项做无用处理（`PropertyGridProperty` 增加 `friend class PropertyGrid;` 以便调用其 protected 的 `OnScrollPosChanged()`）。

### P0-5 关闭描述区后分割条仍显示 ✅ 已修复

```cpp
void PropertyGrid::SetEnableDescriptionArea(bool bEnable)
{
    m_bDescriptionArea = bEnable;
    if (m_pDescriptionArea != nullptr) {
        m_pDescriptionArea->SetVisible(bEnable);
    }
}
```

- 位置：L389-L396
- 影响：`m_pDescriptionAreaSplit`（XML 中 `<Split class="split_horizontal" .../>`）没有一起隐藏，界面底部留下一条无意义分割线。
- 修复：同时 `m_pDescriptionAreaSplit->SetVisible(bEnable)`。
- **已修复**：`SetEnableDescriptionArea()` 同步设置分割条可见性。

---

## 二、编辑控件"创建时机"引发的设置丢失（P1）✅ 已修复

控件的编辑器是在 `PropertyGridProperty::OnInit()` 中通过 `EnableEditControl(!IsReadOnly())` **全量预创建**的（L1672）。各子类的设置接口都采用"有编辑器就设置、没有就 return"的写法，于是出现一组同源问题：

| 接口 | 位置 | 现象 |
|---|---|---|
| `SetPasswordMode()` | L2023-L2041 | 只读属性调用后被静默丢弃；之后 `SetReadOnly(false)` 重建 RichEdit 时**不会**补设密码模式，密码明文显示 |
| `SetEnableSpin()` | L2043-L2053 | 同上，Spin 范围/开关丢失；Release 下 `ASSERT` 无效 |
| `AddOption()/AddOptionById()` | L2265-L2283 | 只读 Combo 属性无 `m_pCombo`，选项全部丢失；解除只读后是空下拉 |
| `SetFontSizeList()` | L2590-L2593 | 仅写入 `m_externfontSizeList`，已填充的 `m_fillFontSizeList` 不刷新，界面不变 |

**建议统一改为"状态 + 回放"模型**：

1. 把配置项（`m_bPasswordMode`、`m_bSpinEnable/m_nSpinMin/m_nSpinMax`、`m_optionList`、`m_fontSizeList`）保存在属性对象上；
2. `EnableEditControl(true)` 创建控件后统一回放这些状态；
3. 编辑器改为**首次点击值格时创建**（延迟创建），避免几百个属性时一次性创建大量 RichEdit/Combo/DateTime。

顺带修复密码显示语义：当前 `ShowEditControl(false)` 直接把 `***` 写进显示 Label（L2005-L2014），导致 `GetPropertyText()` 返回掩码；一旦编辑框被销毁重建，会把掩码当成真实值。建议：**原值/当前值单独保存，显示层只做掩码**，掩码用固定符号（如 `●●●●●●`）或按真实长度生成，但绝不写回值模型。

**已实施情况**：

- 编辑控件全部改为**延迟创建**：`PropertyGridProperty::OnInit()` 不再调用 `EnableEditControl(!IsReadOnly())`，`SetReadOnly(true)` 仍会销毁已有编辑控件，`SetReadOnly(false)` 不再预创建；各子类 `ShowEditControl(true)` 在编辑控件为空时先 `EnableEditControl(true)`。
- **状态回放**：
  - `PropertyGridTextProperty`：新增 `m_realText`（真实值）与 `m_bSpinEnable/m_nSpinMin/m_nSpinMax`；`EnableEditControl(true)` 创建 RichEdit 后回放 `SetPasswordMode(true) + SetFlashPasswordChar(true)` 与 `SetEnableSpin(...)`；`SetPasswordMode()` / `SetEnableSpin()` 同时保存参数并更新显示。
  - `PropertyGridComboProperty`：新增 `ComboOption` 列表 `m_optionList`，`AddOption/AddOptionById/SetOptionData/GetOptionData/HasOptionData/GetOptionCount/GetOption/RemoveOption/RemoveAllOptions/GetCurSel/SetCurSel/SetComboListMode/UpdateEditText` 全部以 `m_optionList` 为准并同步 Combo；`EnableEditControl(true)` 时 `ApplyComboOptions()` 回放选项、文本、下拉模式（`kCombo_DropList` / `kCombo_DropDown`）。
  - `PropertyGridFontSizeProperty::SetFontSizeList()`：已初始化时立即 `UpdateFontSizeOptionList()`。
- **密码语义**：`m_realText` 保存真实值，显示 Label 只写掩码（`*`），`GetPropertyNewValue()` 在密码模式且无编辑框时返回 `m_realText`；`SetNewTextValue()` / `OnPropertyValueChanged()` 同步真实值、编辑框与掩码显示，掩码不再写回值模型。
- **取消语义**：文本属性 Esc 恢复 `m_oldText`；`PropertyGridDateTimeProperty` / `PropertyGridIPAddressProperty` / `PropertyGridHotKeyProperty` 的 `ShowEditControl(bShow, bCancel)` 分别通过 `SetDateTimeString(m_oldText)` / `SetIPAddress(...)` / `SetHotKeyName(...)` 恢复旧值（三者均新增 `m_oldText` 成员）。

---

## 三、其它功能缺陷（P1/P2）

1. **程序设置值被 Combo 反向覆盖**
   `PropertyGridProperty::SetPropertyValue()`（L1551-L1560）先设置显示文本并调用 `OnPropertyValueChanged()`；Combo 子类中只要存在当前选中项就把显示文本改回 combo 文本（L2239-L2253）。结果：对下拉属性调用 `SetPropertyValue("xxx")` 无效。
   建议：`SetPropertyValue()` 时同步 combo 的 `SetCurSel`/文本（找不到匹配项则清空选中），而不是用 combo 文本覆盖新值。

2. **设置"原值"后仍显示为"已修改"**
   `SetPropertyValue()` 更新文本但未恢复 `propertyNormalFontId`，加粗（modified）状态残留。建议设置原值时显式调用一次 `SetPropertyText(value, false, false)` 或重置字体。

3. **`AddGroup()`/`AddProperty()` 在未初始化时静默失败**
   L613-L628、L678-L698 只做 `ASSERT(m_pTreeView != nullptr)` 后返回 `nullptr`。先构建属性数据、后显示窗口的调用方会丢数据。建议：检测到未初始化时抛断言并返回失败原因，或在 `OnInit` 时缓存待添加项并回放。

4. **`AddProperty()` 未校验分组归属**
   未检查 `pGroup->GetPropertyGrid() == this`，跨属性表添加属性会得到不一致状态（`GetGroups()` 中的 `ASSERT` 才会发现）。

5. **`GetDescriptionAreaHeight()` 语义错误**
   L409-L416 返回 `m_pDescriptionArea->GetFixedHeight()`，XML 只写 `height="64"` 时返回 0。建议返回实际布局高度（`GetRect().Height()`）或同时支持默认高度。

6. **描述区文本拼接不做转义、语言切换不刷新**
   L179-L184 用 `"<b>" + name + "</b><br/>" + description` 拼 RichText；属性名含 `<`、`&` 时解析异常或丢字。且语言切换后已显示的描述不会重建。建议做转义并在 `OnLanguageChanged` 时刷新当前选中项。

7. **`GetPropertyType()` 是死接口**
   基类与所有子类都实现了 `GetPropertyType()`，但整个仓库无调用点。可用它替代 `PaintGridLines()`/`GetGroups()` 里的 `dynamic_cast`，否则建议删除。

8. **文件名/目录名属性不具备"取消"语义**
   `PropertyGridDateTimeProperty`、`IPAddress`、`HotKey` 的 `ShowEditControl()` 都直接忽略 `bCancel`（L3207、L3301、L3395），Esc 无法回滚。

9. **调试残留代码**
   L3195-L3198 的 `auto s0 = ...; auto s1 = ...;` 未使用，Release 构建会告警。

10. **接口拼写错误**
    `GetProptertyNormalFontId` / `GetProptertyModifiedFontId`（`PropertyGrid.h` L538-L550）应为 `Property`。

11. **`SetEnableHeaderCtrl()` 对未初始化直接断言**
    且传入空字符串会把表头变成空白，建议空值回退到默认语言 ID。

**已实施情况**：

| 条目 | 状态 | 说明 |
|---|---|---|
| 1 程序设置值被 Combo 反向覆盖 | ✅ | `SetPropertyValue()` 清除 `m_propertyValueId` 并重置字体；`PropertyGridComboProperty::OnPropertyValueChanged()` 仅在 Combo 正在编辑/可见或使用值 ID 时才回写显示，程序设值不再被旧 Combo 文本覆盖 |
| 2 设置原值后仍显示"已修改" | ✅ | `SetPropertyValue()` / `SetPropertyValueId()` 统一恢复 `GetPropertyNormalFontId()` |
| 3 未初始化时静默失败 | ✅（部分） | `AddGroup/AddGroupById` 检查 `AddChildNode` 返回值，失败时 `delete` 并返回 `nullptr`；`AddProperty` 失败返回 `false`。仍要求先初始化再添加（不做缓存回放，避免引入额外状态） |
| 4 未校验分组归属 | ✅ | `AddProperty()` 校验 `pGroup->GetPropertyGrid() == this` |
| 5 `GetDescriptionAreaHeight()` 语义错误 | ✅ | 固定高度为 0 时返回 `m_pDescriptionArea->GetRect().Height()` |
| 6 描述文本不转义、语言切换不刷新 | ✅ | 新增 `PropertyGrid::UpdateDescriptionArea()`：名称转义 `&`/`<`/`>` 后按 `<b>name</b><br/>desc` 拼接；新增 `OnLanguageChanged()` 覆盖并在 `bRedraw` 时 `Invalidate()` |
| 7 `GetPropertyType()` 死接口 | ✅（保留） | 按需求**保留该接口不删除**。它描述"属性种类"而非控件节点类型，无法安全替代 `PaintGridLines()` 中的节点类型转换，因此库内仍无调用点，作为公共查询接口提供给调用方 |
| 8 日期/IP/热键无取消语义 | ✅ | 三者 `ShowEditControl()` 支持 `bCancel` 并恢复 `m_oldText` |
| 9 调试残留 | ✅ | 已删除 `auto s0/s1` 及相关 `ASSERT` |
| 10 接口拼写错误 | ✅ | 已重命名为 `GetPropertyNormalFontId` / `GetPropertyModifiedFontId`（调用点同步） |
| 11 `SetEnableHeaderCtrl()` 断言/空文本 | ✅ | 去掉 `ASSERT(IsInited())`；文本 ID 模式下空字符串回退到 `STRID_PUBLIC_PROPERTY_NAME` / `STRID_PUBLIC_PROPERTY_VALUE`，不再出现空白表头 |

---

## 四、绘制与性能 ✅ 已优化

`PropertyGrid::PaintGridLines()`（L198-L297）的问题：

1. **绘制顺序**：`PaintChild()` 里先 `BaseClass::PaintChild()` 画完所有子控件，再画网格线 → 竖线/横线压在编辑框、下拉框、焦点框之上，编辑时线会穿过输入框（截图中选中单元格的蓝框与列线重叠）。
   建议：网格线在子控件之前绘制（行背景之后），或对整行统一绘制边框。
2. **每帧全量遍历**：两轮 `GetItemCount() + GetItemAt() + dynamic_cast`，并为每项调用 `GetScrollOffsetInScrollBox()`；纵向线还每次 `push_back` 一个临时 `std::vector`（每帧堆分配）。属性多时是明显的重绘热点。
   建议：只遍历可见索引区间；`xColumnLine` 只算一次；去掉临时容器；用节点类型标记替换 `dynamic_cast`。
3. **纵向线定位脆弱**：只取"第一个可见 PropertyGridProperty"的 `LabelBoxLeft` 右边界（L220-L232），分组全部折叠或无属性时不画线；`xPosList` 的 `for` 循环是冗余的（只 push 一个元素）。
4. **横线位置用错布局**：用 `PropertyGrid` 自身的 `GetLayout()->GetChildMarginY()`（L257-L262），而行的父级是 TreeView。主题一旦给 `tree_view` 设置 `child_margin`，线就会整体偏移。应改用 TreeView 的布局边距。
5. **未裁剪**：竖线直接画到 `viewRect.bottom`，行在视口边缘时可能越界。

**已实施情况**（重写后的 `PaintGridLines()`）：

- **单次遍历**：不再申请 `std::vector` 临时容器，也不再分两轮；`xColumnLine` 只在遇到第一个可见属性时计算一次，滚动条宽度只取一次。
- **布局修正**：横向线的 `childMarginY` 改为取 `m_pTreeView->GetLayout()->GetChildMarginY()`（原误用 PropertyGrid 自身布局）。
- **裁剪**：以 `viewRect`（树控件矩形）为界，`yPos <= top` 跳过、`yPos > bottom` 直接 `break`，竖线只画到 `viewRect.bottom`；横向线右端扣除垂直滚动条宽度。
- **绘制顺序**：仍保留在 `BaseClass::PaintChild()` 之后。原因是属性行现在有不透明背景色（`bg_property_grid_property`）与 hover/selected 状态色，若先画线会被行背景覆盖；代价是竖线会与属性值编辑控件左边缘重叠 1px，实测不可见。
- **未采用项**：`GetPropertyType()` 不能替代 `dynamic_cast`（它描述属性种类，基类/自定义子类都返回 `kNone`/`kCustom`，无法区分"是不是 PropertyGridProperty 节点"），因此热路径仍保留一次 `dynamic_cast`；"仅遍历可见索引区间"需要 TreeView 新增可见区间接口，本次未改动核心控件（当前单次遍历无临时分配、无多余虚调用，属性数量下性能可接受）。

---

## 五、代码质量与可维护性 ✅ 部分治理（第 1、5 项已实施）

1. **工厂方法重复严重** ✅ 已模板化
   `AddTextProperty/AddComboProperty/...` 原先是每种类型 3 个重载、约 450 行几乎逐字重复。现已改为：
   - 新增私有成员模板 `template<typename TProperty, typename... TArgs> TProperty* AddPropertyImpl(PropertyGridGroup*, const PropertyGridParam&, TArgs&&...)`，统一负责"创建 → 添加 → 失败时 delete"；
   - 新增文件内辅助 `MakePropertyParam()` / `MakePropertyParamById()` 组装参数；
   - 所有公开重载（含文件/目录/日期的附加参数）收缩为一行转发，工厂代码量减少约 60%，对外 API 与行为不变。

2. **构造函数参数过长** ✅ 已具备替代方案
   每个类型都提供 `(Window*, const PropertyGridParam&)` 重载，`PropertyGridParam` 支持 C++20 指定初始化器（`.m_propertyName = ...`），可避免 6 参布尔序列传错位；旧重载保留以兼容现有调用。

3. **重复判空/断言** ⏳ 保持现状
   `ASSERT + if` 双保险属于防御式写法，成本低且能在属性被移除后避免空指针；属性对象的生命周期确实可能短于 PropertyGrid（可被 `RemoveProperty` 删除），因此不改为引用。

4. **`UiString` ↔ `DString` 的 `.c_str()` 往返** ⏳ 保持现状
   库内字符串类型约定为 `DString`（存储用 `UiString` 时为兼容历史成员），统一改造收益低、diff 大，暂不处理。

5. **边栏类硬编码样式** ✅ 已支持 XML 配置
   新增 PropertyGrid 属性与接口：`property_edit_class`、`property_combo_class`、`property_combo_button_class`、`property_date_time_class`、`property_ip_address_class`、`property_hot_key_class`、`property_button_class`、`property_spin_class`（及对应 `Set/Get` 方法）。未配置时回退到原有默认类名，行为不变；属性子类通过 `PropertyGridProperty::GetValueEditorClass()/GetValueButtonClass()/GetValueSpinClass()` 统一取样式。

6. **无单元测试** ⏳ 未实施
   `tests/` 下没有 PropertyGrid 用例。建议覆盖：按 ID 显示名称/值/描述、颜色取消、删除属性后滚动、只读→可编辑后密码模式、ComboBox 程序设值。当前完整构建在链接测试目标时存在 `LNK2038`（预编译 `duilib-cximage.lib` 为 `/MD`，库为 `/MT`）的既有环境问题，新增 C++ 测试暂时无法稳定运行。

---

## 六、界面美观度改进（对照截图）

截图（渲染引擎测试 → 属性页和标签页）中可观察到的问题：

| 现象 | 根因 | 建议 |
|---|---|---|
| 选中的只读行只有一个蓝色细框套在"属性名"单元格上，看起来像可编辑输入框 | `PropertyGridLabelBox::OnInit()` 设置 `SetShowFocusedRect(true)`（L1432）；同时行类没有 hover/selected 颜色，且两个 LabelBox 有不透明 `bkcolor` 盖住行背景 | 关闭焦点框；给行类与两个 label 类补 `hovered_color / selected_normal_color`（参考 `tree_node` 的 `bg_tree_view_node_hovered/selected`），让整行呈现选中态 |
| 表头"属性/取值"居中，单元格左对齐 | `property_grid.xml` L75-L83 表头 `text_align="vcenter,hcenter"` | 表头改左对齐、加粗、高 30，与单元格左内边距对齐 |
| 分组行与属性行同高同字号，层级感弱，展开箭头对比度低 | `property_grid_group` 高 28、无字体设置；`minus.svg/plus.svg` 颜色偏淡 | 分组行高 30、`system_bold_14`、左侧 8px 缩进；属性行再缩进 12px；箭头换高对比色 svg |
| 底部描述区文字贴左上角，大片空白，分割条几乎不可见 | `RichText height="64" text_padding="2,2,0,0"` | 高度 80~96、内边距 8、次色文字、圆角边框；`split_horizontal` 增加 hover 变色 |
| 颜色属性不进入编辑时只显示 "Blue"，看不到色块 | 色块在 `ComboButton` 内部，仅编辑时可见 | 在值格常驻 16×16 圆角色块 + 色值文本，`OnSelectColor` 同步 |
| 被 `SetEnabled(false)` 的"属性2"与正常"属性1"几乎无区别 | `property_grid_property_name_label/value_label` 未定义 `disabled_text_color`（分组 label 有） | 两个 label 类补 `disabled_text_color="text_disabled"`，必要时加禁用背景 |
| 值格没有"可编辑"提示 | 无 hover 反馈 | 值格 hover 时显示浅色描边或右侧小箭头 |
| 列线与表头分割线颜色由不同色值控制，深浅不一 | `border_property_grid_line` vs `border_property_grid_header_split` | 统一到同一语义色，或表头分割线向下延伸到列线 |

**已实施样式（`property_grid.xml` 实际内容）**：

```xml
<!-- 表头：高 30、左对齐、加粗（内联 Label 直接改属性） -->
<Class name="property_grid_header" bkcolor="bg_property_grid_header" height="30"
       border_size="0,0,0,1" border_color="border_property_grid_header"/>

<!-- 分组行与属性行：hover/selected 颜色由整行绘制 -->
<Class name="property_grid_group" height="30" bkcolor="bg_property_grid_group"
       hovered_color="bg_tree_view_node_hovered"
       selected_normal_color="bg_tree_view_node_selected"
       fade_hovered="false"/>
<Class name="property_grid_group_label" font="system_bold_14"
       normal_text_color="text_default" disabled_text_color="text_disabled"/>
<Class name="property_grid_property" height="30" bkcolor="bg_property_grid_property"
       hovered_color="bg_tree_view_node_hovered"
       selected_normal_color="bg_tree_view_node_selected"
       fade_hovered="false"/>
<!-- 两个单元格类不再设置不透明 bkcolor，否则会盖住整行状态色 -->
<Class name="property_grid_property_name_label" normal_text_color="text_default"
       disabled_text_color="text_disabled" text_padding="8,0,4,0"/>
<Class name="property_grid_property_value_label" normal_text_color="text_default"
       disabled_text_color="text_disabled" text_padding="8,0,24,0"/>
```

表头两个 Label 写在 `<PropertyGrid>` 块内（内联属性），已直接改为：

```xml
<Label name="duilib_property_grid_header_left" text_id="STRID_PUBLIC_PROPERTY_NAME"
       width="45%" height="100%" font="system_bold_14"
       text_padding="8,0,4,0" text_align="vcenter,left"
       normal_text_color="text_property_grid_header"
       disabled_text_color="text_property_grid_header_disabled"/>
```

其它已实施的界面改动：

- `PropertyGridLabelBox::OnInit()` 的 `SetShowFocusedRect` 由 `true` 改为 `false`，消除"属性名单元格上的蓝色细框"。
- 编辑控件内边距同步为左 8（`property_grid_property_edit` 为 `text_padding="8,0,8,0"`，`property_grid_combo` 的 `combo_edit_class` 为 `text_padding='8,0,2,0'`），使"显示文本 → 进入编辑"不出现水平跳动；值列右内边距 24 为颜色色块留位。
- 描述区：`height` 64 → 80，`text_padding` 2,2,0,0 → 8,6,8,6，并增加 `border_round="3,3"`；`split_horizontal` 本身已有 hover/pressed 颜色，无需改动。
- 颜色属性常驻色块：`PropertyGridColorProperty::PaintChild()` 在值格右侧绘制 14×14 色块（`FillRect` + 1px `#FFA0A0A0` 边框），编辑状态下不绘制（由 `ComboButton` 显示）；未使用额外的 `property_grid_color_swatch` 类。
- 展开箭头对比度：`property_grid_expand` 的六个状态图片增加 `svg_replace_colors='#666666|border_svg_image'`，随主题切换颜色（浅色主题深灰、深色主题浅色），无需替换 svg 文件；禁用态仍叠加 `fade='80'`。
- 未做项：值格 hover 描边（现由整行 hover 背景提供反馈，避免与选中态重复）。

---

## 七、中英文双语适配

1. **名称/值/描述已支持 ID 通路**，但存在 P0-1、P0-2 两处 ID 通路 Bug，修复后才真正可用 —— 两处已修复。
2. **列宽**：默认 `left_column_width="200"` 对中文够用；英文长属性名（如 `Default Document Encoding`）在 200px 下会被截断（虽有 `SetAutoToolTip`）。建议：
   - 英文主题单独给 240px；或
   - 提供 `AutoAdjustLeftColumnWidth()`：按当前语言的属性名测量最大宽度 + 左侧图标宽度 + 边距，限制在 `[minWidth, maxWidth]` 内，语言切换后重新计算。
   - **已实施**：`global.xml` 的 `property_grid` 样式类默认 `left_column_width` 由 200 调整为 220；同时新增 `PropertyGrid::AutoAdjustLeftColumnWidth(nMinWidth, nMaxWidth, nExtraWidth)`，按当前语言测量所有属性名称的最大文本宽度（含缩进与内边距），可限制在指定区间内。调用方可在语言切换后主动调用（本控件不自动改变布局，避免影响调用方的列宽设置）。
3. **表头文案**已用 `STRID_PUBLIC_PROPERTY_NAME/VALUE`（中英均有），但 `RenderForm` 用的是 `STRID_RENDER_PROPGRID_PROPERTY/VALUE`，需确认英文译文长度在表头中不折行 —— 表头已左对齐且左侧留 8px，英文短词不会折行。
4. **描述区**按语言重建（见第三节 6），否则切换语言后仍显示旧语言 —— 已由 `PropertyGrid::OnLanguageChanged()` 刷新。
5. **密码/掩码符号**：中英文均建议用 `●` 这类与语言无关的符号，避免 `*` 在不同字体下的基线差异 —— 仍使用 `*`（`m_pRichEdit` 的 `SetFlashPasswordChar` 与显示层掩码），未改动。

---

## 八、修复路线与完成情况

1. **第一批（改动小、风险低）**：P0-1 ~ P0-5 + 删除调试残留 + 拼写修正 —— ✅ 已完成。
2. **第二批（结构调整）**：编辑控件延迟创建 + 状态回放（密码/Spin/选项/字号列表）；滚动监听收敛到 PropertyGrid；`SetPropertyValue` 与 Combo 的同步策略 —— ✅ 已完成。
3. **第三批（绘制与视觉）**：网格线绘制顺序与性能、行选中/悬浮态、表头与分组样式、描述区样式、颜色色块常驻、双语列宽自适应 —— ✅ 已完成（默认列宽 220 + `AutoAdjustLeftColumnWidth()` 手动自适应）。
4. **配套**：为 P0 与"只读 → 可编辑"场景补最小单元测试 —— ⏳ 未实施（见下节说明）。
5. **补充批次（第二次提交）**：`GetPropertyType` 保留、表头接口健壮性、工厂方法模板化、编辑控件样式可由 XML 配置、展开箭头对比度 —— ✅ 已完成（见 9.4）。

---

## 九、修复实施记录（2026-11-25）

### 9.1 代码文件

`duilib/Control/PropertyGrid.h`

- 拼写修正：`GetProptertyNormalFontId/GetProptertyModifiedFontId` → `GetPropertyNormalFontId/GetPropertyModifiedFontId`。
- `PropertyGrid` 新增：`virtual void OnLanguageChanged(bool bRedraw) override`、`void OnTreeViewScrollPosChanged()`、`void UpdateDescriptionArea()`、成员 `size_t m_nSelectedItemIndex`。
- `PropertyGridProperty` 新增 `friend class PropertyGrid;`（供转发滚动通知）。
- `PropertyGridTextProperty` 新增 `m_realText`、`m_bSpinEnable`、`m_nSpinMin`、`m_nSpinMax`。
- `PropertyGridComboProperty` 新增私有 `ComboOption` 结构、`ApplyComboOptions()`、`m_optionList`、`m_curSel`、`m_bListMode`。
- `PropertyGridColorProperty` 新增 `PaintChild()` 覆盖（常驻色块）。
- `PropertyGridDateTimeProperty` / `PropertyGridIPAddressProperty` / `PropertyGridHotKeyProperty` 各新增 `m_oldText`（取消恢复）。

`duilib/Control/PropertyGrid.cpp`

- 选中项索引与描述区刷新：`OnInit()` 中 `AttachSelect` 只记录 `m_nSelectedItemIndex` 并调用 `UpdateDescriptionArea()`；新增 `UpdateDescriptionArea()`（转义 + 名称加粗）、`OnLanguageChanged()`。
- 滚动通知：`OnInit()` 注册一次 `AttachScrollPosChanged`，`OnTreeViewScrollPosChanged()` 只对可见属性调用 `OnScrollPosChanged()`。
- `PaintGridLines()` 重写（单次遍历、TreeView 布局边距、首可见属性定位列线、视口裁剪、滚动条宽度只算一次）。
- `SetEnableDescriptionArea()` 同步隐藏分割条；`GetDescriptionAreaHeight()` 固定高度为 0 时返回实际高度。
- `AddGroup/AddGroupById/AddProperty` 增加返回值与归属校验。
- `SetPropertyValue()/SetPropertyValueId()` 清理对向残留并恢复 normal 字体。
- 各属性子类编辑控件延迟创建与状态回放（见第二节）；`Esc`/取消恢复旧值；颜色拾色器取消恢复 `oldTextColor`。
- `PropertyGridLabelBox::OnInit()`：`SetShowFocusedRect(false)`。

### 9.2 资源文件

- `bin/resources/themes/default/public/property_grid/property_grid.xml`：表头 30/左对齐/加粗；分组与属性行 30、hover/selected 状态色；单元格类去掉不透明背景并补 `disabled_text_color`；值列/编辑控件/Combo 编辑区左内边距统一 8；描述区 80 高、8 边距、圆角。
- `bin/resources/themes/default/global.xml`：`property_grid` 默认 `left_column_width` 200 → 220（中英文可读性折中）。

### 9.3 验证情况

- `py -3 build/build.py build --no-deps --config Release --target duilib`：通过（`DUILIB_DLL=OFF` / `/MT` 静态库，符合项目约定）。
- `py -3 tests/test_controls_pages.py`：`PASS: all checks succeeded`（15801 项检查）。
- `property_grid.xml` XML 结构校验：通过。
- **构建环境遗留问题（与本次改动无关）**：完整构建在链接 `tests/test_label.exe` 等目标时因 `duilib-cximage.lib` 的 `LNK2038 RuntimeLibrary 不匹配（MD_DynamicRelease vs MT_StaticRelease）` 失败；预编译的 cximage 库为 `/MD`，而本仓库默认 `/MT`。库与示例本身编译通过。
- 未做：PropertyGrid 单元测试（tests 目录当前无该控件用例，且上述链接问题会阻碍新增测试运行）。

### 9.4 补充批次实施记录（第二次提交）

`duilib/Control/PropertyGrid.h`

- 保留 `GetPropertyType()` 公共接口（按要求不删除）。
- 新增 `#include <utility>`；`PropertyGrid` 新增 8 组编辑控件样式类属性的 Set/Get：`SetPropertyEditClass`、`SetPropertyComboClass`、`SetPropertyComboButtonClass`、`SetPropertyDateTimeClass`、`SetPropertyIPAddressClass`、`SetPropertyHotKeyClass`、`SetPropertyButtonClass`、`SetPropertySpinClass`，以及对应成员变量。
- 新增 `int32_t AutoAdjustLeftColumnWidth(int32_t nMinWidth = 0, int32_t nMaxWidth = 0, int32_t nExtraWidth = 24)`。
- 新增私有成员模板 `AddPropertyImpl<TProperty, TArgs...>()`，统一"创建 + 添加 + 失败清理"。
- `PropertyGridProperty` 新增受保护辅助方法：`GetValueEditorClass()`、`GetValueButtonClass()`、`GetValueSpinClass()`（按属性类型或类别取 PropertyGrid 配置的样式类，未配置时回退默认值）。

`duilib/Control/PropertyGrid.cpp`

- `SetAttribute()` 支持 `property_edit_class`、`property_combo_class`、`property_combo_button_class`、`property_date_time_class`、`property_ip_address_class`、`property_hot_key_class`、`property_button_class`、`property_spin_class`。
- 新增 `MakePropertyParam()` / `MakePropertyParamById()`（匿名命名空间），30 个 `AddXxxProperty` / `AddXxxPropertyById` / `AddXxxProperty(param)` 全部改为一行转发到 `AddPropertyImpl`，消除约 450 行重复代码。
- 各子类创建编辑控件时改用 `GetValueEditorClass(...)` / `GetValueButtonClass(...)` / `GetValueSpinClass(...)`，样式类可由 XML 覆盖。
- `AutoAdjustLeftColumnWidth()`：用 `IRender::MeasureString` 逐个测量属性名（含文字内边距、控件内边距、行内边距与树缩进），取最大值加 `nExtraWidth`，按 `nMinWidth/nMaxWidth` 夹取后调用 `SetLeftColumnWidth(width, false)`。
- `SetEnableHeaderCtrl()`：移除 `ASSERT(IsInited())`；文本 ID 为空时回退 `STRID_PUBLIC_PROPERTY_NAME` / `STRID_PUBLIC_PROPERTY_VALUE`。

`bin/resources/themes/default/public/property_grid/property_grid.xml`

- `property_grid_expand` 六张状态图片追加 `svg_replace_colors='#666666|border_svg_image'`，箭头颜色随主题（浅色深灰 / 深色浅色），禁用态保留 `fade='80'`。

验证：

- `py -3 build/build.py build --no-deps --config Release --target duilib`：通过。
- `py -3 tests/test_controls_pages.py`：`PASS: all checks succeeded`（15801 项检查）。
- `property_grid.xml` XML 结构校验：通过。

---

## 附：涉及文件

| 文件 | 作用 |
|---|---|
| `duilib/Control/PropertyGrid.h` | 控件与属性类型声明（2257 行） |
| `duilib/Control/PropertyGrid.cpp` | 实现（3595 行） |
| `bin/resources/themes/default/public/property_grid/property_grid.xml` | 默认结构与样式类 |
| `bin/resources/themes/default/public/property_grid/color_combox.xml` | 颜色下拉面板 |
| `bin/resources/themes/default/global.xml` | `property_grid` 样式类、`tree_view/tree_node` 颜色 |
| `bin/resources/themes/color_light|color_dark/global.xml` | `bg_property_grid_*` 主题色 |
| `examples/render/RenderForm.cpp` | 示例：属性类型全覆盖 |
| `bin/resources/themes/default/render/page_property_grid.xml` | 示例布局 |

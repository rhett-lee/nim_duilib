# chat / render / TreeView / ListCtrl 示例双语改造计划

## Context

`examples/controls` 已完成中英双语改造（XML 用 `*_id`，C++ 用 `Lang().GetStringByID`，动态文本在 `OnLanguageChanged()` 重刷）。本次按同样方式处理另外四个示例，并满足两条硬约束：

1. 英文文案不能导致界面排版混乱（截断、重叠、溢出）。
2. 运行期中英文切换后布局与文案都保持正常；render 必须做 UI 自动化截图验收。

审计结论：四个示例的 C++ 代码与大部分 XML **已经**完成双语化（chat 51 处、render 94 处、TreeView 111 处、ListCtrl 29 处 STRID 引用，`_T("...中文...")` 字面量仅存在于注释中）。剩余缺口集中在以下几处，本计划只处理这些缺口。

## 现存缺口

| 位置 | 问题 |
| --- | --- |
| `bin/resources/themes/default/render/page_image2.xml` / `page_image3.xml` | 各 46 处 `text="中文"`（图片绘制条件说明，两文件内容完全相同） |
| `bin/resources/themes/default/render/page_text1.xml` | 35 处 `text="中文"`（含 9 个 RichText 节点及其内联富文本） |
| `bin/resources/themes/default/render/page_text2.xml` | 28 处 `text="中文"`（竖排文字演示） |
| `examples/render/RenderForm.cpp` | 菜单栏顶层菜单名为硬编码英文 `File/Edit/Selection/View1/View2/Help`，中文模式不切换 |
| `bin/resources/themes/default/chat/wechat.xml` | 9 个 `Avatar` 的 `text="张/李/..."` 中文占位 |
| `examples/TreeView/MainForm.*` | 未实现 `OnLanguageChanged()`，切换语言后状态栏文本、空视图提示不刷新 |
| `tests/test_controls_pages.py` | R6（`*_id` 中英文必须都存在）与 R15（禁止 UI 文本属性硬编码中文）只覆盖 `controls` 目录 |

## 改造方案

### 1. 语言资源（`bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`）

在文件末尾成对追加约 116 条键，格式沿用现有 `key.ljust(33) + " = " + value`，保留 UTF-8 BOM 与原换行符：

- `STRID_RENDER_IMG2_01` ~ `_46`：image2 / image3 共用（两文件文案完全一致）。
- `STRID_RENDER_TEXT1_01` ~ `_35`、`STRID_RENDER_TEXT2_01` ~ `_28`。
- `STRID_RENDER_MENU_FILE / EDIT / SELECTION / VIEW1 / VIEW2 / HELP`。
- `STRID_CHAT_AVATAR_PLACEHOLDER`（中文“张”，英文“A”）。

注意：值中包含 `=`、`%`、`'` 均安全（[LangManager.cpp](file:///e:/develop/nim_duilib/duilib/Core/LangManager.cpp#L155-L192) 只按第一个 `=` 分割，`GetStringByID` 原样返回，不做格式化）。

### 2. render 四个页面（全部翻译，含示例文案）

- 逐值把 `text="..."` 精确替换为 `text_id="..."`，其余属性与文件格式不动，避免整文件 diff。
- 英文文案控制长度：
  - 图片页 Label 类已具备 `multi_line="true" replace_newline="true" height="auto"`，英文换行变高由 `HFlowScrollBox` 承接；英文标签控制在 2 行内。
  - 文字页中固定高度行（`height="56"` / `"80"`）按原中文行数控制英文行数，避免纵向裁切；“超长文本/省略号”样本保持单行，本来就用于演示省略号。
- `page_text1.xml` 的 RichText：把整段内容（对齐说明 + 内联超链接 / 字体颜色演示）合并进一个 `text_id`，值内用 `<br/>` 换行并保留 `<a>`、`<b>`、`<font face=...>` 等标记，删除重复的内联中文子节点（`SetTextId` 会整体替换内容）。ASCII 的 `RichTextBox/RichTextHBox/RichTextVBox` 示例不动。
- tab 宽度已固定，英文标题均短于中文，无需调整。

### 3. render 菜单栏（`examples/render/RenderForm.cpp`）

`MenuBar::AddTopMenu` 的第 3 参是 `menuTextId`（[MenuBar.h](file:///e:/develop/nim_duilib/duilib/Control/MenuBar.h)），菜单项内部走 `SetTextId`，切换语言会自动刷新。把菜单名由字面量改为 STRID，第 2 参文本传空即可，无需新增 `OnLanguageChanged()`。

### 4. chat 头像占位（`themes/default/chat/wechat.xml`）

9 处 `text="张"` 等统一改为 `text_id="STRID_CHAT_AVATAR_PLACEHOLDER"`。该值是占位，运行期由 `ChatForm::RefreshSessionList()/RefreshContactList()` 用名称首字符覆盖，不影响实际展示。

### 5. TreeView 语言切换刷新（`examples/TreeView/MainForm.h/.cpp`）

新增 `virtual bool OnLanguageChanged() override;`：

1. 调用 `BaseClass::OnLanguageChanged()`（XML `text_id` 自动重刷）。
2. 调用现有的 `UpdateStatusBar()` 重刷状态栏/视图模式文本。
3. 若当前为空视图，重刷空视图提示文本。
4. 树节点文本通过 `SetTextId` 设置（`DirectoryTree.cpp` 的虚拟目录节点），列头通过 `columnInfo.textId` 设置，均由框架自动刷新；运行验证若未刷新，再在 `OnLanguageChanged()` 中显式重刷。

ListCtrl 已有 `OnLanguageChanged()`（`examples/ListCtrl/MainForm.cpp:132`），无需改动。

### 6. 静态校验扩展（`tests/test_controls_pages.py`）

新增常量 `VERIFY_THEME_DIRS = ["chat", "render", "tree_view", "list_ctrl"]`，对这四个目录下的 XML：

- 复用 `collect_language_keys()` 扩展 R6：所有 `*_id`（text_id/prompt_text_id/title_id/x_axis_title_id/y_axis_title_id/tooltip_text_id 等）引用的 STRID 必须同时存在于中英文语言文件。
- 复用 R15 的硬编码中文检测（`text`/`prompt_text`/`title`/`x_axis_title`/`y_axis_title`/`tooltip_text`），跳过 RichText 子树与 XML 注释。

## 验证

1. `python tests/test_controls_pages.py --source-root .` → 0 失败。
2. VS DevShell 下 `cmake --build e:\develop\nim_duilib\out\x64\Debug --target chat render TreeView ListCtrl` → 全部成功。
3. `ctest --test-dir out\x64\Debug -C Debug --output-on-failure` → 全部通过。
4. render UI 自动化（Skill `TRAE-computer-use-ptc`）：
   - 启动 `bin/render.exe`，中文模式下逐个切换全部标签页截图；
   - 点标题栏语言按钮切到英文，逐页再次截图，核对标签页标题、说明标签、属性表格、菜单栏；
   - 检查无文字截断/控件重叠/内容溢出，再切回中文确认可逆。
5. chat（登录 + 主窗口）、TreeView、ListCtrl：启动并切换语言，截图核对会话/联系人/树节点/列头/状态栏，确认无排版异常。
6. 关闭所有测试进程。

## 明确不做

- 不处理 `.rc` 中的 Win32 原生菜单资源（duilib 自绘窗口不使用）。
- 不重构无关代码，不新增文档。
- 演示数据（联系人姓名、聊天正文等）已通过 STRID 提供双语，保持不变。

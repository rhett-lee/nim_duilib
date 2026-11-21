简体中文 | [English](UrlBar.en.md)

## 浏览器地址栏（UrlBar）

`UrlBar` 是框架自绘的浏览器地址栏控件（继承 `HBox`），用于输入和展示网址。外观与交互参考
常见浏览器（如 Edge）：

* 左侧显示安全状态图标：https 显示锁图标，其它协议显示地球图标，**点击时触发事件**；
* 中间为单行编辑框，获得焦点时自动全选文本，获得焦点后显示清除按钮；
* 最右侧显示收藏按钮，有未收藏（空心星）和已收藏（实心星）两种状态，**点击切换状态并触发事件**；
* 输入时按历史记录过滤并弹出联想下拉，上下键导航、回车确认、Esc 收起。

联想下拉的展开/关闭规则：

* 展开：仅在编辑框中输入文本且存在匹配项时展开；编辑框获得焦点、鼠标点击地址栏均不展开；
* 关闭：按 Esc；按回车提交（含选中候选项）；点击候选项；输入为空或无匹配项；编辑框失去焦点；
  鼠标按下地址栏和下拉浮层之外的区域；窗口被隐藏或最小化；
* 浮层为不抢焦点窗口（`WS_EX_NOACTIVATE`），展开期间输入焦点始终保留在编辑框内，可继续输入。

* 关联头文件：[duilib/Control/UrlBar.h](../duilib/Control/UrlBar.h)
* 皮肤类（`global.xml`）：`url_bar` / `url_bar_icon_secure` / `url_bar_icon_insecure` /
  `url_bar_edit` / `url_bar_clear_btn` / `url_bar_fav_btn`
* 图标资源：`public/url_bar/lock.svg`、`public/url_bar/globe.svg`；
  收藏按钮复用 `public/checkbox/star_outline.svg`（未收藏）与 `public/checkbox/star_filled.svg`（已收藏），
  清除按钮复用 `public/button/window-close.svg`
* 联想下拉复用 AutoComplete 的皮肤：`auto_complete_flyout` / `auto_complete_list` / `auto_complete_item`
* 完整演示：`examples/cef`、`examples/cef_browser`、`examples/webview2`、`examples/webview2_browser`

### 1. XML 示例

```xml
<UrlBar class="url_bar" name="url_bar"
        width="stretch" height="34" valign="center"
        prompt_text_id="STRID_CEF_BROWSER_INPUT_ADDRESS"
        search_url_template="https://www.bing.com/search?q=%s"
        history_file="user/cef_browser_url_history.txt"
        default_context_menu="true"
        clear_button_enabled="true"
        favorite_button_enabled="true"
        favorite="false"/>
```

### 2. XML 属性

除 `HBox` 通用属性外，还支持以下属性：

| 属性 | 默认值 | 说明 |
| :--- | :--- | :--- |
| `prompt_text` | 空 | 编辑框占位提示文字 |
| `prompt_text_id` | 空 | 编辑框占位提示文字的多语言 ID |
| `prompt_color` | 空 | 编辑框占位提示文字颜色 |
| `text` | 空 | 初始网址文本 |
| `text_id` | 空 | 初始网址文本的多语言 ID（与 `text` 同时设置时，后设置的属性生效） |
| `search_url_template` | 空 | 搜索 URL 模板，需包含 `%s` 占位符（如 `https://www.bing.com/search?q=%s`）；输入内容不像网址时按此模板生成搜索 URL，为空则补全 `https://` |
| `history_enabled` | `true` | 是否记录并展示网址历史 |
| `max_history` | `20` | 网址历史最多保留条数 |
| `history_file` | 空 | 网址历史持久化文件（相对路径基于可执行程序目录），为空则不持久化 |
| `max_visible` | `8` | 联想下拉最多可见候选行数，超出出现滚动条 |
| `secure` | `false` | 初始安全状态：`true` 显示锁图标，`false` 显示地球图标 |
| `default_context_menu` | 空 | 是否启用编辑框的默认右键菜单（转发给内部编辑框） |
| `clear_button_enabled` | `true` | 是否启用编辑框的清除按钮 |
| `favorite_button_enabled` | `true` | 是否显示最右侧的收藏按钮 |
| `favorite` | `false` | 初始收藏状态：`true` 显示实心星（已收藏），`false` 显示空心星（未收藏） |

### 3. 事件

| 事件 | 触发时机 | 参数 | 便捷监听接口 |
| :--- | :--- | :--- | :--- |
| `kEventTextChanged` | 编辑框文本变化时实时触发 | 无 | 内部编辑框的 `kEventTextChanged`（可 `GetEditControl()->AttachTextChanged`） |
| `kEventUrlChanged` | 用户提交新网址（回车或选中联想项）后触发 | 无；网址通过 `GetUrl()` 获取 | `AttachUrlChanged()` |
| `kEventUrlCancel` | 用户按 Esc 取消编辑、恢复为已提交网址后触发 | 无 | `AttachUrlCancel()` |
| `kEventUrlBarIconClick` | 点击左侧安全状态图标（锁/地球）后触发 | 无 | `AttachIconClick()` |
| `kEventUrlBarFavoriteClick` | 点击最右侧收藏按钮切换收藏状态后触发 | 无；当前状态通过 `IsFavorite()` 获取 | `AttachFavoriteClick()` |

XML 内联事件名分别为 `text_changed`、`url_changed`、`url_cancel`、`url_bar_icon_click`、
`url_bar_favorite_click`（也可使用首字母大写的 `UrlChanged` 等写法，详见
[XmlEvents.md](XmlEvents.md)）。

```cpp
UrlBar* pUrlBar = dynamic_cast<UrlBar*>(FindControl(_T("url_bar")));
if (pUrlBar != nullptr) {
    //提交网址：在回调中执行导航
    pUrlBar->AttachUrlChanged([this](const ui::EventArgs& /*args*/) {
        DString url = m_pUrlBar->GetUrl();
        Navigate(url);
        return true;
    });
    //左侧安全状态图标点击：业务层可弹出站点信息面板
    pUrlBar->AttachIconClick([this](const ui::EventArgs& /*args*/) {
        ShowSiteInfo();
        return true;
    });
    //收藏按钮点击：状态已切换，业务层读取当前状态后添加/取消收藏
    pUrlBar->AttachFavoriteClick([this](const ui::EventArgs& /*args*/) {
        bool bFavorite = m_pUrlBar->IsFavorite();
        UpdateFavorite(m_pUrlBar->GetUrl(), bFavorite);
        return true;
    });
}
```

### 4. C++ 接口

| 接口 | 说明 |
| :--- | :--- |
| `GetEditControl()` | 获取内部编辑框（`RichEdit*`） |
| `GetUrl()` / `SetUrl()` | 获取最近一次提交的网址 / 设置当前网址（用户正在编辑时不覆盖其输入） |
| `GetEditText()` / `SelectAll()` | 获取编辑框中的原始文本 / 全选编辑框文本 |
| `Submit()` | 提交编辑框中的输入（效果与按回车相同），归一化后触发 `kEventUrlChanged` |
| `SetFocus()` | 让控件获取焦点（实际焦点设置到内部编辑框） |
| `SetSearchUrlTemplate()` / `GetSearchUrlTemplate()` | 设置/获取搜索 URL 模板 |
| `SetSecure()` / `IsSecure()` | 设置/读取安全状态（锁图标/地球图标） |
| `SetFavoriteButtonEnabled()` / `IsFavoriteButtonEnabled()` | 显示/隐藏收藏按钮（默认显示） |
| `SetFavorite()` / `IsFavorite()` | 设置/读取收藏状态（未收藏/已收藏两种状态） |
| `SetClearButtonEnabled()` / `IsClearButtonEnabled()` | 启用/禁用清除按钮（默认启用） |
| `SetHistoryEnabled()` / `IsHistoryEnabled()` | 是否记录并展示网址历史（默认启用） |
| `SetMaxHistory()` / `GetMaxHistory()` | 网址历史最多保留条数 |
| `SetMaxVisible()` / `GetMaxVisible()` | 联想下拉最多可见候选行数 |
| `GetHistory()` / `ClearHistory()` / `AddHistory()` | 读取/清空/追加网址历史 |
| `SetHistoryFile()` / `GetHistoryFile()` | 设置/获取历史持久化文件 |

### 5. 皮肤类

| 皮肤类 | 说明 |
| :--- | :--- |
| `url_bar` | 地址栏容器：圆角胶囊背景、边框及悬停边框 |
| `url_bar_icon_secure` | 左侧安全图标（https，锁） |
| `url_bar_icon_insecure` | 左侧非安全图标（其它协议，地球） |
| `url_bar_edit` | 内部单行编辑框 |
| `url_bar_clear_btn` | 编辑框内右侧的清除按钮 |
| `url_bar_fav_btn` | 最右侧的收藏按钮（未收藏为空心星，已收藏为实心星） |

修改这些皮肤类即可统一调整地址栏的外观（尺寸、圆角、配色、图标等），无需改动业务代码。

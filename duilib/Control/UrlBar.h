#ifndef UI_CONTROL_URL_BAR_H_
#define UI_CONTROL_URL_BAR_H_

#include "duilib/Box/HBox.h"
#include <vector>

namespace ui
{
class RichEdit;
class AutoCompleteFlyout;
class Control;

/** 浏览器地址栏控件（UrlBar）：安全状态图标 + 网址编辑框 + 历史联想下拉
*
*   外观与交互参考常见浏览器的地址栏：
*   - 左侧显示安全状态图标：https 显示锁图标，其它协议显示地球图标，点击时触发事件
*   - 中间为单行编辑框，获得焦点时自动全选文本，文本非空时显示清除按钮
*   - 右侧显示收藏按钮，点击时触发事件
*   - 输入时按历史记录过滤并弹出联想下拉，上下键导航、回车确认、Esc 收起
*
*   联想下拉的展开/关闭规则（参考 Edge 地址栏）：
*   展开：仅在编辑框中输入文本且存在匹配项时展开；编辑框获得焦点、鼠标点击地址栏均不展开。
*   关闭：按 Esc；按回车提交（含选中候选项）；点击候选项；输入文本无匹配项或为空；
*         编辑框失去焦点；鼠标按下地址栏和下拉浮层之外的区域（含网页区域、标题栏/标签栏等
*         非客户区、其它窗口）；窗口被隐藏或最小化。
*   浮层为不抢焦点窗口（WS_EX_NOACTIVATE）：展开期间输入焦点始终保留在编辑框内，可继续输入。
*   - 按回车提交时对输入进行网址归一化（补全协议、本地路径转 file://、搜索模板），
*     归一化后的网址通过 kEventUrlChanged 事件通知业务层执行导航
*   - 按 Esc 取消编辑，恢复为最近一次提交的网址，并触发 kEventUrlCancel
*
*   XML 属性（除 HBox 通用属性外）：
*     prompt_text         编辑框占位提示文字
*     prompt_text_id      编辑框占位提示文字的多语言 ID
*     prompt_color        编辑框占位提示文字颜色
*     text                初始网址文本
*     text_id             初始网址文本的多语言 ID（与 text 同时设置时，后设置的属性生效）
*     search_url_template 搜索URL模板（需包含"%s"，如 "https://www.bing.com/search?q=%s"），
*                         输入内容不像网址时按此模板生成搜索URL；为空则补全"https://"
*     history_enabled     是否记录并展示网址历史（默认 true）
*     max_history         网址历史最多保留条数（默认 20）
*     history_file        网址历史持久化文件（相对路径基于可执行程序目录），为空则不持久化
*     max_visible         联想下拉最多可见候选行数（默认 8，超出出现滚动条）
*     secure              初始安全状态（true 显示锁图标，false 显示地球图标；默认 false）
*     default_context_menu 是否启用编辑框的默认右键菜单（转发给内部编辑框）
*     clear_button_enabled 是否启用编辑框的清除按钮（默认 true）
*     favorite_button_enabled 是否显示右侧收藏按钮（默认 true）
*
*   事件：
*   - kEventUrlChanged：用户提交了新网址（回车或选中联想项）后触发，无参数；
*                       归一化后的网址通过 GetUrl() 获取
*   - kEventUrlCancel：用户按 Esc 取消编辑、恢复为已提交网址后触发，无参数
*   - kEventTextChanged：内部编辑框文本变化时触发（实时，随输入触发）
*   - kEventUrlBarIconClick：点击左侧安全状态图标后触发，无参数
*   - kEventUrlBarFavoriteClick：点击右侧收藏按钮后触发，无参数
*
*   皮肤类（global.xml 中定义）：url_bar / url_bar_icon_secure / url_bar_icon_insecure /
*   url_bar_edit / url_bar_clear_btn / url_bar_fav_btn；下拉浮层复用 auto_complete_flyout /
*   auto_complete_list / auto_complete_item
*/
class DUILIB_API UrlBar : public HBox
{
    typedef HBox BaseClass;

    //内部编辑框子类需要调用私有的按键预处理接口
    friend class UrlBarEdit;
public:
    explicit UrlBar(Window* pWindow);
    UrlBar(const UrlBar& r) = delete;
    UrlBar& operator=(const UrlBar& r) = delete;
    virtual ~UrlBar() override;

    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 让控件获取焦点（实际焦点设置到内部编辑框）
    */
    virtual void SetFocus() override;

    /** 获取内部编辑框控件
    */
    RichEdit* GetEditControl() const;

    /** 获取当前网址（最近一次提交的网址，带完整协议）
    */
    DString GetUrl() const;

    /** 设置当前网址（用于页面跳转后同步地址栏显示）
    *   若编辑框正在被用户编辑（已获得焦点且文本被修改过），则不覆盖用户输入
    */
    void SetUrl(const DString& url);

    /** 获取编辑框中的原始文本（可能是未提交的输入）
    */
    DString GetEditText() const;

    /** 全选编辑框中的文本
    */
    void SelectAll();

    /** 提交编辑框中的输入（效果与按回车相同）
    *   归一化网址后触发 kEventUrlChanged 事件，供业务层执行导航
    */
    void Submit();

    /** 设置搜索URL模板（需包含"%s"占位符）
    */
    void SetSearchUrlTemplate(const DString& urlTemplate);
    DString GetSearchUrlTemplate() const;

    /** 设置安全状态（true 显示锁图标，false 显示地球图标）
    */
    void SetSecure(bool bSecure);
    bool IsSecure() const;

    /** 设置是否显示右侧收藏按钮（默认显示）
    */
    void SetFavoriteButtonEnabled(bool bEnabled);
    bool IsFavoriteButtonEnabled() const;

    /** 设置是否启用编辑框的清除按钮（默认启用）
    */
    void SetClearButtonEnabled(bool bEnabled);
    bool IsClearButtonEnabled() const;

    /** @name 网址历史
    *   提交网址时自动记录（MRU：最近使用的排在前面），同时作为联想下拉的候选数据源
    */
    ///@{
    /** 设置是否记录并展示网址历史
    */
    void SetHistoryEnabled(bool bEnabled);
    bool IsHistoryEnabled() const;

    /** 设置网址历史最多保留条数
    */
    void SetMaxHistory(int32_t nMax);
    int32_t GetMaxHistory() const;

    /** 设置联想下拉最多可见候选行数
    */
    void SetMaxVisible(int32_t nMaxVisible);
    int32_t GetMaxVisible() const;

    /** 获取网址历史（最近使用的排在前面）
    */
    std::vector<DString> GetHistory() const;

    /** 清空网址历史
    */
    void ClearHistory();

    /** 主动追加一条网址历史（通常无需业务层调用，提交网址时控件自动记录）
    */
    void AddHistory(const DString& url);

    /** 设置网址历史的持久化文件（相对路径基于可执行程序目录），为空则不持久化
    */
    void SetHistoryFile(const DString& historyFile);
    DString GetHistoryFile() const;
    ///@}

public:
    /** 监听网址提交事件（回车或选中联想项后触发）
    */
    void AttachUrlChanged(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventUrlChanged, callback, callbackID);
    }

    /** 监听取消编辑事件（按 Esc 恢复为已提交网址后触发）
    */
    void AttachUrlCancel(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventUrlCancel, callback, callbackID);
    }

    /** 监听左侧安全状态图标点击事件
    */
    void AttachIconClick(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventUrlBarIconClick, callback, callbackID);
    }

    /** 监听右侧收藏按钮点击事件
    */
    void AttachFavoriteClick(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventUrlBarFavoriteClick, callback, callbackID);
    }

protected:
    /** 初始化接口：创建安全状态图标、编辑框等子控件
    */
    virtual void OnInit() override;

    /** 是否需要绘制焦点框
    *   焦点实际落在内部编辑框上，这里改由本控件绘制焦点框，
    *   使焦点框与地址栏的胶囊形圆角边框保持一致
    */
    virtual bool ShouldPaintFocusRect() const override;

private:
    /** 编辑框文本变化处理：刷新联想下拉、安全状态图标并转发 kEventTextChanged
    */
    void OnEditInput();

    /** 编辑框失去焦点：关闭历史联想下拉
    */
    void OnEditKillFocus();

    /** 编辑框按键预处理（上下/回车/Esc）
    * @return true 表示按键已处理，不再传递给编辑框
    */
    bool HandleEditKeyDown(const EventArgs& msg);

    /** 提交编辑框中的输入：归一化为网址、记录历史并触发 kEventUrlChanged
    */
    void CommitInput();

    /** 取消编辑：恢复为最近一次提交的网址并触发 kEventUrlCancel
    */
    void CancelEdit();

    /** 刷新联想浮层（打开/更新/关闭），带防重入保护
    */
    void UpdatePopup();

    /** 刷新联想浮层的实际实现（仅由 UpdatePopup 调用）
    */
    void UpdatePopupInternal();

    /** 打开联想浮层
    */
    void OpenPopup();

    /** 关闭联想浮层（如已打开）
    */
    void ClosePopup();

    /** 候选项被确认（键盘回车/鼠标点击）：回填网址并提交
    * @param [in] nIndex 候选项在 m_candidates 中的序号
    */
    void OnCandidateSelected(size_t nIndex);

    /** 生成候选列表 m_candidates（按输入文本过滤历史记录）
    */
    void BuildCandidates();

    /** 判断单个候选项是否匹配输入文本（不区分大小写的包含匹配）
    */
    bool MatchItem(const DString& item, const DString& input) const;

    /** 按当前网址更新安全状态图标
    */
    void UpdateSecureIcon();

    /** 显示指定的安全状态图标
    */
    void ApplySecureIcon(bool bSecure);

    /** 从持久化文件加载网址历史
    */
    void LoadHistory();

    /** 保存网址历史到持久化文件
    */
    void SaveHistory();

    /** 获取网址历史文件的完整路径
    */
    DString GetHistoryFilePath() const;

private:
    /** 内部编辑框
    */
    RichEdit* m_pEdit;

    /** 安全状态图标（https）
    */
    Control* m_pIconSecure;

    /** 非安全状态图标（其它协议）
    */
    Control* m_pIconInsecure;

    /** 右侧收藏按钮
    */
    Control* m_pFavButton;

    /** 初始网址文本（OnInit 前缓存 XML 中的 text 属性）
    */
    UiString m_initText;

    /** 初始网址文本的多语言 ID（OnInit 前缓存）
    */
    UiString m_initTextId;

    /** 占位提示文字（OnInit 前缓存）
    */
    UiString m_promptText;

    /** 占位提示文字的多语言 ID（OnInit 前缓存）
    */
    UiString m_promptTextId;

    /** 占位提示文字颜色（OnInit 前缓存）
    */
    UiString m_promptColor;

    /** 内部编辑框是否启用默认右键菜单（OnInit 前缓存）
    */
    UiString m_defaultContextMenu;

    /** 搜索URL模板（包含"%s"占位符）
    */
    UiString m_searchUrlTemplate;

    /** 最近一次提交的网址（带完整协议）
    */
    UiString m_committedUrl;

    /** 当前是否处于安全状态（https）
    */
    bool m_bSecure;

    /** 编辑框文本是否被用户修改过（用于避免页面跳转覆盖用户输入）
    */
    bool m_bEditModifiedByUser;

    /** 浮层刷新防重入标记
    */
    bool m_bUpdatingPopup;

    /** 用户是否已用上下键导航候选（决定回车是"提交输入"还是"选中候选"）
    */
    bool m_bUserNavigated;

    /** 联想浮层（关闭后由框架销毁并置空）
    */
    AutoCompleteFlyout* m_pFlyout;

    /** 网址历史（MRU：最近使用的排在前面）
    */
    std::vector<UiString> m_history;

    /** 当前展示的候选列表（与浮层显示一致）
    */
    std::vector<UiString> m_candidates;

    /** 下拉最多可见候选行数（默认 8）
    */
    int32_t m_nMaxVisible;

    /** 网址历史最多保留条数（默认 20）
    */
    int32_t m_nMaxHistory;

    /** 是否记录并展示网址历史（默认 true）
    */
    bool m_bHistoryEnabled;

    /** 是否启用编辑框的清除按钮（默认 true）
    */
    bool m_bClearButtonEnabled;

    /** 是否显示右侧收藏按钮（默认 true）
    */
    bool m_bFavoriteButtonEnabled;

    /** 网址历史持久化文件（相对路径基于可执行程序目录）
    */
    UiString m_historyFile;
};

}//namespace ui

#endif //UI_CONTROL_URL_BAR_H_

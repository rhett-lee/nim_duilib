#ifndef UI_CONTROL_SEARCHBOX_H_
#define UI_CONTROL_SEARCHBOX_H_

#include "duilib/Box/HBox.h"
#include "duilib/Control/AutoComplete.h"
#include <vector>

namespace ui
{
class RichEdit;

/** 搜索框控件（SearchBox）：左侧搜索图标 + 中间编辑框 + 右侧清除按钮的组合控件
*
*   内部结构：
*   - 搜索图标：普通 Control，不响应鼠标、不获取焦点
*   - 编辑框：RichEdit，默认开启占位提示（prompt），按回车触发 kEventReturn 事件
*   - 清除按钮：复用 RichEdit 内置的 clear_btn_class 机制，文本非空时自动显示
*
*   XML 属性（除 HBox 通用属性外）：
*     prompt_text     占位提示文字
*     prompt_text_id  占位提示文字的多语言 ID
*     prompt_color    占位提示文字颜色
*     text            初始搜索文本
*     text_id         初始搜索文本的多语言 ID（与 text 同时设置时，后设置的属性生效）
*     debounce_interval 防抖间隔（毫秒）：搜索文本连续变化时，仅在输入停顿达到该间隔后才触发
*                      kEventTextChanged；设为 0（默认）则每次变化立即触发，向后兼容。
*                      用于避免搜索场景下每次按键都触发搜索请求。
 *     suggestions     初始建议数据源，多项以分号";"分隔（搜索历史由控件在提交时自动记录）
 *     max_visible     下拉最多可见候选行数（默认 8，超出出现滚动条）
 *     history_enabled 是否记录并展示搜索历史（默认 true）
 *     max_history     搜索历史最多保留条数（默认 10，超过后丢弃最旧）
 *     filter_enabled  是否按输入文本过滤候选（默认 true）；关闭则始终全量展示不过滤
 *     min_chars       过滤触发的最少输入字符数（默认 1）
 *     match_mode      过滤匹配模式：prefix（默认，前缀）/ contains（包含）
 *     ignore_case     过滤时是否忽略大小写（默认 true）
 *
 *   联想浮层展示规则（明确且一致）：
 *   - 编辑框获得焦点时立即评估并展开：有可展示内容则展开，否则不弹。
 *   - 文本每次变化实时刷新：无可展示内容时关闭。
 *   - 过滤开关 filter_enabled（默认 true）：开启时输入达到 min_chars 才按
 *       match_mode/ignore_case 过滤候选；输入为空或不足 min_chars 时展示全部
 *       （历史置顶 + 建议，去重）。关闭时始终全量展示（不做输入过滤）。
 *   - 浮层不抢焦点，编辑框失焦不作为关闭条件；点击浮层与锚点以外区域自动关闭。
 *   - 焦点归还：鼠标点击候选项时框架会先把原生键盘焦点移到浮层窗口，浮层关闭前
 *     统一交还本窗口（见 Flyout::DoClose），保证选中或按 Esc 收起后仍可继续键盘输入，
 *     且不会因此重新弹出浮层。
 *   - 选中候选项回填后不重新展开（抑制一次聚焦触发），避免刚选中又弹出的抖动。
 *   - 回车语义：未用上下键导航候选时，回车提交“输入框当前文本”；
 *       用上下键导航并高亮某候选后，回车选中该候选（回填并触发 kEventAutoCompleteSelect）。
*
*   事件：
*   - kEventTextChanged：搜索文本变化时触发（内部编辑框事件转发）
*   - kEventReturn：按下回车键且未选中候选项时触发，用于执行搜索（提交时会记录搜索历史）
*   - kEventAutoCompleteSelect：选中候选项（鼠标点击或键盘回车）后触发，WPARAM 为候选项
*                     在当前候选列表中的序号；此时 GetSearchText() 已被回填为候选项文本
*
*   皮肤类（global.xml 中定义）：search_box / search_box_icon /
*   search_box_edit / search_box_clear_btn；下拉浮层复用 AutoComplete 的皮肤类
*   auto_complete_flyout / auto_complete_list / auto_complete_item
*/
class DUILIB_API SearchBox : public HBox
{
    typedef HBox BaseClass;

    //内部编辑框子类需要调用私有的按键预处理接口
    friend class SearchBoxEdit;
public:
    explicit SearchBox(Window* pWindow);
    SearchBox(const SearchBox& r) = delete;
    SearchBox& operator=(const SearchBox& r) = delete;
    virtual ~SearchBox() override;

    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 让控件获取焦点（实际焦点设置到内部编辑框）
    */
    virtual void SetFocus() override;

    /** 获取内部编辑框控件
    */
    RichEdit* GetEditControl() const;

    /** 设置搜索文本
    */
    void SetSearchText(const DString& text);

    /** 获取搜索文本
    */
    DString GetSearchText() const;

    /** 设置防抖间隔（毫秒）
     *   搜索文本连续变化时，仅在输入停顿达到该间隔后才触发 kEventTextChanged；
     *   设为 0（默认）则每次变化立即触发（向后兼容）。搜索场景可用此避免每次按键都触发搜索请求。
     * @note 防抖同样作用于程序化 SetSearchText 引起的文本变化。
     */
    void SetDebounceInterval(size_t nIntervalMs);
    size_t GetDebounceInterval() const { return m_nDebounceIntervalMs; }

    /** @name 联想（建议）数据源
    *   建议数据源由业务层提供；搜索历史由控件在提交搜索时自动记录（见历史相关接口）
    */
    ///@{
    /** 设置建议数据源（浮层全量展示该数据与历史合并去重后的列表）
    */
    void SetCandidates(const std::vector<DString>& items);

    /** 获取建议数据源
    */
    std::vector<DString> GetCandidates() const;

    /** 追加一个建议项（已存在相同文本时忽略）
    */
    void AddCandidate(const DString& text);

    /** 清空建议数据源
    */
    void ClearCandidates();
    ///@}

    /** @name 联想展示参数
     */
    ///@{
    /** 设置下拉最多可见候选行数（超出出现滚动条）
     */
    void SetMaxVisible(int32_t nMaxVisible);
    int32_t GetMaxVisible() const;

    /** 设置是否按输入文本过滤候选（默认 true）
     *   开启时输入达到 min_chars 才过滤；关闭则始终全量展示（不做输入过滤）
     */
    void SetFilterEnabled(bool bEnabled);
    bool IsFilterEnabled() const;

    /** 设置过滤触发的最少输入字符数（默认 1）
     */
    void SetMinChars(int32_t nMinChars);
    int32_t GetMinChars() const;

    /** 设置过滤匹配模式（prefix 前缀 / contains 包含），默认 prefix
     */
    void SetMatchMode(AutoCompleteMatchMode matchMode);
    AutoCompleteMatchMode GetMatchMode() const;

    /** 设置过滤时是否忽略大小写（默认 true）
     */
    void SetIgnoreCase(bool bIgnoreCase);
    bool IsIgnoreCase() const;
    ///@}

    /** @name 搜索历史
    *   控件在“提交搜索”时（回车且未选中候选项，或选中候选项）自动记录搜索词，
    *   最多保留 max_history 条（MRU：最近使用的排在前面）。历史同时作为联想候选项参与匹配。
    */
    ///@{
    /** 设置是否记录并展示搜索历史
    */
    void SetHistoryEnabled(bool bEnabled);
    bool IsHistoryEnabled() const;

    /** 设置搜索历史最多保留条数（超过后丢弃最旧）
    */
    void SetMaxHistory(int32_t nMax);
    int32_t GetMaxHistory() const;

    /** 获取搜索历史（最近使用的排在前面）
    */
    std::vector<DString> GetHistory() const;

    /** 清空搜索历史
    */
    void ClearHistory();

    /** 主动追加一条搜索历史（通常无需业务层调用，提交搜索时控件自动记录）
    */
    void AddSearchHistory(const DString& text);
    ///@}

public:
    /** 监听候选项选中事件（WPARAM 为选中项在当前候选列表中的序号）
    */
    void AttachSelect(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventAutoCompleteSelect, callback, callbackID);
    }

    /** 监听回车按键按下事件（未选中候选项时触发，用于发起搜索）
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    * @param [in] callback 回车被按下的自定义回调函数
    */
    void AttachReturn(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventReturn, callback, callbackID);
    }

protected:
    /** 初始化接口：创建搜索图标、编辑框等子控件
    */
    virtual void OnInit() override;

private:
    /** 内部编辑框
    */
    RichEdit* m_pEdit;

    /** 初始搜索文本（OnInit 前缓存 XML 中的 text 属性）
    */
    UiString m_initText;

    /** 初始搜索文本的多语言 ID（OnInit 前缓存 XML 中的 text_id 属性）
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

    /** 防抖间隔（毫秒），0 表示不防抖（立即触发）
    */
    size_t m_nDebounceIntervalMs = 0;

    /** 待触发的防抖任务 ID（PostDelayedTask 返回值），0 表示无待触发任务
    */
    size_t m_nDebounceTaskId = 0;

    // ---- 联想（建议 + 历史）下拉 ----
    /** 联想浮层（关闭后由框架销毁并置空）
    */
    AutoCompleteFlyout* m_pFlyout = nullptr;

    /** 建议数据源
    */
    std::vector<UiString> m_items;

    /** 搜索历史（MRU：最近使用的排在前面）
    */
    std::vector<UiString> m_history;

    /** 当前展示的候选列表（与浮层显示一致）
    */
    std::vector<UiString> m_candidates;

    /** 下拉最多可见候选行数（默认 8）
     */
    int32_t m_nMaxVisible = 8;

    /** 是否按输入文本过滤候选（默认 true）；关闭则始终全量展示
     */
    bool m_bFilterEnabled = true;

    /** 过滤触发的最少输入字符数（默认 1）
     */
    int32_t m_nMinChars = 1;

    /** 过滤匹配模式（默认前缀）
     */
    AutoCompleteMatchMode m_matchMode = AutoCompleteMatchMode::kPrefix;

    /** 过滤时是否忽略大小写（默认 true）
     */
    bool m_bIgnoreCase = true;

    /** 用户是否已用上下键导航候选（决定回车是“提交输入”还是“选中候选”）
     */
    bool m_bUserNavigated = false;

    /** 是否记录并展示搜索历史（默认 true）
     */
    bool m_bHistoryEnabled = true;

    /** 搜索历史最多保留条数（默认 10）
    */
    int32_t m_nMaxHistory = 10;

    /** 选中候选项回填后由 SetFocus 触发的"展开"是否应被抑制（一次性标记）
     */
    bool m_bSuppressNextFocusPopup = false;

    /** 延迟清除"聚焦不展开"抑制标记的任务 ID（0 表示无待执行任务）
     */
    size_t m_nSuppressClearTaskId = 0;

    /** 浮层刷新防重入标记：AutoCompleteFlyout::ShowAt 内部恢复父窗口焦点时会同步触发
    *   编辑框 kEventSetFocus → OnEditFocus → UpdatePopup 嵌套调用，须阻断以避免重复创建浮层
    */
    bool m_bUpdatingPopup = false;

    /** 文本变化处理：实时刷新联想浮层，并按防抖间隔决定是否延迟触发 kEventTextChanged
    */
    void OnEditInput();

    /** 编辑框获得焦点：若可展示内容则展开联想浮层
    *   选中候选项回填后会再次 SetFocus，此时由 m_bSuppressNextFocusPopup 抑制一次重开
    */
    void OnEditFocus();

    /** 延迟清除"聚焦不展开"抑制标记
    *   浮层关闭时框架会把键盘焦点交还本窗口并同步补发编辑框 kEventSetFocus（见
    *   Flyout::DoClose 的焦点归还逻辑），该标记用于挡住这次补发导致的重新展开；
    *   延迟清除可避免误吞随后到来的合法聚焦（如用户再次点击编辑框）。
    */
    void ScheduleClearSuppressFlag();

    /** 编辑框按键预处理（浮层打开时拦截上下/回车/Esc）
    * @return true 表示按键已处理，不再传递给编辑框
    */
    bool HandleEditKeyDown(const EventArgs& msg);

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

    /** 候选项被确认（键盘回车/鼠标点击）
    * @param [in] nIndex 候选项在 m_candidates 中的序号
    */
    void OnCandidateSelected(size_t nIndex);

    /** 提交搜索（回车且无选中项时）：记录历史并触发 kEventReturn
    */
    void CommitSearch();

    /** 生成候选列表 m_candidates（建议 + 历史合并去重；过滤模式下按输入过滤）
     */
    void BuildCandidates();

    /** 判断单个候选项是否匹配输入文本（受 m_matchMode / m_bIgnoreCase 控制）
     */
    bool MatchItem(const DString& item, const DString& input) const;

    /** 解析 suggestions 属性（分号分隔）
    */
    void ParseInitItems(const DString& items);
};

} // namespace ui

#endif // UI_CONTROL_SEARCHBOX_H_

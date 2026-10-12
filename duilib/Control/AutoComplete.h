#ifndef UI_CONTROL_AUTO_COMPLETE_H_
#define UI_CONTROL_AUTO_COMPLETE_H_

#include "duilib/Core/Box.h"
#include "duilib/Control/Flyout.h"
#include <functional>
#include <vector>

namespace ui
{
class RichEdit;
class VListBox;

/** 自动完成的文本匹配模式
*/
enum class AutoCompleteMatchMode : uint8_t
{
    kPrefix     = 0,    //前缀匹配（默认）：候选项以输入文本开头
    kContains   = 1     //包含匹配：候选项任意位置包含输入文本
};

/** 自动完成联想浮层（AutoCompleteFlyout）：以 Flyout 承载候选列表
*
*   特点：
*   1. 默认不抢焦点（WS_EX_NOACTIVATE），显示期间编辑框持续获得键盘焦点，可直接继续输入
*   2. 候选项数量变化时，浮层高度随之调整（宽度固定为编辑框宽度）；文本变化时复用
*      同一个浮层窗口就地刷新内容，不销毁重建
*   3. 支持鼠标点击/悬停选择，键盘上下方向键、回车由宿主控件转发处理
*   4. 非模态窗口：对象在窗口关闭后由框架自动 delete，调用方不需要手动释放
*/
class DUILIB_API AutoCompleteFlyout : public Flyout
{
    typedef Flyout BaseClass;
public:
    /** 单个候选项被选中（鼠标点击/键盘回车）的回调，参数为候选项序号（从 0 开始）
    */
    typedef std::function<void(size_t nIndex)> ItemSelectEvent;

    /** 构造函数
    * @param [in] pParentWindow 父窗口（锚点控件必须属于该窗口）
    */
    explicit AutoCompleteFlyout(Window* pParentWindow);
    AutoCompleteFlyout(const AutoCompleteFlyout& r) = delete;
    AutoCompleteFlyout& operator=(const AutoCompleteFlyout& r) = delete;
    virtual ~AutoCompleteFlyout() override = default;

    /** 在锚点控件下方显示候选浮层
    * @param [in] pAnchor 锚点控件（宿主编辑框/容器）
    * @param [in] items 候选项文本列表
    * @param [in] nSelectedIndex 初始选中项序号
    * @param [in] nWidthDip 浮层宽度（DIP，通常取编辑框宽度）
    * @param [in] placement 期望弹出方位
    * @return 显示成功返回 true；失败时对象已被框架销毁
    */
    bool ShowAt(Control* pAnchor, const std::vector<UiString>& items, size_t nSelectedIndex,
                int32_t nWidthDip, Placement placement = Placement::Bottom);

    /** 就地更新候选项内容（复用已创建的浮层窗口，不销毁重建）
    *   仅刷新列表内容，并按新的候选项行数调整浮层高度、重新计算位置
    * @param [in] items 新的候选项文本列表（不可为空）
    * @param [in] nSelectedIndex 新的选中项序号
    * @return 更新成功返回 true（浮层窗口尚未创建时会返回 false）
    */
    bool UpdateItems(const std::vector<UiString>& items, size_t nSelectedIndex);

    /** 获取当前选中项序号，无选中项返回 Box::InvalidIndex
    */
    size_t GetSelectedIndex() const;

    /** 设置当前选中项（不触发选择事件）
    */
    void SetSelectedIndex(size_t nIndex);

    /** 移动当前选中项（键盘上下方向键）
    * @param [in] nDelta +1 向下一项，-1 向上一项；到达首尾后停留在边界
    */
    void MoveSelection(int32_t nDelta);

    /** 设置最多可见候选项行数（超出显示滚动条）
    */
    void SetMaxVisible(int32_t nMaxVisible);
    int32_t GetMaxVisible() const;

    /** 监听候选项选中事件（鼠标点击/键盘回车）
    */
    void AttachItemSelect(const ItemSelectEvent& callback);

public:
    //WindowImplBase 接口
    virtual void OnInitWindow() override;

private:
    /** 初始化内部控件、绑定事件
    */
    void InitControls();

    /** 按 m_items 构建列表内容
    */
    void RebuildList();

    /** 按当前候选行数调整内容容器（及根容器）的高度
    *   容器高度 = 行数 * 单行高度 + 容器自身内边距；
    *   忽略内边距会导致列表可用高度不足，从而错误地出现滚动条
    */
    void ApplyContentHeight();

    /** 计算单个列表项高度（DIP）
    */
    int32_t GetItemHeight() const;

private:
    /** 候选项文本
    */
    std::vector<UiString> m_items;

    /** 当前选中项序号
    */
    size_t m_nSelectedIndex;

    /** 最多可见行数
    */
    int32_t m_nMaxVisible;

    /** 浮层宽度（DIP）
    */
    int32_t m_nWidthDip;

    /** 浮层与锚点的间距（DIP）
    */
    int32_t m_nGapDip;

    /** 内部候选列表控件
    */
    VListBox* m_pList;

    /** 浮层内容容器（高度随候选行数变化）
    */
    Box* m_pContent;

    /** 候选项选中回调
    */
    std::vector<ItemSelectEvent> m_itemSelectCallbacks;
};

/** 自动完成输入框控件（AutoComplete）：边输入边联想下拉
*
*   内部结构：
*   - 编辑框：单行 RichEdit（class: auto_complete_edit），占满整个控件
*   - 联想浮层：输入文本满足条件时，在编辑框下方弹出 AutoCompleteFlyout 候选列表，
*               浮层不抢焦点，跟随编辑框宽度
*
*   与 FilterCombo 的区别：AutoComplete 以编辑框为主体，候选浮层跟随输入实时过滤，
*   不占用布局空间；FilterCombo 以下拉框为主体，点击按钮展开固定选项列表。
*
*   交互：
*   - 输入文本达到 min_chars 后，自动从数据源过滤并弹出候选浮层
*   - 上下方向键移动候选项，回车确认，Esc 关闭浮层；鼠标点击候选项直接确认
*   - 确认后用候选项文本回填编辑框（不触发文本变化事件），并触发 kEventAutoCompleteSelect
*
*   XML 属性（除 Box 通用属性外）：
*     items        初始候选数据源，多项以分号";"分隔
*     prompt_text  编辑框占位提示文字
*     prompt_text_id 编辑框占位提示文字的多语言 ID
*     prompt_color 编辑框占位提示文字颜色
*     min_chars    输入多少个字符后开始联想（默认 1）
*     max_visible  浮层最多可见候选行数（默认 8）
*     match_mode   匹配模式：prefix（默认，前缀）/ contains（包含）
*     ignore_case  匹配时是否忽略大小写（默认 true）
*
*   事件：
*   - kEventAutoCompleteSelect：选中候选项后触发，WPARAM 为候选项在当前候选列表中的序号
*   - kEventTextChanged：内部编辑框文本变化时触发
*
*   皮肤类（global.xml 中定义）：auto_complete / auto_complete_edit
*   浮层皮肤类：auto_complete_flyout / auto_complete_list / auto_complete_item
*/
class DUILIB_API AutoComplete : public Box
{
    typedef Box BaseClass;
public:
    explicit AutoComplete(Window* pWindow);
    AutoComplete(const AutoComplete& r) = delete;
    AutoComplete& operator=(const AutoComplete& r) = delete;
    virtual ~AutoComplete() override = default;

    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 让控件获取焦点（实际焦点设置到内部编辑框）
    */
    virtual void SetFocus() override;

    /** 设置候选数据源（联想始终基于该原始数据过滤）
    */
    void SetItems(const std::vector<DString>& items);

    /** 获取候选数据源
    */
    std::vector<DString> GetItems() const;

    /** 追加一个候选项（已存在相同文本时忽略）
    */
    void AddItem(const DString& text);

    /** 清空候选数据源
    */
    void ClearItems();

    /** 设置/获取编辑框文本（程序化设置不触发联想浮层）
    */
    void SetText(const DString& text);
    DString GetText() const;

    /** 设置最少输入字符数（达到后才弹出联想）
    */
    void SetMinChars(int32_t nMinChars);
    int32_t GetMinChars() const;

    /** 设置浮层最多可见候选行数
    */
    void SetMaxVisible(int32_t nMaxVisible);
    int32_t GetMaxVisible() const;

    /** 设置文本匹配模式
    */
    void SetMatchMode(AutoCompleteMatchMode matchMode);
    AutoCompleteMatchMode GetMatchMode() const;

    /** 设置匹配时是否忽略大小写
    */
    void SetIgnoreCase(bool bIgnoreCase);
    bool IsIgnoreCase() const;

    /** 获取内部编辑框控件
    */
    RichEdit* GetEditControl() const;

public:
    /** 监听候选项选中事件（WPARAM 为选中项在当前候选列表中的序号）
    */
    void AttachSelect(const EventCallback& callback, EventCallbackID callbackID = 0)
    {
        AttachEvent(kEventAutoCompleteSelect, callback, callbackID);
    }

protected:
    /** 初始化接口：创建内部编辑框
    */
    virtual void OnInit() override;

private:
    friend class AutoCompleteEdit;

    /** 编辑框文本变化处理：转发事件 + 刷新联想浮层
    */
    void HandleEditTextChanged();

    /** 编辑框按键预处理（浮层打开时拦截上下/回车/Esc）
    * @return true 表示按键已处理，不再传递给编辑框
    */
    bool HandleEditKeyDown(const EventArgs& msg);

    /** 根据编辑框当前文本重新过滤候选并刷新浮层（打开/更新/关闭）
    */
    void UpdatePopup();

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

    /** 按输入文本生成候选列表 m_candidates（始终基于原始数据源 m_items）
    */
    void BuildCandidates(const DString& input);

    /** 判断单个数据项是否匹配输入文本
    */
    bool MatchItem(const DString& item, const DString& input) const;

    /** 解析 items 属性（分号分隔）
    */
    void ParseInitItems(const DString& items);

private:
    /** 内部编辑框
    */
    RichEdit* m_pEdit;

    /** 原始候选数据源
    */
    std::vector<UiString> m_items;

    /** 当前过滤后的候选列表（与浮层显示一致）
    */
    std::vector<UiString> m_candidates;

    /** 联想浮层（关闭后由框架销毁并置空）
    */
    AutoCompleteFlyout* m_pFlyout;

    /** 最少输入字符数
    */
    int32_t m_nMinChars;

    /** 浮层最多可见候选行数
    */
    int32_t m_nMaxVisible;

    /** 文本匹配模式
    */
    AutoCompleteMatchMode m_matchMode;

    /** 是否忽略大小写
    */
    bool m_bIgnoreCase;

    /** 正在程序化回填文本（抑制文本变化回调，避免浮层重弹）
    */
    bool m_bInUpdate;

    /** OnInit 前缓存的 XML 属性
    */
    UiString m_promptText;
    UiString m_promptTextId;
    UiString m_promptColor;
};

} // namespace ui

#endif // UI_CONTROL_AUTO_COMPLETE_H_

#include "SearchBox.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/AutoComplete.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Callback.h"
#include "duilib/Core/Keycode.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>

namespace ui
{

/** SearchBox 内部使用的单行编辑框：联想浮层打开时拦截上下方向键、回车、Esc
*/
class SearchBoxEdit : public RichEdit
{
    typedef RichEdit BaseClass;
public:
    explicit SearchBoxEdit(Window* pWindow) :
        RichEdit(pWindow),
        m_pOwner(nullptr)
    {
    }

    /** 设置宿主控件
    */
    void SetOwner(SearchBox* pOwner)
    {
        m_pOwner = pOwner;
    }

    virtual bool OnKeyDown(const EventArgs& msg) override
    {
        //浮层打开时，导航类按键交给宿主处理，不传递给编辑框（避免移动光标/回车换行）
        if ((m_pOwner != nullptr) && m_pOwner->HandleEditKeyDown(msg)) {
            return true;
        }
        return BaseClass::OnKeyDown(msg);
    }

private:
    /** 宿主控件
    */
    SearchBox* m_pOwner;
};

SearchBox::SearchBox(Window* pWindow) :
    BaseClass(pWindow),
    m_pEdit(nullptr),
    m_nDebounceIntervalMs(0),
    m_nDebounceTaskId(0)
{
}

DString SearchBox::GetType() const
{
    return DUI_CTR_SEARCH_BOX;
}

void SearchBox::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    //这些属性需要转发给内部编辑框；OnInit 之前子控件尚未创建，先缓存
    if ((strName == _T("prompt_text")) || (strName == _T("prompttext"))) {
        m_promptText = strValue;
        if (m_pEdit != nullptr) {
            m_pEdit->SetAttribute(strName, strValue);
        }
    }
    else if ((strName == _T("prompt_text_id")) || (strName == _T("prompt_textid")) || (strName == _T("prompttextid"))) {
        m_promptTextId = strValue;
        if (m_pEdit != nullptr) {
            m_pEdit->SetAttribute(strName, strValue);
        }
    }
    else if ((strName == _T("prompt_color")) || (strName == _T("promptcolor"))) {
        m_promptColor = strValue;
        if (m_pEdit != nullptr) {
            m_pEdit->SetAttribute(strName, strValue);
        }
    }
    else if (strName == _T("text")) {
        m_initText = strValue;
        if (m_pEdit != nullptr) {
            //与其它属性保持一致，走 SetAttribute 转发，复用 RichEdit 的完整 text 处理
            //（含 IsReplaceNewline 换行符替换等）
            m_pEdit->SetAttribute(strName, strValue);
        }
    }
    else if ((strName == _T("text_id")) || (strName == _T("textid"))) {
        m_initTextId = strValue;
        if (m_pEdit != nullptr) {
            m_pEdit->SetAttribute(strName, strValue);
        }
    }
    else if (strName == _T("debounce_interval")) {
        SetDebounceInterval(static_cast<size_t>(StringUtil::StringToInt32(strValue.c_str())));
    }
    else if (strName == _T("suggestions")) {
        ParseInitItems(strValue2);
    }
    else if (strName == _T("max_visible")) {
        SetMaxVisible(StringUtil::StringToInt32(strValue.c_str()));
    }
    else if (strName == _T("history_enabled")) {
        m_bHistoryEnabled = (strValue2 == _T("true"));
    }
    else if (strName == _T("max_history")) {
        SetMaxHistory(StringUtil::StringToInt32(strValue.c_str()));
    }
    else if (strName == _T("filter_enabled")) {
        m_bFilterEnabled = (strValue2 == _T("true"));
    }
    else if (strName == _T("min_chars")) {
        SetMinChars(StringUtil::StringToInt32(strValue.c_str()));
    }
    else if (strName == _T("match_mode")) {
        if (strValue2 == _T("contains")) {
            m_matchMode = AutoCompleteMatchMode::kContains;
        }
        else {
            m_matchMode = AutoCompleteMatchMode::kPrefix;
        }
    }
    else if (strName == _T("ignore_case")) {
        m_bIgnoreCase = (strValue2 == _T("true"));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void SearchBox::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    //左侧搜索图标（不响应鼠标、不获取焦点，点击穿透到容器）
    Control* pIcon = new Control(GetWindow());
    pIcon->SetClass(_T("search_box_icon"));
    pIcon->SetNoFocus();
    pIcon->SetMouseEnabled(false);
    AddItem(pIcon);

    //中间编辑框
    SearchBoxEdit* pEdit = new SearchBoxEdit(GetWindow());
    pEdit->SetOwner(this);
    m_pEdit = pEdit;
    m_pEdit->SetClass(_T("search_box_edit"));
    m_pEdit->SetAttribute(_T("want_tab"), _T("false"));
    //默认开启占位提示
    m_pEdit->SetAttribute(_T("prompt_mode"), _T("true"));
    if (!m_promptText.empty()) {
        m_pEdit->SetAttribute(_T("prompt_text"), DString(m_promptText.c_str()));
    }
    if (!m_promptTextId.empty()) {
        m_pEdit->SetAttribute(_T("prompt_text_id"), DString(m_promptTextId.c_str()));
    }
    if (!m_promptColor.empty()) {
        m_pEdit->SetAttribute(_T("prompt_color"), DString(m_promptColor.c_str()));
    }
    //右侧清除按钮（文本非空时自动显示，点击清空，复用 RichEdit 内置机制）
    m_pEdit->SetAttribute(_T("clear_btn_class"), _T("search_box_clear_btn"));
    if (!m_initText.empty()) {
        m_pEdit->SetText(m_initText.c_str());
    }
    if (!m_initTextId.empty()) {
        m_pEdit->SetAttribute(_T("text_id"), DString(m_initTextId.c_str()));
    }
    AddItem(m_pEdit);

    //转发内部编辑框事件：文本变化、回车搜索、失焦关闭联想
    m_pEdit->AttachTextChanged([this](const EventArgs& /*args*/) {
        OnEditInput();
        return true;
        });
    m_pEdit->AttachReturn([this](const EventArgs& /*args*/) {
        //提交搜索（回车且浮层未拦截）：关闭浮层、记录历史并触发 kEventReturn
        CommitSearch();
        return true;
        });
    m_pEdit->AttachSetFocus([this](const EventArgs& /*args*/) {
        OnEditFocus();
        return true;
        });
    //注意：不挂 kEventKillFocus 关闭浮层。原因：
    //1. 浮层为不抢焦点模式，其外部点击检测（AutoDismiss）已覆盖“点击其他区域关闭”场景；
    //2. 浮层窗口创建过程中可能短暂夺走前台焦点（Flyout 内部会恢复父窗口焦点），
    //   若在 KillFocus 中关闭浮层，会在 ShowAt 调用栈内重入 DoClose，引发重复浮层/焦点抖动。
}

void SearchBox::SetFocus()
{
    if (m_pEdit != nullptr) {
        m_pEdit->SetFocus();
    }
    else {
        BaseClass::SetFocus();
    }
}

RichEdit* SearchBox::GetEditControl() const
{
    return m_pEdit;
}

void SearchBox::SetSearchText(const DString& text)
{
    if (m_pEdit != nullptr) {
        m_pEdit->SetText(text);
    }
    else {
        m_initText = text;
    }
}

DString SearchBox::GetSearchText() const
{
    if (m_pEdit != nullptr) {
        return m_pEdit->GetText();
    }
    return DString(m_initText.c_str());
}

void SearchBox::SetDebounceInterval(size_t nIntervalMs)
{
    m_nDebounceIntervalMs = nIntervalMs;
}

void SearchBox::OnEditInput()
{
    //实时刷新联想浮层（不防抖）；kEventTextChanged 仍按防抖间隔延迟触发
    UpdatePopup();
    if (m_nDebounceIntervalMs > 0) {
        //取消上一次未触发的防抖任务，确保仅在输入停顿达到间隔后触发一次
        if (m_nDebounceTaskId != 0) {
            GlobalManager::Instance().Thread().CancelTask(m_nDebounceTaskId);
            m_nDebounceTaskId = 0;
        }
        m_nDebounceTaskId = GlobalManager::Instance().Thread().PostDelayedTask(
            ui::kThreadUI,
            UiBind(this, [this]() {
                m_nDebounceTaskId = 0;
                SendEvent(kEventTextChanged);
            }),
            static_cast<int32_t>(m_nDebounceIntervalMs));
    }
    else {
        SendEvent(kEventTextChanged);
    }
}

SearchBox::~SearchBox()
{
    if (m_nDebounceTaskId != 0) {
        GlobalManager::Instance().Thread().CancelTask(m_nDebounceTaskId);
        m_nDebounceTaskId = 0;
    }
    if (m_nSuppressClearTaskId != 0) {
        GlobalManager::Instance().Thread().CancelTask(m_nSuppressClearTaskId);
        m_nSuppressClearTaskId = 0;
    }
    ClosePopup();
}

void SearchBox::OnEditFocus()
{
    //选中候选项回填后会再次 SetFocus，此时不应重新展开浮层（避免刚选中又弹出）
    if (m_bSuppressNextFocusPopup) {
        m_bSuppressNextFocusPopup = false;
        return;
    }
    UpdatePopup();
}

void SearchBox::ScheduleClearSuppressFlag()
{
    if (m_nSuppressClearTaskId != 0) {
        GlobalManager::Instance().Thread().CancelTask(m_nSuppressClearTaskId);
        m_nSuppressClearTaskId = 0;
    }
    //延迟清除抑制标记：浮层关闭时框架同步补发的 kEventSetFocus 由该标记挡住，
    //下一消息周期后恢复正常的“聚焦即展开”行为，避免误吞用户后续的合法聚焦
    m_nSuppressClearTaskId = GlobalManager::Instance().Thread().PostDelayedTask(
        ui::kThreadUI,
        UiBind(this, [this]() {
            m_nSuppressClearTaskId = 0;
            m_bSuppressNextFocusPopup = false;
        }),
        100);
}

void SearchBox::CommitSearch()
{
    //抑制随后由 Toast 恢复焦点 / SetFocus 可能触发的 kEventSetFocus → UpdatePopup 重新展开，
    //确保提交（或选中）后下拉保持关闭；该标记在下一消息周期清除，避免误吞后续合法聚焦
    m_bSuppressNextFocusPopup = true;
    ClosePopup();
    AddSearchHistory(GetSearchText());
    if ((m_pEdit != nullptr) && !m_pEdit->IsFocused()) {
        //回车提交后确保编辑框持有焦点，保证可以连续输入
        m_pEdit->SetFocus();
    }
    SendEvent(kEventReturn);
    //延迟清除抑制标记：覆盖 Toast 恢复焦点可能触发的异步 kEventSetFocus，
    //随后恢复正常的“聚焦即展开”行为
    ScheduleClearSuppressFlag();
}

// ----------------------------------------------------------------------------
// SearchBox 联想（建议 + 历史）下拉
// ----------------------------------------------------------------------------

void SearchBox::SetCandidates(const std::vector<DString>& items)
{
    m_items.clear();
    for (const DString& text : items) {
        DString item = StringUtil::Trim(text.c_str());
        if (!item.empty()) {
            m_items.push_back(item);
        }
    }
}

std::vector<DString> SearchBox::GetCandidates() const
{
    std::vector<DString> result;
    result.reserve(m_items.size());
    for (const UiString& item : m_items) {
        result.push_back(item.c_str());
    }
    return result;
}

void SearchBox::AddCandidate(const DString& text)
{
    DString item = StringUtil::Trim(text.c_str());
    if (item.empty()) {
        return;
    }
    for (const UiString& existing : m_items) {
        if (item == existing.c_str()) {
            return;
        }
    }
    m_items.push_back(item);
}

void SearchBox::ClearCandidates()
{
    m_items.clear();
    ClosePopup();
}

void SearchBox::SetMaxVisible(int32_t nMaxVisible)
{
    if (nMaxVisible < 1) {
        nMaxVisible = 1;
    }
    if (nMaxVisible > 100) {
        nMaxVisible = 100;
    }
    m_nMaxVisible = nMaxVisible;
    if (m_pFlyout != nullptr) {
        m_pFlyout->SetMaxVisible(m_nMaxVisible);
    }
}

int32_t SearchBox::GetMaxVisible() const
{
    return m_nMaxVisible;
}

void SearchBox::SetFilterEnabled(bool bEnabled)
{
    m_bFilterEnabled = bEnabled;
    //过滤开关变化后立即按新规则刷新浮层
    if (m_pEdit != nullptr) {
        UpdatePopup();
    }
}

bool SearchBox::IsFilterEnabled() const
{
    return m_bFilterEnabled;
}

void SearchBox::SetMinChars(int32_t nMinChars)
{
    if (nMinChars < 0) {
        nMinChars = 0;
    }
    if (nMinChars > 100) {
        nMinChars = 100;
    }
    m_nMinChars = nMinChars;
}

int32_t SearchBox::GetMinChars() const
{
    return m_nMinChars;
}

void SearchBox::SetMatchMode(AutoCompleteMatchMode matchMode)
{
    m_matchMode = matchMode;
}

AutoCompleteMatchMode SearchBox::GetMatchMode() const
{
    return m_matchMode;
}

void SearchBox::SetIgnoreCase(bool bIgnoreCase)
{
    m_bIgnoreCase = bIgnoreCase;
}

bool SearchBox::IsIgnoreCase() const
{
    return m_bIgnoreCase;
}

void SearchBox::SetHistoryEnabled(bool bEnabled)
{
    m_bHistoryEnabled = bEnabled;
    if (!m_bHistoryEnabled) {
        ClosePopup();
    }
}

bool SearchBox::IsHistoryEnabled() const
{
    return m_bHistoryEnabled;
}

void SearchBox::SetMaxHistory(int32_t nMax)
{
    if (nMax < 1) {
        nMax = 1;
    }
    if (nMax > 1000) {
        nMax = 1000;
    }
    m_nMaxHistory = nMax;
    while (static_cast<int32_t>(m_history.size()) > m_nMaxHistory) {
        m_history.pop_back();
    }
}

int32_t SearchBox::GetMaxHistory() const
{
    return m_nMaxHistory;
}

std::vector<DString> SearchBox::GetHistory() const
{
    std::vector<DString> result;
    result.reserve(m_history.size());
    for (const UiString& item : m_history) {
        result.push_back(item.c_str());
    }
    return result;
}

void SearchBox::ClearHistory()
{
    m_history.clear();
    ClosePopup();
}

void SearchBox::AddSearchHistory(const DString& text)
{
    UiString t(StringUtil::Trim(text.c_str()).c_str());
    if (t.empty() || !m_bHistoryEnabled) {
        return;
    }
    //移除已存在的相同项，再置顶（MRU），避免重复
    //注意：必须用内容比较（*it == t），不能用 c_str() 指针比较（两个 UiString 缓冲区不同，指针永远不相等）
    for (auto it = m_history.begin(); it != m_history.end(); ++it) {
        if (*it == t) {
            m_history.erase(it);
            break;
        }
    }
    m_history.insert(m_history.begin(), t);
    while (static_cast<int32_t>(m_history.size()) > m_nMaxHistory) {
        m_history.pop_back();
    }
}

bool SearchBox::HandleEditKeyDown(const EventArgs& msg)
{
    if ((m_pFlyout == nullptr) || !m_pFlyout->IsOpen()) {
        //浮层未打开：交还编辑框默认处理（回车 → kEventReturn）
        return false;
    }
    if (msg.vkCode == kVK_DOWN) {
        m_bUserNavigated = true;
        m_pFlyout->MoveSelection(1);
        return true;
    }
    if (msg.vkCode == kVK_UP) {
        m_bUserNavigated = true;
        m_pFlyout->MoveSelection(-1);
        return true;
    }
    if (msg.vkCode == kVK_ESCAPE) {
        ClosePopup();
        return true;
    }
    if (msg.vkCode == kVK_RETURN) {
        //已用上下键导航并高亮某候选：回车选中该候选（回填并触发 kEventAutoCompleteSelect）
        if (m_bUserNavigated) {
            const size_t nIndex = m_pFlyout->GetSelectedIndex();
            if (nIndex != Box::InvalidIndex) {
                OnCandidateSelected(nIndex);
                return true;
            }
        }
        //未导航或浮层未打开：提交“输入框当前文本”（记录历史 + kEventReturn），不回填候选项
        CommitSearch();
        return true;
    }
    return false;
}

void SearchBox::UpdatePopup()
{
    //防重入：AutoCompleteFlyout::ShowAt 创建浮层窗口时可能短暂夺走父窗口前台焦点，
    //框架随即恢复父窗口焦点，这会同步触发编辑框 kEventSetFocus → OnEditFocus →
    //UpdatePopup 嵌套调用；不阻断会重复创建浮层（出现两个下拉）
    if (m_bUpdatingPopup) {
        return;
    }
    m_bUpdatingPopup = true;
    UpdatePopupInternal();
    m_bUpdatingPopup = false;
}

void SearchBox::UpdatePopupInternal()
{
    if (m_pEdit == nullptr) {
        return;
    }
    //每次重建候选列表时清除“已导航”状态：导航只针对当前候选集有效，
    //文本变化后候选集已变，回车应重新按“提交输入文本”处理
    m_bUserNavigated = false;
    BuildCandidates();

    if (m_candidates.empty()) {
        ClosePopup();
        return;
    }

    //浮层已打开时：就地刷新候选项内容与高度，复用同一个窗口
    if ((m_pFlyout != nullptr) && m_pFlyout->IsOpen()) {
        if (m_pFlyout->UpdateItems(m_candidates, 0)) {
            return;
        }
        //更新失败（窗口已失效）：退回到重建路径
        m_pFlyout = nullptr;
    }
    OpenPopup();
}

void SearchBox::OpenPopup()
{
    if (m_pEdit == nullptr) {
        return;
    }
    //浮层宽度与控件客户区一致（GetPos 为物理像素，XML 尺寸单位为 DIP，需换算）
    int32_t nWidth = GetPos().Width();
    if (nWidth <= 0) {
        return;
    }
    Window* pOwnerWindow = GetWindow();
    if (pOwnerWindow != nullptr) {
        if (pOwnerWindow->Dpi().HasPixelDensity()) {
            pOwnerWindow->Dpi().ClientSizeToWindowSize(nWidth);
        }
        else {
            nWidth = pOwnerWindow->Dpi().GetUnscaleWindowSize(nWidth);
        }
    }

    AutoCompleteFlyout* pFlyout = new AutoCompleteFlyout(GetWindow());
    pFlyout->SetMaxVisible(m_nMaxVisible);
    pFlyout->AttachItemSelect(UiBind(this, [this](size_t nIndex) {
        OnCandidateSelected(nIndex);
        }, std::placeholders::_1));
    pFlyout->AttachClosed(UiBind(this, [this, pFlyout](Flyout::CloseReason reason) {
        m_pFlyout = nullptr;
        //鼠标点击过浮层内容时，框架会把原生键盘焦点移到浮层窗口；浮层关闭时
        //Flyout::DoClose 会把焦点交还本窗口，并同步补发编辑框 kEventSetFocus。
        //除“点击外部”（焦点已合法落到别处）外的关闭都不应因此重新展开浮层
        //（例如 Esc 的意图就是收起），用一次性抑制标记挡住这次补发。
        //仅在浮层确实持有焦点（即焦点交还一定会发生）时才置位，避免无谓抑制。
        if ((reason != Flyout::CloseReason::kClickOutside) && pFlyout->IsWindowFocused()) {
            m_bSuppressNextFocusPopup = true;
            ScheduleClearSuppressFlag();
        }
        }, std::placeholders::_1));

    //ShowAt 失败时对象已被框架销毁，回调不会再触发
    if (pFlyout->ShowAt(this, m_candidates, 0, nWidth, Flyout::Placement::Bottom)) {
        m_pFlyout = pFlyout;
    }
}

void SearchBox::ClosePopup()
{
    if (m_pFlyout != nullptr) {
        //先置空，避免 Dismiss 回调期间重入
        AutoCompleteFlyout* pFlyout = m_pFlyout;
        m_pFlyout = nullptr;
        pFlyout->Dismiss();
    }
}

void SearchBox::OnCandidateSelected(size_t nIndex)
{
    if (nIndex >= m_candidates.size()) {
        return;
    }
    const DString text = m_candidates[nIndex].c_str();

    //回填编辑框但不触发 textChanged，避免回填后浮层重新弹出
    if (m_pEdit != nullptr) {
        m_pEdit->SetTextNoEvent(text);
        const int32_t nTextLength = m_pEdit->GetTextLength();
        m_pEdit->SetSel(nTextLength, nTextLength);
    }
    //抑制随后由 Toast 恢复焦点触发的重新展开（选中后下拉应关闭）
    m_bSuppressNextFocusPopup = true;
    ClosePopup();
    AddSearchHistory(text);
    if ((m_pEdit != nullptr) && !m_pEdit->IsFocused()) {
        //回填后重新聚焦，保证可以连续输入
        m_pEdit->SetFocus();
    }
    SendEvent(kEventAutoCompleteSelect, static_cast<WPARAM>(nIndex));
    //延迟清除抑制标记：覆盖 Toast 恢复焦点可能触发的异步 kEventSetFocus
    ScheduleClearSuppressFlag();
}

void SearchBox::BuildCandidates()
{
    m_candidates.clear();
    //收集合并数据源：历史（置顶）+ 建议，去重
    std::vector<UiString> merged;
    merged.reserve(m_history.size() + m_items.size());
    auto pushUnique = [&merged](const UiString& s) {
        for (const UiString& e : merged) {
            if (e == s) {   //内容比较，不能用 c_str() 指针比较
                return;
            }
        }
        merged.push_back(s);
    };
    if (m_bHistoryEnabled) {
        for (const UiString& h : m_history) {
            pushUnique(h);
        }
    }
    for (const UiString& s : m_items) {
        pushUnique(s);
    }

    if (!m_bFilterEnabled) {
        //不过滤：全量展示（历史 + 建议）
        m_candidates.swap(merged);
        return;
    }

    //过滤模式：输入达到 min_chars 才过滤；否则（空输入/不足）展示全部（聚焦态）
    const DString input = StringUtil::Trim(m_pEdit->GetText().c_str());
    if (static_cast<int32_t>(input.size()) < m_nMinChars) {
        m_candidates.swap(merged);
        return;
    }
    std::vector<UiString> filtered;
    filtered.reserve(merged.size());
    for (const UiString& s : merged) {
        if (MatchItem(s.c_str(), input)) {
            filtered.push_back(s);
        }
    }
    m_candidates.swap(filtered);
}

bool SearchBox::MatchItem(const DString& item, const DString& input) const
{
    if (input.empty()) {
        return true;
    }
    if (!m_bIgnoreCase) {
        const size_t pos = item.find(input);
        return (m_matchMode == AutoCompleteMatchMode::kContains) ? (pos != DString::npos) : (pos == 0);
    }
    const DString lowerItem = StringUtil::MakeLowerString(item);
    const DString lowerInput = StringUtil::MakeLowerString(input);
    const size_t pos = lowerItem.find(lowerInput);
    return (m_matchMode == AutoCompleteMatchMode::kContains) ? (pos != DString::npos) : (pos == 0);
}

void SearchBox::ParseInitItems(const DString& items)
{
    std::list<DString> itemList = StringUtil::Split(items, _T(";"));
    for (const DString& item : itemList) {
        AddCandidate(item);
    }
}

} // namespace ui

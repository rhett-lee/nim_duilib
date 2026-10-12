#include "duilib/Control/AutoComplete.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Box/ListBox.h"
#include "duilib/Box/ListBoxItem.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/Keycode.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/Callback.h"
#include <algorithm>
#include <functional>

namespace ui
{
/** AutoComplete 内部使用的单行编辑框：联想浮层打开时拦截上下方向键、回车、Esc
*/
class AutoCompleteEdit : public RichEdit
{
    typedef RichEdit BaseClass;
public:
    explicit AutoCompleteEdit(Window* pWindow) :
        RichEdit(pWindow),
        m_pOwner(nullptr)
    {
    }

    /** 设置宿主控件
    */
    void SetOwner(AutoComplete* pOwner)
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
    AutoComplete* m_pOwner;
};

AutoCompleteFlyout::AutoCompleteFlyout(Window* pParentWindow) :
    Flyout(pParentWindow),
    m_nSelectedIndex(Box::InvalidIndex),
    m_nMaxVisible(8),
    m_nWidthDip(200),
    m_nGapDip(4),
    m_pList(nullptr)
{
    //浮层不抢焦点：父窗保持激活，编辑框可持续输入
    SetNoFocus(true);
    //点浮层外部自动关闭
    SetAutoDismiss(true);
    //固定显示在锚点下方，不做上下翻转
    SetAllowFlip(false);
    SetGap(m_nGapDip);
}

bool AutoCompleteFlyout::ShowAt(Control* pAnchor, const std::vector<UiString>& items, size_t nSelectedIndex,
                                int32_t nWidthDip, Placement placement)
{
    if ((pAnchor == nullptr) || items.empty() || (nWidthDip <= 0)) {
        delete this;
        return false;
    }
    m_items = items;
    m_nSelectedIndex = nSelectedIndex;
    m_nWidthDip = nWidthDip;

    const int32_t nRows = std::min<int32_t>(static_cast<int32_t>(m_items.size()), m_nMaxVisible);
    const int32_t nHeight = nRows * GetItemHeight();

    //内嵌皮肤 XML：Window size 为不含阴影的客户区尺寸，框架自动附加阴影
    //浮层随候选项变化时由宿主关闭后重新创建，故这里按当前行数给出固定尺寸即可
    DString sXml = StringUtil::Printf(
        _T("<Window size=\"%d,%d\" caption=\"0,0,0,0\" use_system_caption=\"false\" ")
        _T("shadow_type=\"default\" shadow_attached=\"true\" layered_window=\"true\" size_box=\"0,0,0,0\">")
        _T("<VBox class=\"auto_complete_flyout\" width=\"%d\" height=\"%d\">")
        _T("<VListBox class=\"auto_complete_list\" name=\"auto_complete_list\" ")
        _T("width=\"stretch\" height=\"stretch\"/>")
        _T("</VBox></Window>"),
        m_nWidthDip, nHeight, m_nWidthDip, nHeight);

    return Flyout::ShowAt(pAnchor, sXml, placement);
}

void AutoCompleteFlyout::OnInitWindow()
{
    BaseClass::OnInitWindow();
    InitControls();
}

void AutoCompleteFlyout::InitControls()
{
    ASSERT(m_pList == nullptr);
    if (m_pList != nullptr) {
        return;
    }
    m_pList = dynamic_cast<VListBox*>(FindControl(_T("auto_complete_list")));
    ASSERT(m_pList != nullptr);
    if (m_pList == nullptr) {
        return;
    }

    //鼠标点击列表项：确认选择
    m_pList->AttachClick([this](const EventArgs& msg) {
        const size_t nIndex = static_cast<size_t>(msg.wParam);
        if (nIndex < m_items.size()) {
            std::vector<ItemSelectEvent> callbacks = m_itemSelectCallbacks;
            for (const ItemSelectEvent& callback : callbacks) {
                callback(nIndex);
            }
        }
        return true;
        });

    //鼠标悬停列表项：同步高亮（与键盘导航状态保持一致）
    m_pList->AttachItemMouseEnter([this](const EventArgs& msg) {
        const size_t nIndex = static_cast<size_t>(msg.wParam);
        if (nIndex < m_items.size()) {
            m_pList->SelectItem(nIndex, false, false);
        }
        return true;
        });

    RebuildList();
}

void AutoCompleteFlyout::RebuildList()
{
    ASSERT(m_pList != nullptr);
    if (m_pList == nullptr) {
        return;
    }
    m_pList->RemoveAllItems();
    for (const UiString& text : m_items) {
        ListBoxItem* pItem = new ListBoxItem(this);
        pItem->SetClass(_T("auto_complete_item"));
        pItem->SetText(text.c_str());
        pItem->SetFixedHeight(UiFixedInt(GetItemHeight()), true, true);
        m_pList->AddItem(pItem);
    }
    if (m_nSelectedIndex < m_items.size()) {
        m_pList->SelectItem(m_nSelectedIndex, false, false);
        m_pList->EnsureVisible(m_nSelectedIndex, ListBoxVerVisible::kVisible, ListBoxHorVisible::kVisible);
    }
}

size_t AutoCompleteFlyout::GetSelectedIndex() const
{
    if (m_pList != nullptr) {
        return m_pList->GetCurSel();
    }
    return (m_nSelectedIndex < m_items.size()) ? m_nSelectedIndex : Box::InvalidIndex;
}

void AutoCompleteFlyout::SetSelectedIndex(size_t nIndex)
{
    if (nIndex >= m_items.size()) {
        return;
    }
    m_nSelectedIndex = nIndex;
    if (m_pList != nullptr) {
        m_pList->SelectItem(nIndex, false, false);
        m_pList->EnsureVisible(nIndex, ListBoxVerVisible::kVisible, ListBoxHorVisible::kVisible);
    }
}

void AutoCompleteFlyout::MoveSelection(int32_t nDelta)
{
    if (m_items.empty()) {
        return;
    }
    size_t nCurrent = GetSelectedIndex();
    if (nCurrent == Box::InvalidIndex) {
        SetSelectedIndex(nDelta > 0 ? 0 : (m_items.size() - 1));
        return;
    }
    int32_t nIndex = static_cast<int32_t>(nCurrent) + nDelta;
    if (nIndex < 0) {
        nIndex = 0;
    }
    if (nIndex >= static_cast<int32_t>(m_items.size())) {
        nIndex = static_cast<int32_t>(m_items.size()) - 1;
    }
    SetSelectedIndex(static_cast<size_t>(nIndex));
}

void AutoCompleteFlyout::SetMaxVisible(int32_t nMaxVisible)
{
    if (nMaxVisible < 1) {
        nMaxVisible = 1;
    }
    if (nMaxVisible > 100) {
        nMaxVisible = 100;
    }
    m_nMaxVisible = nMaxVisible;
}

int32_t AutoCompleteFlyout::GetMaxVisible() const
{
    return m_nMaxVisible;
}

void AutoCompleteFlyout::AttachItemSelect(const ItemSelectEvent& callback)
{
    m_itemSelectCallbacks.push_back(callback);
}

int32_t AutoCompleteFlyout::GetItemHeight() const
{
    //与 global.xml 中 auto_complete_item 的 fixed_height 保持一致（DIP）
    return 28;
}

// ----------------------------------------------------------------------------
// AutoComplete
// ----------------------------------------------------------------------------

AutoComplete::AutoComplete(Window* pWindow) :
    BaseClass(pWindow),
    m_pEdit(nullptr),
    m_pFlyout(nullptr),
    m_nMinChars(1),
    m_nMaxVisible(8),
    m_matchMode(AutoCompleteMatchMode::kPrefix),
    m_bIgnoreCase(true),
    m_bInUpdate(false)
{
}

DString AutoComplete::GetType() const
{
    return DUI_CTR_AUTO_COMPLETE;
}

void AutoComplete::SetAttribute(const DString& strName, const DString& strValue)
{
    DString strValue2 = GetExpandVarStrings(strValue);
    if (strName == _T("items")) {
        ParseInitItems(strValue2);
    }
    else if (strName == _T("prompt_text")) {
        m_promptText = strValue2;
        if (m_pEdit != nullptr) {
            m_pEdit->SetAttribute(_T("prompt_text"), strValue2);
        }
    }
    else if (strName == _T("prompt_text_id")) {
        m_promptTextId = strValue2;
        if (m_pEdit != nullptr) {
            m_pEdit->SetAttribute(_T("prompt_text_id"), strValue2);
        }
    }
    else if (strName == _T("prompt_color")) {
        m_promptColor = strValue2;
        if (m_pEdit != nullptr) {
            m_pEdit->SetAttribute(_T("prompt_color"), strValue2);
        }
    }
    else if (strName == _T("min_chars")) {
        SetMinChars(StringUtil::StringToInt32(strValue2.c_str()));
    }
    else if (strName == _T("max_visible")) {
        SetMaxVisible(StringUtil::StringToInt32(strValue2.c_str()));
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

void AutoComplete::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    ASSERT(m_pEdit == nullptr);
    if (m_pEdit == nullptr) {
        AutoCompleteEdit* pEdit = new AutoCompleteEdit(GetWindow());
        pEdit->SetOwner(this);
        pEdit->SetClass(_T("auto_complete_edit"));
        pEdit->SetAttribute(_T("single_line"), _T("true"));
        pEdit->SetAttribute(_T("want_tab"), _T("false"));
        pEdit->SetAttribute(_T("want_return"), _T("false"));
        pEdit->SetAttribute(_T("prompt_mode"), _T("true"));
        if (!m_promptText.empty()) {
            pEdit->SetAttribute(_T("prompt_text"), m_promptText.c_str());
        }
        if (!m_promptTextId.empty()) {
            pEdit->SetAttribute(_T("prompt_text_id"), m_promptTextId.c_str());
        }
        if (!m_promptColor.empty()) {
            pEdit->SetAttribute(_T("prompt_color"), m_promptColor.c_str());
        }
        Box::AddItem(pEdit);
        m_pEdit = pEdit;

        //文本变化：转发事件并刷新联想浮层
        m_pEdit->AttachTextChanged(UiBind(this, [this](const EventArgs& /*msg*/) {
            HandleEditTextChanged();
            return true;
            }, std::placeholders::_1));

        //编辑框失去焦点：关闭联想浮层（点击不抢焦点的浮层不会导致失焦）
        m_pEdit->AttachKillFocus(UiBind(this, [this](const EventArgs& /*msg*/) {
            ClosePopup();
            return true;
            }, std::placeholders::_1));
    }
}

void AutoComplete::SetFocus()
{
    if (m_pEdit != nullptr) {
        m_pEdit->SetFocus();
    }
}

void AutoComplete::SetItems(const std::vector<DString>& items)
{
    m_items.clear();
    for (const DString& text : items) {
        DString item = StringUtil::Trim(text.c_str());
        if (!item.empty()) {
            m_items.push_back(item);
        }
    }
}

std::vector<DString> AutoComplete::GetItems() const
{
    std::vector<DString> result;
    result.reserve(m_items.size());
    for (const UiString& item : m_items) {
        result.push_back(item.c_str());
    }
    return result;
}

void AutoComplete::AddItem(const DString& text)
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

void AutoComplete::ClearItems()
{
    m_items.clear();
    ClosePopup();
}

void AutoComplete::SetText(const DString& text)
{
    //程序化设置文本，不触发联想浮层
    if (m_pEdit != nullptr) {
        m_pEdit->SetTextNoEvent(text);
    }
}

DString AutoComplete::GetText() const
{
    if (m_pEdit != nullptr) {
        return m_pEdit->GetText();
    }
    return DString();
}

void AutoComplete::SetMinChars(int32_t nMinChars)
{
    if (nMinChars < 0) {
        nMinChars = 0;
    }
    if (nMinChars > 100) {
        nMinChars = 100;
    }
    m_nMinChars = nMinChars;
}

int32_t AutoComplete::GetMinChars() const
{
    return m_nMinChars;
}

void AutoComplete::SetMaxVisible(int32_t nMaxVisible)
{
    if (nMaxVisible < 1) {
        nMaxVisible = 1;
    }
    if (nMaxVisible > 100) {
        nMaxVisible = 100;
    }
    m_nMaxVisible = nMaxVisible;
    if (m_pFlyout != nullptr) {
        m_pFlyout->SetMaxVisible(nMaxVisible);
    }
}

int32_t AutoComplete::GetMaxVisible() const
{
    return m_nMaxVisible;
}

void AutoComplete::SetMatchMode(AutoCompleteMatchMode matchMode)
{
    m_matchMode = matchMode;
}

AutoCompleteMatchMode AutoComplete::GetMatchMode() const
{
    return m_matchMode;
}

void AutoComplete::SetIgnoreCase(bool bIgnoreCase)
{
    m_bIgnoreCase = bIgnoreCase;
}

bool AutoComplete::IsIgnoreCase() const
{
    return m_bIgnoreCase;
}

RichEdit* AutoComplete::GetEditControl() const
{
    return m_pEdit;
}

bool AutoComplete::HandleEditKeyDown(const EventArgs& msg)
{
    if (m_pFlyout == nullptr) {
        return false;
    }
    if (msg.vkCode == kVK_DOWN) {
        m_pFlyout->MoveSelection(1);
        return true;
    }
    if (msg.vkCode == kVK_UP) {
        m_pFlyout->MoveSelection(-1);
        return true;
    }
    if (msg.vkCode == kVK_RETURN) {
        const size_t nIndex = m_pFlyout->GetSelectedIndex();
        if (nIndex != Box::InvalidIndex) {
            OnCandidateSelected(nIndex);
        }
        else {
            ClosePopup();
        }
        return true;
    }
    if (msg.vkCode == kVK_ESCAPE) {
        ClosePopup();
        return true;
    }
    return false;
}

void AutoComplete::HandleEditTextChanged()
{
    //转发文本变化事件
    SendEvent(kEventTextChanged);

    //程序化回填期间不重新弹出浮层
    if (m_bInUpdate) {
        return;
    }
    UpdatePopup();
}

void AutoComplete::UpdatePopup()
{
    if (m_pEdit == nullptr) {
        return;
    }
    const DString input = StringUtil::Trim(m_pEdit->GetText().c_str());
    BuildCandidates(input);

    if ((static_cast<int32_t>(input.size()) < m_nMinChars) || m_candidates.empty()) {
        ClosePopup();
        return;
    }

    //候选结果变化时：关闭旧浮层并按新行数重建（新浮层高度与候选数一致，
    //复用已实现的创建路径，避免对已创建的分层窗口做动态缩放导致列表布局异常）
    if (m_pFlyout != nullptr) {
        ClosePopup();
    }
    OpenPopup();
}

void AutoComplete::OpenPopup()
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
    //GetPos 返回布局后的物理像素，而拼入 XML 的尺寸是 DIP，需按当前 DPI 反算
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
    pFlyout->AttachClosed(UiBind(this, [this](Flyout::CloseReason /*reason*/) {
        m_pFlyout = nullptr;
        }, std::placeholders::_1));

    //ShowAt 失败时对象已被框架销毁，回调不会再触发
    if (pFlyout->ShowAt(this, m_candidates, 0, nWidth, Flyout::Placement::Bottom)) {
        m_pFlyout = pFlyout;
    }
}

void AutoComplete::ClosePopup()
{
    if (m_pFlyout != nullptr) {
        //先置空，避免 Dismiss 回调期间重入
        AutoCompleteFlyout* pFlyout = m_pFlyout;
        m_pFlyout = nullptr;
        pFlyout->Dismiss();
    }
}

void AutoComplete::OnCandidateSelected(size_t nIndex)
{
    if (nIndex >= m_candidates.size()) {
        return;
    }
    const DString text = m_candidates[nIndex].c_str();

    //回填编辑框但不触发 textChanged，避免回填后浮层重新弹出
    m_bInUpdate = true;
    if (m_pEdit != nullptr) {
        m_pEdit->SetTextNoEvent(text);
        const int32_t nTextLength = m_pEdit->GetTextLength();
        m_pEdit->SetSel(nTextLength, nTextLength);
    }
    m_bInUpdate = false;

    ClosePopup();
    if (m_pEdit != nullptr) {
        m_pEdit->SetFocus();
    }
    SendEvent(kEventAutoCompleteSelect, static_cast<WPARAM>(nIndex));
}

void AutoComplete::BuildCandidates(const DString& input)
{
    m_candidates.clear();
    if (input.empty()) {
        //输入为空且允许 0 字符联想时，展示全部数据
        for (const UiString& item : m_items) {
            m_candidates.push_back(item);
        }
        return;
    }
    for (const UiString& item : m_items) {
        if (MatchItem(item.c_str(), input)) {
            m_candidates.push_back(item);
        }
    }
}

bool AutoComplete::MatchItem(const DString& item, const DString& input) const
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

void AutoComplete::ParseInitItems(const DString& items)
{
    std::list<DString> itemList = StringUtil::Split(items, _T(";"));
    for (const DString& item : itemList) {
        AddItem(item);
    }
}

} // namespace ui

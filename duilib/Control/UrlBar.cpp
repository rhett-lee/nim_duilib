#include "UrlBar.h"
#include "UrlBarUtil.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/AutoComplete.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Callback.h"
#include "duilib/Core/Keycode.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{

/** UrlBar 内部使用的单行编辑框：拦截上下方向键、回车、Esc
*/
class UrlBarEdit : public RichEdit
{
    typedef RichEdit BaseClass;
public:
    explicit UrlBarEdit(Window* pWindow) :
        RichEdit(pWindow),
        m_pOwner(nullptr)
    {
    }

    /** 设置宿主控件
    */
    void SetOwner(UrlBar* pOwner)
    {
        m_pOwner = pOwner;
    }

    virtual bool OnKeyDown(const EventArgs& msg) override
    {
        //导航类按键交给宿主处理，不传递给编辑框（避免移动光标/回车换行）
        if ((m_pOwner != nullptr) && m_pOwner->HandleEditKeyDown(msg)) {
            return true;
        }
        return BaseClass::OnKeyDown(msg);
    }

private:
    /** 宿主控件
    */
    UrlBar* m_pOwner;
};

UrlBar::UrlBar(Window* pWindow) :
    BaseClass(pWindow),
    m_pEdit(nullptr),
    m_pIconSecure(nullptr),
    m_pIconInsecure(nullptr),
    m_bSecure(false),
    m_bEditModifiedByUser(false),
    m_bUserTyping(false),
    m_bSuppressNextFocusPopup(false),
    m_nSuppressClearTaskId(0),
    m_bUpdatingPopup(false),
    m_bUserNavigated(false),
    m_pFlyout(nullptr),
    m_nMaxVisible(8),
    m_nMaxHistory(20),
    m_bHistoryEnabled(true)
{
}

UrlBar::~UrlBar()
{
    if (m_nSuppressClearTaskId != 0) {
        GlobalManager::Instance().Thread().CancelTask(m_nSuppressClearTaskId);
        m_nSuppressClearTaskId = 0;
    }
    ClosePopup();
}

DString UrlBar::GetType() const
{
    return DUI_CTR_URL_BAR;
}

void UrlBar::SetAttribute(const DString& strName, const DString& strValue2)
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
            m_pEdit->SetAttribute(strName, strValue);
        }
    }
    else if ((strName == _T("text_id")) || (strName == _T("textid"))) {
        m_initTextId = strValue;
        if (m_pEdit != nullptr) {
            m_pEdit->SetAttribute(strName, strValue);
        }
    }
    else if (strName == _T("search_url_template")) {
        SetSearchUrlTemplate(strValue);
    }
    else if (strName == _T("history_enabled")) {
        SetHistoryEnabled(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("max_history")) {
        SetMaxHistory(StringUtil::StringToInt32(strValue.c_str()));
    }
    else if (strName == _T("history_file")) {
        SetHistoryFile(strValue);
    }
    else if (strName == _T("max_visible")) {
        SetMaxVisible(StringUtil::StringToInt32(strValue.c_str()));
    }
    else if (strName == _T("secure")) {
        SetSecure(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("default_context_menu")) {
        m_defaultContextMenu = strValue;
        if (m_pEdit != nullptr) {
            m_pEdit->SetAttribute(strName, strValue);
        }
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void UrlBar::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    //焦点框改由本控件绘制（与胶囊形圆角边框一致），内部编辑框不绘制直角焦点矩形
    SetShowFocusedRect(true);

    //左侧安全状态图标（不响应鼠标、不获取焦点）
    m_pIconSecure = new Control(GetWindow());
    m_pIconSecure->SetClass(_T("url_bar_icon_secure"));
    m_pIconSecure->SetNoFocus();
    m_pIconSecure->SetMouseEnabled(false);
    m_pIconSecure->SetVisible(false);
    AddItem(m_pIconSecure);

    m_pIconInsecure = new Control(GetWindow());
    m_pIconInsecure->SetClass(_T("url_bar_icon_insecure"));
    m_pIconInsecure->SetNoFocus();
    m_pIconInsecure->SetMouseEnabled(false);
    AddItem(m_pIconInsecure);

    //中间编辑框
    UrlBarEdit* pEdit = new UrlBarEdit(GetWindow());
    pEdit->SetOwner(this);
    m_pEdit = pEdit;
    m_pEdit->SetClass(_T("url_bar_edit"));
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
    if (!m_defaultContextMenu.empty()) {
        m_pEdit->SetAttribute(_T("default_context_menu"), DString(m_defaultContextMenu.c_str()));
    }
    //右侧清除按钮（文本非空时自动显示，点击清空，复用 RichEdit 内置机制）
    m_pEdit->SetAttribute(_T("clear_btn_class"), _T("url_bar_clear_btn"));
    //获得焦点时全选，便于直接输入新网址
    m_pEdit->SetSelAllOnFocus(true);
    if (!m_initText.empty()) {
        m_pEdit->SetText(m_initText.c_str());
        m_committedUrl = m_initText;
    }
    if (!m_initTextId.empty()) {
        m_pEdit->SetAttribute(_T("text_id"), DString(m_initTextId.c_str()));
    }
    AddItem(m_pEdit);

    //转发内部编辑框事件：文本变化、回车提交、聚焦展开联想
    m_pEdit->AttachTextChanged([this](const EventArgs& /*args*/) {
        OnEditInput();
        return true;
        });
    m_pEdit->AttachReturn([this](const EventArgs& /*args*/) {
        CommitInput();
        return true;
        });
    m_pEdit->AttachSetFocus([this](const EventArgs& /*args*/) {
        Invalidate();
        OnEditFocus();
        return true;
        });
    m_pEdit->AttachKillFocus([this](const EventArgs& /*args*/) {
        //焦点移出编辑框时重绘，更新本控件绘制的焦点框
        Invalidate();
        return true;
        });

    //加载网址历史
    LoadHistory();
    //按当前网址初始化安全状态图标
    UpdateSecureIcon();
}

bool UrlBar::ShouldPaintFocusRect() const
{
    //焦点实际落在内部编辑框上：编辑框获得焦点时，由本控件按胶囊形圆角绘制焦点框
    if ((m_pEdit == nullptr) || !m_pEdit->IsFocused() || !IsShowFocusedRect()) {
        return false;
    }
    //窗口不是激活状态时(如浮层菜单已打开)，不绘制焦点框
    Window* pWindow = GetWindow();
    if ((pWindow == nullptr) || !pWindow->IsWindowFocused()) {
        return false;
    }
    //与Windows原生行为一致：仅当最近一次输入来自键盘时才显示焦点框，鼠标点击后不显示
    return pWindow->IsKeyboardCueVisible();
}

void UrlBar::SetFocus()
{
    if (m_pEdit != nullptr) {
        m_pEdit->SetFocus();
    }
    else {
        BaseClass::SetFocus();
    }
}

RichEdit* UrlBar::GetEditControl() const
{
    return m_pEdit;
}

DString UrlBar::GetUrl() const
{
    return DString(m_committedUrl.c_str());
}

void UrlBar::SetUrl(const DString& url)
{
    m_committedUrl = url.c_str();
    if (m_pEdit == nullptr) {
        m_initText = url.c_str();
        UpdateSecureIcon();
        return;
    }
    //用户正在编辑时不覆盖其输入
    if (m_pEdit->IsFocused() && m_bEditModifiedByUser) {
        return;
    }
    m_bSuppressNextFocusPopup = true;
    m_pEdit->SetTextNoEvent(url);
    const int32_t nTextLength = m_pEdit->GetTextLength();
    m_pEdit->SetSel(nTextLength, nTextLength);
    m_bEditModifiedByUser = false;
    m_bUserTyping = false;
    m_bUserNavigated = false;
    ClosePopup();
    UpdateSecureIcon();
    ScheduleClearSuppressFlag();
}

DString UrlBar::GetEditText() const
{
    if (m_pEdit != nullptr) {
        return m_pEdit->GetText();
    }
    return DString(m_initText.c_str());
}

void UrlBar::SelectAll()
{
    if (m_pEdit != nullptr) {
        m_pEdit->SetSelAll();
    }
}

void UrlBar::Submit()
{
    CommitInput();
}

void UrlBar::SetSearchUrlTemplate(const DString& urlTemplate)
{
    m_searchUrlTemplate = urlTemplate.c_str();
}

DString UrlBar::GetSearchUrlTemplate() const
{
    return DString(m_searchUrlTemplate.c_str());
}

void UrlBar::SetSecure(bool bSecure)
{
    ApplySecureIcon(bSecure);
}

bool UrlBar::IsSecure() const
{
    return m_bSecure;
}

void UrlBar::SetHistoryEnabled(bool bEnabled)
{
    m_bHistoryEnabled = bEnabled;
    if (!m_bHistoryEnabled) {
        ClosePopup();
    }
}

bool UrlBar::IsHistoryEnabled() const
{
    return m_bHistoryEnabled;
}

void UrlBar::SetMaxHistory(int32_t nMax)
{
    if (nMax < 1) {
        nMax = 1;
    }
    if (nMax > 1000) {
        nMax = 1000;
    }
    m_nMaxHistory = nMax;
    const size_t nOldSize = m_history.size();
    while (static_cast<int32_t>(m_history.size()) > m_nMaxHistory) {
        m_history.pop_back();
    }
    if (m_history.size() != nOldSize) {
        SaveHistory();
    }
}

int32_t UrlBar::GetMaxHistory() const
{
    return m_nMaxHistory;
}

void UrlBar::SetMaxVisible(int32_t nMaxVisible)
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

int32_t UrlBar::GetMaxVisible() const
{
    return m_nMaxVisible;
}

std::vector<DString> UrlBar::GetHistory() const
{
    std::vector<DString> result;
    result.reserve(m_history.size());
    for (const UiString& item : m_history) {
        result.push_back(item.c_str());
    }
    return result;
}

void UrlBar::ClearHistory()
{
    m_history.clear();
    ClosePopup();
    SaveHistory();
}

void UrlBar::AddHistory(const DString& url)
{
    if (!m_bHistoryEnabled) {
        return;
    }
    const DString text = StringUtil::Trim(url.c_str());
    if (text.empty()) {
        return;
    }
    UiString historyUrl(text.c_str());
    for (auto it = m_history.begin(); it != m_history.end(); ++it) {
        //网址主机名不区分大小写，避免同一网址以不同大小写重复记录
        if (StringUtil::IsEqualNoCase(it->c_str(), historyUrl.c_str())) {
            m_history.erase(it);
            break;
        }
    }
    m_history.insert(m_history.begin(), historyUrl);
    while (static_cast<int32_t>(m_history.size()) > m_nMaxHistory) {
        m_history.pop_back();
    }
    SaveHistory();
}

void UrlBar::SetHistoryFile(const DString& historyFile)
{
    UiString newHistoryFile = historyFile.c_str();
    if (m_historyFile == newHistoryFile) {
        return;
    }
    m_historyFile = newHistoryFile;
    if (IsInited()) {
        //切换持久化文件后重新加载历史
        m_history.clear();
        LoadHistory();
    }
}

DString UrlBar::GetHistoryFile() const
{
    return DString(m_historyFile.c_str());
}

void UrlBar::OnEditInput()
{
    m_bEditModifiedByUser = true;
    m_bUserTyping = true;
    //输入过程中实时更新安全状态图标（例如粘贴了https网址）
    UpdateSecureIcon();
    //实时刷新联想浮层
    UpdatePopup();
    //转发文本变化事件
    SendEvent(kEventTextChanged);
}

void UrlBar::OnEditFocus()
{
    //选中候选项回填后会再次 SetFocus，此时不应重新展开浮层
    if (m_bSuppressNextFocusPopup) {
        m_bSuppressNextFocusPopup = false;
        return;
    }
    //聚焦时展示全部历史记录；后续输入时再按文本过滤
    m_bUserTyping = false;
    UpdatePopup();
}

void UrlBar::ScheduleClearSuppressFlag()
{
    if (m_nSuppressClearTaskId != 0) {
        GlobalManager::Instance().Thread().CancelTask(m_nSuppressClearTaskId);
        m_nSuppressClearTaskId = 0;
    }
    m_nSuppressClearTaskId = GlobalManager::Instance().Thread().PostDelayedTask(
        ui::kThreadUI,
        UiBind(this, [this]() {
            m_nSuppressClearTaskId = 0;
            m_bSuppressNextFocusPopup = false;
        }),
        100);
}

bool UrlBar::HandleEditKeyDown(const EventArgs& msg)
{
    if (msg.vkCode == kVK_ESCAPE) {
        CancelEdit();
        return true;
    }
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
    if (msg.vkCode == kVK_RETURN) {
        //已用上下键导航并高亮某候选：回车选中该候选
        if (m_bUserNavigated) {
            const size_t nIndex = m_pFlyout->GetSelectedIndex();
            if (nIndex != Box::InvalidIndex) {
                OnCandidateSelected(nIndex);
                return true;
            }
        }
        CommitInput();
        return true;
    }
    return false;
}

void UrlBar::CommitInput()
{
    if (m_pEdit == nullptr) {
        return;
    }
    const DString rawText = m_pEdit->GetText();
    const DString input = StringUtil::Trim(rawText.c_str());
    if (input.empty()) {
        //空输入不导航，仅关闭联想浮层
        ClosePopup();
        return;
    }
    const DString url = UrlNormalizeInput(input, DString(m_searchUrlTemplate.c_str()));
    if (url.empty()) {
        return;
    }

    m_bSuppressNextFocusPopup = true;
    ClosePopup();
    m_bUserTyping = false;
    m_bUserNavigated = false;
    m_bEditModifiedByUser = false;
    m_committedUrl = url.c_str();
    if (url != rawText) {
        //回填归一化后的网址（不触发文本变化，避免联想浮层重新弹出）
        m_pEdit->SetTextNoEvent(url);
        const int32_t nTextLength = m_pEdit->GetTextLength();
        m_pEdit->SetSel(nTextLength, nTextLength);
    }
    UpdateSecureIcon();
    AddHistory(url);
    if (!m_pEdit->IsFocused()) {
        //提交后确保编辑框持有焦点，保证可以连续输入
        m_pEdit->SetFocus();
    }
    SendEvent(kEventUrlChanged);
    ScheduleClearSuppressFlag();
}

void UrlBar::CancelEdit()
{
    if (m_pEdit == nullptr) {
        return;
    }
    m_bSuppressNextFocusPopup = true;
    ClosePopup();
    m_bEditModifiedByUser = false;
    m_bUserTyping = false;
    m_bUserNavigated = false;
    //恢复为最近一次提交的网址，并全选便于重新输入
    m_pEdit->SetTextNoEvent(DString(m_committedUrl.c_str()));
    const int32_t nTextLength = m_pEdit->GetTextLength();
    m_pEdit->SetSel(0, nTextLength);
    UpdateSecureIcon();
    SendEvent(kEventUrlCancel);
    ScheduleClearSuppressFlag();
}

void UrlBar::UpdatePopup()
{
    //防重入：AutoCompleteFlyout::ShowAt 创建浮层窗口时可能短暂夺走父窗口前台焦点，
    //框架随即恢复父窗口焦点，这会同步触发编辑框 kEventSetFocus → OnEditFocus →
    //UpdatePopup 嵌套调用；不阻断会重复创建浮层
    if (m_bUpdatingPopup) {
        return;
    }
    m_bUpdatingPopup = true;
    UpdatePopupInternal();
    m_bUpdatingPopup = false;
}

void UrlBar::UpdatePopupInternal()
{
    if (m_pEdit == nullptr) {
        return;
    }
    //重建候选列表时清除“已导航”状态
    m_bUserNavigated = false;
    BuildCandidates();

    if (m_candidates.empty()) {
        ClosePopup();
        return;
    }

    //浮层已打开时：就地刷新候选项内容与高度，复用同一个窗口
    if ((m_pFlyout != nullptr) && m_pFlyout->IsOpen()) {
        //默认不选中任何候选项（避免整行灰色高亮），由键盘上下键选择
        if (m_pFlyout->UpdateItems(m_candidates, Box::InvalidIndex)) {
            return;
        }
        //更新失败（窗口已失效）：退回到重建路径
        m_pFlyout = nullptr;
    }
    OpenPopup();
}

void UrlBar::OpenPopup()
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
    //跟随锚点移动：窗口移动/布局变化时浮层同步重新定位，避免浮层与地址栏脱离
    pFlyout->SetFollowAnchor(true);
    pFlyout->AttachItemSelect(UiBind(this, [this](size_t nIndex) {
        OnCandidateSelected(nIndex);
        }, std::placeholders::_1));
    pFlyout->AttachClosed(UiBind(this, [this](Flyout::CloseReason /*reason*/) {
        m_pFlyout = nullptr;
        //浮层关闭后框架会把原生键盘焦点交还本窗口，部分场景（如点击网页内容时
        //编辑框仍保持焦点）会再次触发 kEventSetFocus → OnEditFocus → 重新展开浮层；
        //这里统一用一次性抑制标记挡住，避免浮层“点击外部后立即重新弹出”
        m_bSuppressNextFocusPopup = true;
        ScheduleClearSuppressFlag();
        }, std::placeholders::_1));

    //ShowAt 失败时对象已被框架销毁，回调不会再触发
    //初始不选中任何候选项（避免地址栏一展开就出现灰色高亮行），由键盘上下键选择
    if (pFlyout->ShowAt(this, m_candidates, Box::InvalidIndex, nWidth, Flyout::Placement::Bottom)) {
        m_pFlyout = pFlyout;
    }
}

void UrlBar::ClosePopup()
{
    if (m_pFlyout != nullptr) {
        //先置空，避免 Dismiss 回调期间重入
        AutoCompleteFlyout* pFlyout = m_pFlyout;
        m_pFlyout = nullptr;
        pFlyout->Dismiss();
    }
}

void UrlBar::OnCandidateSelected(size_t nIndex)
{
    if (nIndex >= m_candidates.size()) {
        return;
    }
    const DString url = m_candidates[nIndex].c_str();

    m_bSuppressNextFocusPopup = true;
    ClosePopup();
    m_bUserTyping = false;
    m_bUserNavigated = false;
    m_bEditModifiedByUser = false;
    m_committedUrl = url.c_str();
    if (m_pEdit != nullptr) {
        //回填网址但不触发 textChanged，避免回填后浮层重新弹出
        m_pEdit->SetTextNoEvent(url);
        const int32_t nTextLength = m_pEdit->GetTextLength();
        m_pEdit->SetSel(nTextLength, nTextLength);
    }
    UpdateSecureIcon();
    AddHistory(url);
    if ((m_pEdit != nullptr) && !m_pEdit->IsFocused()) {
        m_pEdit->SetFocus();
    }
    SendEvent(kEventUrlChanged);
    ScheduleClearSuppressFlag();
}

void UrlBar::BuildCandidates()
{
    m_candidates.clear();
    if (!m_bHistoryEnabled || m_history.empty()) {
        return;
    }
    DString input;
    if (m_bUserTyping && (m_pEdit != nullptr)) {
        input = StringUtil::Trim(m_pEdit->GetText().c_str());
    }
    m_candidates.reserve(m_history.size());
    for (const UiString& historyUrl : m_history) {
        if (input.empty() || MatchItem(historyUrl.c_str(), input)) {
            m_candidates.push_back(historyUrl);
        }
    }
}

bool UrlBar::MatchItem(const DString& item, const DString& input) const
{
    if (input.empty()) {
        return true;
    }
    const DString lowerItem = StringUtil::MakeLowerString(item);
    const DString lowerInput = StringUtil::MakeLowerString(input);
    return lowerItem.find(lowerInput) != DString::npos;
}

void UrlBar::UpdateSecureIcon()
{
    const DString scheme = UrlGetScheme(GetEditText());
    ApplySecureIcon(scheme == _T("https"));
}

void UrlBar::ApplySecureIcon(bool bSecure)
{
    m_bSecure = bSecure;
    if (m_pIconSecure != nullptr) {
        m_pIconSecure->SetVisible(bSecure);
    }
    if (m_pIconInsecure != nullptr) {
        m_pIconInsecure->SetVisible(!bSecure);
    }
}

void UrlBar::LoadHistory()
{
    if (!m_bHistoryEnabled || m_historyFile.empty()) {
        return;
    }
    const DString historyFilePath = GetHistoryFilePath();
    std::vector<uint8_t> fileData;
    if (!FileUtil::ReadFileData(FilePath(historyFilePath.c_str()), fileData) || fileData.empty()) {
        return;
    }
    DStringA utf8Data(fileData.begin(), fileData.end());
    size_t nStart = 0;
    while (nStart < utf8Data.size()) {
        size_t nEnd = utf8Data.find('\n', nStart);
        if (nEnd == DStringA::npos) {
            nEnd = utf8Data.size();
        }
        std::string line = utf8Data.substr(nStart, nEnd - nStart);
        if (!line.empty() && (line.back() == '\r')) {
            line.pop_back();
        }
        if (!line.empty()) {
            const DString url = StringUtil::Trim(StringConvert::UTF8ToT(line).c_str());
            if (!url.empty()) {
                bool bExists = false;
                for (const UiString& item : m_history) {
                    if (StringUtil::IsEqualNoCase(item.c_str(), url.c_str())) {
                        bExists = true;
                        break;
                    }
                }
                if (!bExists) {
                    m_history.push_back(url.c_str());
                }
            }
        }
        nStart = nEnd + 1;
    }
    while (static_cast<int32_t>(m_history.size()) > m_nMaxHistory) {
        m_history.pop_back();
    }
}

void UrlBar::SaveHistory()
{
    if (m_historyFile.empty()) {
        return;
    }
    DStringA utf8Data;
    for (const UiString& url : m_history) {
        utf8Data += StringConvert::TToUTF8(url.c_str());
        utf8Data += '\n';
    }
    FilePath historyFilePath(GetHistoryFilePath().c_str());
    if (historyFilePath.IsEmpty()) {
        return;
    }
    //确保历史文件所在目录存在
    FilePathUtil::CreateDirectories(historyFilePath.GetParentPath().ToString());
    FileUtil::WriteFileData(historyFilePath, utf8Data);
}

DString UrlBar::GetHistoryFilePath() const
{
    if (m_historyFile.empty()) {
        return DString();
    }
    FilePath historyFilePath(m_historyFile.c_str());
    if (historyFilePath.IsRelativePath()) {
        //相对路径：拼接可执行程序所在目录
        historyFilePath = FilePathUtil::JoinFilePath(FilePathUtil::GetCurrentModuleDirectory(), historyFilePath);
    }
    return historyFilePath.ToString();
}

}//namespace ui

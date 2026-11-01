#include "duilib/Control/Popover.h"
#include "duilib/Control/Flyout.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Callback.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>

namespace ui
{

namespace
{
/** 将文本中的 XML 特殊字符转义，用于动态生成卡片的 XML 文本
*/
DString EscapeXmlText(const DString& strText)
{
    DString strResult;
    strResult.reserve(strText.size() + 16);
    for (DString::value_type ch : strText) {
        switch (ch) {
        case _T('&'):  strResult += _T("&amp;");  break;
        case _T('<'):  strResult += _T("&lt;");   break;
        case _T('>'):  strResult += _T("&gt;");   break;
        case _T('\"'): strResult += _T("&quot;"); break;
        case _T('\''): strResult += _T("&apos;"); break;
        default:       strResult += ch;           break;
        }
    }
    return strResult;
}

/** 归一化方位字符串：忽略大小写、下划线与连字符（"top_left"/"topLeft" 均等价于 "topleft"）
*/
DString NormalizePlacement(const DString& strValue)
{
    DString strResult = StringUtil::MakeLowerString(strValue);
    StringUtil::ReplaceAll(_T("_"), _T(""), strResult);
    StringUtil::ReplaceAll(_T("-"), _T(""), strResult);
    return strResult;
}

/** 解析 XML 中的 placement 属性（无法识别时回退为顶部居中）
*/
PopoverPlacement ParsePlacement(const DString& strValue)
{
    const DString strPos = NormalizePlacement(strValue);
    if (strPos == _T("topleft")) {
        return PopoverPlacement::kTopLeft;
    }
    if (strPos == _T("topright")) {
        return PopoverPlacement::kTopRight;
    }
    if (strPos == _T("bottom")) {
        return PopoverPlacement::kBottom;
    }
    if (strPos == _T("bottomleft")) {
        return PopoverPlacement::kBottomLeft;
    }
    if (strPos == _T("bottomright")) {
        return PopoverPlacement::kBottomRight;
    }
    if (strPos == _T("left")) {
        return PopoverPlacement::kLeft;
    }
    if (strPos == _T("lefttop")) {
        return PopoverPlacement::kLeftTop;
    }
    if (strPos == _T("leftbottom")) {
        return PopoverPlacement::kLeftBottom;
    }
    if (strPos == _T("right")) {
        return PopoverPlacement::kRight;
    }
    if (strPos == _T("righttop")) {
        return PopoverPlacement::kRightTop;
    }
    if (strPos == _T("rightbottom")) {
        return PopoverPlacement::kRightBottom;
    }
    return PopoverPlacement::kTop;
}

/** 解析 XML 中的 trigger 属性（忽略大小写；无法识别时回退为点击触发）
*/
PopoverTrigger ParseTrigger(const DString& strValue)
{
    const DString strTrigger = StringUtil::MakeLowerString(strValue);
    if (strTrigger == _T("hover")) {
        return PopoverTrigger::kHover;
    }
    if (strTrigger == _T("focus")) {
        return PopoverTrigger::kFocus;
    }
    if (strTrigger == _T("manual")) {
        return PopoverTrigger::kManual;
    }
    return PopoverTrigger::kClick;
}

/** Popover 方位映射到 Flyout 方位（Popover 的 12 方位由 Flyout 的 12 个方位承载）
*/
Flyout::Placement ToFlyoutPlacement(PopoverPlacement placement)
{
    switch (placement) {
    case PopoverPlacement::kTop:         return Flyout::Placement::TopCenter;
    case PopoverPlacement::kTopLeft:     return Flyout::Placement::Top;
    case PopoverPlacement::kTopRight:    return Flyout::Placement::TopEnd;
    case PopoverPlacement::kBottom:      return Flyout::Placement::BottomCenter;
    case PopoverPlacement::kBottomLeft:  return Flyout::Placement::Bottom;
    case PopoverPlacement::kBottomRight: return Flyout::Placement::BottomEnd;
    case PopoverPlacement::kLeft:        return Flyout::Placement::LeftCenter;
    case PopoverPlacement::kLeftTop:     return Flyout::Placement::Left;
    case PopoverPlacement::kLeftBottom:  return Flyout::Placement::LeftEnd;
    case PopoverPlacement::kRight:       return Flyout::Placement::RightCenter;
    case PopoverPlacement::kRightTop:    return Flyout::Placement::Right;
    case PopoverPlacement::kRightBottom: return Flyout::Placement::RightEnd;
    default:                             return Flyout::Placement::TopCenter;
    }
}
} // namespace

Popover::Popover(Window* pWindow) :
    BaseClass(pWindow)
{
}

Popover::~Popover()
{
    //析构期间不再向外派发事件；仍要关闭已打开的卡片，避免留下失去锚点的浮层
    m_bDestroying = true;
    CancelAllTasks();
    Flyout* pFlyout = GetOpenFlyout();
    if (pFlyout != nullptr) {
        pFlyout->Dismiss();
    }
    m_bOpen = false;
}

DString Popover::GetType() const
{
    return DUI_CTR_POPOVER;
}

void Popover::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("title")) {
        SetTitle(strValue);
    }
    else if ((strName == _T("title_id")) || (strName == _T("titleid"))) {
        SetTitleId(strValue);
    }
    else if (strName == _T("content")) {
        SetContent(strValue);
    }
    else if ((strName == _T("content_id")) || (strName == _T("contentid"))) {
        SetContentId(strValue);
    }
    else if ((strName == _T("content_xml")) || (strName == _T("contentxml"))) {
        SetContentXml(strValue);
    }
    else if (strName == _T("trigger")) {
        SetTrigger(ParseTrigger(strValue));
    }
    else if (strName == _T("placement")) {
        SetPlacement(ParsePlacement(strValue));
    }
    else if (strName == _T("gap")) {
        SetGap(StringUtil::StringToInt32(strValue.c_str()));
    }
    else if ((strName == _T("card_width")) || (strName == _T("cardwidth"))) {
        SetCardWidth(StringUtil::StringToInt32(strValue.c_str()));
    }
    else if ((strName == _T("auto_dismiss")) || (strName == _T("autodismiss"))) {
        SetAutoDismiss(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("hover_delay")) || (strName == _T("hoverdelay"))) {
        SetHoverDelay(StringUtil::StringToInt32(strValue.c_str()));
    }
    else if ((strName == _T("close_delay")) || (strName == _T("closedelay"))) {
        SetCloseDelay(StringUtil::StringToInt32(strValue.c_str()));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Popover::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    //卡片内容在打开时生成，语言切换后旧卡片文本会过期，直接关闭让下次打开时按新语言重建
    Hide();
}

void Popover::SetTitle(const DString& strTitle)
{
    //标题变化只影响下次打开：已打开的卡片保持原内容，避免闪烁
    m_strTitle = strTitle;
}

DString Popover::GetTitle() const
{
    if (!m_strTitleId.empty()) {
        return GlobalManager::Instance().Lang().GetStringByID(m_strTitleId.c_str());
    }
    return m_strTitle;
}

void Popover::SetTitleId(const DString& strTitleId)
{
    m_strTitleId = strTitleId;
}

void Popover::SetContent(const DString& strContent)
{
    m_strContent = strContent;
}

DString Popover::GetContent() const
{
    if (!m_strContentId.empty()) {
        return GlobalManager::Instance().Lang().GetStringByID(m_strContentId.c_str());
    }
    return m_strContent;
}

void Popover::SetContentId(const DString& strContentId)
{
    m_strContentId = strContentId;
}

void Popover::SetContentXml(const DString& strContentXml)
{
    m_strContentXml = strContentXml;
}

void Popover::SetTrigger(PopoverTrigger trigger)
{
    m_trigger = trigger;
}

void Popover::SetPlacement(PopoverPlacement placement)
{
    m_placement = placement;
}

void Popover::SetGap(int32_t nGap)
{
    m_nGap = std::max(0, nGap);
}

void Popover::SetCardWidth(int32_t nWidth)
{
    m_nCardWidth = std::max(120, nWidth);
}

void Popover::SetAutoDismiss(bool bAutoDismiss)
{
    m_bAutoDismiss = bAutoDismiss;
    m_bAutoDismissDefined = true;
}

void Popover::SetHoverDelay(int32_t nDelayMs)
{
    m_nHoverDelayMs = std::max(0, nDelayMs);
}

void Popover::SetCloseDelay(int32_t nDelayMs)
{
    m_nCloseDelayMs = std::max(0, nDelayMs);
}

void Popover::Show()
{
    if (m_bOpen || !IsEnabled() || !IsMouseEnabled() || !IsVisible()) {
        return;
    }
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return;
    }

    //同一时刻本控件只保留一个卡片
    Flyout* pActive = Flyout::GetActiveFlyout();
    if ((pActive != nullptr) && (pActive->GetAnchor() == this)) {
        return;
    }

    //先标记为打开，避免打开回调中再次触发 Show 造成重入
    m_bOpen = true;

    //非模态浮层，关闭后由框架自动 delete，调用方不需要释放
    Flyout* pFlyout = new Flyout(pWindow);
    pFlyout->SetGap(m_nGap);
    pFlyout->SetAutoDismiss(m_bAutoDismiss);
    //manual（受控）触发时，默认不因"点击外部"关闭：否则鼠标按在外部按钮上的瞬间卡片先被关闭，
    //随后该按钮的 Click 事件里读到的状态已是"已关闭"，导致 Toggle/重建卡片等逻辑表现异常
    const bool bClickOutsideDismiss = m_bAutoDismiss &&
        (m_bAutoDismissDefined || (m_trigger != PopoverTrigger::kManual));
    pFlyout->SetClickOutsideDismiss(bClickOutsideDismiss);
    pFlyout->AttachClosed(UiBind(this, [this](Flyout::CloseReason reason) {
        OnFlyoutClosed(static_cast<int32_t>(reason));
        }, std::placeholders::_1));

    if (!pFlyout->ShowAt(this, BuildCardXml(), ToFlyoutPlacement(m_placement))) {
        //创建失败时 ShowAt 内部已销毁对象，不能再访问 pFlyout
        m_bOpen = false;
        return;
    }

    InitializeCard(pFlyout);
    Invalidate();
    EmitOpenChanged(true, PopoverCloseReason::kManual);
}

void Popover::Hide()
{
    Flyout* pFlyout = GetOpenFlyout();
    if (pFlyout != nullptr) {
        pFlyout->Dismiss();
        return;
    }
    if (m_bOpen) {
        //浮层已不在（例如被其它浮层替换），仅同步状态
        m_bOpen = false;
        CancelAllTasks();
        Invalidate();
    }
}

void Popover::Toggle()
{
    if (m_bOpen) {
        Hide();
    }
    else {
        Show();
    }
}

Window* Popover::GetCardWindow() const
{
    Flyout* pFlyout = GetOpenFlyout();
    return pFlyout;
}

Control* Popover::FindCardControl(const DString& strName) const
{
    Window* pWindow = GetCardWindow();
    if (pWindow == nullptr) {
        return nullptr;
    }
    return pWindow->FindControl(strName);
}

bool Popover::MouseEnter(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseEnter(msg);
    if (m_trigger == PopoverTrigger::kHover) {
        m_bHoverInside = true;
        CancelCloseTask();
        if (!m_bOpen && IsEnabled() && IsMouseEnabled() && IsVisible()) {
            ScheduleOpen();
        }
    }
    return bRet;
}

bool Popover::MouseLeave(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseLeave(msg);
    if (m_trigger == PopoverTrigger::kHover) {
        m_bHoverInside = false;
        CancelOpenTask();
        if (m_bOpen) {
            ScheduleCloseCheck();
        }
    }
    return bRet;
}

bool Popover::ButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDown(msg);
    //焦点触发：点击锚点时让锚点获得键盘焦点，由 OnSetFocus 打开卡片
    if ((m_trigger == PopoverTrigger::kFocus) && !IsFocused() && IsEnabled() && IsMouseEnabled()) {
        SetFocus();
    }
    return bRet;
}

bool Popover::ButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonUp(msg);
    if ((m_trigger == PopoverTrigger::kClick) && IsEnabled() && IsMouseEnabled() &&
        IsPointInWithScrollOffset(msg.ptMouse)) {
        Toggle();
    }
    return bRet;
}

bool Popover::OnSetFocus(const EventArgs& msg)
{
    bool bRet = BaseClass::OnSetFocus(msg);
    if ((m_trigger == PopoverTrigger::kFocus) && IsEnabled() && IsMouseEnabled()) {
        Show();
    }
    return bRet;
}

bool Popover::OnKillFocus(const EventArgs& msg)
{
    bool bRet = BaseClass::OnKillFocus(msg);
    if (m_trigger == PopoverTrigger::kFocus) {
        Hide();
    }
    return bRet;
}

void Popover::OnSetVisible(bool bChanged)
{
    BaseClass::OnSetVisible(bChanged);
    if (!IsVisible()) {
        CancelAllTasks();
        Hide();
    }
}

Flyout* Popover::GetOpenFlyout() const
{
    Flyout* pFlyout = Flyout::GetActiveFlyout();
    if ((pFlyout != nullptr) && (pFlyout->GetAnchor() == this)) {
        return pFlyout;
    }
    return nullptr;
}

DString Popover::BuildCardXml() const
{
    const int32_t nCardWidth = std::max(120, m_nCardWidth);
    const bool bHasContent = !m_strContentXml.empty() || !m_strContent.empty() || !m_strContentId.empty();

    DString xml;
    xml += _T("<Window size=\"1,1\" caption=\"0,0,0,0\" use_system_caption=\"false\"");
    //卡片使用较小的自绘阴影（分层窗口的默认阴影为 big_round，四边约 30~36 像素，视觉上过重）
    xml += _T(" shadow_type=\"small_round\" shadow_attached=\"true\" layered_window=\"true\" size_box=\"0,0,0,0\">");
    xml += _T("<VBox class=\"popover_card\" name=\"popover_card\" width=\"");
    xml += StringUtil::Int32ToString(nCardWidth);
    xml += _T("\" height=\"auto\">");

    if (!m_strTitle.empty() || !m_strTitleId.empty()) {
        xml += _T("<Label class=\"popover_title\"");
        if (!m_strTitleId.empty()) {
            xml += _T(" text_id=\"");
            xml += EscapeXmlText(m_strTitleId);
            xml += _T("\"");
        }
        else {
            xml += _T(" text=\"");
            xml += EscapeXmlText(m_strTitle);
            xml += _T("\"");
        }
        if (bHasContent) {
            xml += _T(" margin=\"0,0,0,6\"");
        }
        xml += _T("/>");
    }

    if (!m_strContentXml.empty()) {
        //富内容：调用方提供的 XML 片段原样插入卡片
        xml += m_strContentXml;
    }
    else if (!m_strContent.empty() || !m_strContentId.empty()) {
        xml += _T("<Label class=\"popover_content\"");
        if (!m_strContentId.empty()) {
            xml += _T(" text_id=\"");
            xml += EscapeXmlText(m_strContentId);
            xml += _T("\"");
        }
        else {
            xml += _T(" text=\"");
            xml += EscapeXmlText(m_strContent);
            xml += _T("\"");
        }
        xml += _T("/>");
    }

    xml += _T("</VBox></Window>");
    return xml;
}

void Popover::InitializeCard(Flyout* pFlyout)
{
    if (pFlyout == nullptr) {
        return;
    }
    Control* pCard = pFlyout->FindControl(_T("popover_card"));
    if (pCard == nullptr) {
        return;
    }
    //鼠标进入卡片：取消即将执行的关闭，允许鼠标从锚点移入卡片继续操作
    pCard->AttachEvent(kEventMouseEnter, UiBind(this, [this](const EventArgs& /*msg*/) -> bool {
        CancelCloseTask();
        return true;
        }, std::placeholders::_1), 0);
    //鼠标离开卡片：按 hover 语义延时检查（移回锚点时会由锚点的 MouseEnter 取消）
    pCard->AttachEvent(kEventMouseLeave, UiBind(this, [this](const EventArgs& /*msg*/) -> bool {
        if ((m_trigger == PopoverTrigger::kHover) && m_bOpen) {
            ScheduleCloseCheck();
        }
        return true;
        }, std::placeholders::_1), 0);
}

void Popover::EmitOpenChanged(bool bOpen, PopoverCloseReason reason)
{
    SendEvent(kEventPopoverOpenChanged,
              static_cast<WPARAM>(bOpen ? 1 : 0),
              static_cast<LPARAM>(static_cast<uint32_t>(reason)));
}

void Popover::OnFlyoutClosed(int32_t nCloseReason)
{
    if (m_bDestroying) {
        m_bOpen = false;
        return;
    }
    if (!m_bOpen) {
        return;
    }
    m_bOpen = false;
    CancelAllTasks();
    Invalidate();
    EmitOpenChanged(false, static_cast<PopoverCloseReason>(static_cast<uint32_t>(nCloseReason)));
}

bool Popover::IsMouseOverAnchorOrCard() const
{
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return false;
    }
    UiPoint ptCursor;
    pWindow->GetCursorPos(ptCursor);

    UiRect rcAnchor = GetScreenRectPhysical();
    if (rcAnchor.ContainsPt(ptCursor)) {
        return true;
    }

    Flyout* pFlyout = GetOpenFlyout();
    if (pFlyout != nullptr) {
        //用可视区域（不含窗口阴影）判断，阴影区域不应被当成"在卡片上"
        UiRect rcCard;
        pFlyout->GetVisualRect(rcCard);
        if (rcCard.ContainsPt(ptCursor)) {
            return true;
        }
    }
    return false;
}

UiRect Popover::GetScreenRectPhysical() const
{
    UiRect rcAnchor;
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return rcAnchor;
    }
    rcAnchor = GetPos();
    //扣除各级滚动容器累计的滚动偏移，换算为窗口客户区视觉坐标
    UiPoint ptScrollOffset = GetScrollOffsetInScrollBox();
    rcAnchor.Offset(-ptScrollOffset.x, -ptScrollOffset.y);

    //DIP -> 物理像素 -> 屏幕坐标（与 Flyout 的锚点坐标换算一致）
    pWindow->Dpi().ClientSizeToWindowSize(rcAnchor);
    pWindow->ClientToScreen(rcAnchor);
    return rcAnchor;
}

void Popover::ScheduleOpen()
{
    CancelOpenTask();
    if (m_nHoverDelayMs <= 0) {
        Show();
        return;
    }
    m_nOpenTaskId = GlobalManager::Instance().Thread().PostDelayedTask(
        kThreadUI,
        UiBind(this, [this]() {
            m_nOpenTaskId = 0;
            if (m_bHoverInside) {
                Show();
            }
        }),
        m_nHoverDelayMs);
}

void Popover::ScheduleCloseCheck()
{
    CancelCloseTask();
    m_nCloseTaskId = GlobalManager::Instance().Thread().PostDelayedTask(
        kThreadUI,
        UiBind(this, [this]() {
            m_nCloseTaskId = 0;
            if (m_bOpen && !IsMouseOverAnchorOrCard()) {
                Hide();
            }
        }),
        std::max(0, m_nCloseDelayMs));
}

void Popover::CancelOpenTask()
{
    if (m_nOpenTaskId != 0) {
        GlobalManager::Instance().Thread().CancelTask(m_nOpenTaskId);
        m_nOpenTaskId = 0;
    }
}

void Popover::CancelCloseTask()
{
    if (m_nCloseTaskId != 0) {
        GlobalManager::Instance().Thread().CancelTask(m_nCloseTaskId);
        m_nCloseTaskId = 0;
    }
}

void Popover::CancelAllTasks()
{
    CancelOpenTask();
    CancelCloseTask();
}

} // namespace ui

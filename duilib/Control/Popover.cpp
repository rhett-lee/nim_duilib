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

/** Flyout 方位反查为 Popover 方位（用于按"实际方位"重建卡片，使箭头朝向与卡片位置一致）
*/
PopoverPlacement FromFlyoutPlacement(Flyout::Placement placement)
{
    switch (placement) {
    case Flyout::Placement::Top:          return PopoverPlacement::kTopLeft;
    case Flyout::Placement::TopEnd:       return PopoverPlacement::kTopRight;
    case Flyout::Placement::TopCenter:    return PopoverPlacement::kTop;
    case Flyout::Placement::Bottom:       return PopoverPlacement::kBottomLeft;
    case Flyout::Placement::BottomEnd:    return PopoverPlacement::kBottomRight;
    case Flyout::Placement::BottomCenter: return PopoverPlacement::kBottom;
    case Flyout::Placement::Left:         return PopoverPlacement::kLeftTop;
    case Flyout::Placement::LeftEnd:      return PopoverPlacement::kLeftBottom;
    case Flyout::Placement::LeftCenter:   return PopoverPlacement::kLeft;
    case Flyout::Placement::Right:        return PopoverPlacement::kRightTop;
    case Flyout::Placement::RightEnd:     return PopoverPlacement::kRightBottom;
    case Flyout::Placement::RightCenter:  return PopoverPlacement::kRight;
    default:                              return PopoverPlacement::kTop;
    }
}

/** 卡片相对锚点的方位（四个方向；同一方向内的对齐方式不影响箭头朝向）
*/
enum class PopoverSide : uint8_t
{
    kTop = 0,       //卡片在锚点上方，箭头向下
    kBottom = 1,    //卡片在锚点下方，箭头向上
    kLeft = 2,      //卡片在锚点左侧，箭头向右
    kRight = 3      //卡片在锚点右侧，箭头向左
};

/** 获取方位所在的边（上/下/左/右）
*/
PopoverSide GetPlacementSide(PopoverPlacement placement)
{
    switch (placement) {
    case PopoverPlacement::kTop:
    case PopoverPlacement::kTopLeft:
    case PopoverPlacement::kTopRight:
        return PopoverSide::kTop;
    case PopoverPlacement::kBottom:
    case PopoverPlacement::kBottomLeft:
    case PopoverPlacement::kBottomRight:
        return PopoverSide::kBottom;
    case PopoverPlacement::kLeft:
    case PopoverPlacement::kLeftTop:
    case PopoverPlacement::kLeftBottom:
        return PopoverSide::kLeft;
    default:
        return PopoverSide::kRight;
    }
}

/** 尖角箭头的尺寸与最小边距（未经 DPI 缩放的逻辑值）
*/
const int32_t kArrowLength = 16;            //箭头沿卡片边的长度
const int32_t kArrowDepth = 8;              //箭头垂直卡片边的深度（尖角伸出卡片边的长度）
const int32_t kArrowOverlap = 1;            //箭头压入卡片边的长度（覆盖卡片边线，避免边线横穿箭头）
const int32_t kArrowMinEdgeMargin = 12;     //箭头距卡片相邻边的最小距离（避开卡片圆角）

/** 卡片在箭头一侧预留的箭头带宽度（逻辑值，与 kArrowDepth 一致）
*   卡片在该侧内缩这么多，窗口阴影的主体边缘才能与卡片边缘对齐（见 BuildCardXml 中的阴影配置）
*/
const int32_t kArrowBand = kArrowDepth;

/** 把箭头沿卡片边的偏移夹持在有效范围内（偏移为箭头起始边相对卡片起始边的距离）
* @param [in] nOffset 期望的偏移（指向锚点中心）
* @param [in] nCardLength 卡片沿箭头方向的长度
* @param [in] nArrowLength 箭头沿卡片边的长度
* @param [in] nMinMargin 箭头距卡片相邻边的最小距离（避开卡片圆角）
* @return 返回夹持后的偏移
*/
int32_t ClampArrowOffset(int32_t nOffset, int32_t nCardLength, int32_t nArrowLength, int32_t nMinMargin)
{
    const int32_t nMaxOffset = nCardLength - nArrowLength - nMinMargin;
    if (nMaxOffset <= nMinMargin) {
        //卡片过窄（可用范围为空）：箭头居中放置
        return std::max(0, (nCardLength - nArrowLength) / 2);
    }
    return std::max(nMinMargin, std::min(nOffset, nMaxOffset));
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
    else if (strName == _T("arrow")) {
        SetArrow(StringUtil::IsValueTrue(strValue));
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

void Popover::SetArrow(bool bArrow)
{
    //箭头影响卡片 XML 的结构，已显示的卡片保持原样，下次打开时生效
    m_bArrow = bArrow;
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

    //manual（受控）触发时，默认不因"点击外部"关闭：否则鼠标按在外部按钮上的瞬间卡片先被关闭，
    //随后该按钮的 Click 事件里读到的状态已是"已关闭"，导致 Toggle/重建卡片等逻辑表现异常
    const bool bClickOutsideDismiss = m_bAutoDismiss &&
        (m_bAutoDismissDefined || (m_trigger != PopoverTrigger::kManual));

    //卡片按期望方位创建；若因空间不足被翻转到锚点的另一侧，显示的箭头会指向错误方向，
    //此时按"实际方位"重建一次卡片，并固定该方位（不再翻转），保证箭头始终指向锚点
    PopoverPlacement placement = m_placement;
    bool bAllowFlip = true;
    for (int32_t nAttempt = 0; nAttempt < 2; ++nAttempt) {
        //非模态浮层，关闭后由框架自动 delete，调用方不需要释放
        Flyout* pFlyout = new Flyout(pWindow);
        pFlyout->SetGap(m_nGap);
        pFlyout->SetAutoDismiss(m_bAutoDismiss);
        pFlyout->SetClickOutsideDismiss(bClickOutsideDismiss);
        pFlyout->SetAllowFlip(bAllowFlip);
        pFlyout->AttachClosed(UiBind(this, [this](Flyout::CloseReason reason) {
            OnFlyoutClosed(static_cast<int32_t>(reason));
            }, std::placeholders::_1));
        //布局完成后（首次绘制前）按卡片的实际布局矩形把尖角对准锚点中心：
        //必须在 ShowAt 之前注册，避免首帧布局早于回调注册而漏掉定位
        pFlyout->AttachLayouted(UiBind(this, [this, pFlyout]() {
            PositionCardArrow(pFlyout);
        }));

        if (!pFlyout->ShowAt(this, BuildCardXml(placement), ToFlyoutPlacement(placement))) {
            //创建失败时 ShowAt 内部已销毁对象，不能再访问 pFlyout
            m_bOpen = false;
            return;
        }

        const PopoverPlacement actualPlacement = FromFlyoutPlacement(pFlyout->GetActualPlacement());
        if (m_bArrow && (nAttempt == 0) &&
            (GetPlacementSide(placement) != GetPlacementSide(actualPlacement))) {
            //发生了翻转：先关闭本次浮层（关闭回调被忽略，不改变打开状态），再按实际方位重建
            m_bRebuilding = true;
            pFlyout->Dismiss();
            m_bRebuilding = false;
            placement = actualPlacement;
            bAllowFlip = false;
            continue;
        }

        InitializeCard(pFlyout);
        Invalidate();
        EmitOpenChanged(true, PopoverCloseReason::kManual);
        return;
    }
    m_bOpen = false;
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

DString Popover::BuildCardXml(PopoverPlacement placement) const
{
    const int32_t nCardWidth = std::max(120, m_nCardWidth);
    const bool bHasContent = !m_strContentXml.empty() || !m_strContent.empty() || !m_strContentId.empty();

    //卡片内容（标题 + 正文）
    DString strCardBody;
    if (!m_strTitle.empty() || !m_strTitleId.empty()) {
        strCardBody += _T("<Label class=\"popover_title\"");
        if (!m_strTitleId.empty()) {
            strCardBody += _T(" text_id=\"");
            strCardBody += EscapeXmlText(m_strTitleId);
            strCardBody += _T("\"");
        }
        else {
            strCardBody += _T(" text=\"");
            strCardBody += EscapeXmlText(m_strTitle);
            strCardBody += _T("\"");
        }
        if (bHasContent) {
            strCardBody += _T(" margin=\"0,0,0,6\"");
        }
        strCardBody += _T("/>");
    }

    if (!m_strContentXml.empty()) {
        //富内容：调用方提供的 XML 片段原样插入卡片
        strCardBody += m_strContentXml;
    }
    else if (!m_strContent.empty() || !m_strContentId.empty()) {
        strCardBody += _T("<Label class=\"popover_content\"");
        if (!m_strContentId.empty()) {
            strCardBody += _T(" text_id=\"");
            strCardBody += EscapeXmlText(m_strContentId);
            strCardBody += _T("\"");
        }
        else {
            strCardBody += _T(" text=\"");
            strCardBody += EscapeXmlText(m_strContent);
            strCardBody += _T("\"");
        }
        strCardBody += _T("/>");
    }

    //卡片相对锚点的方位：箭头朝向与"箭头带"所在的边由它决定
    const PopoverSide side = GetPlacementSide(placement);
    const bool bHorizontalSide = (side == PopoverSide::kTop) || (side == PopoverSide::kBottom);

    //显示箭头时，卡片在箭头一侧内缩 kArrowBand，形成"箭头带"：
    //窗口阴影的主体边缘与该侧的卡片边缘对齐（见下面 shadow_image 的选择），
    //箭头带内只绘制阴影渐变，不会出现白色实心带，也不会与窗口阴影带之间出现硬裁切的边界
    DString strCardMargin;
    if (m_bArrow) {
        const DString strBand = StringUtil::Int32ToString(kArrowBand);
        switch (side) {
        case PopoverSide::kTop:
            //卡片在锚点上方：箭头在卡片下边
            strCardMargin = _T("0,0,0,") + strBand;
            break;
        case PopoverSide::kBottom:
            //卡片在锚点下方：箭头在卡片上边
            strCardMargin = _T("0,") + strBand + _T(",0,0");
            break;
        case PopoverSide::kLeft:
            //卡片在锚点左侧：箭头在卡片右边
            strCardMargin = _T("0,0,") + strBand + _T(",0");
            break;
        default:
            //卡片在锚点右侧：箭头在卡片左边
            strCardMargin = strBand + _T(",0,0,0");
            break;
        }
    }

    //阴影图片：按方位选择"箭头一侧的阴影主体内缩 kArrowBand"的阴影图（public/shadow/shadow_popover_arrow_*.svg），
    //使九宫格绘制时阴影主体的边缘正好落在卡片边缘上（corner = 阴影主体边距 + 卡片圆角 8）：
    //此时箭头带内只保留阴影的渐变部分，阴影主体的实心填充被卡片遮住，
    //因此不再出现白色实心带，阴影在"箭头带 + 窗口阴影带"内连续过渡
    DString strShadowFile = _T("shadow_popover.svg");
    DString strShadowCorner = _T("20,20,22,22");
    DString strArrowClass;
    if (m_bArrow) {
        switch (side) {
        case PopoverSide::kTop:
            strShadowFile = _T("shadow_popover_arrow_down.svg");
            strShadowCorner = _T("20,20,22,30");  //下边：14 + 8(箭头带) + 8(卡片圆角)
            strArrowClass = _T("popover_arrow_down");
            break;
        case PopoverSide::kBottom:
            strShadowFile = _T("shadow_popover_arrow_up.svg");
            strShadowCorner = _T("20,28,22,22");  //上边：12 + 8(箭头带) + 8(卡片圆角)
            strArrowClass = _T("popover_arrow_up");
            break;
        case PopoverSide::kLeft:
            strShadowFile = _T("shadow_popover_arrow_right.svg");
            strShadowCorner = _T("20,20,30,22");  //右边：14 + 8(箭头带) + 8(卡片圆角)
            strArrowClass = _T("popover_arrow_right");
            break;
        default:
            strShadowFile = _T("shadow_popover_arrow_left.svg");
            strShadowCorner = _T("28,20,22,22");  //左边：12 + 8(箭头带) + 8(卡片圆角)
            strArrowClass = _T("popover_arrow_left");
            break;
        }
    }

    DString xml;
    xml += _T("<Window size=\"1,1\" caption=\"0,0,0,0\" use_system_caption=\"false\"");
    //卡片使用紧凑的自绘阴影（模糊半径 4、偏移 (1,2)，阴影边只有 12/14 像素），避免默认阴影过重、
    //边线过粗的观感；shadow_border_round 关闭客户区圆角裁剪（卡片自身的 border_round 已提供圆角）
    xml += _T(" shadow_type=\"custom\"");
    xml += _T(" shadow_image=\"file='public/shadow/");
    xml += strShadowFile;
    xml += _T("' window_shadow_mode='true' corner='");
    xml += strShadowCorner;
    xml += _T("'\"");
    xml += _T(" shadow_corner=\"12,12,14,14\" shadow_border_round=\"0,0\" shadow_border_size=\"0\" shadow_snap=\"false\"");
    xml += _T(" shadow_attached=\"true\" layered_window=\"true\" size_box=\"0,0,0,0\">");

    //卡片根容器（其父容器为"卡片框"，卡片框负责放置尖角箭头）
    DString strCardXml;
    strCardXml += _T("<VBox class=\"popover_card\" name=\"popover_card\" width=\"");
    strCardXml += StringUtil::Int32ToString(nCardWidth);
    strCardXml += _T("\" height=\"auto\"");
    if (!strCardMargin.empty()) {
        strCardXml += _T(" margin=\"");
        strCardXml += strCardMargin;
        strCardXml += _T("\"");
    }
    strCardXml += _T(">");
    strCardXml += strCardBody;
    strCardXml += _T("</VBox>");

    if (!m_bArrow) {
        xml += strCardXml;
        xml += _T("</Window>");
        return xml;
    }

    //尖角箭头：卡片框内的浮动控件，排在卡片之后（绘制在卡片之上，可覆盖箭头侧的卡片边线），
    //箭头带不接收鼠标消息；箭头颜色由主题类 popover_arrow_* 提供（卡片背景色 + 卡片边线色）
    DString strArrow;
    strArrow += _T("<Control class=\"");
    strArrow += strArrowClass;
    strArrow += _T("\" name=\"popover_arrow\" float=\"true\" keep_float_pos=\"true\"");
    if (bHorizontalSide) {
        strArrow += _T(" width=\"");
        strArrow += StringUtil::Int32ToString(kArrowLength);
        strArrow += _T("\" height=\"");
        strArrow += StringUtil::Int32ToString(kArrowDepth + kArrowOverlap);
    }
    else {
        strArrow += _T(" width=\"");
        strArrow += StringUtil::Int32ToString(kArrowDepth + kArrowOverlap);
        strArrow += _T("\" height=\"");
        strArrow += StringUtil::Int32ToString(kArrowLength);
    }
    strArrow += _T("\"/>");

    //卡片框：水平方位为 VBox（卡片与箭头带上下排列），左右方位为 HBox（左右排列）
    DString strFrame;
    if (bHorizontalSide) {
        strFrame += _T("<VBox name=\"popover_frame\" width=\"");
        strFrame += StringUtil::Int32ToString(nCardWidth);
        strFrame += _T("\" height=\"auto\">");
    }
    else {
        strFrame += _T("<HBox name=\"popover_frame\" width=\"auto\" height=\"auto\">");
    }
    strFrame += strCardXml;
    strFrame += strArrow;
    strFrame += bHorizontalSide ? _T("</VBox>") : _T("</HBox>");

    xml += strFrame;
    xml += _T("</Window>");
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

void Popover::PositionCardArrow(Flyout* pFlyout)
{
    if ((pFlyout == nullptr) || !m_bArrow) {
        return;
    }
    Control* pCard = pFlyout->FindControl(_T("popover_card"));
    Control* pArrow = pFlyout->FindControl(_T("popover_arrow"));
    if ((pCard == nullptr) || (pArrow == nullptr)) {
        return;
    }
    const UiRect rcCard = pCard->GetPos();
    if (rcCard.IsEmpty()) {
        //布局结果无效（窗口过小等）：保持箭头在默认位置
        return;
    }

    //锚点中心在浮层客户区坐标系（布局坐标）下的位置
    UiRect rcAnchor = GetScreenRectPhysical();
    pFlyout->ScreenToClient(rcAnchor);
    const UiPoint ptAnchorCenter((rcAnchor.left + rcAnchor.right) / 2,
                                 (rcAnchor.top + rcAnchor.bottom) / 2);

    const int32_t nLength = pFlyout->Dpi().GetScaleInt(kArrowLength);
    //箭头厚度取 XML 声明的总厚度（kArrowDepth + kArrowOverlap）缩放后的值，
    //保证此处设置的矩形与控件的固定尺寸一致（否则重排后尺寸可能差 1 像素）
    const int32_t nThickness = pFlyout->Dpi().GetScaleInt(kArrowDepth + kArrowOverlap);
    const int32_t nOverlap = std::max(1, nThickness - pFlyout->Dpi().GetScaleInt(kArrowDepth));
    const int32_t nMinMargin = pFlyout->Dpi().GetScaleInt(kArrowMinEdgeMargin);

    //按实际方位（可能已翻转到锚点另一侧）决定箭头所在的边与朝向
    const PopoverSide side = GetPlacementSide(FromFlyoutPlacement(pFlyout->GetActualPlacement()));
    UiRect rcArrow;
    switch (side) {
    case PopoverSide::kTop:
    {
        //卡片在锚点上方：箭头贴卡片下边，尖角向下
        const int32_t nOffsetX = ClampArrowOffset(ptAnchorCenter.x - rcCard.left - nLength / 2,
                                                 rcCard.Width(), nLength, nMinMargin);
        rcArrow.left = rcCard.left + nOffsetX;
        rcArrow.right = rcArrow.left + nLength;
        rcArrow.top = rcCard.bottom - nOverlap;
        rcArrow.bottom = rcArrow.top + nThickness;
        break;
    }
    case PopoverSide::kBottom:
    {
        //卡片在锚点下方：箭头贴卡片上边，尖角向上
        const int32_t nOffsetX = ClampArrowOffset(ptAnchorCenter.x - rcCard.left - nLength / 2,
                                                 rcCard.Width(), nLength, nMinMargin);
        rcArrow.left = rcCard.left + nOffsetX;
        rcArrow.right = rcArrow.left + nLength;
        rcArrow.bottom = rcCard.top + nOverlap;
        rcArrow.top = rcArrow.bottom - nThickness;
        break;
    }
    case PopoverSide::kLeft:
    {
        //卡片在锚点左侧：箭头贴卡片右边，尖角向右
        const int32_t nOffsetY = ClampArrowOffset(ptAnchorCenter.y - rcCard.top - nLength / 2,
                                                 rcCard.Height(), nLength, nMinMargin);
        rcArrow.top = rcCard.top + nOffsetY;
        rcArrow.bottom = rcArrow.top + nLength;
        rcArrow.left = rcCard.right - nOverlap;
        rcArrow.right = rcArrow.left + nThickness;
        break;
    }
    default:
    {
        //卡片在锚点右侧：箭头贴卡片左边，尖角向左
        const int32_t nOffsetY = ClampArrowOffset(ptAnchorCenter.y - rcCard.top - nLength / 2,
                                                 rcCard.Height(), nLength, nMinMargin);
        rcArrow.top = rcCard.top + nOffsetY;
        rcArrow.bottom = rcArrow.top + nLength;
        rcArrow.right = rcCard.left + nOverlap;
        rcArrow.left = rcArrow.right - nThickness;
        break;
    }
    }

    //浮动控件（float="true" keep_float_pos="true"）：此处设置的位置会在后续布局中被保持
    pArrow->SetPos(rcArrow);
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
    if (m_bRebuilding) {
        //因方位翻转正在重建卡片：旧浮层的关闭属于内部流程，不改变打开状态、也不派发事件
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

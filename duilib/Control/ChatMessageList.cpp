#include "ChatMessageList.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>

namespace ui
{

ChatMessageList::ChatMessageList(Window* pWindow) :
    BaseClass(pWindow),
    m_nAvatarSize(36),
    m_nBubbleMaxWidth(480),
    m_nBubblePaddingH(12),
    m_nBubblePaddingV(8),
    m_nBubbleRadius(10),
    m_nBubbleSpacing(14),
    m_bShowTime(true),
    m_strTimeFormat(_T("HH:mm")),
    m_strAvatarFont(_T("system_bold_14")),
    m_strTextFont(_T("system_regular_14")),
    m_strTimeFont(_T("system_regular_12")),
    m_strBubbleColorSelf(_T("color_accent")),
    m_strBubbleColorOther(_T("bg_window_card")),
    m_strTextColorSelf(_T("color_white")),
    m_strTextColorOther(_T("text_default")),
    m_strAvatarTextColor(_T("color_white")),
    m_strSystemTextColor(_T("text_muted")),
    m_strScrollbarTrackColor(_T("bg_scrollbar_normal")),
    m_strScrollbarThumbColor(_T("bg_scrollbar_thumb_normal")),
    m_bAutoHideScrollbar(true),
    m_bAutoScrollToBottom(true),
    m_nScrollPos(0),
    m_nContentHeight(0),
    m_bDraggingScrollbar(false),
    m_bMouseHover(false),
    m_dragStartY(0),
    m_dragStartScrollPos(0)
{
}

ChatMessageList::~ChatMessageList()
{
}

DString ChatMessageList::GetType() const
{
    return DUI_CTR_CHATMESSAGELIST;
}

void ChatMessageList::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("avatar_size")) {
        m_nAvatarSize = _wtoi(strValue.c_str());
    }
    else if (strName == _T("bubble_max_width")) {
        m_nBubbleMaxWidth = _wtoi(strValue.c_str());
    }
    else if (strName == _T("bubble_padding_h")) {
        m_nBubblePaddingH = _wtoi(strValue.c_str());
    }
    else if (strName == _T("bubble_padding_v")) {
        m_nBubblePaddingV = _wtoi(strValue.c_str());
    }
    else if (strName == _T("bubble_radius")) {
        m_nBubbleRadius = _wtoi(strValue.c_str());
    }
    else if (strName == _T("bubble_spacing")) {
        m_nBubbleSpacing = _wtoi(strValue.c_str());
    }
    else if (strName == _T("show_time")) {
        m_bShowTime = (strValue != _T("false"));
    }
    else if (strName == _T("time_format")) {
        m_strTimeFormat = strValue;
    }
    else if (strName == _T("avatar_font")) {
        m_strAvatarFont = strValue;
    }
    else if (strName == _T("text_font")) {
        m_strTextFont = strValue;
    }
    else if (strName == _T("time_font")) {
        m_strTimeFont = strValue;
    }
    else if (strName == _T("bubble_color_self")) {
        m_strBubbleColorSelf = strValue;
    }
    else if (strName == _T("bubble_color_other")) {
        m_strBubbleColorOther = strValue;
    }
    else if (strName == _T("text_color_self")) {
        m_strTextColorSelf = strValue;
    }
    else if (strName == _T("text_color_other")) {
        m_strTextColorOther = strValue;
    }
    else if (strName == _T("avatar_text_color")) {
        m_strAvatarTextColor = strValue;
    }
    else if (strName == _T("system_text_color")) {
        m_strSystemTextColor = strValue;
    }
    else if (strName == _T("scrollbar_track_color")) {
        m_strScrollbarTrackColor = strValue;
    }
    else if (strName == _T("scrollbar_thumb_color")) {
        m_strScrollbarThumbColor = strValue;
    }
    else if (strName == _T("auto_hide_scrollbar")) {
        m_bAutoHideScrollbar = (strValue != _T("false"));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void ChatMessageList::AddMessage(const Message& message)
{
    m_messages.push_back(message);
    if (m_bAutoScrollToBottom) {
        //延迟到下一轮消息循环再滚动，此时控件已完成布局、内容高度才准确
        Window* pWindow = GetWindow();
        if (pWindow != nullptr) {
            ChatMessageList* pThis = this;
            GlobalManager::Instance().Thread().PostTask(kThreadUI, [pThis]() {
                //任务执行时窗口可能已被销毁，此时不能再操作控件
                if ((pThis != nullptr) && (pThis->GetWindow() != nullptr) &&
                    !pThis->GetWindow()->IsClosingWnd()) {
                    pThis->ScrollToBottom();
                }
                });
        }
    }
    Invalidate();
}

void ChatMessageList::AddMessage(ChatMessageSide side, const DString& strText,
                                 const DString& strAvatarText, const DString& strTime)
{
    Message message;
    message.side = side;
    message.text = strText;
    message.avatarText = strAvatarText;
    message.time = strTime;
    AddMessage(message);
}

void ChatMessageList::ClearMessages()
{
    m_messages.clear();
    m_nScrollPos = 0;
    m_nContentHeight = 0;
    Invalidate();
}

const ChatMessageList::Message* ChatMessageList::GetMessageAt(size_t index) const
{
    if (index >= m_messages.size()) {
        return nullptr;
    }
    return &m_messages[index];
}

void ChatMessageList::ScrollToBottom()
{
    UpdateScrollRange(GetWindow() != nullptr ? GetWindow()->GetRender() : nullptr);
    const int32_t nMax = std::max(0, m_nContentHeight - GetVisibleHeight());
    SetScrollPos(nMax);
}

int32_t ChatMessageList::GetVisibleHeight() const
{
    //可见内容区高度（控件高度扣除内边距），与绘制和滚动范围的计算保持一致
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    return rc.Height();
}

void ChatMessageList::SetAutoScrollToBottom(bool bAutoScroll)
{
    m_bAutoScrollToBottom = bAutoScroll;
}

void ChatMessageList::SetAvatarSize(int32_t nSize)
{
    m_nAvatarSize = nSize;
    Invalidate();
}

void ChatMessageList::SetBubbleMaxWidth(int32_t nWidth)
{
    m_nBubbleMaxWidth = nWidth;
    Invalidate();
}

void ChatMessageList::SetBubbleRadius(int32_t nRadius)
{
    m_nBubbleRadius = nRadius;
    Invalidate();
}

void ChatMessageList::SetShowTime(bool bShowTime)
{
    m_bShowTime = bShowTime;
    Invalidate();
}

void ChatMessageList::SetBubbleColor(ChatMessageSide side, const DString& strColor)
{
    if (side == ChatMessageSide::kSelf) {
        m_strBubbleColorSelf = strColor;
    }
    else if (side == ChatMessageSide::kOther) {
        m_strBubbleColorOther = strColor;
    }
    Invalidate();
}

void ChatMessageList::SetTextColor(ChatMessageSide side, const DString& strColor)
{
    if (side == ChatMessageSide::kSelf) {
        m_strTextColorSelf = strColor;
    }
    else if (side == ChatMessageSide::kOther) {
        m_strTextColorOther = strColor;
    }
    Invalidate();
}

void ChatMessageList::SetAvatarTextColor(const DString& strColor)
{
    m_strAvatarTextColor = strColor;
    Invalidate();
}

void ChatMessageList::SetSystemTextColor(const DString& strColor)
{
    m_strSystemTextColor = strColor;
    Invalidate();
}

void ChatMessageList::SetScrollbarTrackColor(const DString& strColor)
{
    m_strScrollbarTrackColor = strColor;
    Invalidate();
}

void ChatMessageList::SetScrollbarThumbColor(const DString& strColor)
{
    m_strScrollbarThumbColor = strColor;
    Invalidate();
}

void ChatMessageList::SetAutoHideScrollbar(bool bAutoHide)
{
    m_bAutoHideScrollbar = bAutoHide;
    Invalidate();
}

void ChatMessageList::PaintStateImages(IRender* pRender)
{
    BaseClass::PaintStateImages(pRender);
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    if ((rc.Width() <= 0) || (rc.Height() <= 0)) {
        return;
    }

    if (m_messages.empty()) {
        return;
    }

    //更新滚动范围
    UpdateScrollRange(pRender);

    //裁剪到控件区域，避免消息超出边界
    std::unique_ptr<AutoClip> spClip = CreateRectClip(pRender, rc, true);

    int32_t y = rc.top - m_nScrollPos;
    const size_t nCount = m_messages.size();
    for (size_t index = 0; index < nCount; ++index) {
        const Message& message = m_messages[index];
        //时间分隔条：与上一条消息时间不同才显示
        bool bShowDivider = false;
        if (m_bShowTime && !message.time.empty()) {
            if (index == 0) {
                bShowDivider = true;
            }
            else {
                bShowDivider = (message.time != m_messages[index - 1].time);
            }
        }
        int32_t nDividerHeight = 0;
        if (bShowDivider) {
            nDividerHeight = Dpi().GetScaleInt(28);
        }
        const int32_t nMessageHeight = CalcMessageHeight(pRender, message, rc.Width());
        UiRect rcMessage(rc.left, y + nDividerHeight, rc.right, y + nDividerHeight + nMessageHeight);
        if (bShowDivider) {
            PaintTimeDivider(pRender, message.time, UiRect(rc.left, y, rc.right, y + nDividerHeight));
        }
        //仅绘制可见消息
        if (rcMessage.bottom > rc.top && rcMessage.top < rc.bottom) {
            PaintMessage(pRender, message, rcMessage);
        }
        y += nDividerHeight + nMessageHeight + Dpi().GetScaleInt(m_nBubbleSpacing);
    }
    spClip.reset();

    //绘制滚动条
    DrawScrollbar(pRender, rc);
}

int32_t ChatMessageList::CalcMessageHeight(IRender* pRender, const Message& message, int32_t nContentWidth)
{
    if (pRender == nullptr) {
        return 0;
    }
    if (message.side == ChatMessageSide::kSystem) {
        //系统提示：单行小字
        return MeasureTextHeight(pRender, message.text, nContentWidth, m_strTimeFont);
    }

    const int32_t nAvatarSize = Dpi().GetScaleInt(m_nAvatarSize);
    const int32_t nPadH = Dpi().GetScaleInt(m_nBubblePaddingH);
    const int32_t nPadV = Dpi().GetScaleInt(m_nBubblePaddingV);
    const int32_t nGap = Dpi().GetScaleInt(10);

    //气泡最大宽度：受控件宽度限制（减头像、间距、边距）
    int32_t nMaxBubbleWidth = Dpi().GetScaleInt(m_nBubbleMaxWidth);
    const int32_t nAvailable = nContentWidth - nAvatarSize - nGap - Dpi().GetScaleInt(24);
    if (nMaxBubbleWidth > nAvailable) {
        nMaxBubbleWidth = std::max(nAvailable, Dpi().GetScaleInt(80));
    }

    //先量测文本的自然宽度（单行），用于确定气泡宽度
    MeasureStringParam measureParam;
    measureParam.pFont = GetIFontById(m_strTextFont);
    measureParam.uFormat = TEXT_SINGLELINE;
    const int32_t nNaturalWidth = pRender->MeasureString(message.text, measureParam).Width();

    int32_t nTextWidth = nNaturalWidth;
    if (nTextWidth > nMaxBubbleWidth - 2 * nPadH) {
        nTextWidth = nMaxBubbleWidth - 2 * nPadH;
    }
    if (nTextWidth < 1) {
        nTextWidth = 1;
    }

    //按最终文本宽度量测高度（可能多行）
    const int32_t nTextHeight = MeasureTextHeight(pRender, message.text, nTextWidth, m_strTextFont);
    const int32_t nBubbleHeight = nTextHeight + 2 * nPadV;

    return std::max(nAvatarSize, nBubbleHeight);
}

int32_t ChatMessageList::CalcTotalHeight(IRender* pRender, int32_t nContentWidth)
{
    int32_t nTotal = 0;
    const size_t nCount = m_messages.size();
    for (size_t index = 0; index < nCount; ++index) {
        const Message& message = m_messages[index];
        bool bShowDivider = false;
        if (m_bShowTime && !message.time.empty()) {
            bShowDivider = (index == 0) || (message.time != m_messages[index - 1].time);
        }
        if (bShowDivider) {
            nTotal += Dpi().GetScaleInt(28);
        }
        nTotal += CalcMessageHeight(pRender, message, nContentWidth);
        nTotal += Dpi().GetScaleInt(m_nBubbleSpacing);
    }
    if (!m_messages.empty()) {
        //最后一条消息之后不保留间距
        nTotal -= Dpi().GetScaleInt(m_nBubbleSpacing);
    }
    return nTotal;
}

void ChatMessageList::UpdateScrollRange(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    m_nContentHeight = CalcTotalHeight(pRender, rc.Width());
    //钳位滚动位置
    const int32_t nMax = std::max(0, m_nContentHeight - rc.Height());
    if (m_nScrollPos > nMax) {
        m_nScrollPos = nMax;
    }
    if (m_nScrollPos < 0) {
        m_nScrollPos = 0;
    }
}

UiRect ChatMessageList::CalcBubbleRect(const UiRect& rcMessage, const Message& message, int32_t nBubbleWidth,
                                       int32_t nMessageHeight) const
{
    const int32_t nAvatarSize = Dpi().GetScaleInt(m_nAvatarSize);
    const int32_t nGap = Dpi().GetScaleInt(10);
    if (message.side == ChatMessageSide::kSelf) {
        //自己：气泡靠右，右侧留头像位置
        const int32_t nRight = rcMessage.right - nAvatarSize - nGap;
        UiRect rcBubble(nRight - nBubbleWidth, rcMessage.top, nRight, rcMessage.top + nMessageHeight);
        return rcBubble;
    }
    else {
        //对方：气泡靠左，左侧留头像位置
        const int32_t nLeft = rcMessage.left + nAvatarSize + nGap;
        UiRect rcBubble(nLeft, rcMessage.top, nLeft + nBubbleWidth, rcMessage.top + nMessageHeight);
        return rcBubble;
    }
}

void ChatMessageList::PaintMessage(IRender* pRender, const Message& message, const UiRect& rcMessage)
{
    if (message.side == ChatMessageSide::kSystem) {
        //系统提示：居中小字
        UiRect rcText = rcMessage;
        DrawChatText(pRender, message.text, rcText, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE,
                     m_strSystemTextColor, m_strTimeFont);
        return;
    }

    const int32_t nAvatarSize = Dpi().GetScaleInt(m_nAvatarSize);
    const int32_t nPadH = Dpi().GetScaleInt(m_nBubblePaddingH);
    const int32_t nPadV = Dpi().GetScaleInt(m_nBubblePaddingV);
    const int32_t nGap = Dpi().GetScaleInt(10);

    //头像
    UiRect rcAvatar;
    if (message.side == ChatMessageSide::kSelf) {
        rcAvatar = UiRect(rcMessage.right - nAvatarSize, rcMessage.top,
                          rcMessage.right, rcMessage.top + nAvatarSize);
    }
    else {
        rcAvatar = UiRect(rcMessage.left, rcMessage.top,
                          rcMessage.left + nAvatarSize, rcMessage.top + nAvatarSize);
    }
    PaintAvatar(pRender, rcAvatar, message.avatarText);

    //计算气泡宽度（与 CalcMessageHeight 保持一致）
    int32_t nMaxBubbleWidth = Dpi().GetScaleInt(m_nBubbleMaxWidth);
    const int32_t nAvailable = rcMessage.Width() - nAvatarSize - nGap - Dpi().GetScaleInt(24);
    if (nMaxBubbleWidth > nAvailable) {
        nMaxBubbleWidth = std::max(nAvailable, Dpi().GetScaleInt(80));
    }
    MeasureStringParam measureParam;
    measureParam.pFont = GetIFontById(m_strTextFont);
    measureParam.uFormat = TEXT_SINGLELINE;
    const int32_t nNaturalWidth = pRender->MeasureString(message.text, measureParam).Width();
    int32_t nTextWidth = nNaturalWidth;
    if (nTextWidth > nMaxBubbleWidth - 2 * nPadH) {
        nTextWidth = nMaxBubbleWidth - 2 * nPadH;
    }
    if (nTextWidth < 1) {
        nTextWidth = 1;
    }
    const int32_t nTextHeight = MeasureTextHeight(pRender, message.text, nTextWidth, m_strTextFont);
    const int32_t nBubbleHeight = nTextHeight + 2 * nPadV;
    const int32_t nBubbleWidth = nTextWidth + 2 * nPadH;

    UiRect rcBubble = CalcBubbleRect(rcMessage, message, nBubbleWidth, nBubbleHeight);

    //气泡底色
    UiColor colorBubble = GetUiColor(message.side == ChatMessageSide::kSelf ?
                                     m_strBubbleColorSelf : m_strBubbleColorOther);
    if (colorBubble.IsEmpty()) {
        colorBubble = UiColor(0xFFFFFFFF);
    }
    const int32_t nRadius = Dpi().GetScaleInt(m_nBubbleRadius);
    pRender->FillRoundRect(UiRectF(static_cast<float>(rcBubble.left), static_cast<float>(rcBubble.top),
                                   static_cast<float>(rcBubble.right), static_cast<float>(rcBubble.bottom)),
                            static_cast<float>(nRadius), static_cast<float>(nRadius), colorBubble);

    //气泡文字
    UiRect rcText(rcBubble.left + nPadH, rcBubble.top + nPadV,
                  rcBubble.right - nPadH, rcBubble.bottom - nPadV);
    const DString& strTextColor = (message.side == ChatMessageSide::kSelf) ?
                                  m_strTextColorSelf : m_strTextColorOther;
    DrawChatText(pRender, message.text, rcText, TEXT_LEFT | TEXT_TOP, strTextColor, m_strTextFont);
}

void ChatMessageList::PaintTimeDivider(IRender* pRender, const DString& strTime, const UiRect& rcDivider)
{
    if (strTime.empty()) {
        return;
    }
    DrawChatText(pRender, strTime, rcDivider, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE,
                 m_strSystemTextColor, m_strTimeFont);
}

void ChatMessageList::PaintAvatar(IRender* pRender, const UiRect& rcAvatar, const DString& strText)
{
    const float fRadius = static_cast<float>(rcAvatar.Width()) / 2.0f;
    const UiPointF center(static_cast<float>(rcAvatar.left) + fRadius,
                          static_cast<float>(rcAvatar.top) + fRadius);
    UiColor colorAvatar = GetAvatarColor(strText);
    pRender->FillCircle(center, fRadius, colorAvatar);

    if (!strText.empty()) {
        UiRect rcText = rcAvatar;
        DrawChatText(pRender, strText, rcText, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE,
                     m_strAvatarTextColor, m_strAvatarFont);
    }
}

UiColor ChatMessageList::GetAvatarColor(const DString& strText) const
{
    if (strText.empty()) {
        return UiColor(0xFF9E9E9E);
    }
    //根据文字内容生成稳定的配色，同一个人始终同色
    static const UiColor colors[] = {
        UiColor(0xFF5B9BF8),   //蓝
        UiColor(0xFF67C23A),   //绿
        UiColor(0xFFE6A23C),   //橙
        UiColor(0xFFF56C6C),   //红
        UiColor(0xFF9B7FE0),   //紫
        UiColor(0xFF36B3A6),   //青
        UiColor(0xFFE08080),   //粉
        UiColor(0xFF7F8C8D)    //灰
    };
    uint32_t nHash = 0;
    for (size_t i = 0; i < strText.size(); ++i) {
        nHash = nHash * 131 + (uint32_t)(uint16_t)strText[i];
    }
    return colors[nHash % (sizeof(colors) / sizeof(colors[0]))];
}

int32_t ChatMessageList::MeasureTextHeight(IRender* pRender, const DString& strText, int32_t nWidth,
                                           const DString& strFontId)
{
    if ((pRender == nullptr) || strText.empty()) {
        return 0;
    }
    MeasureStringParam param;
    param.pFont = GetIFontById(strFontId);
    param.rectSize = nWidth;
    UiRect rc = pRender->MeasureString(strText, param);
    return rc.Height();
}

void ChatMessageList::DrawChatText(IRender* pRender, const DString& strText, const UiRect& rc,
                                   uint32_t uFormat, const DString& strColor, const DString& strFontId)
{
    if ((pRender == nullptr) || strText.empty()) {
        return;
    }
    UiColor color = GetUiColor(strColor);
    if (color.IsEmpty()) {
        color = UiColor(0xFF000000);
    }
    DrawStringParam param;
    param.textRect = rc;
    param.dwTextColor = color;
    param.pFont = GetIFontById(strFontId);
    param.uFormat = uFormat;
    pRender->DrawString(strText, param);
}

UiRect ChatMessageList::CalcScrollThumbRect(const UiRect& rc) const
{
    if ((m_nContentHeight <= 0) || (rc.Height() <= 0)) {
        return UiRect();
    }
    const int32_t nScrollbarWidth = Dpi().GetScaleInt(8);
    const int32_t nContentHeight = m_nContentHeight;
    const int32_t nVisibleHeight = rc.Height();
    if (nContentHeight <= nVisibleHeight) {
        return UiRect();
    }
    int32_t nThumbHeight = static_cast<int32_t>((int64_t)nVisibleHeight * nVisibleHeight / nContentHeight);
    nThumbHeight = std::max(nThumbHeight, Dpi().GetScaleInt(24));
    nThumbHeight = std::min(nThumbHeight, nVisibleHeight);

    const int32_t nMaxScroll = nContentHeight - nVisibleHeight;
    int32_t nThumbTop = rc.top;
    if (nMaxScroll > 0) {
        nThumbTop = rc.top + static_cast<int32_t>((int64_t)m_nScrollPos * (nVisibleHeight - nThumbHeight) / nMaxScroll);
    }
    return UiRect(rc.right - nScrollbarWidth - 2, nThumbTop,
                  rc.right - 2, nThumbTop + nThumbHeight);
}

void ChatMessageList::DrawScrollbar(IRender* pRender, const UiRect& rc)
{
    if (m_bAutoHideScrollbar && !m_bMouseHover) {
        return;
    }
    UiRect rcThumb = CalcScrollThumbRect(rc);
    if (rcThumb.IsEmpty()) {
        return;
    }
    UiColor colorTrack = GetUiColor(m_strScrollbarTrackColor);
    if (colorTrack.IsEmpty()) {
        colorTrack = UiColor(0x20000000);
    }
    UiColor colorThumb = GetUiColor(m_strScrollbarThumbColor);
    if (colorThumb.IsEmpty()) {
        colorThumb = UiColor(0x80000000);
    }
    const int32_t nScrollbarWidth = Dpi().GetScaleInt(8);
    UiRect rcTrack(rc.right - nScrollbarWidth - 2, rc.top, rc.right - 2, rc.bottom);
    pRender->FillRoundRect(UiRectF(static_cast<float>(rcTrack.left), static_cast<float>(rcTrack.top),
                                   static_cast<float>(rcTrack.right), static_cast<float>(rcTrack.bottom)),
                            static_cast<float>(nScrollbarWidth) / 2.0f,
                            static_cast<float>(nScrollbarWidth) / 2.0f, colorTrack);
    pRender->FillRoundRect(UiRectF(static_cast<float>(rcThumb.left), static_cast<float>(rcThumb.top),
                                   static_cast<float>(rcThumb.right), static_cast<float>(rcThumb.bottom)),
                            static_cast<float>(nScrollbarWidth) / 2.0f,
                            static_cast<float>(nScrollbarWidth) / 2.0f, colorThumb);
}

bool ChatMessageList::HitTestScrollThumb(const UiPoint& ptMouse) const
{
    UiRect rcThumb = CalcScrollThumbRect(GetRect());
    if (rcThumb.IsEmpty()) {
        return false;
    }
    return rcThumb.ContainsPt(ptMouse);
}

bool ChatMessageList::ButtonDown(const EventArgs& msg)
{
    if (!IsEnabled()) {
        return BaseClass::ButtonDown(msg);
    }
    //滚动条滑块拖动优先
    if (HitTestScrollThumb(msg.ptMouse)) {
        m_bDraggingScrollbar = true;
        m_dragStartY = msg.ptMouse.y;
        m_dragStartScrollPos = m_nScrollPos;
        if (GetWindow() != nullptr) {
            GetWindow()->SetCapture();
        }
        return true;
    }
    return BaseClass::ButtonDown(msg);
}

bool ChatMessageList::ButtonUp(const EventArgs& msg)
{
    if (m_bDraggingScrollbar) {
        m_bDraggingScrollbar = false;
        if (GetWindow() != nullptr) {
            GetWindow()->ReleaseCapture();
        }
        return true;
    }
    return BaseClass::ButtonUp(msg);
}

bool ChatMessageList::MouseMove(const EventArgs& msg)
{
    if (m_bDraggingScrollbar) {
        UiRect rc = GetRect();
        UiPadding rcPadding = GetControlPadding();
        rc.Deflate(rcPadding);
        const int32_t nMaxScroll = std::max(0, m_nContentHeight - rc.Height());
        if (nMaxScroll > 0) {
            const int32_t nVisibleHeight = rc.Height();
            UiRect rcThumb = CalcScrollThumbRect(rc);
            const int32_t nThumbHeight = rcThumb.Height();
            const int32_t nTravel = nVisibleHeight - nThumbHeight;
            if (nTravel > 0) {
                const int32_t nDelta = msg.ptMouse.y - m_dragStartY;
                SetScrollPos(m_dragStartScrollPos + static_cast<int32_t>((int64_t)nDelta * nMaxScroll / nTravel));
            }
        }
        return true;
    }
    return BaseClass::MouseMove(msg);
}

bool ChatMessageList::MouseWheel(const EventArgs& msg)
{
    IRender* pRender = (GetWindow() != nullptr) ? GetWindow()->GetRender() : nullptr;
    if (pRender == nullptr) {
        return BaseClass::MouseWheel(msg);
    }
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    const int32_t nMaxScroll = std::max(0, m_nContentHeight - rc.Height());
    if (nMaxScroll <= 0) {
        return BaseClass::MouseWheel(msg);
    }
    //滚轮滚动3行
    const int32_t nDelta = -msg.wParam * Dpi().GetScaleInt(40);
    SetScrollPos(m_nScrollPos + nDelta);
    return true;
}

bool ChatMessageList::MouseEnter(const EventArgs& msg)
{
    m_bMouseHover = true;
    if (m_bAutoHideScrollbar) {
        Invalidate();
    }
    return BaseClass::MouseEnter(msg);
}

bool ChatMessageList::MouseLeave(const EventArgs& msg)
{
    m_bMouseHover = false;
    if (m_bAutoHideScrollbar) {
        Invalidate();
    }
    return BaseClass::MouseLeave(msg);
}

void ChatMessageList::SetScrollPos(int32_t nPos)
{
    const int32_t nMax = std::max(0, m_nContentHeight - GetVisibleHeight());
    if (nPos < 0) {
        nPos = 0;
    }
    if (nPos > nMax) {
        nPos = nMax;
    }
    if (nPos == m_nScrollPos) {
        return;
    }
    m_nScrollPos = nPos;
    Invalidate();
}

}//namespace ui
